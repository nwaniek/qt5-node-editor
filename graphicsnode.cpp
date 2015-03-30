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

#include <algorithm>

#include "graphicsbezieredge.hpp"
#include "graphicsnodesocket.hpp"


GraphicsNode::GraphicsNode(QGraphicsItem *parent)
: QGraphicsItem(parent)
, _changed(false)
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
	return QRectF(-_pen_width - _socket_size,
			-_pen_width,
			_width + _pen_width + _socket_size,
			_height + _pen_width).normalized();
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
	QPainterPath path_outline = QPainterPath();
	path_outline.addRoundedRect(QRect(0, 0, _width, _height), edge_size, edge_size);
	painter->setPen(isSelected() ? _pen_selected : _pen_default);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(path_outline.simplified());

}


void GraphicsNode::
mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	// TODO: ordering after selection/deselection cycle
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
add_sink(const QString &text)
{
	auto s = new GraphicsNodeSocket(GraphicsNodeSocket::SINK, text, this);
	_sinks.push_back(s);
	_changed = true;
	prepareGeometryChange();
	updateGeometry();
	repositionSockets();
	return s;
}


const GraphicsNodeSocket* GraphicsNode::
add_source(const QString &text)
{
	auto s = new GraphicsNodeSocket(GraphicsNodeSocket::SOURCE, text, this);
	_sources.push_back(s);
	_changed = true;
	prepareGeometryChange();
	updateGeometry();
	repositionSockets();
	return s;
}


void GraphicsNode::
connect_source(int i, GraphicsDirectedEdge *edge)
{
	_sources[i]->set_edge(edge);
}


void GraphicsNode::
connect_sink(int i, GraphicsDirectedEdge *edge)
{
	_sinks[i]->set_edge(edge);
}


void GraphicsNode::
repositionSockets()
{
	int ypos;

	// compute position of sockets. sinks are placed left/top
	ypos = _top_margin;
	for (size_t i = 0; i < _sinks.size(); i++) {
		auto s = _sinks[i];
		auto rect = s->boundingRect();

		s->setPos(0, ypos);
		ypos += rect.height() + _item_padding;
	}

	// sources are placed bottom/right
	ypos = _height - _bottom_margin;
	for (size_t i = _sources.size(); i > 0; i--) {
		auto s = _sources[i-1];
		auto rect = s->boundingRect();

		// TODO: bottom-margin
		ypos -= rect.height();
		s->setPos(_width, ypos);
		ypos -= _item_padding;
	}
}

void GraphicsNode::
updateGeometry()
{
	if (!_changed) return;

	qreal height = _top_margin;

	for (size_t i = 0; i < _sinks.size(); i++) {
		auto s = _sinks[i];
		auto rect = s->boundingRect();
		height += rect.height() + _item_padding;
	}

	// TODO contained element size

	for (size_t i = _sources.size(); i > 0; i--) {
		auto s = _sources[i-1];
		auto rect = s->boundingRect();
		height += rect.height() + _item_padding;
	}

	height += _bottom_margin;
	_height = std::max(height, _min_height);

	// TODO width computation
	_changed = false;
}


GraphicsNodeSocket* GraphicsNode::
get_source_socket(const size_t id)
{
	if (id < _sources.size())
		return _sources[id];
	else
		return nullptr;
}

GraphicsNodeSocket* GraphicsNode::
get_sink_socket(const size_t id)
{
	if (id < _sinks.size())
		return _sinks[id];
	else
		return nullptr;
}
