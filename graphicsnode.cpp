#include "graphicsnode.hpp"
#include <QPushButton>
#include <QPen>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QLabel>
#include <iostream>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QTextDocument>
#include <QGraphicsDropShadowEffect>

#include "graphicsbezieredge.hpp"
#include "graphicsnodesocket.hpp"


GraphicsNode::GraphicsNode(QGraphicsItem *parent)
: QGraphicsItem(parent)
, _width(150)
, _height(120)
, _pen_default(QColor("#7F000000"))
, _pen_selected(QColor("#FFFF36A7"))
// , _pen_selected(QColor("#FFFFA836"))
// , _pen_selected(QColor("#FF00FF27"))
, _pen_sources(QColor("#FF000000"))
, _pen_sinks(QColor("#FF000000"))
, _brush_title(QColor("#E3212121"))
, _brush_background(QColor("#E31a1a1a"))
, _brush_sources(QColor("#FFFF7700"))
, _brush_sinks(QColor("#FF0077FF"))
, _effect(new QGraphicsDropShadowEffect())
{
	for (auto p : {&_pen_default, &_pen_selected, &_pen_default, &_pen_selected}) {
		p->setWidth(0);
	}

	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);


	_effect->setBlurRadius(13.0);
	_effect->setColor(QColor("#99121212"));
	setGraphicsEffect(_effect);

	/*
	auto q = new QGraphicsTextItem("GraphicsNode GraphicsNode GraphicsNode", this);
	q->setDefaultTextColor(Qt::white);
	q->setPos(4, 0);
	q->setTextWidth(142);
	auto opts = q->document()->defaultTextOption();
	opts.setAlignment(Qt::AlignRight);
	q->document()->setDefaultTextOption(opts);
	*/
}

GraphicsNode::
~GraphicsNode()
{
	delete _effect;
}


QRectF GraphicsNode::
boundingRect() const
{
	// TODO : compute width/height from contained elements
	const qreal pen_width = 1.0;

	// size of sockets
	const qreal socket_size = 6.0;

	return QRectF(-pen_width - socket_size,
			-pen_width,
			_width + pen_width + socket_size,
			_height + pen_width).normalized();
}


void GraphicsNode::
paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	const qreal edge_size = 10.0;
	const qreal title_height = 20.0;

	// path for the caption of this node
	QPainterPath path_title;
	path_title.setFillRule(Qt::WindingFill);
	path_title.addRoundedRect(QRect(0, 0, _width, title_height), edge_size, edge_size);
	path_title.addRect(0, title_height - edge_size, edge_size, edge_size);
	path_title.addRect(_width - edge_size, title_height - edge_size, edge_size, edge_size);
	painter->setPen(Qt::NoPen);
	painter->setBrush(_brush_title);
	painter->drawPath(path_title.simplified());

	// path for the content of this node
	QPainterPath path_content;
	path_content.setFillRule(Qt::WindingFill);
	path_content.addRoundedRect(QRect(0, title_height, _width, _height - title_height), edge_size, edge_size);
	path_content.addRect(0, title_height, edge_size, edge_size);
	path_content.addRect(_width - edge_size, title_height, edge_size, edge_size);
	painter->setPen(Qt::NoPen);
	painter->setBrush(_brush_background);
	painter->drawPath(path_content.simplified());

	// path for the outline
	QPainterPath path_outline;
	path_outline.addRoundedRect(QRect(0, 0, _width, _height), edge_size, edge_size);
	painter->setPen(isSelected() ? _pen_selected : _pen_default);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(path_outline.simplified());

}


void GraphicsNode::
mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsItem::mousePressEvent(event);
	if (isSelected())
		setZValue(1);
	else
		setZValue(0);
}


QVariant GraphicsNode::
itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == QGraphicsItem::ItemSelectedChange) {
		if (value == true)
			setZValue(1);
		else
			setZValue(0);
	} else
	if (change == QGraphicsItem::ItemPositionChange
	|| change == QGraphicsItem::ItemPositionHasChanged) {
		for (auto sink: _sinks)
			sink->notifyPositionChange();

		for (auto source: _sources)
			source->notifyPositionChange();
	}
	return QGraphicsItem::itemChange(change, value);
}





const GraphicsNodeSocket* GraphicsNode::
add_sink()
{
	auto s = new GraphicsNodeSocket(GraphicsNodeSocket::SINK, this);
	_sinks.push_back(s);
	return s;
}

const GraphicsNodeSocket* GraphicsNode::
add_sink(const QString &text)
{
	auto s = new GraphicsNodeSocket(GraphicsNodeSocket::SINK, text, this);
	_sinks.push_back(s);
	s->setPos(0, 30 + 20 * (_sinks.size() - 1));
	return s;
}

const GraphicsNodeSocket* GraphicsNode::
add_source(const QString &text)
{
	auto s = new GraphicsNodeSocket(GraphicsNodeSocket::SOURCE, text, this);
	_sources.push_back(s);
	s->setPos(150, 30 + 20 * (_sinks.size() + _sources.size() - 1));
	return s;
}


void GraphicsNode::
connect_source(int i, GraphicsBezierEdge *edge)
{
	_sources[i]->set_edge(edge);
}


void GraphicsNode::
connect_sink(int i, GraphicsBezierEdge *edge)
{
	_sinks[i]->set_edge(edge);
}
