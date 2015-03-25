#ifndef __GRAPHICSNODE_HPP__0707C377_95A2_4E0B_B98B_7E4813001982
#define __GRAPHICSNODE_HPP__0707C377_95A2_4E0B_B98B_7E4813001982

#include <vector>

#include <QPen>
#include <QBrush>
#include <QRectF>
#include <QPointF>
#include <QGraphicsItem>
#include <QGraphicsObject>
#include <QVariant>
#include <QString>

#include "graphicsnodesink.hpp"
#include "graphicsnodesource.hpp"


class QPushButton;
class QGraphicsSceneMouseEvent;
class QGraphicsDropShadowEffect;
class GraphicsBezierEdge;


class GraphicsNode : public QGraphicsItem
{
public:
	GraphicsNode(QGraphicsItem *parent = nullptr);
	~GraphicsNode();

	virtual QRectF boundingRect() const;
	virtual void paint(QPainter *painter,
			const QStyleOptionGraphicsItem *option,
			QWidget *widget = 0);


	// add a sink to this node
	const GraphicsNodeSink* add_sink();
	const GraphicsNodeSink* add_sink(const QString &text);

	const GraphicsNodeSource* add_source(const QString &text);

	// connecting sources and sinks
	void connect_source(int i, GraphicsBezierEdge *edge);
	void connect_sink(int i, GraphicsBezierEdge *edge);


protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
	QPen _pen_default;
	QPen _pen_selected;
	QPen _pen_sources;
	QPen _pen_sinks;

	QBrush _brush_background;
	QBrush _brush_sources;
	QBrush _brush_sinks;

	QGraphicsDropShadowEffect *_effect;

	std::vector<GraphicsNodeSource*> _sources;
	std::vector<GraphicsNodeSink*> _sinks;

};

#endif /* __GRAPHICSNODE_HPP__0707C377_95A2_4E0B_B98B_7E4813001982 */

