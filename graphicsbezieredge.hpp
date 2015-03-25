#ifndef __GRAPHICSBEZIEREDGE_HPP__95A023FB_C28F_48F5_9B9C_04F3DB5B7DB1
#define __GRAPHICSBEZIEREDGE_HPP__95A023FB_C28F_48F5_9B9C_04F3DB5B7DB1

#include <QPen>
#include <QPoint>
#include <QPointF>
#include <QGraphicsPathItem>

class QGraphicsDropShadowEffect;
class QGraphicsSceneMouseEvent;
class GraphicsNode;
class GraphicsNodeSink;
class GraphicsNodeSource;


class GraphicsBezierEdge : public QGraphicsPathItem
{
public:
	// GraphicsBezierEdge(float factor=0.5f);
	explicit GraphicsBezierEdge(float factor=0.5f);
	GraphicsBezierEdge(int x0, int y0, int x1, int y1, float factor=0.5f);
	GraphicsBezierEdge(QPoint start, QPoint stop, float factor=0.5f);
	GraphicsBezierEdge(QPointF start, QPointF stop, float factor=0.5f);
	GraphicsBezierEdge(GraphicsNode *n1, int sourceid, GraphicsNode *n2, int sinkid, float factor=0.5f);
	GraphicsBezierEdge(GraphicsNodeSource *source, GraphicsNodeSink *sink, float factor=0.5f);

	~GraphicsBezierEdge();

	void connect(GraphicsNodeSource *source, GraphicsNodeSink *sink);
	void connect(GraphicsNode *n1, int sourceid, GraphicsNode *n2, int sinkid);

	void set_start(int x0, int y0);
	void set_stop(int x1, int y1);

	void set_start(QPoint p);
	void set_stop(QPoint p);

	void set_start(QPointF p);
	void set_stop(QPointF p);

	virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);


private:
	QPainterPath update_path() const;
	QPen _pen;
	QGraphicsDropShadowEffect *_effect;
	QPoint _start;
	QPoint _stop;
	float _factor;
};


#endif /* __GRAPHICSBEZIEREDGE_HPP__95A023FB_C28F_48F5_9B9C_04F3DB5B7DB1 */

