#include "graphicsnodesource.hpp"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QFont>
#include <QFontMetrics>
#include <iostream>
#include "graphicsbezieredge.hpp"

static const qreal pen_width = 1.0;
static const qreal width = 30.0;
static const qreal height = 20.0;
static const qreal circle_radius = 6.0;
static const qreal text_offset = 3.0;


GraphicsNodeSource::
GraphicsNodeSource(QGraphicsItem *parent)
: GraphicsNodeSource("", parent)
{ }


GraphicsNodeSource::
GraphicsNodeSource(const QString &text, QGraphicsItem *parent)
: QGraphicsItem(parent)
, _pen_circle(QColor("#FF000000"))
, _brush_circle(QColor("#FFFF7700"))
, _text(text)
, _edge(nullptr)
{
	_pen_circle.setWidth(0);
}


QRectF GraphicsNodeSource::
boundingRect() const
{
	QFont font;
	QFontMetrics fm(font);
	int text_width = fm.width(_text);
	int text_height = fm.height();

	return QRectF(-pen_width - circle_radius, -pen_width - height/2, circle_radius*2 + text_offset + text_width, height);
}


void GraphicsNodeSource::
paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	// draw the bounding box for debugging purposes
	// painter->setPen(_pen_circle);
	// painter->drawRect(-circle_radius, -height/2, width, height);
	// painter->drawRect(boundingRect());

	// the sink anchor is at (0,0)
	painter->setPen(_pen_circle);
	painter->setBrush(_brush_circle);
	painter->drawEllipse(-circle_radius, -circle_radius, circle_radius*2, circle_radius*2);

	// draw the text of this sink
	if (_text != "") {
		const qreal size = 32767.0;
		QPointF corner(- circle_radius - text_offset, -size);
		corner.ry() += size/2.0;
		corner.rx() -= size;
		QRectF rect(corner, QSizeF(size, size));

		painter->setPen(QPen(Qt::white));
		painter->drawText(rect, Qt::AlignRight | Qt::AlignVCenter, _text, 0);
	}

	/*
	const qreal size = 32767.0;
	QPointF corner(point.x(), point.y() - size);
	if (flags & Qt::AlignHCenter) corner.rx() -= size/2.0;
	else if (flags & Qt::AlignRight) corner.rx() -= size;
	if (flags & Qt::AlignVCenter) corner.ry() += size/2.0;
	else if (flags & Qt::AlignTop) corner.ry() += size;
	else flags |= Qt::AlignBottom;
	QRectF rect(corner, QSizeF(size, size));
	painter.drawText(rect, flags, text, boundingRect);
	*/
}

void GraphicsNodeSource::
mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{

}

void GraphicsNodeSource::
mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	if (_edge) {
		scene()->removeItem(_edge);
		delete _edge;
		_edge = nullptr;
	}
}


void GraphicsNodeSource::
mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	// start a temporary edge here
	if (event->button() == Qt::LeftButton) {

		if (event->pos().x() >= -circle_radius
		&&  event->pos().x() <=  circle_radius
		&&  event->pos().y() >= -circle_radius
		&&  event->pos().y() <=  circle_radius)
		{

			// if there is an edge going in here already, take it and drag
			// it somewhere else


			// if there is no edge here, create a new one
			if (!_edge) {
				_edge = new GraphicsBezierEdge(mapToScene(0, 0), mapToScene(event->pos()));
				scene()->addItem(_edge);
			}

		}
		else {
			QGraphicsItem::mousePressEvent(event);
		}
	}
}

void GraphicsNodeSource::
set_edge(GraphicsBezierEdge *edge) {
	// TODO: handle edge conflict
	_edge = edge;
	notifyPositionChange();
}


void GraphicsNodeSource::
notifyPositionChange() {
	if (!_edge) return;

	_edge->set_start(mapToScene(0,0));
}
