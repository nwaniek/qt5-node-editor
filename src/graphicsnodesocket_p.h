#ifndef GRAPHICSNODESOCKET_P_H
#define GRAPHICSNODESOCKET_P_H

#include "graphicsnodesocket.hpp"

#define PEN_COLOR_CIRCLE      QColor("#FF000000")
#define PEN_COLOR_TEXT        QColor("#FFFFFFFF")

class GraphicsNodeSocketPrivate
{
public:
    explicit GraphicsNodeSocketPrivate() {}

    GraphicsNodeSocket::SocketType _socket_type;
    QPen _pen_circle {PEN_COLOR_CIRCLE};
    const QPen _pen_text {PEN_COLOR_TEXT};
    QBrush _brush_circle;
    QString _text;
    GraphicsNode *_node;

    GraphicsDirectedEdge *_edge {nullptr};
    QObject* m_data;
    int m_index;

    const qreal _pen_width = 1.0;
    const qreal _circle_radius = 6.0;
    const qreal _text_offset = 3.0;

    const qreal _min_width = 30;
    const qreal _min_height = 12.0;

    // Helper
    void drawAlignedText(QPainter *painter);
};

#endif
