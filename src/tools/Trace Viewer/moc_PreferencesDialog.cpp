/****************************************************************************
** PreferencesDialog meta object code from reading C++ file 'PreferencesDialog.h'
**
** Created: Thu Jul 24 11:54:55 2003
**      by: The Qt MOC ($Id: moc_PreferencesDialog.cpp,v 1.1 2003-09-22 11:14:05 vmoya Exp $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_PreferencesDialog
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 8
#elif Q_MOC_OUTPUT_REVISION != 8
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "PreferencesDialog.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *PreferencesDialog::className() const
{
    return "PreferencesDialog";
}

QMetaObject *PreferencesDialog::metaObj = 0;

void PreferencesDialog::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("PreferencesDialog","QDialog");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION
QString PreferencesDialog::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("PreferencesDialog",s);
}

#endif // QT_NO_TRANSLATION
QMetaObject* PreferencesDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QDialog::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    QMetaData::Access *slot_tbl_access = 0;
    metaObj = QMetaObject::new_metaobject(
	"PreferencesDialog", "QDialog",
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    metaObj->set_slot_access( slot_tbl_access );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    return metaObj;
}
