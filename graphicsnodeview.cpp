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
, _mode(none)
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
	if ((event->buttons() & Qt::LeftButton)
	&& _mode && _tmp_edge) {
		if (_mode == source_to_sink)
			_tmp_edge->set_stop(mapToScene(event->pos()));
		else
			_tmp_edge->set_start(mapToScene(event->pos()));


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
		if (event->buttons() == 0 && _mode == none) {
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
leftMouseButtonRelease(QMouseEvent *event)
{
	if ((_mode != none) && _tmp_edge) {
		// TODO: notify data model

		auto sock = socket_at(event->pos());
		if (!sock || !can_accept_edge(sock)) {
			scene()->removeItem(_tmp_edge);
			delete _tmp_edge;
		}
		else {
			if (_mode == sink_to_source)
				_sock_source = sock;
			else
				_sock_sink = sock;
			_tmp_edge->connect(_sock_source, _sock_sink);
			_tmp_edge->setZValue(-1);
		}

		// tidy up
		_mode = none;
		_tmp_edge = nullptr;
		_sock_source = nullptr;
		_sock_sink = nullptr;
	}
	QGraphicsView::mouseReleaseEvent(event);
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

			// TODO: move socket type logic somewhere appropriate
			_tmp_edge = new GraphicsBezierEdge();
			if (sock->socket_type() == GraphicsNodeSocket::SINK) {
				_tmp_edge->set_stop(sock->sceneAnchorPos());
				_tmp_edge->set_start(mapToScene(event->pos()));
				_mode = sink_to_source;
				_sock_sink = sock;
			}
			else {
				_tmp_edge->set_start(sock->sceneAnchorPos());
				_tmp_edge->set_stop(mapToScene(event->pos()));
				_mode = source_to_sink;
				_sock_source = sock;
			}
			_tmp_edge->setZValue(2);
			scene()->addItem(_tmp_edge);

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
	if (!sock)
		return false;

	return
		((_mode == source_to_sink) && (sock->socket_type() == GraphicsNodeSocket::SINK))
		||
		((_mode == sink_to_source) && (sock->socket_type() == GraphicsNodeSocket::SOURCE));
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
