/* See LICENSE file for copyright and license details. */

#include "graphicsnode.hpp"
#include <QPushButton>
#include <QPen>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QLabel>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QTextDocument>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsProxyWidget>

#include <algorithm>
#include <iostream>

#include "graphicsbezieredge.hpp"
#include "graphicsnodesocket.hpp"

#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

class GraphicsNodePrivate final
{
public:
    explicit GraphicsNodePrivate(GraphicsNode* q) : q_ptr(q) {}

    // TODO: change pairs of sizes to QPointF, QSizeF, or quadrupels to QRectF

    constexpr static const qreal _hard_min_width  = 150.0;
    constexpr static const qreal _hard_min_height = 120.0;

    QSizeF m_MinSize {150.0, 120.0};

    constexpr static const qreal _top_margin    = 30.0;
    constexpr static const qreal _bottom_margin = 15.0;
    constexpr static const qreal _item_padding  = 5.0;
    constexpr static const qreal _lr_padding    = 10.0;

    constexpr static const qreal _pen_width = 1.0;
    constexpr static const qreal _socket_size = 6.0;

    bool _changed {false};

    QSizeF m_Size {150, 120};

    QPen _pen_default  {QColor("#7F000000")};
    QPen _pen_selected {QColor("#FFFF36A7")};
    QPen _pen_sources  {QColor("#FF000000")};
    QPen _pen_sinks    {QColor("#FF000000")};

    QBrush _brush_title      {QColor("#E3212121")};
    QBrush _brush_background {QColor("#E31a1a1a")};
    QBrush m_brushSources    {QColor("#FFFF7700")};
    QBrush m_brushSinks      {QColor("#FF0077FF")};

    //TODO lazy load, add option to disable, its nice, but sllooowwww
    QGraphicsDropShadowEffect *_effect        {new QGraphicsDropShadowEffect()};
    QGraphicsTextItem         *_title_item    {nullptr};
    QGraphicsProxyWidget      *_central_proxy {nullptr};

    QString _title;

    QVector<GraphicsNodeSocket*> _sources;
    QVector<GraphicsNodeSocket*> _sinks;

    // Helpers
    void updateGeometry();
    void updateSizeHints();
    void propagateChanges();

    GraphicsNode* q_ptr;
};

// Necessary for some compilers...
constexpr const qreal GraphicsNodePrivate::_hard_min_width;
constexpr const qreal GraphicsNodePrivate::_hard_min_height;


GraphicsNode::GraphicsNode(QGraphicsItem *parent)
: QObject(nullptr), QGraphicsItem(parent), d_ptr(new GraphicsNodePrivate(this))
{
    d_ptr->_title_item = new QGraphicsTextItem(this);

    for (auto p : {
      &d_ptr->_pen_default, &d_ptr->_pen_selected,
      &d_ptr->_pen_default, &d_ptr->_pen_selected
    })
        p->setWidth(0);

    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);

    d_ptr->_title_item->setDefaultTextColor(Qt::white);
    d_ptr->_title_item->setPos(0, 0);
    d_ptr->_title_item->setTextWidth(d_ptr->m_Size.width() - 2*d_ptr->_lr_padding);

    d_ptr->_effect->setBlurRadius(13.0);
    d_ptr->_effect->setColor(QColor("#99121212"));
    setGraphicsEffect(d_ptr->_effect);

}


void GraphicsNode::
setTitle(const QString &title)
{
    d_ptr->_title = title;
    d_ptr->_title_item->setPlainText(title);
}

int GraphicsNode::
type() const
{
    return GraphicsNodeItemTypes::TypeNode;
}

QSizeF GraphicsNode::
size() const
{
    return d_ptr->m_Size;
}

GraphicsNode::
~GraphicsNode()
{
    if (d_ptr->_central_proxy) delete d_ptr->_central_proxy;
    delete d_ptr->_title_item;
    delete d_ptr->_effect;
    delete d_ptr;
}


QRectF GraphicsNode::
boundingRect() const
{
    return QRectF(
        -d_ptr->_pen_width/2.0 - d_ptr->_socket_size,
        -d_ptr->_pen_width/2.0,
        d_ptr->m_Size.width()  + d_ptr->_pen_width/2.0 + 2.0 * d_ptr->_socket_size,
        d_ptr->m_Size.height() + d_ptr->_pen_width/2.0
    ).normalized();
}


void GraphicsNode::
paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const qreal edge_size = 10.0;
    const qreal title_height = 20.0;

    // path for the caption of this node
    QPainterPath path_title;
    path_title.setFillRule(Qt::WindingFill);
    path_title.addRoundedRect(QRect(0, 0, d_ptr->m_Size.width(), title_height), edge_size, edge_size);
    path_title.addRect(0, title_height - edge_size, edge_size, edge_size);
    path_title.addRect(d_ptr->m_Size.width() - edge_size, title_height - edge_size, edge_size, edge_size);
    painter->setPen(Qt::NoPen);
    painter->setBrush(d_ptr->_brush_title);
    painter->drawPath(path_title.simplified());

    // path for the content of this node
    QPainterPath path_content;
    path_content.setFillRule(Qt::WindingFill);
    path_content.addRoundedRect(QRect(0, title_height, d_ptr->m_Size.width(), d_ptr->m_Size.height() - title_height), edge_size, edge_size);
    path_content.addRect(0, title_height, edge_size, edge_size);
    path_content.addRect(d_ptr->m_Size.width() - edge_size, title_height, edge_size, edge_size);
    painter->setPen(Qt::NoPen);
    painter->setBrush(d_ptr->_brush_background);
    painter->drawPath(path_content.simplified());

    // path for the outline
    QPainterPath path_outline = QPainterPath();
    path_outline.addRoundedRect(QRect(0, 0, d_ptr->m_Size.width(), d_ptr->m_Size.height()), edge_size, edge_size);
    painter->setPen(isSelected() ? d_ptr->_pen_selected : d_ptr->_pen_default);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path_outline.simplified());

    // debug bounding box
#if 0
    QPen debugPen = QPen(QColor(Qt::red));
    debugPen.setWidth(0);
    auto r = boundingRect();
    painter->setPen(debugPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(r);

    painter->drawPoint(0,0);
#endif
}


void GraphicsNode::
mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // TODO: ordering after selection/deselection cycle
    QGraphicsItem::mousePressEvent(event);

    setZValue(isSelected() ? 1 : 0);
}


void GraphicsNode::
setSize(const qreal width, const qreal height)
{
    setSize(QPointF(width, height));
}


void GraphicsNode::
setSize(const QPointF size)
{
    setSize(QSizeF(size.x(), size.y()));
}


void GraphicsNode::
setSize(const QSizeF size)
{
    d_ptr->m_Size = size;
    d_ptr->_changed = true;
    prepareGeometryChange();
    d_ptr->updateGeometry();
}


QVariant GraphicsNode::
itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case QGraphicsItem::ItemSelectedChange: {
        if (value == true)
            setZValue(1);
        else
            setZValue(0);
        break;
    }
    case QGraphicsItem::ItemPositionChange:
    case QGraphicsItem::ItemPositionHasChanged:
        d_ptr->propagateChanges();
        break;

    default:
        break;
    }

    return QGraphicsItem::itemChange(change, value);
}


GraphicsNodeSocket* GraphicsNode::
addSink(const QString &text,QObject *data,int id)
{
    auto s = new GraphicsNodeSocket(GraphicsNodeSocket::SINK, text, this,data,id);
    d_ptr->_sinks.push_back(s);
    d_ptr->_changed = true;
    prepareGeometryChange();
    d_ptr->updateGeometry();

    return s;
}


GraphicsNodeSocket* GraphicsNode::
addSource(const QString &text,QObject *data,int id)
{
    auto s = new GraphicsNodeSocket(GraphicsNodeSocket::SOURCE, text, this,data,id);
    d_ptr->_sources.push_back(s);
    d_ptr->_changed = true;
    prepareGeometryChange();
    d_ptr->updateGeometry();

    return s;
}


void GraphicsNode::
clearSink()
{
    d_ptr->_sinks.clear();
    d_ptr->_changed = true;
    prepareGeometryChange();
    d_ptr->updateGeometry();
}


void GraphicsNode::
clearSource()
{
    d_ptr->_sources.clear();
    d_ptr->_changed = true;
    prepareGeometryChange();
    d_ptr->updateGeometry();
}

void GraphicsNode::
connectSource(int i, GraphicsDirectedEdge *edge)
{
    const auto old_edge = d_ptr->_sources[i]->get_edge();
    d_ptr->_sources[i]->set_edge(edge);

    if (old_edge)
        old_edge->sourceDisconnected(this, d_ptr->_sources[i]);
}


void GraphicsNode::
connectSink(int i, GraphicsDirectedEdge *edge)
{
    const auto old_edge = d_ptr->_sinks[i]->get_edge();
    d_ptr->_sinks[i]->set_edge(edge);

    if (old_edge)
        old_edge->sinkDisconnected(this, d_ptr->_sinks[i]);
}


void GraphicsNodePrivate::
updateGeometry()
{
    if (!_changed) return;

    // compute if we have reached the minimum size
    updateSizeHints();

    m_Size = {
        std::max(m_MinSize.width() , m_Size.width()  ),
        std::max(m_MinSize.height(), m_Size.height() )
    };

    // title
    _title_item->setTextWidth(m_Size.width());

    qreal ypos1 = _top_margin;

    for (auto s : qAsConst(_sinks)) {
        auto size = s->getSize();

        // sockets are centered around 0/0
        s->setPos(0, ypos1 + size.height()/2.0);
        ypos1 += size.height() + _item_padding;
    }

    // sources are placed bottom/right
    qreal ypos2 = m_Size.height() - _bottom_margin;

    for (int i = _sources.size(); i > 0; i--) {
        auto s = _sources[i-1];
        auto size = s->getSize();

        ypos2 -= size.height();
        s->setPos(m_Size.width(), ypos2 + size.height()/2.0);
        ypos2 -= _item_padding;
    }


    // central widget
    if (_central_proxy) {
        QRectF geom {
            _lr_padding,
            ypos1,
            m_Size.width() - 2.0 * _lr_padding,
            ypos2 - ypos1
        };

        _central_proxy->setGeometry(geom);
    }

    _changed = false;
    propagateChanges();
}


GraphicsNodeSocket* GraphicsNode::
getSourceSocket(const size_t id) const
{
    return (id < d_ptr->_sources.size()) ?
        d_ptr->_sources[id] :
        nullptr;
}

GraphicsNodeSocket* GraphicsNode::
getSinkSocket(const size_t id) const
{
    return (id < d_ptr->_sinks.size()) ?
        d_ptr->_sinks[id] :
        nullptr;
}


void GraphicsNode::
setCentralWidget (QWidget *widget)
{
    if (d_ptr->_central_proxy)
        delete d_ptr->_central_proxy;

    d_ptr->_central_proxy = new QGraphicsProxyWidget(this);
    d_ptr->_central_proxy->setWidget(widget);
    d_ptr->_changed = true;
    prepareGeometryChange();
    d_ptr->updateGeometry();
}


void GraphicsNodePrivate::
updateSizeHints() {
    qreal min_width(0.0), min_height(_top_margin + _bottom_margin);

    // sinks
    for (auto s : qAsConst(_sinks)) {
        auto size = s->getMinimalSize();

        min_height += size.height() + _item_padding;
        min_width   = std::max(size.width(), min_width);
    }

    // central widget
    if (_central_proxy) {
        if (const auto wgt = _central_proxy->widget()) {
            // only take the size hint if the value is valid, and if
            // the minimumSize is not set (similar to
            // QWidget/QLayout standard behavior
            const auto sh = wgt->minimumSizeHint();
            const auto sz = wgt->minimumSize();

            if (sh.isValid()) {
                min_height += (sz.height() > 0) ? sz.height() : sh.height();

                min_width = std::max(
                    qreal((sz.width() > 0) ? sz.width() : sh.width())
                        + 2.0*_lr_padding,
                    min_width
                );

            } else {
                min_height += sh.height();
                min_width   = std::max(
                    qreal(sh.width()) + 2.0*_lr_padding,
                    min_width
                );
            }
        }
    }

    // sources
    for (auto s : qAsConst(_sources)) {
        const auto size = s->getMinimalSize();

        min_height += size.height() + _item_padding;
        min_width   = std::max(size.width(), min_width);
    }

    m_MinSize = {
        std::max(min_width, _hard_min_width  ),
        std::max(min_height, _hard_min_height)
    };
}


void GraphicsNodePrivate::
propagateChanges()
{
    for (auto s : qAsConst(_sinks))
        s->notifyPositionChange();

    for (auto s : qAsConst(_sources))
        s->notifyPositionChange();
}
