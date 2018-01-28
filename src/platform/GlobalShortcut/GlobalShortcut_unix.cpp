/*
    Copyright © 2014-2015 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.

    This file incorporates work covered by the following copyright and  
    permission notice: 

        Copyright 2005-2018 The Mumble Developers. All rights reserved.
        Use of this source code is governed by a BSD-style license
        that can be found in the LICENSE file inside the GlobalShortcut
        directory or at <https://www.mumble.info/LICENSE>.
*/

#include <QtCore/qsystemdetection.h>
#if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
#include <QFile>
#include <QDir>
#include <QSet>
#include <QFileSystemWatcher>
#include <QSocketNotifier>

#include "GlobalShortcut.h"
#include "GlobalShortcut_unix.h"

// TODO?: guard this whole class with #ifdef PLATFORM_EXTENSIONS
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#ifndef NO_XINPUT2
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>
#endif
#include <X11/Xutil.h>
#ifdef Q_OS_LINUX
#include <linux/input.h>
#include <fcntl.h>
#endif


/**
 * Returns a platform specific GlobalShortcutEngine object.
 *
 * @see GlobalShortcutX
 * @see GlobalShortcutMac
 * @see GlobalShortcutWin
 */
GlobalShortcutEngine *GlobalShortcutEngine::platformInit() {
	return new GlobalShortcutX();
}

GlobalShortcutX::GlobalShortcutX() {
	iXIopcode =  -1;
	bRunning = false;

	display = XOpenDisplay(NULL);

	if (! display) {
		qWarning("GlobalShortcutX: Unable to open dedicated display connection.");
		return;
	}

#ifdef Q_OS_LINUX
	// TODO: if statement
	if (/*g.s.bEnableEvdev*/ true) {
		QString dir = QLatin1String("/dev/input");
		QFileSystemWatcher *fsw = new QFileSystemWatcher(QStringList(dir), this);
		connect(fsw, SIGNAL(directoryChanged(const QString &)), this, SLOT(directoryChanged(const QString &)));
		directoryChanged(dir);

		if (qsKeyboards.isEmpty()) {
			foreach(QFile *f, qmInputDevices)
				delete f;
			qmInputDevices.clear();

			delete fsw;
			qWarning("GlobalShortcutX: Unable to open any keyboard input devices under /dev/input, falling back to XInput");
		} else {
			return;
		}
	}
#endif

	for (int i=0; i < ScreenCount(display); ++i)
		qsRootWindows.insert(RootWindow(display, i));

#ifndef NO_XINPUT2
	int evt, error;

	// TODO: g.s.bEnableXInput2?
	if (/*g.s.bEnableXInput2*/true && XQueryExtension(display, "XInputExtension", &iXIopcode, &evt, &error)) {
		int major = XI_2_Major;
		int minor = XI_2_Minor;
		int rc = XIQueryVersion(display, &major, &minor);
		if (rc != BadRequest) {
			qWarning("GlobalShortcutX: Using XI2 %d.%d", major, minor);

			queryXIMasterList();

			XIEventMask evmask;
			unsigned char mask[(XI_LASTEVENT + 7)/8];

			memset(&evmask, 0, sizeof(evmask));
			memset(mask, 0, sizeof(mask));

			XISetMask(mask, XI_RawButtonPress);
			XISetMask(mask, XI_RawButtonRelease);
			XISetMask(mask, XI_RawKeyPress);
			XISetMask(mask, XI_RawKeyRelease);
			XISetMask(mask, XI_HierarchyChanged);

			evmask.deviceid = XIAllDevices;
			evmask.mask_len = sizeof(mask);
			evmask.mask = mask;

			foreach(Window w, qsRootWindows)
				XISelectEvents(display, w, &evmask, 1);
			XFlush(display);

			connect(new QSocketNotifier(ConnectionNumber(display), QSocketNotifier::Read, this), SIGNAL(activated(int)), this, SLOT(displayReadyRead(int)));

			return;
		}
	}
#endif
	qWarning("GlobalShortcutX: No XInput support, falling back to polled input. This wastes a lot of CPU resources, so please enable one of the other methods.");
	bRunning=true;
	start(QThread::TimeCriticalPriority);
}

GlobalShortcutX::~GlobalShortcutX() {
	bRunning = false;
	wait();

	if (display)
		XCloseDisplay(display);
}

// Tight loop polling
void GlobalShortcutX::run() {
	Window root = XDefaultRootWindow(display);
	Window root_ret, child_ret;
	int root_x, root_y;
	int win_x, win_y;
	unsigned int mask[2];
	int idx = 0;
	int next = 0;
	char keys[2][32];

	memset(keys[0], 0, 32);
	memset(keys[1], 0, 32);
	mask[0] = mask[1] = 0;

	while (bRunning) {
		msleep(10);

		idx = next;
		next = idx ^ 1;
		if (XQueryPointer(display, root, &root_ret, &child_ret, &root_x, &root_y, &win_x, &win_y, &mask[next]) && XQueryKeymap(display, keys[next])) {
			for (int i=0;i<256;++i) {
				int index = i / 8;
				int keymask = 1 << (i % 8);
				bool oldstate = (keys[idx][index] & keymask) != 0;
				bool newstate = (keys[next][index] & keymask) != 0;
				if (oldstate != newstate) {
					handleButton(i, newstate);
				}
			}
			for (int i=8;i<=12;++i) {
				bool oldstate = (mask[idx] & (1 << i)) != 0;
				bool newstate = (mask[next] & (1 << i)) != 0;
				if (oldstate != newstate) {
					handleButton(0x110 + i, newstate);
				}
			}
		}
	}
}

// Find XI2 master devices so they can be ignored.
void GlobalShortcutX::queryXIMasterList() {
#ifndef NO_XINPUT2
	XIDeviceInfo *info, *dev;
	int ndevices;

	qsMasterDevices.clear();

	dev = info = XIQueryDevice(display, XIAllDevices, &ndevices);
	for (int i=0;i<ndevices;++i) {
		switch (dev->use) {
			case XIMasterPointer:
			case XIMasterKeyboard:
				qsMasterDevices.insert(dev->deviceid);
				break;
			default:
				break;
		}

		++dev;
	}
	XIFreeDeviceInfo(info);
#endif
}

// XInput2 event is ready on socketnotifier.
void GlobalShortcutX::displayReadyRead(int) {
#ifndef NO_XINPUT2
	XEvent evt;

	while (XPending(display)) {
		XNextEvent(display, &evt);
		XGenericEventCookie *cookie = & evt.xcookie;

		if ((cookie->type != GenericEvent) || (cookie->extension != iXIopcode) || !XGetEventData(display, cookie))
			continue;

		XIDeviceEvent *xide = reinterpret_cast<XIDeviceEvent *>(cookie->data);
		switch (cookie->evtype) {
			case XI_RawKeyPress:
			case XI_RawKeyRelease:
				if (! qsMasterDevices.contains(xide->deviceid))
					handleButton(xide->detail, cookie->evtype == XI_RawKeyPress);
				break;
			case XI_RawButtonPress:
			case XI_RawButtonRelease:
				if (! qsMasterDevices.contains(xide->deviceid))
					handleButton(xide->detail + 0x117, cookie->evtype == XI_RawButtonPress);
				break;
			case XI_HierarchyChanged:
				queryXIMasterList();
		}

		XFreeEventData(display, cookie);
	}
#endif
}

// One of the raw /dev/input devices has ready input
void GlobalShortcutX::inputReadyRead(int) {
#ifdef Q_OS_LINUX
	// TODO: should this be configurable?
//	if (!g.s.bEnableEvdev) {
//		return;
//	}

	struct input_event ev;

	QFile *f=qobject_cast<QFile *>(sender()->parent());
	if (!f)
		return;

	bool found = false;

	while (f->read(reinterpret_cast<char *>(&ev), sizeof(ev)) == sizeof(ev)) {
		found = true;
		if (ev.type != EV_KEY)
			continue;
		bool down;
		switch (ev.value) {
			case 0:
				down = false;
				break;
			case 1:
				down = true;
				break;
			default:
				continue;
		}
		int evtcode = ev.code + 8;
		handleButton(evtcode, down);
	}

	if (! found) {
		int fd = f->handle();
		int version = 0;
		if ((ioctl(fd, EVIOCGVERSION, &version) < 0) || (((version >> 16) & 0xFF) < 1)) {
			qWarning("GlobalShortcutX: Removing dead input device %s", qPrintable(f->fileName()));
			qmInputDevices.remove(f->fileName());
			qsKeyboards.remove(f->fileName());
			delete f;
		}
	}
#endif
}

#define test_bit(bit, array)    (array[bit/8] & (1<<(bit%8)))

// The /dev/input directory changed
void GlobalShortcutX::directoryChanged(const QString &dir) {
#ifdef Q_OS_LINUX
	// TODO: need this?
//	if (!g.s.bEnableEvdev) {
//		return;
//	}

	QDir d(dir, QLatin1String("event*"), 0, QDir::System);
	foreach(QFileInfo fi, d.entryInfoList()) {
		QString path = fi.absoluteFilePath();
		if (! qmInputDevices.contains(path)) {
			QFile *f = new QFile(path, this);
			if (f->open(QIODevice::ReadOnly)) {
				int fd = f->handle();
				int version;
				char name[256];
				uint8_t events[EV_MAX/8 + 1];
				memset(events, 0, sizeof(events));
				if ((ioctl(fd, EVIOCGVERSION, &version) >= 0) && (ioctl(fd, EVIOCGNAME(sizeof(name)), name)>=0) && (ioctl(fd, EVIOCGBIT(0,sizeof(events)), &events) >= 0) && test_bit(EV_KEY, events) && (((version >> 16) & 0xFF) > 0)) {
					name[255]=0;
					qWarning("GlobalShortcutX: %s: %s", qPrintable(f->fileName()), name);
					// Is it grabbed by someone else?
					if ((ioctl(fd, EVIOCGRAB, 1) < 0)) {
						qWarning("GlobalShortcutX: Device exclusively grabbed by someone else (X11 using exclusive-mode evdev?)");
						delete f;
					} else {
						ioctl(fd, EVIOCGRAB, 0);
						uint8_t keys[KEY_MAX/8 + 1];
						if ((ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keys)), &keys) >= 0) && test_bit(KEY_SPACE, keys))
							qsKeyboards.insert(f->fileName());

						fcntl(f->handle(), F_SETFL, O_NONBLOCK);
						connect(new QSocketNotifier(f->handle(), QSocketNotifier::Read, f), SIGNAL(activated(int)), this, SLOT(inputReadyRead(int)));

						qmInputDevices.insert(f->fileName(), f);
					}
				} else {
					delete f;
				}
			} else {
				delete f;
			}
		}
	}
#else
	Q_UNUSED(dir);
#endif
}

QString GlobalShortcutX::toString(const QVariant& shortcut) {
	bool ok;
	unsigned int key=shortcut.toUInt(&ok);
	if (!ok)
		return QString();
	if ((key < 0x118) || (key >= 0x128)) {
		// TODO: shift mask broken?
		KeySym ks=XkbKeycodeToKeysym(display, static_cast<KeyCode>(key), 0, /*event.xkey.state & ShiftMask ? 1 : 0*/0);
		if (ks == NoSymbol) {
			return QLatin1String("0x")+QString::number(key,16);
		} else {
			const char *str=XKeysymToString(ks);
			if (*str == '\0') {
				return QLatin1String("KS0x")+QString::number(ks, 16);
			} else {
				return QLatin1String(str);
			}
		}
	} else {
		return tr("Mouse %1").arg(key-0x118);
	}
}
// these fucking X11 defines break Qt, try to contain them
#undef Bool
#undef CursorShape
#undef Expose
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef None
#undef Status
#undef Unsorted

#endif // defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
