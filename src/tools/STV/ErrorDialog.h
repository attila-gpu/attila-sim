#ifndef ERRORDIALOG_H
    #define ERRORDIALOG_H

#include <QDialog>
#include "ui_ErrorDialog.h"

class ErrorDialogImp : public QDialog, public Ui::ErrorDialog
{
    Q_OBJECT

public:

    ErrorDialogImp(QWidget* parent = 0, const char* name = 0, bool modal = FALSE) :
        QDialog(parent, name, modal, 0)
    {
        setupUi(this);
    }

};


#endif // ERRORDIALOG_H
