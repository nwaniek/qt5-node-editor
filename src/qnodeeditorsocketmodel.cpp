#include "qnodeeditorsocketmodel.h"

#include "graphicsnode.hpp"
#include "graphicsnodescene.hpp"
#include "graphicsbezieredge.hpp"
#include "graphicsbezieredge_p.h"

#include "qreactiveproxymodel.h"

#include <QtCore/QDebug>

#include "qobjectmodel.h" //TODO remove

#define REACTIVE_MODEL qobject_cast<QReactiveProxyModel*>(d_ptr->q_ptr->sourceModel())

struct NodeWrapper
{
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

    GraphicsNodeSocket* m_pSource {nullptr};
    GraphicsBezierEdge  m_Edge;
    GraphicsNodeSocket* m_pSink {nullptr};
};

class QNodeEditorSocketModelPrivate final : public QObject
{
public:
    explicit QNodeEditorSocketModelPrivate(QObject* p) : QObject(p) {}

    QNodeEditorEdgeModel  m_EdgeModel {this};
    QVector<NodeWrapper*> m_lWrappers;
    QVector<EdgeWrapper*> m_lEdges;
    GraphicsNodeScene*    m_pScene;

    // helper
    GraphicsNode* insertNode(int idx, const QString& title);
    NodeWrapper* getNode(const QModelIndex& idx, bool r = false) const;

    void insertSockets(const QModelIndex& parent, int first, int last);
    void updateSockets(const QModelIndex& parent, int first, int last);
    void removeSockets(const QModelIndex& parent, int first, int last);

    GraphicsDirectedEdge* initiateConnectionFromSource(
        const QModelIndex& index,
        GraphicsNodeSocket::SocketType type,
        const QPointF& point
    );

    QNodeEditorSocketModel* q_ptr;
public Q_SLOTS:
    void slotRowsInserted(const QModelIndex& parent, int first, int last);
    void slotConnectionsInserted(const QModelIndex& parent, int first, int last);
    void slotConnectionsChanged(const QModelIndex& tl, const QModelIndex& br);
};

QNodeEditorSocketModel::QNodeEditorSocketModel( QReactiveProxyModel* rmodel, GraphicsNodeScene* scene ) : 
    QIdentityProxyModel(rmodel), d_ptr(new QNodeEditorSocketModelPrivate(this))
{
    Q_ASSERT(rmodel);

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
    
}

void QNodeEditorSocketModel::setSourceModel(QAbstractItemModel *sm)
{
    // This models can only work with a QReactiveProxyModel (no proxies)
    Q_ASSERT(qobject_cast<QReactiveProxyModel*>(sm));

    QIdentityProxyModel::setSourceModel(sm);

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

    return QIdentityProxyModel::setData(idx, value, role);
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
    auto nodew = d_ptr->getNode(idx);

    return nodew ? nodew->m_lSources.size() : 0;
}

int QNodeEditorSocketModel::sinkSocketCount(const QModelIndex& idx) const
{
    auto nodew = d_ptr->getNode(idx);

    return nodew ? nodew->m_lSinks.size() : 0;
}

GraphicsNode* QNodeEditorSocketModel::getNode(const QModelIndex& idx, bool recursive)
{
    if ((!idx.isValid()) || idx.model() != this)
        return Q_NULLPTR;

    auto i = (recursive && idx.parent().isValid()) ? idx.parent() : idx;

    auto nodew = d_ptr->getNode(i);

    return nodew ? nodew->m_pNode : Q_NULLPTR;
}

QVector<GraphicsNodeSocket*> QNodeEditorSocketModel::getSourceSockets(const QModelIndex& idx) const
{
    auto nodew = d_ptr->getNode(idx);

    return nodew ? nodew->m_lSources : QVector<GraphicsNodeSocket*>();
}

QVector<GraphicsNodeSocket*> QNodeEditorSocketModel::getSinkSockets(const QModelIndex& idx) const
{
    auto nodew = d_ptr->getNode(idx);

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
    return idx.model() == edgeModel() ? &d_ptr->m_lEdges[idx.row()]->m_Edge : nullptr;

    //FIXME support SocketModel index
}

GraphicsDirectedEdge* QNodeEditorSocketModel::getSinkEdge(const QModelIndex& idx)
{
    return idx.model() == edgeModel() ? &d_ptr->m_lEdges[idx.row()]->m_Edge : nullptr;

    //FIXME support SocketModel index
}

QNodeEditorEdgeModel* QNodeEditorSocketModel::edgeModel() const
{
    return &d_ptr->m_EdgeModel;
}

GraphicsDirectedEdge* QNodeEditorSocketModelPrivate::initiateConnectionFromSource( const QModelIndex& idx, GraphicsNodeSocket::SocketType type, const QPointF& point )
{
    if (!idx.parent().isValid()) {
        qWarning() << "Cannot initiate an edge from an invalid node index";
        return nullptr;
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
    return d_ptr->initiateConnectionFromSource(index, GraphicsNodeSocket::SocketType::SOURCE, point);
}

GraphicsDirectedEdge* QNodeEditorSocketModel::initiateConnectionFromSink(const QModelIndex& index, const QPointF& point)
{
    return d_ptr->initiateConnectionFromSource(index, GraphicsNodeSocket::SocketType::SINK, point);
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
    auto idx2 = q_ptr->index(idx, 0);

    Q_ASSERT(idx2.isValid());

    if (idx == 0 && m_lWrappers.size())
        Q_ASSERT(false);

    auto n = new GraphicsNode(q_ptr, idx2);
    n->setTitle(title);

    auto nw = new NodeWrapper{
        n, {}, {}, {}, {}
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
    qDebug() << "IN QNodeEditorSocketModelPrivate::insertSockets" << first << last;

    auto nodew = getNode(parent);
    Q_ASSERT(nodew);

    Q_ASSERT(parent.isValid() && (!parent.parent().isValid()) && parent.model() == q_ptr);

    for (int i = 0; i < q_ptr->rowCount(parent); i++) {
        const auto idx = q_ptr->index(i, 0, parent);

        // It doesn't attempt to insert the socket at the correct index as
        // many items will be rejected

        // SOURCES
        if (idx.flags() & Qt::ItemIsDragEnabled) {
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

        // SINKS
        if (idx.flags() & (Qt::ItemIsDropEnabled | Qt::ItemIsEditable)) {
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
{
    
}

bool QNodeEditorEdgeModel::canConnect(const QModelIndex& idx1, const QModelIndex& idx2) const
{
    return true; //TODO
}

bool QNodeEditorEdgeModel::connectSocket(const QModelIndex& idx1, const QModelIndex& idx2)
{
    if (idx1.model() != d_ptr->q_ptr || idx2.model() != d_ptr->q_ptr)
        return false;

    auto m = REACTIVE_MODEL;

    m->connectIndices(
        d_ptr->q_ptr->mapToSource(idx1),
        d_ptr->q_ptr->mapToSource(idx2)
    );
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
            srcIdx = mapToSource(idx).data(QReactiveProxyModel::ConnectionsRoles::SOURCE_INDEX).toModelIndex();
            break;
        case QReactiveProxyModel::ConnectionsColumns::DESTINATION:
            srcIdx = mapToSource(idx).data(QReactiveProxyModel::ConnectionsRoles::DESTINATION_INDEX).toModelIndex();
            break;
    }


    auto sock = (!idx.column()) ?
        d_ptr->q_ptr->getSourceSocket(srcIdx) : d_ptr->q_ptr->getSinkSocket(srcIdx);

    if (sock && role == Qt::SizeHintRole) {
        return sock->graphicsItem()->mapToScene(0,0);
    }

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
        auto src  = m_EdgeModel.index(i, 0).data(CRole::SOURCE_INDEX     ).toModelIndex();
        auto sink = m_EdgeModel.index(i, 2).data(CRole::DESTINATION_INDEX).toModelIndex();

        // Empty connections don't need edges yet
        if ((!src.isValid()) && (!sink.isValid()))
            continue;

        // Make sure the edge exists
        if (m_lEdges.size()-1 <= i && !m_lEdges[i]) {
            m_lEdges.resize(std::max(m_lEdges.size(), i+1));
            m_lEdges[i] = new EdgeWrapper(&m_EdgeModel, m_EdgeModel.index(i, 1));
            m_pScene->addItem(m_lEdges[i]->m_Edge.graphicsItem());
        }

        // Update the node mapping
        if (m_lEdges[i]->m_pSource = q_ptr->getSourceSocket(src))
            m_lEdges[i]->m_pSource->setEdge(m_EdgeModel.index(i, 0));

        if (m_lEdges[i]->m_pSink   = q_ptr->getSinkSocket(sink))
            m_lEdges[i]->m_pSink->setEdge(m_EdgeModel.index(i, 2));

        m_lEdges[i]->m_Edge.update();
    }
}

QNodeEditorSocketModel* QNodeEditorEdgeModel::socketModel() const
{
    return d_ptr->q_ptr;
}
