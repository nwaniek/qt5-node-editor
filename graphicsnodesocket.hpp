#ifndef __GRAPHICSNODESOCKET_HPP__99275D3E_35A8_4D63_8E10_995E5DC83C8C
#define __GRAPHICSNODESOCKET_HPP__99275D3E_35A8_4D63_8E10_995E5DC83C8C

#include <QString>
#include <QRectF>
#include <QGraphicsItem>
#include <QPen>
#include <QBrush>
#include <QPointF>
#include "graphicsnodedefs.hpp"

class QGraphicsSceneMouseEvent;
class QGraphicsSceneDragDropEvent;
class GraphicsBezierEdge;


class GraphicsNodeSocket : public QGraphicsItem
{
public:
	enum GraphicsNodeSocketType
	{
		SINK,
		SOURCE
	};


	GraphicsNodeSocket(GraphicsNodeSocketType socket_type, QGraphicsItem *parent = nullptr);
	GraphicsNodeSocket(GraphicsNodeSocketType socket_type, const QString &text, QGraphicsItem *parent = nullptr);

	virtual QRectF boundingRect() const;
	virtual void paint(QPainter *painter,
			const QStyleOptionGraphicsItem *option,
			QWidget *widget = 0);

	void set_edge(GraphicsBezierEdge *edge);
	void notifyPositionChange();

	GraphicsNodeSocketType socket_type() const;

	int type() const {
		return GraphicsNodeItemTypes::TypeSocket;
	};

	bool isInSocketCircle(const QPointF &p) const;

	// return the anchor position relative to the scene in which the socket
	// is living in
	QPointF sceneAnchorPos() const;

protected:
	// event handling
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
	void drawAlignedText(QPainter *painter);


private:
	const GraphicsNodeSocketType _socket_type;
	QPen _pen_circle;
	const QPen _pen_text;
	const QBrush _brush_circle;
	const QString _text;

	GraphicsBezierEdge *_edge;


private:// some constants. TODO: need to be defined somewhere else (customizable?)
	const qreal _pen_width = 1.0;
	const qreal _circle_radius = 6.0;
	const qreal _text_offset = 3.0;

	const qreal _min_width = 30;
	const qreal _min_height = 12.0;
};

#endif /* __GRAPHICSNODESOCKET_HPP__99275D3E_35A8_4D63_8E10_995E5DC83C8C */

