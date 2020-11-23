#!/bin/bash

#    Copyright © 2016-2019 by The qTox Project Contributors
#
#    This program is libre software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Fail out on error
set -eu -o pipefail

readonly BIN_NAME="qTox.dmg"
export MACOSX_DEPLOYMENT_TARGET=10.13

# Build OSX
build() {
    bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -i
    bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -b
    bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -d
    bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -dmg
}

# check if binary was built
check() {
    if [[ ! -s "$BIN_NAME" ]]
    then
        echo "There's no $BIN_NAME !"
        exit 1
    fi
}

make_hash() {
    shasum -a 256 "$BIN_NAME" > "$BIN_NAME".sha256
}

install_qt() {
    brew install --build-from-source qt5
}

install_other() {
    # abilinski: urg brewfile doesn't seem to support --build-from-source
    # brew bundle install -f ./osx/Brewfile
    # accelerate builds with ccache
    brew install --build-from-source ccache
    brew install --build-from-source git
    brew install --build-from-source wget
    brew install --build-from-source libtool
    brew install --build-from-source cmake
    brew install --build-from-source pkgconfig
    brew install --build-from-source check
    brew install --build-from-source libvpx
    brew install --build-from-source opus
    brew install --build-from-source libsodium
    brew install --build-from-source cmake
    brew install --build-from-source ffmpeg
    brew install --build-from-source libexif
    brew install --build-from-source qrencode
    brew install --build-from-source qt5
    brew install --build-from-source sqlcipher
    brew install --build-from-source openal-soft
}

main() {
    if [ $# -eq 0 ]; then
        build
        check
        make_hash
    fi
    if [ $1 = "1" ]; then
        install_qt
    elif [ $1 = "2" ]; then
        install_other
    else
        echo "bad arg"
    fi
}
main "$@"
