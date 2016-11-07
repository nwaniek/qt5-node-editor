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
    friend class GraphicsNodeView; //To allow intermediate positions
    friend class GraphicsNodeSocket; //To allow intermediate positions
    friend class GraphicsNodeSocketPrivate; //To allow intermediate positions

    Q_OBJECT
public:

    virtual ~GraphicsDirectedEdge();

    GraphicsNodeSocket *source() const;
    GraphicsNodeSocket *sink() const;

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
    friend class GraphicsNodeView; //For the constructor
public:

protected:
    GraphicsBezierEdge(QNodeEditorEdgeModel* m, GraphicsNodeSocket *source, GraphicsNodeSocket *sink, qreal factor=0.5f);
    explicit GraphicsBezierEdge(QNodeEditorEdgeModel* m, qreal factor=0.5f);
};

#endif /* GRAPHICS_DIRECTED_EDGE_H */

