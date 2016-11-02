/* See LICENSE file for copyright and license details. */

#include "graphicsbezieredge.hpp"
#include <algorithm>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPathItem>
#include <QMetaProperty>

#include "graphicsnode.hpp"
#include "graphicsnodesocket.hpp"

#include "graphicsnodesocket_p.h"
#include "graphicsbezieredge_p.h"

class GraphicsEdgeItem : public QGraphicsPathItem
{
public:
    explicit GraphicsEdgeItem(GraphicsDirectedEdgePrivate* s) : d_ptr(s) {}

    virtual int type() const override;

    virtual void updatePath() = 0;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    GraphicsDirectedEdgePrivate* d_ptr;
};

class GraphicsBezierItem final : public GraphicsEdgeItem
{
public:
    explicit GraphicsBezierItem(GraphicsDirectedEdgePrivate* s) :
        GraphicsEdgeItem(s) {}

    virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) override;
    virtual int type() const override;
    virtual void updatePath() override;
};

GraphicsDirectedEdge::
GraphicsDirectedEdge(const QPoint& start, const QPoint& stop, qreal factor)
: QObject(), d_ptr(new GraphicsDirectedEdgePrivate(this))

{
    d_ptr->_start  = start;
    d_ptr->_stop   = stop;
    d_ptr->_factor = factor;

    d_ptr->_pen.setWidth(2);
    d_ptr->m_pGrpahicsItem->setZValue(-1);

    d_ptr->_effect->setBlurRadius(15.0);
    d_ptr->_effect->setColor(QColor("#99050505"));
    d_ptr->m_pGrpahicsItem->setGraphicsEffect(d_ptr->_effect);
}

GraphicsDirectedEdge::
GraphicsDirectedEdge(qreal factor)
: GraphicsDirectedEdge({0, 0}, {0, 0}, factor) {}

GraphicsDirectedEdge::
GraphicsDirectedEdge(GraphicsNodeSocket *source, GraphicsNodeSocket *sink, qreal factor)
: GraphicsDirectedEdge({0, 0}, {0, 0}, factor)
{
    d_ptr->connect(source, sink);
}

GraphicsNodeSocket *GraphicsDirectedEdge::
source() const
{
    return d_ptr->_source;
}

GraphicsNodeSocket *GraphicsDirectedEdge::
sink() const
{
    return d_ptr->_sink;
}

int GraphicsBezierItem::
type() const
{
    return GraphicsNodeItemTypes::TypeBezierEdge;
}

QGraphicsItem *GraphicsDirectedEdge::
graphicsItem() const
{
    Q_ASSERT(d_ptr->m_pGrpahicsItem);
    return d_ptr->m_pGrpahicsItem;
}


GraphicsDirectedEdge::
~GraphicsDirectedEdge()
{
    delete d_ptr->_effect;
    delete d_ptr->m_pGrpahicsItem;
    delete d_ptr;
}


void GraphicsEdgeItem::
mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //FIXME currently dead code, need to be implemented
    QGraphicsPathItem::mousePressEvent(event);
}


void GraphicsDirectedEdge::onSourceDataChange()
{
    // get Source Data

    //TODO figure out this dead code
    /*QObject* data1 = d_ptr->_source->d_ptr->m_data;
    const QMetaObject* mo1 = data1->metaObject();
    QMetaProperty mp1 = mo1->property(d_ptr->_source->d_ptr->m_index);
    const char *name1 = mp1.name();

    //auto var = data1->property(name1);

    // set sink
    QObject* data2 = d_ptr->_sink->d_ptr->m_data;
    const QMetaObject* mo2 = data2->metaObject();
    QMetaProperty mp2 = mo2->property(d_ptr->_sink->d_ptr->m_index);
    const char * name2 = mp2.name();*/
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
    m_pGrpahicsItem->updatePath();
}


void GraphicsDirectedEdgePrivate::
setStop(QPoint p)
{
    _stop = p;
    m_pGrpahicsItem->updatePath();
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

GraphicsBezierEdge::GraphicsBezierEdge(qreal factor)
    : GraphicsDirectedEdge(factor)
{ d_ptr->m_pGrpahicsItem = new GraphicsBezierItem(d_ptr); }

GraphicsBezierEdge::GraphicsBezierEdge(GraphicsNodeSocket *source, GraphicsNodeSocket *sink, qreal factor)
    : GraphicsDirectedEdge(source, sink, factor)
{ d_ptr->m_pGrpahicsItem = new GraphicsBezierItem(d_ptr); }

void GraphicsBezierItem::
updatePath()
{
    QPainterPath path(d_ptr->_start);

    // compute anchor point offsets
    const qreal min_dist = 0.f;

    const qreal dist = (d_ptr->_start.x() <= d_ptr->_stop.x()) ?
        std::max(min_dist, (d_ptr->_stop.x() - d_ptr->_start.x()) * d_ptr->_factor):
        std::max(min_dist, (d_ptr->_start.x() - d_ptr->_stop.x()) * d_ptr->_factor);

    const QPoint c1 {
        d_ptr->_start.x() + dist,
        d_ptr->_start.y()
    };

    const QPoint c2 {
        d_ptr->_stop.x() - dist,
        d_ptr->_stop.y()
    };

    path.cubicTo(c1, c2, d_ptr->_stop);
    setPath(path);
}

int GraphicsEdgeItem::
type() const
{
    return GraphicsNodeItemTypes::TypeBezierEdge;
}

void GraphicsBezierItem::
paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->setPen(d_ptr->_pen);
    painter->drawPath(path());
}

