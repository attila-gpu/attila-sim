/****************************************************************************
** LoadingFileDialog meta object code from reading C++ file 'LoadingFileDialog.h'
**
** Created: Wed Jul 16 08:27:31 2003
**      by: The Qt MOC ($Id: moc_LoadingFileDialog.cpp,v 1.1 2003-09-22 11:14:04 vmoya Exp $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_LoadingFileDialog
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 8
#elif Q_MOC_OUTPUT_REVISION != 8
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "LoadingFileDialog.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *LoadingFileDialog::className() const
{
    return "LoadingFileDialog";
}

QMetaObject *LoadingFileDialog::metaObj = 0;

void LoadingFileDialog::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("LoadingFileDialog","QDialog");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION
QString LoadingFileDialog::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("LoadingFileDialog",s);
}

#endif // QT_NO_TRANSLATION
QMetaObject* LoadingFileDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QDialog::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    QMetaData::Access *slot_tbl_access = 0;
    metaObj = QMetaObject::new_metaobject(
	"LoadingFileDialog", "QDialog",
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
