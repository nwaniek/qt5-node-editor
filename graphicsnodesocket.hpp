/* See LICENSE file for copyright and license details. */

#ifndef __GRAPHICSNODESOCKET_HPP__99275D3E_35A8_4D63_8E10_995E5DC83C8C
#define __GRAPHICSNODESOCKET_HPP__99275D3E_35A8_4D63_8E10_995E5DC83C8C

#include <QString>
#include <QRectF>
#include <QGraphicsItem>
#include <QPen>
#include <QBrush>
#include <QPointF>
#include <QSizeF>
#include "graphicsnodedefs.hpp"

class QGraphicsSceneMouseEvent;
class QGraphicsSceneDragDropEvent;
class GraphicsDirectedEdge;


/**
 * visual representation of a socket. the visual representation consists of a
 * circle for User Interaction and a label
 */
class GraphicsNodeSocket : public QGraphicsItem
{
	friend class GraphicsDirectedEdge;
public:
	/*
	 * the socket comes in two flavors: either as sink or as source for a
	 * data stream
	 */
	enum GraphicsNodeSocketType
	{
		SINK,
		SOURCE
	};


	GraphicsNodeSocket(GraphicsNodeSocketType socket_type, QGraphicsItem *parent = nullptr);
	GraphicsNodeSocket(GraphicsNodeSocketType socket_type, const QString &text, QGraphicsItem *parent = nullptr,QObject *data=0,int index=0);

	virtual QRectF boundingRect() const override;

	/*
	 */
	virtual void paint(QPainter *painter,
			const QStyleOptionGraphicsItem *option,
			QWidget *widget = 0) override;

	/**
	 * set the edge for this socket
	 */
	void set_edge(GraphicsDirectedEdge *edge);

	GraphicsDirectedEdge *get_edge();

	/**
	 * notify the socket that its position has changed. this may be either
	 * due to movement within the parent, or due to movement of the parent
	 * within the parent's parent context.
	 */
	void notifyPositionChange();

	/**
	 * get the socket-type of this socket
	 */
	GraphicsNodeSocketType socket_type() const;

	bool is_sink() const;
	bool is_source() const;

	QSizeF getSize() const;
	QSizeF getMinimalSize() const;

	/**
	 * type of the class. usefull within a QGraphicsScene to distinguish
	 * what is really behind a pointer
	 */
	int type() const override {
		return GraphicsNodeItemTypes::TypeSocket;
	};

	/**
	 * determine if a point is actually within the socket circle.
	 */
	bool isInSocketCircle(const QPointF &p) const;

	// return the anchor position relative to the scene in which the socket
	// is living in
	QPointF sceneAnchorPos() const;



protected:
	// event handling
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
	void drawAlignedText(QPainter *painter);


private:
	const GraphicsNodeSocketType _socket_type;
	QPen _pen_circle;
	const QPen _pen_text;
	const QBrush _brush_circle;
	const QString _text;

	/*
	 * edge with which this socket is connected
	 */
	GraphicsDirectedEdge *_edge;
	QObject* m_data;
	int m_index;


private:// some constants. TODO: need to be defined somewhere else (customizable?)
	const qreal _pen_width = 1.0;
	const qreal _circle_radius = 6.0;
	const qreal _text_offset = 3.0;

	const qreal _min_width = 30;
	const qreal _min_height = 12.0;
};

#endif /* __GRAPHICSNODESOCKET_HPP__99275D3E_35A8_4D63_8E10_995E5DC83C8C */

