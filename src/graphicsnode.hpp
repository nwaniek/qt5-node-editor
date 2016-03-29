/* See LICENSE file for copyright and license details. */

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
#include "graphicsnodedefs.hpp"


class QWidget;
class QPushButton;
class QGraphicsProxyWidget;
class QGraphicsSceneMouseEvent;
class QGraphicsDropShadowEffect;
class QGraphicsTextItem;
class GraphicsDirectedEdge;
class GraphicsNodeSocket;


class GraphicsNode : public QGraphicsItem
{
public:
	GraphicsNode(QGraphicsItem *parent = nullptr);
	virtual ~GraphicsNode();

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter,
			const QStyleOptionGraphicsItem *option,
			QWidget *widget = 0) override;


	const GraphicsNodeSocket* add_sink(const QString &text,QObject *data=0,int id=0);
	const GraphicsNodeSocket* add_source(const QString &text,QObject *data=0,int id=0);

	GraphicsNodeSocket* get_source_socket(const size_t id);
	GraphicsNodeSocket* get_sink_socket(const size_t id);

	// connecting sources and sinks
	void connect_source(int i, GraphicsDirectedEdge *edge);
	void connect_sink(int i, GraphicsDirectedEdge *edge);

	int type() const override {
		return GraphicsNodeItemTypes::TypeNode;
	};

	qreal width() const {
		return _width;
	}

	qreal height() const {
		return _height;
	}

	void setTitle(const QString &title);

	void setSize(const qreal width, const qreal height);
	void setSize(const QSizeF size);
	void setSize(const QPointF size);


	/**
	 * set a regular QWidget as central widget
	 */
	void setCentralWidget(QWidget *widget);

protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
	void updateGeometry();
	void updatePath();
	void updateSizeHints();
	void propagateChanges();

private:
	// TODO: change pairs of sizes to QPointF, QSizeF, or quadrupels to QRectF

	const qreal _hard_min_width = 150.0;
	const qreal _hard_min_height = 120.0;

	qreal _min_width = 150.0;
	qreal _min_height = 120.0;

	const qreal _top_margin = 30.0;
	const qreal _bottom_margin = 15.0;
	const qreal _item_padding = 5.0;
	const qreal _lr_padding = 10.0;

	const qreal _pen_width = 1.0;
	const qreal _socket_size = 6.0;

	bool _changed;

	qreal _width;
	qreal _height;

	QPen _pen_default;
	QPen _pen_selected;
	QPen _pen_sources;
	QPen _pen_sinks;

	QBrush _brush_title;
	QBrush _brush_background;
	QBrush _brush_sources;
	QBrush _brush_sinks;

	QGraphicsDropShadowEffect *_effect;
	QGraphicsTextItem *_title_item;
	QGraphicsProxyWidget *_central_proxy = nullptr;

	QString _title;

	std::vector<GraphicsNodeSocket*> _sources;
	std::vector<GraphicsNodeSocket*> _sinks;
};

#endif /* __GRAPHICSNODE_HPP__0707C377_95A2_4E0B_B98B_7E4813001982 */

