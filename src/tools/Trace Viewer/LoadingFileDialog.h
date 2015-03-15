/****************************************************************************
** Form interface generated from reading ui file '.\LoadingFileDialog.ui'
**
** Created: Wed Jul 16 08:27:31 2003
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef LOADINGFILEDIALOG_H
#define LOADINGFILEDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;
class QProgressBar;

class LoadingFileDialog : public QDialog
{ 
    Q_OBJECT

public:
    LoadingFileDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~LoadingFileDialog();

    QLabel* TextLabel1;
    QProgressBar* pBar;

};

#endif // LOADINGFILEDIALOG_H
