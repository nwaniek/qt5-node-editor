#include "graphicsnodeview.hpp"
#include <QWheelEvent>
#include <QScrollBar>
#include <QResizeEvent>
#include <QGraphicsDropShadowEffect>
#include <QResizeEvent>
#include <QGraphicsItem>

#include <iostream>

#include "graphicsnodesocket.hpp"
#include "graphicsnodedefs.hpp"
#include "graphicsbezieredge.hpp"


GraphicsNodeView::GraphicsNodeView(QWidget *parent)
: GraphicsNodeView(nullptr, parent)
{ }


GraphicsNodeView::GraphicsNodeView(QGraphicsScene *scene, QWidget *parent)
: QGraphicsView(scene, parent)
, _panning(false)
, _tmp_edge(nullptr)
, _sock_source(nullptr)
, _sock_sink(nullptr)
{
	setRenderHints(QPainter::Antialiasing |
			QPainter::TextAntialiasing |
			QPainter::HighQualityAntialiasing |
			QPainter::SmoothPixmapTransform);

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setResizeAnchor(NoAnchor);
	setTransformationAnchor(AnchorUnderMouse);
	// setDragMode(QGraphicsView::RubberBandDrag);
}


void GraphicsNodeView::
resizeEvent(QResizeEvent *event)
{
	// always have the origin in the top left corner
	static bool first_resize = true;
	if (first_resize) {
		// TODO: scale awareness
		centerOn(width()/2 - 50, height()/2 - 50);
		first_resize = false;
	}
	QGraphicsView::resizeEvent(event);
}



void GraphicsNodeView::
wheelEvent(QWheelEvent *event) {
	if (event->modifiers() & Qt::ControlModifier) {
		setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
		double scaleFactor = 1.25;
		if (event->delta() < 0) {
			// zoom in
			scale(scaleFactor, scaleFactor);
		}
		else {
			// zoom out
			scale(1.0 / scaleFactor, 1.0 / scaleFactor);
		}
		event->accept();
	}
	else {
		QGraphicsView::wheelEvent(event);
	}
}





void GraphicsNodeView::
mousePressEvent(QMouseEvent *event)
{
	switch (event->button()) {
	case Qt::MiddleButton:
		middleMouseButtonPress(event);
		break;
	case Qt::LeftButton:
		leftMouseButtonPress(event);
		break;
	default:
		QGraphicsView::mousePressEvent(event);
	}
}


void GraphicsNodeView::
middleMouseButtonRelease(QMouseEvent *event)
{
	QMouseEvent fakeEvent(event->type(), event->localPos(), event->screenPos(), event->windowPos(),
			Qt::LeftButton, event->buttons() & ~Qt::LeftButton, event->modifiers());
	QGraphicsView::mouseReleaseEvent(&fakeEvent);
	// setDragMode(QGraphicsView::RubberBandDrag);
	setDragMode(QGraphicsView::NoDrag);
}


void GraphicsNodeView::
leftMouseButtonRelease(QMouseEvent *event)
{
	if (_drag_event) {
		auto sock = socket_at(event->pos());
		if (!sock || !can_accept_edge(sock)) {
			scene()->removeItem(_drag_event->e);
			_drag_event->e->disconnect();
			delete _drag_event->e;
		} else {
			switch (_drag_event->mode) {
			case EdgeDragEvent::move_to_source:
			case EdgeDragEvent::connect_to_source:
				_drag_event->e->connect_source(sock);
				break;

			case EdgeDragEvent::move_to_sink:
			case EdgeDragEvent::connect_to_sink:
				_drag_event->e->connect_sink(sock);
				break;
			}
		}
		delete _drag_event;
		_drag_event = nullptr;
	}
	QGraphicsView::mouseReleaseEvent(event);
}


void GraphicsNodeView::
mouseReleaseEvent(QMouseEvent *event)
{
	viewport()->setCursor(Qt::ArrowCursor);

	switch (event->button()) {
	case Qt::MiddleButton:
		middleMouseButtonRelease(event);
		break;
	case Qt::LeftButton:
		leftMouseButtonRelease(event);
		break;
	default:
		QGraphicsView::mouseReleaseEvent(event);
	}
}


void GraphicsNodeView::
mouseMoveEvent(QMouseEvent *event)
{
	// if the left mouse button was pressed and we actually have a mode and
	// temporary edge already set
	if ((event->buttons() & Qt::LeftButton) && _drag_event) {

		// set start/stop depending on drag mode
		switch (_drag_event->mode) {
		case EdgeDragEvent::move_to_source:
		case EdgeDragEvent::connect_to_source:
			_drag_event->e->set_start(mapToScene(event->pos()));
			break;

		case EdgeDragEvent::move_to_sink:
		case EdgeDragEvent::connect_to_sink:
			_drag_event->e->set_stop(mapToScene(event->pos()));
			break;
		}

		// update visual feedback
		auto item = socket_at(event->pos());
		if (!item) {
			viewport()->setCursor(Qt::ClosedHandCursor);
		}
		else if (item->type() == GraphicsNodeItemTypes::TypeSocket) {
			GraphicsNodeSocket *sock = static_cast<GraphicsNodeSocket*>(item);
			if (!can_accept_edge(sock))
				viewport()->setCursor(Qt::ForbiddenCursor);
			else {
				viewport()->setCursor(Qt::DragMoveCursor);
			}
		}
	}
	else {
		// no button is pressed, so indicate what the user can do with
		// the item by changing the cursor
		if (event->buttons() == 0) {
			auto item = socket_at(event->pos());
			if (item) {
				QPointF scenePos = mapToScene(event->pos());
				QPointF itemPos = item->mapFromScene(scenePos);
				if (item->isInSocketCircle(itemPos))
					viewport()->setCursor(Qt::OpenHandCursor);
				else
					viewport()->setCursor(Qt::ArrowCursor);
			}
			else
				viewport()->setCursor(Qt::ArrowCursor);
		}
		QGraphicsView::mouseMoveEvent(event);
	}
}




void GraphicsNodeView::
leftMouseButtonPress(QMouseEvent *event)
{
	QGraphicsView::mousePressEvent(event);
	// GUI logic: if we click on a socket, we need to handle
	// the event appropriately
	QGraphicsItem *item = itemAt(event->pos());
	if (!item) {
		QGraphicsView::mousePressEvent(event);
		return;
	}

	if (item->type() == GraphicsNodeItemTypes::TypeSocket) {
		QPointF scenePos = mapToScene(event->pos());
		QPointF itemPos = item->mapFromScene(scenePos);
		GraphicsNodeSocket *sock = static_cast<GraphicsNodeSocket*>(item);
		if (sock->isInSocketCircle(itemPos)) {
			viewport()->setCursor(Qt::ClosedHandCursor);


			// initialize a new drag mode event
			_drag_event = new EdgeDragEvent();
			if ((_tmp_edge = sock->get_edge())) {
				_drag_event->e = _tmp_edge;
				if (sock->socket_type() == GraphicsNodeSocket::SINK) {
					_drag_event->e->disconnect_sink();
					_drag_event->e->set_stop(mapToScene(event->pos()));
					_drag_event->mode = EdgeDragEvent::move_to_sink;
				} else {
					_drag_event->e->disconnect_source();
					_drag_event->e->set_start(mapToScene(event->pos()));
					_drag_event->mode = EdgeDragEvent::move_to_source;
				}
			}
			else {
				_drag_event->e = new GraphicsBezierEdge();
				if (sock->socket_type() == GraphicsNodeSocket::SINK) {
					_drag_event->e->set_start(mapToScene(event->pos()));
					_drag_event->e->connect_sink(sock);
					_drag_event->mode = EdgeDragEvent::connect_to_source;
				}
				else {
					_drag_event->e->connect_source(sock);
					_drag_event->e->set_stop(mapToScene(event->pos()));
					_drag_event->mode = EdgeDragEvent::connect_to_sink;
				}
				scene()->addItem(_drag_event->e);
			}
			event->ignore();
			event->ignore();
		}
		else {
			QGraphicsView::mousePressEvent(event);
		}
	}
	else {
		QGraphicsView::mousePressEvent(event);
	}
}


void GraphicsNodeView::
middleMouseButtonPress(QMouseEvent *event)
{
	QMouseEvent releaseEvent(QEvent::MouseButtonRelease,
			event->localPos(), event->screenPos(), event->windowPos(),
			Qt::LeftButton, 0, event->modifiers());
	QGraphicsView::mouseReleaseEvent(&releaseEvent);
	setDragMode(QGraphicsView::ScrollHandDrag);
	QMouseEvent fakeEvent(event->type(), event->localPos(), event->screenPos(), event->windowPos(),
			Qt::LeftButton,
			event->buttons() | Qt::LeftButton,
			event->modifiers());
	QGraphicsView::mousePressEvent(&fakeEvent);
}


bool GraphicsNodeView::
can_accept_edge(GraphicsNodeSocket *sock)
{
	if (!sock) return false;
	if (!_drag_event) return false;

	if ((sock->is_sink() && _drag_event->mode == EdgeDragEvent::move_to_sink)
	||  (sock->is_sink() && _drag_event->mode == EdgeDragEvent::connect_to_sink)
	||  (sock->is_source() && _drag_event->mode == EdgeDragEvent::connect_to_source)
	||  (sock->is_source() && _drag_event->mode == EdgeDragEvent::move_to_source))
		return true;

	return false;
}


GraphicsNodeSocket* GraphicsNodeView::
socket_at(QPoint pos)
{
	// figure out if we are above another socket. this requires
	// stepping through all items that we can actually see, and
	// taking the topmost one

	QList<QGraphicsItem*> itms = items(pos);
	QGraphicsItem *item = nullptr;
	for (int i = itms.count() - 1; i >= 0; i--) {
		if (itms[i]->type() == GraphicsNodeItemTypes::TypeSocket) {
			item = itms[i];
			break;
		}
	}
	if (item)
		return static_cast<GraphicsNodeSocket*>(item);
	else
		return nullptr;
}
