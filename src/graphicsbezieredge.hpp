/* See LICENSE file for copyright and license details. */

#ifndef GRAPHICS_DIRECTED_EDGE_H
#define GRAPHICS_DIRECTED_EDGE_H

#include <QtCore/QPoint>
#include "graphicsnodedefs.hpp"

class GraphicsNodeSocket;

class GraphicsDirectedEdgePrivate;

class QNodeEditorEdgeModel;

class GraphicsDirectedEdge : public QObject
{
    //TODO once the point is stored in the index those 3 can go away
    friend class GraphicsNodeView; //To allow intermediate positions
    friend class GraphicsNodeSocketPrivate; //To allow intermediate positions

    friend class QNodeEditorSocketModelPrivate; // to notify changes

    Q_OBJECT
public:

    virtual ~GraphicsDirectedEdge();

    GraphicsNodeSocket *source() const;
    GraphicsNodeSocket *sink() const;

    void cancel();

    QGraphicsItem *graphicsItem() const;

protected:
    // It cannot be constructed by itself or the user
    explicit GraphicsDirectedEdge(QNodeEditorEdgeModel* m, qreal factor=0.5f);
    GraphicsDirectedEdge(QNodeEditorEdgeModel* m, const QPoint& start, const QPoint& stop, qreal factor=0.5f); //TODO remove
    GraphicsDirectedEdge(QNodeEditorEdgeModel* m, GraphicsNodeSocket *source, GraphicsNodeSocket *sink, qreal factor=0.5f);

    GraphicsDirectedEdgePrivate* d_ptr;
    Q_DECLARE_PRIVATE(GraphicsDirectedEdge)
};

class GraphicsBezierEdge : public GraphicsDirectedEdge
{
    friend class QNodeEditorSocketModelPrivate; // to create
public:

protected:
    GraphicsBezierEdge(QNodeEditorEdgeModel* m, GraphicsNodeSocket *source, GraphicsNodeSocket *sink, qreal factor=0.5f);
    explicit GraphicsBezierEdge(QNodeEditorEdgeModel* m, qreal factor=0.5f);
};

#endif /* GRAPHICS_DIRECTED_EDGE_H */

