#ifndef PREFERENCESDIALOG_H
    #define PREFERENCESDIALOG_H

#include <QDialog>
#include "ui_PreferencesDialog.h"

class PreferencesDialogImp : public QDialog, public Ui::PreferencesDialog
{
    Q_OBJECT

public:

    PreferencesDialogImp(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0) :
        QDialog(parent, name, modal, fl)
    {
        setupUi(this);
    }

};


#endif // PREFERENCESDIALOG_H
