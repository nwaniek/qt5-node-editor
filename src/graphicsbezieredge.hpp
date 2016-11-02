/* See LICENSE file for copyright and license details. */

#ifndef GRAPHICS_DIRECTED_EDGE_H
#define GRAPHICS_DIRECTED_EDGE_H

#include <QPoint>
#include <QPointF>
#include <QGraphicsPathItem>
#include "graphicsnodedefs.hpp"

class QGraphicsDropShadowEffect;
class QGraphicsSceneMouseEvent;
class GraphicsNode;
class GraphicsNodeSocket;

class GraphicsDirectedEdgePrivate;

// TODO: move specific draw stuff out of the graphics-edge
//       this may actually lead to the proper data model for a data layer

class GraphicsDirectedEdge : public QObject, public QGraphicsPathItem
{
    friend class GraphicsNodeView; //To allow intermediate positions
    friend class GraphicsNodeSocket; //To allow intermediate positions

    Q_OBJECT
public:
    GraphicsDirectedEdge(GraphicsNodeSocket *source, GraphicsNodeSocket *sink, qreal factor=0.5f);

    virtual ~GraphicsDirectedEdge();

    GraphicsNodeSocket *source() const;
    GraphicsNodeSocket *sink() const;

    int type() const override;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void updatePath() = 0;

protected Q_SLOTS:
    void onSourceDataChange(); // cant use QVariant argument, since it might be another type

protected:
    // It cannot be constructed by itself or the user
    explicit GraphicsDirectedEdge(qreal factor=0.5f);

    GraphicsDirectedEdge(GraphicsNode *n1, int sourceid, GraphicsNode *n2, int sinkid, qreal factor=0.5f);
    GraphicsDirectedEdge(int x0, int y0, int x1, int y1, qreal factor=0.5f);
    GraphicsDirectedEdge(const QPoint& start, const QPoint& stop, qreal factor=0.5f);
    GraphicsDirectedEdge(const QPointF& start, const QPointF& stop, qreal factor=0.5f);

    GraphicsDirectedEdgePrivate* d_ptr;
    Q_DECLARE_PRIVATE(GraphicsDirectedEdge)
};


class GraphicsBezierEdge : public GraphicsDirectedEdge
{
    virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) override;
    int type() const override;
protected:
    virtual void updatePath() override;
};


#endif /* GRAPHICS_DIRECTED_EDGE_H */

