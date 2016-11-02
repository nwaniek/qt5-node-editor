/* See LICENSE file for copyright and license details. */

#include "graphicsbezieredge.hpp"
#include <QPoint>
#include <utility>
#include <algorithm>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <iostream>
#include <QMetaProperty>

#include "graphicsnode.hpp"
#include "graphicsnodesocket.hpp"

#include "graphicsnodesocket_p.h"
#include "graphicsbezieredge_p.h"


GraphicsDirectedEdge::
GraphicsDirectedEdge(const QPoint& start, const QPoint& stop, qreal factor)
: QObject(), QGraphicsPathItem(), d_ptr(new GraphicsDirectedEdgePrivate(this))

{
    d_ptr->_start  = start;
    d_ptr->_stop   = stop;
    d_ptr->_factor = factor;

    d_ptr->_pen.setWidth(2);
    setZValue(-1);

    d_ptr->_effect->setBlurRadius(15.0);
    d_ptr->_effect->setColor(QColor("#99050505"));
    setGraphicsEffect(d_ptr->_effect);
}


GraphicsDirectedEdge::
GraphicsDirectedEdge(const QPointF& start, const QPointF& stop, qreal factor)
: GraphicsDirectedEdge(start.toPoint(), stop.toPoint(), factor)
{}


GraphicsDirectedEdge::
GraphicsDirectedEdge(int x0, int y0, int x1, int y1, qreal factor)
: GraphicsDirectedEdge(QPoint(x0, y0), QPoint(x1, y1), factor) {}


GraphicsDirectedEdge::
GraphicsDirectedEdge(qreal factor)
: GraphicsDirectedEdge(0, 0, 0, 0, factor) {}


GraphicsDirectedEdge::
GraphicsDirectedEdge(GraphicsNode *n1, int sourceid, GraphicsNode *n2, int sinkid, qreal factor)
: GraphicsDirectedEdge(0, 0, 0, 0, factor)
{
    d_ptr->connect(n1, sourceid, n2, sinkid);
}

GraphicsNodeSocket *GraphicsDirectedEdge::
source() const
{
    return  d_ptr->_source;
}

GraphicsNodeSocket *GraphicsDirectedEdge::
sink() const
{
    return d_ptr->_sink;
}

int GraphicsDirectedEdge::
type() const
{
    return GraphicsNodeItemTypes::TypeBezierEdge;
}


GraphicsDirectedEdge::
GraphicsDirectedEdge(GraphicsNodeSocket *source, GraphicsNodeSocket *sink, qreal factor)
: GraphicsDirectedEdge(0, 0, 0, 0, factor)
{
    d_ptr->connect(source, sink);
}


GraphicsDirectedEdge::
~GraphicsDirectedEdge()
{
    delete d_ptr->_effect;
}


void GraphicsDirectedEdge::
mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsPathItem::mousePressEvent(event);
}


void GraphicsDirectedEdge::onSourceDataChange()
{
    QVariant var;
    // get Source Data

    QObject* data1 = d_ptr->_source->d_ptr->m_data;
    const QMetaObject* mo1 = data1->metaObject();
    QMetaProperty mp1 = mo1->property(d_ptr->_source->d_ptr->m_index);
    const char *name1 = mp1.name();

    var = data1->property(name1);

    // set sink
    QObject* data2 = d_ptr->_sink->d_ptr->m_data;
    const QMetaObject* mo2 = data2->metaObject();
    QMetaProperty mp2 = mo2->property(d_ptr->_sink->d_ptr->m_index);
    const char * name2 = mp2.name();
}


void GraphicsDirectedEdgePrivate::
setStart(int x0, int y0)
{
    setStart(QPoint(x0, y0));
}


void GraphicsDirectedEdgePrivate::
setStop(int x1, int y1)
{
    setStop(QPoint(x1, y1));
}


void GraphicsDirectedEdgePrivate::
setStart(QPointF p)
{
    setStart(p.toPoint());
}


void GraphicsDirectedEdgePrivate::
setStop(QPointF p)
{
    setStop(p.toPoint());
}


void GraphicsDirectedEdgePrivate::
setStart(QPoint p)
{
    _start = p;
    q_ptr->updatePath();
}


void GraphicsDirectedEdgePrivate::
setStop(QPoint p)
{
    _stop = p;
    q_ptr->updatePath();
}


void GraphicsDirectedEdgePrivate::
connect(GraphicsNode *n1, int sourceid, GraphicsNode *n2, int sinkid)
{
    if (_source)
        q_ptr->QObject::disconnect(q_ptr,SLOT(onSourceDataChange()));

    n1->connectSource(sourceid, q_ptr);
    n2->connectSink(sinkid, q_ptr);

    connectSource(n1->getSourceSocket(sourceid));
    _sink = n2->getSinkSocket(sinkid);
}


void GraphicsDirectedEdgePrivate::
connect(GraphicsNodeSocket *source, GraphicsNodeSocket *sink)
{
    if (_source) {
        if (auto data = _source->d_ptr->m_data)
            QObject::disconnect(data,0,q_ptr,0);
    }

    source->setEdge(q_ptr);
    sink->setEdge(q_ptr);
    _source = source;
    _sink = sink;
}


void GraphicsDirectedEdgePrivate::
disconnectBoth()
{
    if (_source) {
        _source->setEdge(nullptr);

        if (auto data = _source->d_ptr->m_data)
            QObject::disconnect(data,0,q_ptr,0);

    }
    if (_sink) _sink->setEdge(nullptr);
}


void GraphicsDirectedEdgePrivate::
disconnectSink()
{
    if (_sink) _sink->setEdge(nullptr);
}


void GraphicsDirectedEdgePrivate::
disconnectSource()
{
    if (_source) {
        _source->setEdge(nullptr);

        if (auto data = _source->d_ptr->m_data)
            QObject::disconnect(data,0,q_ptr,0);
    }
}

void GraphicsDirectedEdgePrivate::
connectSink(GraphicsNodeSocket *sink)
{
    if (_sink) _sink->setEdge(nullptr);
    _sink = sink;
    if (_sink) _sink->setEdge(q_ptr);
}


void GraphicsDirectedEdgePrivate::
connectSource(GraphicsNodeSocket *source)
{
    if (_source){
        _source->setEdge(nullptr);

        if (auto data = _source->d_ptr->m_data)
            QObject::disconnect(data, 0, q_ptr, 0);
    }

    _source = source;

    if (_source){
        _source->setEdge(q_ptr);

        if (auto data = _source->d_ptr->m_data) {
            const QMetaObject* mo = data->metaObject();
            QMetaProperty mp = mo->property(_source->d_ptr->m_index);
            QMetaMethod notifySignal = mp.notifySignal();

            int functionIndex = q_ptr->metaObject()->indexOfSlot("onSourceDataChange()");
            QMetaMethod edgeInternalSlot = q_ptr->metaObject()->method(functionIndex);

            QObject::connect(data, notifySignal, q_ptr, edgeInternalSlot);
        }
    }
}

void GraphicsBezierEdge::
updatePath()
{
    QPainterPath path(d_ptr->_start);

    // compute anchor point offsets
    const qreal min_dist = 0.f;

    const qreal dist = (d_ptr->_start.x() <= d_ptr->_stop.x()) ?
        std::max(min_dist, (d_ptr->_stop.x() - d_ptr->_start.x()) * d_ptr->_factor):
        std::max(min_dist, (d_ptr->_start.x() - d_ptr->_stop.x()) * d_ptr->_factor);

    const QPoint c1 = {
        d_ptr->_start.x() + dist,
        d_ptr->_start.y()
    };

    const QPoint c2 = {
        d_ptr->_stop.x() - dist,
        d_ptr->_stop.y()
    };

    path.cubicTo(c1, c2, d_ptr->_stop);
    setPath(path);
}

int GraphicsBezierEdge::
type() const
{
    return GraphicsNodeItemTypes::TypeBezierEdge;
}

void GraphicsBezierEdge::
paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->setPen(GraphicsDirectedEdge::d_ptr->_pen);
    painter->drawPath(path());
}

