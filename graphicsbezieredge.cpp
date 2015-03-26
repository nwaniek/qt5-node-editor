#include "graphicsbezieredge.hpp"
#include <QPoint>
#include <utility>
#include <algorithm>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <iostream>

#include "graphicsnode.hpp"
#include "graphicsnodesocket.hpp"


GraphicsBezierEdge::
GraphicsBezierEdge(QPoint start, QPoint stop, float factor)
: _pen(QColor("#00FF00"))
, _effect(new QGraphicsDropShadowEffect())
, _start(start)
, _stop(stop)
, _factor(factor)
{
	_pen.setWidth(2);
	setPath(update_path());
	setZValue(-1);

	_effect->setBlurRadius(15.0);
	_effect->setColor(QColor("#99050505"));
	setGraphicsEffect(_effect);
}


GraphicsBezierEdge::
GraphicsBezierEdge(QPointF start, QPointF stop, float factor)
: GraphicsBezierEdge(start.toPoint(), stop.toPoint(), factor) {}


GraphicsBezierEdge::
GraphicsBezierEdge(int x0, int y0, int x1, int y1, float factor)
: GraphicsBezierEdge(QPoint(x0, y0), QPoint(x1, y1), factor) {}


GraphicsBezierEdge::
GraphicsBezierEdge(float factor)
: GraphicsBezierEdge(0, 0, 0, 0, factor) {}


GraphicsBezierEdge::
GraphicsBezierEdge(GraphicsNode *n1, int sourceid, GraphicsNode *n2, int sinkid, float factor)
: GraphicsBezierEdge(0, 0, 0, 0, factor)
{
	connect(n1, sourceid, n2, sinkid);
}

GraphicsBezierEdge::
GraphicsBezierEdge(GraphicsNodeSocket *source, GraphicsNodeSocket *sink, float factor)
: GraphicsBezierEdge(0, 0, 0, 0, factor)
{
	connect(source, sink);
}


GraphicsBezierEdge::
~GraphicsBezierEdge()
{
	delete _effect;
}


void GraphicsBezierEdge::
paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/) {
	painter->setPen(_pen);
	painter->drawPath(path());
}


void GraphicsBezierEdge::
mousePressEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsPathItem::mousePressEvent(event);
}


QPainterPath GraphicsBezierEdge::
update_path() const {
	QPoint c1, c2;
	QPainterPath path(_start);

	// compute anchor point offsets
	const float min_dist = 100.f;
	int dist = 0;
	if (_start.x() <= _stop.x()) {
		dist = std::max(min_dist, (_stop.x() - _start.x()) * _factor);
	} else {
		dist = std::max(min_dist, (_start.x() - _stop.x()) * _factor);
	}

	c1.setX(_start.x() + dist);
	c1.setY(_start.y());

	c2.setX(_stop.x() - dist);
	c2.setY(_stop.y());

	path.cubicTo(c1, c2, _stop);
	return path;
}


void GraphicsBezierEdge::
set_start(int x0, int y0)
{
	set_start(QPoint(x0, y0));
}


void GraphicsBezierEdge::
set_stop(int x1, int y1)
{
	set_stop(QPoint(x1, y1));
}

void GraphicsBezierEdge::
set_start(QPointF p)
{
	set_start(p.toPoint());
}

void GraphicsBezierEdge::
set_stop(QPointF p)
{
	set_stop(p.toPoint());
}

void GraphicsBezierEdge::
set_start(QPoint p)
{
	_start = p;
	this->setPath(update_path());
}

void GraphicsBezierEdge::
set_stop(QPoint p)
{
	_stop = p;
	this->setPath(update_path());
}

void GraphicsBezierEdge::
connect(GraphicsNode *n1, int sourceid, GraphicsNode *n2, int sinkid)
{
	n1->connect_source(sourceid, this);
	n2->connect_sink(sinkid, this);
}

void GraphicsBezierEdge::
connect(GraphicsNodeSocket *source, GraphicsNodeSocket *sink)
{
	source->set_edge(this);
	sink->set_edge(this);
}
