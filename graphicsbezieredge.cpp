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


GraphicsDirectedEdge::
GraphicsDirectedEdge(QPoint start, QPoint stop, qreal factor)
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


GraphicsDirectedEdge::
GraphicsDirectedEdge(QPointF start, QPointF stop, qreal factor)
: GraphicsDirectedEdge(start.toPoint(), stop.toPoint(), factor) {}


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
	connect(n1, sourceid, n2, sinkid);
}

GraphicsDirectedEdge::
GraphicsDirectedEdge(GraphicsNodeSocket *source, GraphicsNodeSocket *sink, qreal factor)
: GraphicsDirectedEdge(0, 0, 0, 0, factor)
{
	connect(source, sink);
}


GraphicsDirectedEdge::
~GraphicsDirectedEdge()
{
	delete _effect;
}





void GraphicsDirectedEdge::
mousePressEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsPathItem::mousePressEvent(event);
}


QPainterPath GraphicsDirectedEdge::
update_path() const {
	QPoint c1, c2;
	QPainterPath path(_start);

	// compute anchor point offsets
	const qreal min_dist = 0.f;
	// const qreal max_dist = 250.f;
	qreal dist = 0;
	if (_start.x() <= _stop.x()) {
		dist = std::max(min_dist, (_stop.x() - _start.x()) * _factor);
	} else {
		dist = std::max(min_dist, (_start.x() - _stop.x()) * _factor);
	}

	// dist = std::min(dist, max_dist);
	c1.setX(_start.x() + dist);
	c1.setY(_start.y());

	c2.setX(_stop.x() - dist);
	c2.setY(_stop.y());

	path.cubicTo(c1, c2, _stop);
	return path;
}


void GraphicsDirectedEdge::
set_start(int x0, int y0)
{
	set_start(QPoint(x0, y0));
}


void GraphicsDirectedEdge::
set_stop(int x1, int y1)
{
	set_stop(QPoint(x1, y1));
}

void GraphicsDirectedEdge::
set_start(QPointF p)
{
	set_start(p.toPoint());
}

void GraphicsDirectedEdge::
set_stop(QPointF p)
{
	set_stop(p.toPoint());
}

void GraphicsDirectedEdge::
set_start(QPoint p)
{
	_start = p;
	this->setPath(update_path());
}

void GraphicsDirectedEdge::
set_stop(QPoint p)
{
	_stop = p;
	this->setPath(update_path());
}

void GraphicsDirectedEdge::
connect(GraphicsNode *n1, int sourceid, GraphicsNode *n2, int sinkid)
{
	n1->connect_source(sourceid, this);
	n2->connect_sink(sinkid, this);
	_source = n1->get_source_socket(sourceid);
	_sink = n2->get_sink_socket(sinkid);
}

void GraphicsDirectedEdge::
connect(GraphicsNodeSocket *source, GraphicsNodeSocket *sink)
{
	source->set_edge(this);
	sink->set_edge(this);
	_source = source;
	_sink = sink;
}

void GraphicsDirectedEdge::
disconnect()
{
	if (_source) _source->set_edge(nullptr);
	if (_sink) _sink->set_edge(nullptr);
}


void GraphicsDirectedEdge::
disconnect_sink()
{
	if (_sink) _sink->set_edge(nullptr);
}

void GraphicsDirectedEdge::
disconnect_source()
{
	if (_source) _source->set_edge(nullptr);
}


void GraphicsDirectedEdge::
connect_sink(GraphicsNodeSocket *sink)
{
	if (_sink) _sink->set_edge(nullptr);
	_sink = sink;
	if (_sink) _sink->set_edge(this);
}

void GraphicsDirectedEdge::
connect_source(GraphicsNodeSocket *source)
{
	if (_source) _source->set_edge(nullptr);
	_source = source;
	if (_source) _source->set_edge(this);
}



void GraphicsBezierEdge::
paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/) {
	painter->setPen(_pen);
	painter->drawPath(path());
}
