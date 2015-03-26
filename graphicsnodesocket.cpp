#include "graphicsnodesocket.hpp"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QFont>
#include <QFontMetrics>
#include <iostream>
#include "graphicsbezieredge.hpp"

// TODO:
// * prepareGeometryChange

#define PEN_COLOR_CIRCLE      QColor("#FF000000")
#define PEN_COLOR_TEXT        QColor("#FFFFFFFF")
#define BRUSH_COLOR_SINK      QColor("#FF0077FF")
#define BRUSH_COLOR_SOURCE    QColor("#FFFF7700")
#define TEXT_ALIGNMENT_SINK   Qt::AlignLeft
#define TEXT_ALIGNMENT_SOURCE Qt::AlignRight

const qreal height = 20.0;
const qreal width = 30.0;


GraphicsNodeSocket::
GraphicsNodeSocket(GraphicsNodeSocketType type, QGraphicsItem *parent)
: GraphicsNodeSocket(type, "", parent)
{ }


GraphicsNodeSocket::
GraphicsNodeSocket(GraphicsNodeSocketType socket_type, const QString &text, QGraphicsItem *parent)
: QGraphicsItem(parent)
, _socket_type(socket_type)
, _pen_circle(PEN_COLOR_CIRCLE)
, _pen_text(PEN_COLOR_TEXT)
, _brush_circle((socket_type == SINK) ? BRUSH_COLOR_SINK : BRUSH_COLOR_SOURCE)
, _text(text)
, _text_alignment((socket_type == SINK) ? TEXT_ALIGNMENT_SINK : TEXT_ALIGNMENT_SOURCE)
, _edge(nullptr)
{
	_pen_circle.setWidth(0);
}


GraphicsNodeSocket::GraphicsNodeSocketType GraphicsNodeSocket::
socket_type() const {
	return _socket_type;
}


QRectF GraphicsNodeSocket::
boundingRect() const
{
	QFont font;
	QFontMetrics fm(font);
	int text_width = fm.width(_text);
	int text_height = fm.height();

	return QRectF(-_pen_width - _circle_radius, -_pen_width - height/2, _circle_radius*2 + _text_offset + text_width, height);
}


void GraphicsNodeSocket::
drawAlignedText(QPainter *painter, int flags)
{
	const qreal size = 32767.0;
	QPointF corner(0, 0);
	// sink
	if (flags & Qt::AlignLeft) {
		corner.setX(_circle_radius + _text_offset);
		corner.setY(-size);
		corner.ry() += size/2.0;
	}
	// source
	else if (flags & Qt::AlignRight) {
		corner.setX(-_circle_radius - _text_offset);
		corner.setY(-size);
		corner.ry() += size/2.0;
		corner.rx() -= size;
	}
	QRectF rect(corner, QSizeF(size, size));
	painter->setPen(_pen_text);
	painter->drawText(rect, flags | Qt::AlignVCenter, _text, 0);
}


void GraphicsNodeSocket::
paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	// the socket anchor is always at (0,0)
	painter->setPen(_pen_circle);
	painter->setBrush(_brush_circle);
	painter->drawEllipse(-_circle_radius, -_circle_radius, _circle_radius*2, _circle_radius*2);
	drawAlignedText(painter, _text_alignment);


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

void GraphicsNodeSocket::
mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{

}

void GraphicsNodeSocket::
mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	/*
	if (_edge) {
		scene()->removeItem(_edge);
		delete _edge;
		_edge = nullptr;
	}
	*/
}


void GraphicsNodeSocket::
mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	// start a temporary edge here
	if (event->button() == Qt::LeftButton) {

		if (event->pos().x() >= -_circle_radius
		&&  event->pos().x() <=  _circle_radius
		&&  event->pos().y() >= -_circle_radius
		&&  event->pos().y() <=  _circle_radius)
		{
			// if there is an edge going in here already, take it and drag
			// it somewhere else

			// if there is no edge here, create a new one
			/*
			if (!_edge) {
				_edge = new GraphicsBezierEdge(mapToScene(event->pos()), mapToScene(0, 0));
				scene()->addItem(_edge);
			}
			*/

		}
		else {
			QGraphicsItem::mousePressEvent(event);
		}
	}
}


void GraphicsNodeSocket::
set_edge(GraphicsBezierEdge *edge) {
	// TODO: handle edge conflict
	_edge = edge;
	notifyPositionChange();
}


void GraphicsNodeSocket::
notifyPositionChange() {
	if (!_edge) return;

	switch (_socket_type) {
	case SINK:
		_edge->set_stop(mapToScene(0,0));
		break;
	case SOURCE:
		_edge->set_start(mapToScene(0,0));
		break;
	}
}
