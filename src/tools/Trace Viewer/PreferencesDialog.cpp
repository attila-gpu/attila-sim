/****************************************************************************
** Form implementation generated from reading ui file '.\PreferencesDialog.ui'
**
** Created: Thu Jul 24 11:54:55 2003
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "PreferencesDialog.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a PreferencesDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
PreferencesDialog::PreferencesDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "PreferencesDialog" );
    resize( 566, 307 ); 
    setProperty( "caption", tr( "Preferences" ) );
    setProperty( "sizeGripEnabled", QVariant( TRUE, 0 ) );
    PreferencesDialogLayout = new QVBoxLayout( this ); 
    PreferencesDialogLayout->setSpacing( 6 );
    PreferencesDialogLayout->setMargin( 11 );

    tabWidget = new QTabWidget( this, "tabWidget" );
    tabWidget->setProperty( "tabShape", (int)QTabWidget::Rounded );

    Widget2 = new QWidget( tabWidget, "Widget2" );

    showDivisionLinesCB = new QCheckBox( Widget2, "showDivisionLinesCB" );
    showDivisionLinesCB->setGeometry( QRect( 50, 150, 148, 20 ) ); 
    showDivisionLinesCB->setProperty( "text", tr( "Show division lines" ) );

    showSignalsRulerCB = new QCheckBox( Widget2, "showSignalsRulerCB" );
    showSignalsRulerCB->setGeometry( QRect( 50, 70, 148, 20 ) ); 
    showSignalsRulerCB->setProperty( "text", tr( "Show signals ruler" ) );

    showCyclesRulerCB = new QCheckBox( Widget2, "showCyclesRulerCB" );
    showCyclesRulerCB->setGeometry( QRect( 50, 30, 148, 20 ) ); 
    showCyclesRulerCB->setProperty( "text", tr( "Show cycles ruler" ) );

    autoHideRulersCB = new QCheckBox( Widget2, "autoHideRulersCB" );
    autoHideRulersCB->setGeometry( QRect( 50, 110, 148, 20 ) ); 
    autoHideRulersCB->setProperty( "text", tr( "Autohide rulers" ) );

    TextLabel1_2 = new QLabel( Widget2, "TextLabel1_2" );
    TextLabel1_2->setGeometry( QRect( 240, 110, 105, 20 ) ); 
    TextLabel1_2->setProperty( "text", tr( "Load at maximum" ) );

    TextLabel3 = new QLabel( Widget2, "TextLabel3" );
    TextLabel3->setGeometry( QRect( 280, 140, 201, 16 ) ); 
    TextLabel3->setProperty( "text", tr( "( 0 means max cycles in trace file )" ) );

    TextLabel2_2 = new QLabel( Widget2, "TextLabel2_2" );
    TextLabel2_2->setGeometry( QRect( 480, 30, 40, 20 ) ); 
    TextLabel2_2->setProperty( "text", tr( "pixels" ) );

    loadMaxCyclesTL = new QLineEdit( Widget2, "loadMaxCyclesTL" );
    loadMaxCyclesTL->setGeometry( QRect( 380, 100, 80, 25 ) ); 
    loadMaxCyclesTL->setProperty( "maxLength", 10 );

    TextLabel2 = new QLabel( Widget2, "TextLabel2" );
    TextLabel2->setGeometry( QRect( 480, 110, 40, 20 ) ); 
    TextLabel2->setProperty( "text", tr( "cycles" ) );

    TextLabel1_3 = new QLabel( Widget2, "TextLabel1_3" );
    TextLabel1_3->setGeometry( QRect( 240, 30, 120, 20 ) ); 
    TextLabel1_3->setProperty( "text", tr( "Default Square Size" ) );

    TextLabel1_3_2 = new QLabel( Widget2, "TextLabel1_3_2" );
    TextLabel1_3_2->setGeometry( QRect( 280, 60, 180, 20 ) ); 
    TextLabel1_3_2->setProperty( "text", tr( "( range from 2 to 100  pixels )" ) );

    sizeSquareTL = new QLineEdit( Widget2, "sizeSquareTL" );
    sizeSquareTL->setGeometry( QRect( 380, 30, 80, 25 ) ); 
    tabWidget->insertTab( Widget2, tr( "Edit configuration file" ) );

    tab = new QWidget( tabWidget, "tab" );

    TextLabel1 = new QLabel( tab, "TextLabel1" );
    TextLabel1->setGeometry( QRect( 80, 40, 146, 16 ) ); 
    TextLabel1->setProperty( "text", tr( "More options in the future..." ) );
    tabWidget->insertTab( tab, tr( "More options" ) );
    PreferencesDialogLayout->addWidget( tabWidget );

    Layout1 = new QHBoxLayout; 
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer );

    buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setProperty( "text", tr( "&Apply preferences" ) );
    buttonOk->setProperty( "autoDefault", QVariant( TRUE, 0 ) );
    buttonOk->setProperty( "default", QVariant( TRUE, 0 ) );
    QToolTip::add(  buttonOk, tr( "save changes to config file" ) );
    Layout1->addWidget( buttonOk );

    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setProperty( "text", tr( "&Cancel" ) );
    buttonCancel->setProperty( "autoDefault", QVariant( TRUE, 0 ) );
    QToolTip::add(  buttonCancel, tr( "discard changes" ) );
    Layout1->addWidget( buttonCancel );
    PreferencesDialogLayout->addLayout( Layout1 );

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
PreferencesDialog::~PreferencesDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

