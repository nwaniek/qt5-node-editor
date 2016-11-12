#include "qnodeeditorsocketmodel.h"

#include "graphicsnode.hpp"
#include "graphicsnodescene.hpp"
#include "graphicsbezieredge.hpp"
#include "graphicsbezieredge_p.h"

#include "qreactiveproxymodel.h"

#include "qmodeldatalistdecoder.h"

#include <QtCore/QDebug>
#include <QtCore/QMimeData>

#include "qobjectmodel.h" //TODO remove

#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

struct NodeWrapper final
{
    ~NodeWrapper() {
        delete m_pNode;
    }

    GraphicsNode* m_pNode;

    // Keep aligned with the source row
    QVector<GraphicsNodeSocket*> m_lSourcesToSrc; //TODO would a QHash make sense?
    QVector<GraphicsNodeSocket*> m_lSinksToSrc; //TODO would a QHash make sense?

    // Keep aligned with the node row
    QVector<GraphicsNodeSocket*> m_lSources;
    QVector<GraphicsNodeSocket*> m_lSinks;

    QRectF m_SceneRect;
};

struct EdgeWrapper final
{
    explicit EdgeWrapper(QNodeEditorEdgeModel* m, const QModelIndex& index)
        : m_Edge(m, index)
    { Q_ASSERT(index.isValid()); }

    GraphicsNodeSocket* m_pSource {Q_NULLPTR};
    GraphicsBezierEdge  m_Edge               ;
    GraphicsNodeSocket* m_pSink   {Q_NULLPTR};
    bool                m_IsShown {  false  };
};

class QNodeEditorSocketModelPrivate final : public QObject
{
public:
    enum class State {
        NORMAL,
        DRAGGING,
    };

    explicit QNodeEditorSocketModelPrivate(QObject* p) : QObject(p) {}

    QNodeEditorEdgeModel  m_EdgeModel {this};
    QVector<NodeWrapper*> m_lWrappers;
    QVector<EdgeWrapper*> m_lEdges;
    GraphicsNodeScene*    m_pScene;
    State                 m_State {State::NORMAL};
    quint32               m_CurrentTypeId {QMetaType::UnknownType};

    // helper
    GraphicsNode* insertNode(int idx, const QString& title);
    NodeWrapper*  getNode(const QModelIndex& idx, bool r = false) const;

    void insertSockets(const QModelIndex& parent, int first, int last);
    void updateSockets(const QModelIndex& parent, int first, int last);
    void removeSockets(const QModelIndex& parent, int first, int last);

    GraphicsDirectedEdge* initiateConnectionFromSource(
        const QModelIndex&             index,
        GraphicsNodeSocket::SocketType type,
        const QPointF&                 point
    );

    QNodeEditorSocketModel* q_ptr;

public Q_SLOTS:
    void slotRowsInserted       (const QModelIndex& parent, int first, int last);
    void slotConnectionsInserted(const QModelIndex& parent, int first, int last);
    void slotConnectionsChanged (const QModelIndex& tl, const QModelIndex& br  );
    void exitDraggingMode();
};

QNodeEditorSocketModel::QNodeEditorSocketModel( QReactiveProxyModel* rmodel, GraphicsNodeScene* scene ) : 
    QTypeColoriserProxy(rmodel), d_ptr(new QNodeEditorSocketModelPrivate(this))
{
    Q_ASSERT(rmodel);

    setBackgroundRole<QString>(QBrush("#ff0000"));
    setBackgroundRole<int>(QBrush("#ff00ff"));
    setBackgroundRole<QAbstractItemModel*>(QBrush("#ffff00"));

    rmodel->addConnectedRole(QObjectModel::Role::ValueRole);

    d_ptr->q_ptr    = this;
    d_ptr->m_pScene = scene;
    setSourceModel(rmodel);

    d_ptr->m_EdgeModel.setSourceModel(rmodel->connectionsModel());

    connect(this, &QAbstractItemModel::rowsInserted,
        d_ptr, &QNodeEditorSocketModelPrivate::slotRowsInserted);

    connect(&d_ptr->m_EdgeModel, &QAbstractItemModel::rowsInserted,
        d_ptr, &QNodeEditorSocketModelPrivate::slotConnectionsInserted);

    connect(&d_ptr->m_EdgeModel, &QAbstractItemModel::dataChanged,
        d_ptr, &QNodeEditorSocketModelPrivate::slotConnectionsChanged);

    rmodel->setCurrentProxy(this);
}

QNodeEditorSocketModel::~QNodeEditorSocketModel()
{
    while (!d_ptr->m_lEdges.isEmpty())
        delete d_ptr->m_lEdges.takeLast();

    while (!d_ptr->m_lWrappers.isEmpty())
        delete d_ptr->m_lWrappers.takeLast();

    delete d_ptr;
}

void QNodeEditorSocketModel::setSourceModel(QAbstractItemModel *sm)
{
    // This models can only work with a QReactiveProxyModel (no proxies)
    Q_ASSERT(qobject_cast<QReactiveProxyModel*>(sm));

    QTypeColoriserProxy::setSourceModel(sm);

    //TODO clear (this can wait, it wont happen anyway)

    d_ptr->slotRowsInserted({}, 0, sourceModel()->rowCount() -1 );
}

bool QNodeEditorSocketModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (!idx.isValid())
        return false;

    if (role == Qt::SizeHintRole && value.canConvert<QRectF>() && !idx.parent().isValid()) {
        auto n = d_ptr->getNode(idx);
        Q_ASSERT(n);
        n->m_SceneRect = value.toRectF();

        Q_EMIT dataChanged(idx, idx);

        // All socket position also changed
        const int cc = rowCount(idx);

        if (cc)
            Q_EMIT dataChanged(index(0,0,idx), index(cc -1, 0, idx));

        return true;
    }

    return QTypeColoriserProxy::setData(idx, value, role);
}

QMimeData *QNodeEditorSocketModel::mimeData(const QModelIndexList &idxs) const
{
    auto md = QTypeColoriserProxy::mimeData(idxs);

    // Assume the QMimeData exist only while the data is being dragged
    if (md) {
        const QModelDataListDecoder decoder(md);

        auto typeId = decoder.typeId(Qt::EditRole);

        if (typeId != QMetaType::UnknownType) {
            d_ptr->m_State = QNodeEditorSocketModelPrivate::State::DRAGGING;
            d_ptr->m_CurrentTypeId = typeId;

            connect(md, &QObject::destroyed, d_ptr,
                &QNodeEditorSocketModelPrivate::exitDraggingMode);

            for (auto n : qAsConst(d_ptr->m_lWrappers))
                n->m_pNode->update();
        }
    }

    return md;
}

Qt::ItemFlags QNodeEditorSocketModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f = QTypeColoriserProxy::flags(idx);

    // Disable everything but compatible sockets
    return f ^ ((
        d_ptr->m_State == QNodeEditorSocketModelPrivate::State::DRAGGING &&
        (!idx.data(Qt::EditRole).canConvert(d_ptr->m_CurrentTypeId)) &&
        f | Qt::ItemIsEnabled
    ) ? Qt::ItemIsEnabled : Qt::NoItemFlags);;
}

void QNodeEditorSocketModelPrivate::exitDraggingMode()
{
    m_CurrentTypeId = QMetaType::UnknownType;
    m_State = QNodeEditorSocketModelPrivate::State::NORMAL;

    for (auto n : qAsConst(m_lWrappers))
        n->m_pNode->update();
}

GraphicsNodeScene* QNodeEditorSocketModel::scene() const
{
    return d_ptr->m_pScene;
}

GraphicsNodeScene* QNodeEditorEdgeModel::scene() const
{
    return d_ptr->m_pScene;
}

int QNodeEditorSocketModel::sourceSocketCount(const QModelIndex& idx) const
{
    const auto nodew = d_ptr->getNode(idx);

    return nodew ? nodew->m_lSources.size() : 0;
}

int QNodeEditorSocketModel::sinkSocketCount(const QModelIndex& idx) const
{
    const auto nodew = d_ptr->getNode(idx);

    return nodew ? nodew->m_lSinks.size() : 0;
}

GraphicsNode* QNodeEditorSocketModel::getNode(const QModelIndex& idx, bool recursive)
{
    if ((!idx.isValid()) || idx.model() != this)
        return Q_NULLPTR;

    const auto i = (recursive && idx.parent().isValid()) ? idx.parent() : idx;

    auto nodew = d_ptr->getNode(i);

    return nodew ? nodew->m_pNode : Q_NULLPTR;
}

QVector<GraphicsNodeSocket*> QNodeEditorSocketModel::getSourceSockets(const QModelIndex& idx) const
{
    const auto nodew = d_ptr->getNode(idx);

    return nodew ? nodew->m_lSources : QVector<GraphicsNodeSocket*>();
}

QVector<GraphicsNodeSocket*> QNodeEditorSocketModel::getSinkSockets(const QModelIndex& idx) const
{
    const auto nodew = d_ptr->getNode(idx);

    return nodew ? nodew->m_lSinks : QVector<GraphicsNodeSocket*>();
}

GraphicsNodeSocket* QNodeEditorSocketModel::getSourceSocket(const QModelIndex& idx)
{
    if (!idx.parent().isValid())
        return Q_NULLPTR;

    if (idx.model() == edgeModel())
        return d_ptr->m_lEdges[idx.row()]->m_pSource;

    const auto nodew = d_ptr->getNode(idx, true);

    if (!nodew)
        return Q_NULLPTR;

    return nodew->m_lSourcesToSrc.size() > idx.row() ?
        nodew->m_lSourcesToSrc[idx.row()] : Q_NULLPTR;
}

GraphicsNodeSocket* QNodeEditorSocketModel::getSinkSocket(const QModelIndex& idx)
{
    if (!idx.parent().isValid())
        return Q_NULLPTR;

    if (idx.model() == edgeModel())
        return d_ptr->m_lEdges[idx.row()]->m_pSink;

    const auto nodew = d_ptr->getNode(idx, true);

    if (!nodew)
        return Q_NULLPTR;

    return nodew->m_lSinksToSrc.size() > idx.row() ?
        nodew->m_lSinksToSrc[idx.row()] : Q_NULLPTR;
}

GraphicsDirectedEdge* QNodeEditorSocketModel::getSourceEdge(const QModelIndex& idx)
{
    return idx.model() == edgeModel() ?
        &d_ptr->m_lEdges[idx.row()]->m_Edge : Q_NULLPTR;

    //FIXME support SocketModel index
}

GraphicsDirectedEdge* QNodeEditorSocketModel::getSinkEdge(const QModelIndex& idx)
{
    return idx.model() == edgeModel() ?
        &d_ptr->m_lEdges[idx.row()]->m_Edge : Q_NULLPTR;

    //FIXME support SocketModel index
}

QNodeEditorEdgeModel* QNodeEditorSocketModel::edgeModel() const
{
    return &d_ptr->m_EdgeModel;
}

GraphicsDirectedEdge* QNodeEditorSocketModelPrivate::initiateConnectionFromSource( const QModelIndex& idx, GraphicsNodeSocket::SocketType type, const QPointF& point )
{
    Q_UNUSED(type ); //TODO use it or delete it
    Q_UNUSED(point); //TODO use it or delete it

    if (!idx.parent().isValid()) {
        qWarning() << "Cannot initiate an edge from an invalid node index";
        return Q_NULLPTR;
    }

    const int last = q_ptr->edgeModel()->rowCount() - 1;

    if (m_lEdges.size() <= last || !m_lEdges[last]) {
        m_lEdges.resize(std::max(m_lEdges.size(), last+1));
        m_lEdges[last] = new EdgeWrapper(
            q_ptr->edgeModel(),
            q_ptr->edgeModel()->index(last, 1)
        );
        m_pScene->addItem(m_lEdges[last]->m_Edge.graphicsItem());
    }

    return &m_lEdges[last]->m_Edge;
}

GraphicsDirectedEdge* QNodeEditorSocketModel::initiateConnectionFromSource(const QModelIndex& index, const QPointF& point)
{
    return d_ptr->initiateConnectionFromSource(
        index, GraphicsNodeSocket::SocketType::SOURCE, point
    );
}

GraphicsDirectedEdge* QNodeEditorSocketModel::initiateConnectionFromSink(const QModelIndex& index, const QPointF& point)
{
    return d_ptr->initiateConnectionFromSource(
        index, GraphicsNodeSocket::SocketType::SINK, point
    );
}

void QNodeEditorSocketModelPrivate::slotRowsInserted(const QModelIndex& parent, int first, int last)
{
    if (last < first) return;

    if (!parent.isValid()) {
        // create new nodes
        for (int i = first; i <= last; i++) {
            const auto idx = q_ptr->index(i, 0);
            if (idx.isValid()) {
                insertNode(idx.row(), idx.data().toString());
                slotRowsInserted(idx, 0, q_ptr->rowCount(idx));
            }
        }
    }
    else if (!parent.parent().isValid())
        insertSockets(parent, first, last);
}

GraphicsNode* QNodeEditorSocketModelPrivate::insertNode(int idx, const QString& title)
{
    const auto idx2 = q_ptr->index(idx, 0);

    Q_ASSERT(idx2.isValid());

    if (idx == 0 && m_lWrappers.size())
        Q_ASSERT(false);

    auto n = new GraphicsNode(q_ptr, idx2);
    n->setTitle(title);

    auto nw = new NodeWrapper{
        n, {}, {}, {}, {}, {}
    };

    m_lWrappers.insert(idx, nw);

    m_pScene->addItem(n->graphicsItem());

    return n;
}

NodeWrapper* QNodeEditorSocketModelPrivate::getNode(const QModelIndex& idx, bool r) const
{
    // for convenience
    auto i = idx.model() == q_ptr->sourceModel() ? q_ptr->mapFromSource(idx) : idx;

    if ((!i.isValid()) || i.model() != q_ptr)
        return Q_NULLPTR;

    if (i.parent().isValid() && r)
        i = i.parent();

    if (i.parent().isValid())
        return Q_NULLPTR;

    // This should have been taken care of already. If it isn't, either the
    // source model is buggy (it will cause crashes later anyway) or this
    // code is (and in that case, the state is already corrupted, ignoring that
    // will cause garbage data to be shown/serialized).
    Q_ASSERT(m_lWrappers.size() > i.row());

    return m_lWrappers[i.row()];
}

void QNodeEditorSocketModelPrivate::insertSockets(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(first)
    Q_UNUSED(last)

    auto nodew = getNode(parent);
    Q_ASSERT(nodew);

    Q_ASSERT(parent.isValid() && (!parent.parent().isValid()));

    Q_ASSERT(parent.model() == q_ptr);

    for (int i = 0; i < q_ptr->rowCount(parent); i++) { //FIXME use first and last
        const auto idx = q_ptr->index(i, 0, parent);

        // It doesn't attempt to insert the socket at the correct index as
        // many items will be rejected

        constexpr static const Qt::ItemFlags sourceFlags(
            Qt::ItemIsDragEnabled |
            Qt::ItemIsSelectable
        );

        // SOURCES
        if ((idx.flags() & sourceFlags) == sourceFlags) {
            auto s = new GraphicsNodeSocket(
                idx,
                GraphicsNodeSocket::SocketType::SOURCE,
                idx.data().toString(),
                nodew->m_pNode,
                (QObject*) q_ptr->sourceModel(),
                i
            );
            nodew->m_lSourcesToSrc.resize(
                std::max(i+1,nodew->m_lSourcesToSrc.size())
            );
            nodew->m_lSourcesToSrc.insert(i, s);
            nodew->m_lSources << s;
        }

        constexpr static const Qt::ItemFlags sinkFlags(
            Qt::ItemIsDropEnabled |
            Qt::ItemIsSelectable  |
            Qt::ItemIsEditable
        );

        // SINKS
        if ((idx.flags() & sinkFlags) == sinkFlags) {
            auto s = new GraphicsNodeSocket(
                idx,
                GraphicsNodeSocket::SocketType::SINK,
                idx.data().toString(),
                nodew->m_pNode,
                (QObject*) q_ptr->sourceModel(),
                i
            );
            nodew->m_lSinksToSrc.resize(
                std::max(i+1,nodew->m_lSinksToSrc.size())
            );
            nodew->m_lSinksToSrc.insert(i, s);
            nodew->m_lSinks << s;
        }

        nodew->m_pNode->update();
    }
}

void QNodeEditorSocketModelPrivate::updateSockets(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)
    //TODO
}

void QNodeEditorSocketModelPrivate::removeSockets(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)
    //TODO
}

QNodeEditorEdgeModel::QNodeEditorEdgeModel(QNodeEditorSocketModelPrivate* parent)
    : QIdentityProxyModel(parent), d_ptr(parent)
{
    
}

QNodeEditorEdgeModel::~QNodeEditorEdgeModel()
{ /* Nothing is owned by the edge model*/ }

bool QNodeEditorEdgeModel::canConnect(const QModelIndex& idx1, const QModelIndex& idx2) const
{
    Q_UNUSED(idx1)
    Q_UNUSED(idx2)

    return true; //TODO
}

bool QNodeEditorEdgeModel::connectSocket(const QModelIndex& idx1, const QModelIndex& idx2)
{
    if (idx1.model() != d_ptr->q_ptr || idx2.model() != d_ptr->q_ptr)
        return false;

    auto m = qobject_cast<QReactiveProxyModel*>(d_ptr->q_ptr->sourceModel());

    m->connectIndices(
        d_ptr->q_ptr->mapToSource(idx1),
        d_ptr->q_ptr->mapToSource(idx2)
    );

    return true;
}

// bool QNodeEditorEdgeModel::setData(const QModelIndex &index, const QVariant &value, int role)
// {
//     if (!index.isValid())
//         return false;
// 
//     switch (role) {
//         case Qt::SizeHintRole:
//             break;
//     }
// 
//     return QIdentityProxyModel::setData(index, value, role);
// }

QVariant QNodeEditorEdgeModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};

    QModelIndex srcIdx = mapToSource(idx);

    switch(idx.column()) {
        case QReactiveProxyModel::ConnectionsColumns::SOURCE:
            srcIdx = mapToSource(idx).data(
                QReactiveProxyModel::ConnectionsRoles::SOURCE_INDEX
            ).toModelIndex();
            break;
        case QReactiveProxyModel::ConnectionsColumns::DESTINATION:
            srcIdx = mapToSource(idx).data(
                QReactiveProxyModel::ConnectionsRoles::DESTINATION_INDEX
            ).toModelIndex();
            break;
        case QReactiveProxyModel::ConnectionsColumns::CONNECTION:
            // Use the socket background as line color
            if (role != Qt::ForegroundRole)
                break;

            srcIdx = mapToSource(index(idx.row(),2)).data(
                QReactiveProxyModel::ConnectionsRoles::DESTINATION_INDEX
            ).toModelIndex();

            if (!srcIdx.isValid())
                srcIdx = mapToSource(index(idx.row(),0)).data(
                QReactiveProxyModel::ConnectionsRoles::SOURCE_INDEX
            ).toModelIndex();

            return d_ptr->q_ptr->mapFromSource(srcIdx).data(Qt::BackgroundRole);

            break;
    }

    auto sock = (!idx.column()) ?
        d_ptr->q_ptr->getSourceSocket(srcIdx) : d_ptr->q_ptr->getSinkSocket(srcIdx);

    if (sock && role == Qt::SizeHintRole)
        return sock->graphicsItem()->mapToScene(0,0);

    return QIdentityProxyModel::data(idx, role);
}

void QNodeEditorSocketModelPrivate::slotConnectionsInserted(const QModelIndex& parent, int first, int last)
{
    //FIXME dead code
    typedef QReactiveProxyModel::ConnectionsColumns Column;

    auto srcSockI  = q_ptr->index(first, Column::SOURCE).data(
        QReactiveProxyModel::ConnectionsRoles::SOURCE_INDEX
    );
    auto destSockI = q_ptr->index(first, Column::DESTINATION).data(
        QReactiveProxyModel::ConnectionsRoles::DESTINATION_INDEX
    );

    // If this happens, then the model is buggy or there is a race condition.
    if ((!srcSockI.isValid()) && (!destSockI.isValid()))
        return;

    //TODO support rows removed

    // Avoid pointless indentation
    if (last > first)
        slotConnectionsInserted(parent, ++first, last);
}

void QNodeEditorSocketModelPrivate::slotConnectionsChanged(const QModelIndex& tl, const QModelIndex& br)
{
    typedef QReactiveProxyModel::ConnectionsRoles CRole;

    for (int i = tl.row(); i <= br.row(); i++) {
        const auto src  = m_EdgeModel.index(i, 0).data(CRole::SOURCE_INDEX     ).toModelIndex();
        const auto sink = m_EdgeModel.index(i, 2).data(CRole::DESTINATION_INDEX).toModelIndex();

        // Make sure the edge exists
        if (m_lEdges.size()-1 <= i && !m_lEdges[i]) {
            m_lEdges.resize(std::max(m_lEdges.size(), i+1));
            m_lEdges[i] = new EdgeWrapper(&m_EdgeModel, m_EdgeModel.index(i, 1));
        }

        auto e = m_lEdges[i];

        auto oldSrc(e->m_pSource), oldSink(e->m_pSink);

        // Update the node mapping
        if ((e->m_pSource = q_ptr->getSourceSocket(src)))
            e->m_pSource->setEdge(m_EdgeModel.index(i, 0));

        if (oldSrc && oldSrc != e->m_pSource)
            oldSrc->setEdge({});

        if ((e->m_pSink = q_ptr->getSinkSocket(sink)))
            e->m_pSink->setEdge(m_EdgeModel.index(i, 2));

        if (oldSink && oldSink != e->m_pSink)
            oldSink->setEdge({});

        // Update the graphic item
        const bool isUsed = e->m_pSource || e->m_pSink;

        e->m_Edge.update();

        if (e->m_IsShown != isUsed && isUsed)
            m_pScene->addItem(e->m_Edge.graphicsItem());
        else if (e->m_IsShown != isUsed && !isUsed)
            m_pScene->removeItem(e->m_Edge.graphicsItem());

        e->m_IsShown = isUsed;
    }
}

QNodeEditorSocketModel* QNodeEditorEdgeModel::socketModel() const
{
    return d_ptr->q_ptr;
}
