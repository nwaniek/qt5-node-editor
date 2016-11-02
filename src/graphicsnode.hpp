/* See LICENSE file for copyright and license details. */

#ifndef GRAPHICS_NODE_H
#define GRAPHICS_NODE_H

#include <QtCore/QRectF>
#include <QtCore/QPointF>
#include <QtCore/QVariant>
#include <QtCore/QString>
#include <QtWidgets/QGraphicsItem>

#include "graphicsnodedefs.hpp"

class QWidget;
class QGraphicsSceneMouseEvent;
class QGraphicsTextItem;
class GraphicsDirectedEdge;
class GraphicsNodeSocket;

class GraphicsNodePrivate;

class GraphicsNode : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    explicit GraphicsNode(QGraphicsItem *parent = nullptr);
    virtual ~GraphicsNode();

    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter,
            const QStyleOptionGraphicsItem *option,
            QWidget *widget = 0) override;

    GraphicsNodeSocket* addSink(const QString &text,QObject *data=0,int id=0);
    GraphicsNodeSocket* addSource(const QString &text,QObject *data=0,int id=0);

    void clearSink();
    void clearSource();

    GraphicsNodeSocket* getSourceSocket(const size_t id) const;
    GraphicsNodeSocket* getSinkSocket(const size_t id) const;

    // connecting sources and sinks
    void connectSource(int i, GraphicsDirectedEdge *edge);
    void connectSink(int i, GraphicsDirectedEdge *edge);

    virtual int type() const override;

    QSizeF size() const;

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
    GraphicsNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(GraphicsNode)
};

Q_DECLARE_METATYPE(GraphicsNode*)


#endif //GRAPHICS_NODE_H

