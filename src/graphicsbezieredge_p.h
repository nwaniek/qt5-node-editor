#ifndef GRAPHICS_DIRECTED_EDGE_PRIVATE_H
#define GRAPHICS_DIRECTED_EDGE_PRIVATE_H

#include <QGraphicsDropShadowEffect>

class GraphicsEdgeItem;

class GraphicsDirectedEdgePrivate final
{
public:
    explicit GraphicsDirectedEdgePrivate(GraphicsDirectedEdge* q) : q_ptr(q) {}

    //TODO support effect lazy loading
    QGraphicsDropShadowEffect *_effect {new QGraphicsDropShadowEffect()};

    QPen _pen {QColor("#00FF00")};
    QPoint _start;
    QPoint _stop;
    qreal _factor;

    GraphicsEdgeItem *m_pGrpahicsItem {nullptr};

    GraphicsDirectedEdge* q_ptr;

    GraphicsNodeSocket *_source {nullptr};
    GraphicsNodeSocket *_sink {nullptr};

    // Helpers
    void setStart(int x0, int y0);
    void setStop(int x1, int y1);

    void setStart(QPoint p);
    void setStop(QPoint p);

    void setStart(QPointF p);
    void setStop(QPointF p);

    void connect(GraphicsNode *n1, int sourceid, GraphicsNode *n2, int sinkid);
    void connect(GraphicsNodeSocket *source, GraphicsNodeSocket *sink); //TODO make private

    void connectSource(GraphicsNodeSocket *source);
    void connectSink(GraphicsNodeSocket *sink);

    void disconnectBoth();
    void disconnectSink();
    void disconnectSource();
};

#endif
