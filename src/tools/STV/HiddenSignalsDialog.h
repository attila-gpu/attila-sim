#ifndef HIDDENSIGNALSDIALOG_H
    #define HIDDENSIGNALSDIALOG_H

#include <QDialog>
#include "ui_HiddenSignalsDialog.h"

class HiddenSignalsDialogImp : public QDialog, public Ui::HiddenSignalsDialog
{
    Q_OBJECT

public:

    HiddenSignalsDialogImp(QWidget* parent = 0, const char* name = 0, bool modal = FALSE) :
        QDialog(parent, name, modal, 0)
    {
        setupUi(this);
    }
};


#endif // HIDDENSIGNALSDIALOG_H
