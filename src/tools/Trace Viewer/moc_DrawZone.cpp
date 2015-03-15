/****************************************************************************
** DrawZone meta object code from reading C++ file 'DrawZone.h'
**
** Created: Mon Jul 28 09:20:25 2003
**      by: The Qt MOC ($Id: moc_DrawZone.cpp,v 1.1 2003-09-22 11:14:04 vmoya Exp $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_DrawZone
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 8
#elif Q_MOC_OUTPUT_REVISION != 8
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "DrawZone.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *DrawZone::className() const
{
    return "DrawZone";
}

QMetaObject *DrawZone::metaObj = 0;

void DrawZone::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QWidget::className(), "QWidget") != 0 )
	badSuperclassWarning("DrawZone","QWidget");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION
QString DrawZone::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("DrawZone",s);
}

#endif // QT_NO_TRANSLATION
QMetaObject* DrawZone::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QWidget::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void(DrawZone::*m1_t0)();
    typedef void(DrawZone::*m1_t1)();
    typedef void(DrawZone::*m1_t2)(int);
    typedef void(DrawZone::*m1_t3)(int);
    typedef void(DrawZone::*m1_t4)(int);
    typedef int(DrawZone::*m1_t5)(int);
    typedef int(DrawZone::*m1_t6)(int);
    typedef void(DrawZone::*m1_t7)(bool);
    typedef void(DrawZone::*m1_t8)(bool);
    typedef void(DrawZone::*m1_t9)(bool);
    typedef void(DrawZone::*m1_t10)(const int*,int,bool,bool);
    typedef void(DrawZone::*m1_t11)(bool,bool);
    typedef void(DrawZone::*m1_t12)();
    typedef void(DrawZone::*m1_t13)()const;
    m1_t0 v1_0 = Q_AMPERSAND DrawZone::zoomIn;
    m1_t1 v1_1 = Q_AMPERSAND DrawZone::zoomOut;
    m1_t2 v1_2 = Q_AMPERSAND DrawZone::zoomIn;
    m1_t3 v1_3 = Q_AMPERSAND DrawZone::zoomOut;
    m1_t4 v1_4 = Q_AMPERSAND DrawZone::setNewItemSize;
    m1_t5 v1_5 = Q_AMPERSAND DrawZone::displaceV;
    m1_t6 v1_6 = Q_AMPERSAND DrawZone::displaceH;
    m1_t7 v1_7 = Q_AMPERSAND DrawZone::showLines;
    m1_t8 v1_8 = Q_AMPERSAND DrawZone::showSignalsRuler;
    m1_t9 v1_9 = Q_AMPERSAND DrawZone::showCyclesRuler;
    m1_t10 v1_10 = Q_AMPERSAND DrawZone::highlightCycles;
    m1_t11 v1_11 = Q_AMPERSAND DrawZone::setHighlightMode;
    m1_t12 v1_12 = Q_AMPERSAND DrawZone::unhighlightCycles;
    m1_t13 v1_13 = Q_AMPERSAND DrawZone::dump;
    QMetaData *slot_tbl = QMetaObject::new_metadata(14);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(14);
    slot_tbl[0].name = "zoomIn()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl_access[0] = QMetaData::Public;
    slot_tbl[1].name = "zoomOut()";
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl_access[1] = QMetaData::Public;
    slot_tbl[2].name = "zoomIn(int)";
    slot_tbl[2].ptr = *((QMember*)&v1_2);
    slot_tbl_access[2] = QMetaData::Public;
    slot_tbl[3].name = "zoomOut(int)";
    slot_tbl[3].ptr = *((QMember*)&v1_3);
    slot_tbl_access[3] = QMetaData::Public;
    slot_tbl[4].name = "setNewItemSize(int)";
    slot_tbl[4].ptr = *((QMember*)&v1_4);
    slot_tbl_access[4] = QMetaData::Public;
    slot_tbl[5].name = "displaceV(int)";
    slot_tbl[5].ptr = *((QMember*)&v1_5);
    slot_tbl_access[5] = QMetaData::Public;
    slot_tbl[6].name = "displaceH(int)";
    slot_tbl[6].ptr = *((QMember*)&v1_6);
    slot_tbl_access[6] = QMetaData::Public;
    slot_tbl[7].name = "showLines(bool)";
    slot_tbl[7].ptr = *((QMember*)&v1_7);
    slot_tbl_access[7] = QMetaData::Public;
    slot_tbl[8].name = "showSignalsRuler(bool)";
    slot_tbl[8].ptr = *((QMember*)&v1_8);
    slot_tbl_access[8] = QMetaData::Public;
    slot_tbl[9].name = "showCyclesRuler(bool)";
    slot_tbl[9].ptr = *((QMember*)&v1_9);
    slot_tbl_access[9] = QMetaData::Public;
    slot_tbl[10].name = "highlightCycles(const int*,int,bool,bool)";
    slot_tbl[10].ptr = *((QMember*)&v1_10);
    slot_tbl_access[10] = QMetaData::Public;
    slot_tbl[11].name = "setHighlightMode(bool,bool)";
    slot_tbl[11].ptr = *((QMember*)&v1_11);
    slot_tbl_access[11] = QMetaData::Public;
    slot_tbl[12].name = "unhighlightCycles()";
    slot_tbl[12].ptr = *((QMember*)&v1_12);
    slot_tbl_access[12] = QMetaData::Public;
    slot_tbl[13].name = "dump()";
    slot_tbl[13].ptr = *((QMember*)&v1_13);
    slot_tbl_access[13] = QMetaData::Public;
    typedef void(DrawZone::*m2_t0)(int,int);
    m2_t0 v2_0 = Q_AMPERSAND DrawZone::realPixelsChanged;
    QMetaData *signal_tbl = QMetaObject::new_metadata(1);
    signal_tbl[0].name = "realPixelsChanged(int,int)";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    metaObj = QMetaObject::new_metaobject(
	"DrawZone", "QWidget",
	slot_tbl, 14,
	signal_tbl, 1,
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

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL realPixelsChanged
void DrawZone::realPixelsChanged( int t0, int t1 )
{
    // No builtin function for signal parameter type int,int
    QConnectionList *clist = receivers("realPixelsChanged(int,int)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef RT0 *PRT0;
    typedef void (QObject::*RT1)(int);
    typedef RT1 *PRT1;
    typedef void (QObject::*RT2)(int,int);
    typedef RT2 *PRT2;
    RT0 r0;
    RT1 r1;
    RT2 r2;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
		r0 = *((PRT0)(c->member()));
		(object->*r0)();
		break;
	    case 1:
		r1 = *((PRT1)(c->member()));
		(object->*r1)(t0);
		break;
	    case 2:
		r2 = *((PRT2)(c->member()));
		(object->*r2)(t0, t1);
		break;
	}
    }
}
