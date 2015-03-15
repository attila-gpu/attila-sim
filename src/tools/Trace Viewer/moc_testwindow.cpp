/****************************************************************************
** TestWindow meta object code from reading C++ file 'testwindow.h'
**
** Created: Thu Jul 17 11:30:38 2003
**      by: The Qt MOC ($Id: moc_testwindow.cpp,v 1.1 2003-09-22 11:14:05 vmoya Exp $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_TestWindow
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 8
#elif Q_MOC_OUTPUT_REVISION != 8
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "testwindow.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *TestWindow::className() const
{
    return "TestWindow";
}

QMetaObject *TestWindow::metaObj = 0;

void TestWindow::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QMainWindow::className(), "QMainWindow") != 0 )
	badSuperclassWarning("TestWindow","QMainWindow");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION
QString TestWindow::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("TestWindow",s);
}

#endif // QT_NO_TRANSLATION
QMetaObject* TestWindow::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QMainWindow::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void(TestWindow::*m1_t0)();
    typedef void(TestWindow::*m1_t1)(int);
    typedef void(TestWindow::*m1_t2)(int,int);
    typedef void(TestWindow::*m1_t3)(QAction*);
    m1_t0 v1_0 = Q_AMPERSAND TestWindow::editPreferences;
    m1_t1 v1_1 = Q_AMPERSAND TestWindow::showDebug;
    m1_t2 v1_2 = Q_AMPERSAND TestWindow::updateRulers;
    m1_t3 v1_3 = Q_AMPERSAND TestWindow::updateDependences;
    QMetaData *slot_tbl = QMetaObject::new_metadata(4);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(4);
    slot_tbl[0].name = "editPreferences()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl_access[0] = QMetaData::Public;
    slot_tbl[1].name = "showDebug(int)";
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl_access[1] = QMetaData::Public;
    slot_tbl[2].name = "updateRulers(int,int)";
    slot_tbl[2].ptr = *((QMember*)&v1_2);
    slot_tbl_access[2] = QMetaData::Public;
    slot_tbl[3].name = "updateDependences(QAction*)";
    slot_tbl[3].ptr = *((QMember*)&v1_3);
    slot_tbl_access[3] = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"TestWindow", "QMainWindow",
	slot_tbl, 4,
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
