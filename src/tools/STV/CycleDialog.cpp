/****************************************************************************
** Form implementation generated from reading ui file '.\CycleDialog.ui'
**
** Created: mié 22. nov 15:58:50 2006
**      by: The User Interface Compiler ($Id: CycleDialog.cpp,v 1.3 2007-06-22 15:36:18 jroca Exp $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include ".\CycleDialog.h"

#include <qvariant.h>
#include <qheader.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>


/*
 *  Constructs a CycleDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CycleDialog::CycleDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "Form1" );
    setBackgroundOrigin( QDialog::WidgetOrigin );
    Form1Layout = new QVBoxLayout( this, 9, 1, "Form1Layout"); 

    ListView1 = new QListView( this, "ListView1" );
    ListView1->addColumn( tr( "Type" ) );
    ListView1->header()->setClickEnabled( FALSE, ListView1->header()->count() - 1 );
    ListView1->addColumn( tr( "Value" ) );
    ListView1->header()->setClickEnabled( FALSE, ListView1->header()->count() - 1 );
    ListView1->setFrameShadow( QListView::Sunken );
    ListView1->setMargin( 4 );
    ListView1->setSelectionMode( QListView::NoSelection );
    ListView1->setItemMargin( 6 );
    ListView1->setRootIsDecorated( FALSE );
    Form1Layout->addWidget( ListView1 );

    Layout1 = new QHBoxLayout( 0, 0, 6, "Layout1"); 
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer );

    PushButton1 = new QPushButton( this, "PushButton1" );
    PushButton1->setAcceptDrops( TRUE );
    PushButton1->setDefault( TRUE );
    Layout1->addWidget( PushButton1 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer_2 );
    Form1Layout->addLayout( Layout1 );
    languageChange();
    resize( QSize(406, 234).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( PushButton1, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( ListView1, SIGNAL( onViewport() ), this, SLOT( ListView1_onViewport() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
CycleDialog::~CycleDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CycleDialog::languageChange()
{
    setCaption( tr( "Cycle Information" ) );
    ListView1->header()->setLabel( 0, tr( "Type" ) );
    ListView1->header()->setLabel( 1, tr( "Value" ) );
    ListView1->clear();
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

    PushButton1->setText( tr( "Accept" ) );
}

void CycleDialog::ListView1_onViewport()
{
    qWarning( "CycleDialog::ListView1_onViewport(): Not implemented yet" );
}

