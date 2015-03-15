#ifndef CYCLEGO_H
    #define CYCLEGO_H

#include <QDialog>
#include "ui_CycleGo.h"

class CycleGoImp : public QDialog, public Ui::CycleGo
{
    Q_OBJECT

public:

    CycleGoImp(QWidget* parent = 0, const char* name = 0, bool modal = FALSE) :
        QDialog(parent, name, modal, 0)
    {
        setupUi(this);
    }
};


#endif // CYCLEGO_H