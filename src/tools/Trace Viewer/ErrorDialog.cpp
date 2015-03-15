/****************************************************************************
** Form implementation generated from reading ui file '.\ErrorDialog.ui'
**
** Created: Tue Jul 15 17:54:11 2003
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "ErrorDialog.h"

#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a ErrorDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
ErrorDialog::ErrorDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "Error" );
    resize( 310, 110 ); 
    setProperty( "caption", tr( "Message error" ) );

    QWidget* privateLayoutWidget = new QWidget( this, "Layout1" );
    privateLayoutWidget->setGeometry( QRect( 10, 70, 310, 30 ) ); 
    Layout1 = new QHBoxLayout( privateLayoutWidget ); 
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer );

    PushButton1 = new QPushButton( privateLayoutWidget, "PushButton1" );
    PushButton1->setProperty( "sizePolicy", QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, PushButton1->sizePolicy().hasHeightForWidth() ) );
    PushButton1->setProperty( "text", tr( "Accept" ) );
    PushButton1->setProperty( "default", QVariant( TRUE, 0 ) );
    Layout1->addWidget( PushButton1 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer_2 );

    MultiLineEdit1 = new QMultiLineEdit( this, "MultiLineEdit1" );
    MultiLineEdit1->setProperty( "enabled", QVariant( FALSE, 0 ) );
    MultiLineEdit1->setGeometry( QRect( 10, 10, 311, 51 ) ); 
    MultiLineEdit1->setProperty( "frameShape", (int)QMultiLineEdit::Box );
    MultiLineEdit1->setProperty( "wordWrap", (int)QMultiLineEdit::WidgetWidth );
    MultiLineEdit1->setProperty( "wrapColumnOrWidth", -1 );
    MultiLineEdit1->setProperty( "text", tr( "Generic error message" ) );

    // signals and slots connections
    connect( PushButton1, SIGNAL( clicked() ), this, SLOT( accept() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
ErrorDialog::~ErrorDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

