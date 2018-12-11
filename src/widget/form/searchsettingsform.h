#ifndef SEARCHSETTINGSFORM_H
#define SEARCHSETTINGSFORM_H

#include <QWidget>
#include "src/widget/searchtypes.h"

class Contact;

namespace Ui {
class SearchSettingsForm;
}

class SearchSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit SearchSettingsForm(const Contact* contact, QWidget *parent = nullptr);
    ~SearchSettingsForm();

    ParameterSearch getParameterSearch();

private:
    Ui::SearchSettingsForm *ui;
    QDate startDate;
    bool isUpdate{false};
    const Contact* contact;

    void updateStartDateLabel();
    void setUpdate(const bool isUpdate);

private slots:
    void onStartSearchSelected(const int index);
    void onRegisterClicked(const bool checked);
    void onWordsOnlyClicked(const bool checked);
    void onRegularClicked(const bool checked);
    void onChoiceDate();

signals:
    void updateSettings(const bool isUpdate);
};

#endif // SEARCHSETTINGSFORM_H
