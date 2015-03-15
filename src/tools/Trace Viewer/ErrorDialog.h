/****************************************************************************
** Form interface generated from reading ui file '.\ErrorDialog.ui'
**
** Created: Tue Jul 15 17:54:11 2003
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef ERROR_H
#define ERROR_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QMultiLineEdit;
class QPushButton;

class ErrorDialog : public QDialog
{ 
    Q_OBJECT

public:
    ErrorDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ErrorDialog();

    QPushButton* PushButton1;
    QMultiLineEdit* MultiLineEdit1;

protected:
    QHBoxLayout* Layout1;
};

#endif // ERROR_H
