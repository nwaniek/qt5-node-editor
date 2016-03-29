/* See LICENSE file for copyright and license details. */

#ifndef __GRAPHICSBEZIEREDGE_HPP__95A023FB_C28F_48F5_9B9C_04F3DB5B7DB1
#define __GRAPHICSBEZIEREDGE_HPP__95A023FB_C28F_48F5_9B9C_04F3DB5B7DB1

#include <QPen>
#include <QPoint>
#include <QPointF>
#include <QGraphicsPathItem>
#include "graphicsnodedefs.hpp"

class QGraphicsDropShadowEffect;
class QGraphicsSceneMouseEvent;
class GraphicsNode;
class GraphicsNodeSocket;


// TODO: move specific draw stuff out of the graphics-edge
//       this may actually lead to the proper data model for a data layer

class GraphicsDirectedEdge : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
	// GraphicsDirectedEdge(qreal factor=0.5f);
	explicit GraphicsDirectedEdge(qreal factor=0.5f);
	GraphicsDirectedEdge(int x0, int y0, int x1, int y1, qreal factor=0.5f);
	GraphicsDirectedEdge(QPoint start, QPoint stop, qreal factor=0.5f);
	GraphicsDirectedEdge(QPointF start, QPointF stop, qreal factor=0.5f);
	GraphicsDirectedEdge(GraphicsNode *n1, int sourceid, GraphicsNode *n2, int sinkid, qreal factor=0.5f);
	GraphicsDirectedEdge(GraphicsNodeSocket *source, GraphicsNodeSocket *sink, qreal factor=0.5f);

	~GraphicsDirectedEdge();

	void connect(GraphicsNodeSocket *source, GraphicsNodeSocket *sink);
	void connect(GraphicsNode *n1, int sourceid, GraphicsNode *n2, int sinkid);

	void connect_source(GraphicsNodeSocket *source);
	void connect_sink(GraphicsNodeSocket *sink);

	void disconnect();
	void disconnect_sink();
	void disconnect_source();

	// methods to manually set a position
	void set_start(int x0, int y0);
	void set_stop(int x1, int y1);

	void set_start(QPoint p);
	void set_stop(QPoint p);

	void set_start(QPointF p);
	void set_stop(QPointF p);

	// virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

	int type() const override {
		return GraphicsNodeItemTypes::TypeBezierEdge;
	}

public slots:
	void sinkDisconnected(GraphicsNode *node, GraphicsNodeSocket *sink);
	void sourceDisconnected(GraphicsNode *node, GraphicsNodeSocket *source);

protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

protected slots:
	void onSourceDataChange(); // cant use QVariant argument, since it might be another type

protected:
	virtual void update_path() = 0;

	QPen _pen;
	QGraphicsDropShadowEffect *_effect;
	QPoint _start;
	QPoint _stop;
	qreal _factor;

	GraphicsNodeSocket *_source;
	GraphicsNodeSocket *_sink;
};


class GraphicsBezierEdge : public GraphicsDirectedEdge
{
	virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) override;
	int type() const override {
		return GraphicsNodeItemTypes::TypeBezierEdge;
	}

protected:
	virtual void update_path() override;
};


#endif /* __GRAPHICSBEZIEREDGE_HPP__95A023FB_C28F_48F5_9B9C_04F3DB5B7DB1 */

