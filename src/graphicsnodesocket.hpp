/* See LICENSE file for copyright and license details. */

#ifndef GRAPHICS_NODE_SOCKET_H
#define GRAPHICS_NODE_SOCKET_H

#include <QtWidgets/QGraphicsItem>

#include <QtCore/QString>
#include <QtCore/QRectF>
#include <QtCore/QPointF>
#include <QtCore/QSizeF>

#include "graphicsnodedefs.hpp"

class QGraphicsSceneMouseEvent;
class QGraphicsSceneDragDropEvent;
class GraphicsDirectedEdge;

class GraphicsNode;

class GraphicsNodeSocketPrivate;

/**
* visual representation of a socket. the visual representation consists of a
* circle for User Interaction and a label
*/
class GraphicsNodeSocket : public QObject, public QGraphicsItem
{
    Q_OBJECT
    friend class GraphicsDirectedEdge;
    friend class GraphicsDirectedEdgePrivate;
public:
    /*
    * the socket comes in two flavors: either as sink or as source for a
    * data stream
    */
    enum class SocketType
    {
        SINK,
        SOURCE
    };


    //TODO make all constructor private, sockets cannot be created externally
    explicit GraphicsNodeSocket(SocketType socket_type, GraphicsNode *parent = nullptr);
    GraphicsNodeSocket(SocketType socket_type, const QString &text, GraphicsNode *parent = nullptr,QObject *data=0,int index=0);

    virtual QRectF boundingRect() const override;

    /*
    */
    virtual void paint(QPainter *painter,
            const QStyleOptionGraphicsItem *option,
            QWidget *widget = 0) override;

    /**
    * set the edge for this socket
    */
    void setEdge(GraphicsDirectedEdge *edge);

    GraphicsDirectedEdge *edge() const;

    QString text() const;
    void setText(const QString& text);

    /**
    * notify the socket that its position has changed. this may be either
    * due to movement within the parent, or due to movement of the parent
    * within the parent's parent context.
    */
    void notifyPositionChange();

    /**
    * get the socket-type of this socket
    */
    SocketType socketType() const;

    bool isSink() const; //TODO get rid of those socketType is enough
    bool isSource() const;

    QSizeF size() const;
    QSizeF minimalSize() const;

    /**
    * type of the class. usefull within a QGraphicsScene to distinguish
    * what is really behind a pointer
    */
    virtual int type() const override;

    /**
    * determine if a point is actually within the socket circle.
    */
    bool isInSocketCircle(const QPointF &p) const;

    // return the anchor position relative to the scene in which the socket
    // is living in
    QPointF sceneAnchorPos() const;

    GraphicsNode *source() const;
    GraphicsNode *sink() const;

protected:
    // event handling
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

Q_SIGNALS:
    void connectedTo(GraphicsNodeSocket* other);

private:
    GraphicsNodeSocketPrivate* d_ptr;
    Q_DECLARE_PRIVATE(GraphicsNodeSocket)
};

#endif /* __GRAPHICSNODESOCKET_HPP__99275D3E_35A8_4D63_8E10_995E5DC83C8C */

