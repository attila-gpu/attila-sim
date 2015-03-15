/****************************************************************************
** Form interface generated from reading ui file '.\PreferencesDialog.ui'
**
** Created: Thu Jul 24 11:54:55 2003
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTabWidget;
class QWidget;

class PreferencesDialog : public QDialog
{ 
    Q_OBJECT

public:
    PreferencesDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PreferencesDialog();

    QTabWidget* tabWidget;
    QWidget* Widget2;
    QCheckBox* showDivisionLinesCB;
    QCheckBox* showSignalsRulerCB;
    QCheckBox* showCyclesRulerCB;
    QCheckBox* autoHideRulersCB;
    QLabel* TextLabel1_2;
    QLabel* TextLabel3;
    QLabel* TextLabel2_2;
    QLineEdit* loadMaxCyclesTL;
    QLabel* TextLabel2;
    QLabel* TextLabel1_3;
    QLabel* TextLabel1_3_2;
    QLineEdit* sizeSquareTL;
    QWidget* tab;
    QLabel* TextLabel1;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;

protected:
    QVBoxLayout* PreferencesDialogLayout;
    QHBoxLayout* Layout1;
};

#endif // PREFERENCESDIALOG_H
