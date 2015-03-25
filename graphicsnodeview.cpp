#include "graphicsnodeview.hpp"
#include <QWheelEvent>
#include <QScrollBar>
#include <QResizeEvent>
#include <QGraphicsDropShadowEffect>
#include <iostream>

GraphicsNodeView::GraphicsNodeView(QWidget *parent)
: GraphicsNodeView(nullptr, parent)
{ }


GraphicsNodeView::GraphicsNodeView(QGraphicsScene *scene, QWidget *parent)
: QGraphicsView(scene, parent)
, _panning(false)
{
	setRenderHints(QPainter::Antialiasing |
			QPainter::TextAntialiasing |
			QPainter::HighQualityAntialiasing |
			QPainter::SmoothPixmapTransform);

	/*
	setRenderHints(QPainter::Antialiasing |
			QPainter::TextAntialiasing |
			QPainter::SmoothPixmapTransform);
	*/


	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	// setResizeAnchor(AnchorViewCenter);
	// setInteractive(true);
	setTransformationAnchor(AnchorUnderMouse);
	// setDragMode(QGraphicsView::RubberBandDrag);


}


void GraphicsNodeView::
wheelEvent(QWheelEvent *event) {
	if (event->modifiers() & Qt::ControlModifier) {
		setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
		double scaleFactor = 1.25;
		if (event->delta() < 0) {
			// zoom in
			scale(scaleFactor, scaleFactor);
		} else {
			// zoom out
			scale(1.0 / scaleFactor, 1.0 / scaleFactor);
		}
		event->accept();
	} else {
		QGraphicsView::wheelEvent(event);
	}
}


void GraphicsNodeView::
mouseMoveEvent(QMouseEvent *event) {
	QGraphicsView::mouseMoveEvent(event);
}


void GraphicsNodeView::
mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MiddleButton) {
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
	else
	{
		QGraphicsView::mousePressEvent(event);
	}
}


void GraphicsNodeView::
mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MiddleButton)
	{
		QMouseEvent fakeEvent(event->type(), event->localPos(), event->screenPos(), event->windowPos(),
				Qt::LeftButton, event->buttons() & ~Qt::LeftButton, event->modifiers());
		QGraphicsView::mouseReleaseEvent(&fakeEvent);
		// setDragMode(QGraphicsView::RubberBandDrag);
		setDragMode(QGraphicsView::NoDrag);
	}
	else
	{
		QGraphicsView::mouseReleaseEvent(event);
	}
}

