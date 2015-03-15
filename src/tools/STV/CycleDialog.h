#ifndef CYCLEDIALOG_H
    #define CYCLEDIALOG_H

#include <QDialog>
#include "ui_CycleDialog.h"

class CycleDialogImp : public QDialog, public Ui::CycleDialog
{
    Q_OBJECT

public:

    CycleDialogImp(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0) :
        QDialog(parent, name, modal, fl)
    {
        setupUi(this);
    }
};

#endif // CYCLEDIALOG_H
