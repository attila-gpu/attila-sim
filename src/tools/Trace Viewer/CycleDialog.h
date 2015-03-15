/****************************************************************************
** Form interface generated from reading ui file '.\CycleDialog.ui'
**
** Created: Tue Jul 15 12:50:21 2003
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef FORM1_H
#define FORM1_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QListView;
class QListViewItem;
class QPushButton;

class CycleDialog : public QDialog
{ 
    Q_OBJECT

public:
    CycleDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CycleDialog();

    QListView* ListView1;
    QPushButton* PushButton1;

protected:
    QVBoxLayout* Form1Layout;
    QHBoxLayout* Layout1;
};

#endif // FORM1_H
