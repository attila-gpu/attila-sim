/****************************************************************************
** Form implementation generated from reading ui file '.\CycleDialog.ui'
**
** Created: Tue Jul 15 12:50:21 2003
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "CycleDialog.h"

#include <qheader.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>


/* 
 *  Constructs a CycleDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CycleDialog::CycleDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "Form1" );
    resize( 406, 228 ); 
    setProperty( "caption", tr( "Cycle Information" ) );
    setProperty( "backgroundOrigin", (int)QDialog::WidgetOrigin );
    Form1Layout = new QVBoxLayout( this ); 
    Form1Layout->setSpacing( 1 );
    Form1Layout->setMargin( 9 );

    ListView1 = new QListView( this, "ListView1" );
    ListView1->addColumn( tr( "Type" ) );
    ListView1->header()->setClickEnabled( FALSE, ListView1->header()->count() - 1 );
    ListView1->addColumn( tr( "Value" ) );
    ListView1->header()->setClickEnabled( FALSE, ListView1->header()->count() - 1 );
    QListViewItem * item = new QListViewItem( ListView1, 0 );
    item->setText( 0, tr( "Cycle" ) );

    item = new QListViewItem( ListView1, item );
    item->setText( 0, tr( "Signal" ) );

    item = new QListViewItem( ListView1, item );
    item->setText( 0, tr( "Cookies" ) );

    item = new QListViewItem( ListView1, item );
    item->setText( 0, tr( "Color ID" ) );

    item = new QListViewItem( ListView1, item );
    item->setText( 0, tr( "Info" ) );

    ListView1->setProperty( "frameShadow", (int)QListView::Sunken );
    ListView1->setProperty( "margin", 4 );
    ListView1->setProperty( "selectionMode", (int)QListView::NoSelection );
    ListView1->setProperty( "itemMargin", 6 );
    ListView1->setProperty( "rootIsDecorated", QVariant( FALSE, 0 ) );
    Form1Layout->addWidget( ListView1 );

    Layout1 = new QHBoxLayout; 
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer );

    PushButton1 = new QPushButton( this, "PushButton1" );
    PushButton1->setProperty( "acceptDrops", QVariant( TRUE, 0 ) );
    PushButton1->setProperty( "text", tr( "Accept" ) );
    PushButton1->setProperty( "default", QVariant( TRUE, 0 ) );
    Layout1->addWidget( PushButton1 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer_2 );
    Form1Layout->addLayout( Layout1 );

    // signals and slots connections
    connect( PushButton1, SIGNAL( clicked() ), this, SLOT( accept() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CycleDialog::~CycleDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

