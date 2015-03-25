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


GraphicsNode::GraphicsNode(QGraphicsItem *parent)
: QGraphicsItem(parent)
// , _pen_default(QColor("#1F1F1F"))
, _pen_default(QColor("#7F000000"))
, _pen_selected(QColor("#7FF19940"))
, _pen_sources(QColor("#FF000000"))
, _pen_sinks(QColor("#FF000000"))
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


	_effect->setBlurRadius(10.0);
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
	const qreal width = 150;
	const qreal height = 120;
	const qreal pen_width = 1;

	return QRectF(-pen_width, -pen_width, width + pen_width, height + pen_width);
}


void GraphicsNode::
paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	// paint background / frame
	painter->setBrush(_brush_background);
	if (isSelected())
		painter->setPen(_pen_selected);
	else
		painter->setPen(_pen_default);

	QPainterPath path;
	path.setFillRule(Qt::WindingFill);
	path.addRoundedRect(QRect(0, 0, 150, 100), 10, 10);
	painter->drawPath(path.simplified());
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





const GraphicsNodeSink* GraphicsNode::
add_sink()
{
	auto s = new GraphicsNodeSink(this);
	_sinks.push_back(s);
	return s;
}

const GraphicsNodeSink* GraphicsNode::
add_sink(const QString &text)
{
	auto s = new GraphicsNodeSink(text, this);
	_sinks.push_back(s);
	s->setPos(10, 30 + 20 * (_sinks.size() - 1));
	return s;
}

const GraphicsNodeSource* GraphicsNode::
add_source(const QString &text)
{
	auto s = new GraphicsNodeSource(text, this);
	_sources.push_back(s);
	s->setPos(140, 30 + 20 * (_sinks.size() + _sources.size() - 1));
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
