#ifndef __GRAPHICSNODESINK_HPP__402FE717_2B88_4D22_8065_4829DA6A93BC
#define __GRAPHICSNODESINK_HPP__402FE717_2B88_4D22_8065_4829DA6A93BC

#include <QString>
#include <QRectF>
#include <QGraphicsItem>
#include <QPen>
#include <QBrush>

class QGraphicsSceneMouseEvent;
class GraphicsBezierEdge;

class GraphicsNodeSink : public QGraphicsItem
{
public:
	GraphicsNodeSink(QGraphicsItem *parent = nullptr);
	GraphicsNodeSink(const QString &text, QGraphicsItem *parent = nullptr);

	virtual QRectF boundingRect() const;
	virtual void paint(QPainter *painter,
			const QStyleOptionGraphicsItem *option,
			QWidget *widget = 0);

	void set_edge(GraphicsBezierEdge *edge);
	void notifyPositionChange();

protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
	QPen _pen_circle;
	QBrush _brush_circle;
	const QString _text;

	GraphicsBezierEdge *_edge;

	bool _edging = false;

};

#endif /* __GRAPHICSNODESINK_HPP__402FE717_2B88_4D22_8065_4829DA6A93BC */

