/****************************************************************************
** Form implementation generated from reading ui file '.\LoadingFileDialog.ui'
**
** Created: Wed Jul 16 08:27:31 2003
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "LoadingFileDialog.h"

#include <qlabel.h>
#include <qprogressbar.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a LoadingFileDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
LoadingFileDialog::LoadingFileDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "LoadingFileDialog" );
    resize( 528, 140 ); 
    setProperty( "caption", tr( "Loading tracefile..." ) );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setGeometry( QRect( 110, 20, 271, 41 ) ); 
    TextLabel1->setProperty( "frameShape", (int)QLabel::NoFrame );
    TextLabel1->setProperty( "frameShadow", (int)QLabel::Raised );
    TextLabel1->setProperty( "text", tr( "Loading tracefile, please wait..." ) );
    TextLabel1->setProperty( "textFormat", (int)QLabel::RichText );
    TextLabel1->setProperty( "alignment", int( QLabel::AlignCenter ) );

    pBar = new QProgressBar( this, "pBar" );
    pBar->setGeometry( QRect( 80, 80, 380, 40 ) ); 
}

/*  
 *  Destroys the object and frees any allocated resources
 */
LoadingFileDialog::~LoadingFileDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

