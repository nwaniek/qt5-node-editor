#ifndef __GRAPHICSNODESOCKET_HPP__99275D3E_35A8_4D63_8E10_995E5DC83C8C
#define __GRAPHICSNODESOCKET_HPP__99275D3E_35A8_4D63_8E10_995E5DC83C8C

#include <QString>
#include <QRectF>
#include <QGraphicsItem>
#include <QPen>
#include <QBrush>

class QGraphicsSceneMouseEvent;
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

protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
	void drawAlignedText(QPainter *painter, int alignment);

private:
	const GraphicsNodeSocketType _socket_type;
	QPen _pen_circle;
	const QPen _pen_text;
	const QBrush _brush_circle;
	const QString _text;
	const int _text_alignment;

	GraphicsBezierEdge *_edge;
	bool _edging = false;

private:// some constants
	const qreal _pen_width = 1.0;
	const qreal _circle_radius = 6.0;
	const qreal _text_offset = 3.0;

};

#endif /* __GRAPHICSNODESOCKET_HPP__99275D3E_35A8_4D63_8E10_995E5DC83C8C */

