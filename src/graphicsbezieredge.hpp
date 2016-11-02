/* See LICENSE file for copyright and license details. */

#ifndef GRAPHICS_DIRECTED_EDGE_H
#define GRAPHICS_DIRECTED_EDGE_H

#include <QtCore/QPoint>
#include "graphicsnodedefs.hpp"

class GraphicsNodeSocket;

class GraphicsDirectedEdgePrivate;

class GraphicsDirectedEdge : public QObject
{
    friend class GraphicsNodeView; //To allow intermediate positions
    friend class GraphicsNodeSocket; //To allow intermediate positions
    friend class GraphicsNodeSocketPrivate; //To allow intermediate positions

    Q_OBJECT
public:
    GraphicsDirectedEdge(GraphicsNodeSocket *source, GraphicsNodeSocket *sink, qreal factor=0.5f);

    virtual ~GraphicsDirectedEdge();

    GraphicsNodeSocket *source() const;
    GraphicsNodeSocket *sink() const;

    QGraphicsItem *graphicsItem() const;

protected Q_SLOTS:
    void onSourceDataChange(); // cant use QVariant argument, since it might be another type

protected:
    // It cannot be constructed by itself or the user
    explicit GraphicsDirectedEdge(qreal factor=0.5f);
    GraphicsDirectedEdge(const QPoint& start, const QPoint& stop, qreal factor=0.5f);

    GraphicsDirectedEdgePrivate* d_ptr;
    Q_DECLARE_PRIVATE(GraphicsDirectedEdge)
};

class GraphicsBezierEdge : public GraphicsDirectedEdge
{
    friend class GraphicsNodeView; //For the constructor
public:
    GraphicsBezierEdge(GraphicsNodeSocket *source, GraphicsNodeSocket *sink, qreal factor=0.5f);

protected:
    explicit GraphicsBezierEdge(qreal factor=0.5f);
};

#endif /* GRAPHICS_DIRECTED_EDGE_H */

