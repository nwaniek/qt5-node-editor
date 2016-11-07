/* See LICENSE file for copyright and license details. */

#include "graphicsbezieredge.hpp"
#include <algorithm>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPathItem>
#include <QMetaProperty>

#include <QtCore/QDebug>

#include "graphicsnode.hpp"
#include "graphicsnodesocket.hpp"

#include "graphicsnodesocket_p.h"
#include "graphicsbezieredge_p.h"

#include "qreactiveproxymodel.h"

#include "qnodeeditorsocketmodel.h"

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
GraphicsDirectedEdge(QNodeEditorEdgeModel* m, const QPoint& start, const QPoint& stop, qreal factor)
: QObject(), d_ptr(new GraphicsDirectedEdgePrivate(this))

{
    d_ptr->m_pModel = m;
    d_ptr->m_pGrpahicsItem = new GraphicsBezierItem(d_ptr);
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
GraphicsDirectedEdge(QNodeEditorEdgeModel* m, qreal factor)
: GraphicsDirectedEdge(m, {0, 0}, {0, 0}, factor) {}

GraphicsDirectedEdge::
GraphicsDirectedEdge(QNodeEditorEdgeModel* m, GraphicsNodeSocket *source, GraphicsNodeSocket *sink, qreal factor)
: GraphicsDirectedEdge(m, {0, 0}, {0, 0}, factor)
{
    d_ptr->connectIndex(source);
    d_ptr->connectIndex(sink);
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

        //if (auto data = _source->d_ptr->m_data)
        //    QObject::disconnect(data,0,q_ptr,0);
    }
}

GraphicsBezierEdge::GraphicsBezierEdge(QNodeEditorEdgeModel* m, qreal factor)
    : GraphicsDirectedEdge(m, factor)
{ d_ptr->m_pGrpahicsItem = new GraphicsBezierItem(d_ptr); }

GraphicsBezierEdge::GraphicsBezierEdge(QNodeEditorEdgeModel* m, GraphicsNodeSocket *source, GraphicsNodeSocket *sink, qreal factor)
    : GraphicsDirectedEdge(m, source, sink, factor)
{ d_ptr->m_pGrpahicsItem = new GraphicsBezierItem(d_ptr); }

void GraphicsBezierItem::
updatePath()
{
    QPainterPath path(d_ptr->_start);

    // compute anchor point offsets
    const qreal min_dist = 0.f; //FIXME this is dead code? can the code below ever get negative?

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

void GraphicsDirectedEdgePrivate::connectIndex(GraphicsNodeSocket *other)
{
    if (!other)
        return;

    typedef GraphicsNodeSocket::SocketType ST; // for readability

    qDebug() << "\n\n\nIN CONNECT INDEX" << q_ptr->source() << q_ptr->sink() << other;

    // Both edge ends cannot have the same socket type
    if (other->socketType() == ST::SINK && (!_source) && _sink)
        return;

    if (other->socketType() == ST::SOURCE && (!_sink) && _source)
        return;

    //TODO handle re-connection
    if (_source && _sink)
        return;

    //FIXME react to the reactive model
    switch(other->socketType()) {
        case ST::SINK:
            if (_sink) _sink->setEdge(nullptr); //TODO turn this into an helper function
            _sink = other;
            if (_sink) _sink->setEdge(q_ptr);
            break;
        case ST::SOURCE:
            if (_source) _source->setEdge(nullptr);
            _source = other;
            if (_source) _source->setEdge(q_ptr);
            break;
    }

    if ((!_sink) || (!_source))
        return;

    m_pModel->connectSocket(_source->index() , _sink->index());
}

