#ifndef __GRAPHICSNODESOURCE_HPP__A25FFE94_34D1_44DA_9F1F_B2F36C3550E5
#define __GRAPHICSNODESOURCE_HPP__A25FFE94_34D1_44DA_9F1F_B2F36C3550E5

#include <QString>
#include <QRectF>
#include <QGraphicsItem>
#include <QPen>
#include <QBrush>
#include <QVariant>

class QGraphicsSceneMouseEvent;
class GraphicsBezierEdge;


class GraphicsNodeSource : public QGraphicsItem
{
public:
	GraphicsNodeSource(QGraphicsItem *parent);
	GraphicsNodeSource(const QString &text, QGraphicsItem *parent = 0);

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

#endif /* __GRAPHICSNODESOURCE_HPP__A25FFE94_34D1_44DA_9F1F_B2F36C3550E5 */

