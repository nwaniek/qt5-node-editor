#include "qnodeeditorsocketmodel.h"

#include "graphicsnode.hpp"
#include "graphicsnodescene.hpp"

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
};

class QNodeEditorSocketModelPrivate final : public QObject
{
public:
    explicit QNodeEditorSocketModelPrivate(QObject* p) : QObject(p) {}

    QNodeEditorEdgeModel m_EdgeModel {this};
    QVector<NodeWrapper*> m_lWrappers;
    GraphicsNodeScene* m_pScene;

    // helper
    GraphicsNode* insertNode(int idx, const QString& title);
    NodeWrapper* getNode(const QModelIndex& idx, bool r = false) const;

    void insertSockets(const QModelIndex& parent, int first, int last);
    void updateSockets(const QModelIndex& parent, int first, int last);
    void removeSockets(const QModelIndex& parent, int first, int last);

    QNodeEditorSocketModel* q_ptr;
public Q_SLOTS:
    void slotRowsInserted(const QModelIndex& parent, int first, int last);
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

    const auto nodew = d_ptr->getNode(idx, true);

    if (!nodew)
        return Q_NULLPTR;

    return nodew->m_lSinksToSrc.size() > idx.row() ?
        nodew->m_lSinksToSrc[idx.row()] : Q_NULLPTR;
}

QNodeEditorEdgeModel* QNodeEditorSocketModel::edgeModel() const
{
    return &d_ptr->m_EdgeModel;
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
    qDebug() << "\n\n\nIN INSERT NODE!!!" << idx;
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
    Q_ASSERT(m_lWrappers.size() > idx.row());

    return m_lWrappers[idx.row()];
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

QNodeEditorSocketModel* QNodeEditorEdgeModel::socketModel() const
{
    return d_ptr->q_ptr;
}
