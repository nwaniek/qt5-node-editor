#include "qreactiveproxymodel.h"

#include <QtCore/QAbstractTableModel>
#include <QtCore/QMimeData>
#include <QtCore/QDebug>

class ConnectedIndicesModel : public QAbstractTableModel
{
    friend class QReactiveProxyModel;
public:
    ConnectedIndicesModel(QObject* parent, QReactiveProxyModelPrivate* d);

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual int columnCount(const QModelIndex& parent = {}) const override;

private:
    QReactiveProxyModelPrivate* d_ptr;
};

struct ConnectionHolder
{
    QPersistentModelIndex source;
    QPersistentModelIndex destination;
    void* sourceIP;
    void* destinationIP;
};

class QReactiveProxyModelPrivate : public QObject
{
public:
    const QString MIME_TYPE = QStringLiteral("qt-model/reactive-connection");
    QVector<int> m_lConnectedRoles;
    ConnectedIndicesModel* m_pConnectionModel;
    QHash<const QMimeData*, QPersistentModelIndex> m_hDraggedIndexCache;
    QVector<ConnectionHolder*> m_lConnections;

    // In case dataChanged() contains a single QModelIndex, use this fast path
    // to avoid doing a query on each connections or QModelIndex
    QHash<void*, ConnectionHolder*> m_hDirectMapping;

    //Helper
    void clear();

public Q_SLOTS:
    void slotMimeDestroyed();
};

QReactiveProxyModel::QReactiveProxyModel(QObject* parent) : QIdentityProxyModel(parent),
    d_ptr(new QReactiveProxyModelPrivate)
{
    d_ptr->m_pConnectionModel = new ConnectedIndicesModel(this, d_ptr);
}

ConnectedIndicesModel::ConnectedIndicesModel(QObject* parent, QReactiveProxyModelPrivate* d)
    : QAbstractTableModel(parent), d_ptr(d)
{

}

QReactiveProxyModel::~QReactiveProxyModel()
{
    delete d_ptr;
}

QMimeData* QReactiveProxyModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.size() != 1)
        return Q_NULLPTR;

    const auto idx = indexes.first();

    if (!idx.isValid())
        return Q_NULLPTR;

    auto md = new QMimeData();
    connect(md, &QObject::destroyed, d_ptr, &QReactiveProxyModelPrivate::slotMimeDestroyed);

    d_ptr->m_hDraggedIndexCache[md] = idx;

    md->setData(d_ptr->MIME_TYPE, "valid");

    return md;
}

bool QReactiveProxyModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    const auto idx = index(row, column, parent);

    if ((!data) || (!idx.isValid()) || !(action & supportedDropActions()))
        return false;

    if (!data->hasFormat(d_ptr->MIME_TYPE))
        return false;


    return true;
}

bool QReactiveProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if ((!data) || !data->hasFormat(d_ptr->MIME_TYPE))
        return false;

    const auto srcIdx  = d_ptr->m_hDraggedIndexCache[data];
    const auto destIdx = index(row, column, parent);

    if ((!srcIdx.isValid()) || !destIdx.isValid())
        return false;

    connectIndices(srcIdx, destIdx);

    return true;
}

QStringList QReactiveProxyModel::mimeTypes() const
{
    static QStringList ret {d_ptr->MIME_TYPE};

    return ret;
}

Qt::DropActions QReactiveProxyModel::supportedDropActions() const
{
    return Qt::LinkAction;
}

Qt::DropActions QReactiveProxyModel::supportedDragActions() const
{
    return Qt::LinkAction;
}

void QReactiveProxyModel::setSourceModel(QAbstractItemModel *sm)
{
    if (sm == sourceModel())
        return;

    d_ptr->clear();

    QIdentityProxyModel::setSourceModel(sm);

}

Qt::ItemFlags QReactiveProxyModel::flags(const QModelIndex &idx) const
{
    return mapToSource(idx).flags();
}

void QReactiveProxyModel::addConnectedRole(int role)
{
    if (d_ptr->m_lConnectedRoles.indexOf(role) == -1)
        d_ptr->m_lConnectedRoles << role;
}

QVector<int> QReactiveProxyModel::connectedRoles() const
{
    return d_ptr->m_lConnectedRoles;
}

QAbstractItemModel* QReactiveProxyModel::connectionsModel() const
{
    return d_ptr->m_pConnectionModel;
}

bool QReactiveProxyModel::connectIndices(const QModelIndex& srcIdx, const QModelIndex& destIdx)
{
    if (!(srcIdx.isValid() && destIdx.isValid()))
        return false;

    if (srcIdx.model() != this || destIdx.model() != this) {
        qWarning() << "Trying to connect QModelIndex from the wrong model";
        return false;
    }

    qDebug() << "CONNECING" << srcIdx << destIdx;

    auto conn = new ConnectionHolder {
        srcIdx,
        destIdx,
        srcIdx.internalPointer(),
        destIdx.internalPointer(),
    };

    // Check if there is an internal pointer or UID, models can work without
    // them, better not assume they exist. It is also not possible to assume
    // they are valid or unique. This is checked when it is retrieved.
    if (srcIdx.internalPointer())
        d_ptr->m_hDirectMapping[ srcIdx.internalPointer () ] = conn;

    if (destIdx.internalPointer())
        d_ptr->m_hDirectMapping[ destIdx.internalPointer() ] = conn;

    // Sync the current source value into the sink
    setData(destIdx, srcIdx.data(Qt::DisplayRole) , Qt::DisplayRole);

    // Register the connection
    const int id = d_ptr->m_lConnections.size();
    d_ptr->m_pConnectionModel->beginInsertRows({}, id, id);
    d_ptr->m_lConnections << conn;
    d_ptr->m_pConnectionModel->endInsertRows();

    return true;
}

bool QReactiveProxyModel::areConnected(const QModelIndex& source, const QModelIndex& destination) const
{
    Q_UNUSED(source)
    Q_UNUSED(destination)
    return false; //TODO
}

QList<QModelIndex> QReactiveProxyModel::sendTo(const QModelIndex& source) const
{
    Q_UNUSED(source)
    return {}; //TODO
}

QList<QModelIndex> QReactiveProxyModel::receiveFrom(const QModelIndex& destination) const
{
    Q_UNUSED(destination)
    return {}; //TODO
}

QVariant ConnectedIndicesModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};

    const auto conn = d_ptr->m_lConnections[idx.row()];

    Q_ASSERT(conn);

    switch(idx.column()) {
        case 0:
            return conn->source.data(role);
        case 1:
            return {}; //Eventually they will be named and have colors
        case 2:
            return conn->destination.data(role);
    }

    return {};
}

int ConnectedIndicesModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : d_ptr->m_lConnections.size();
}

int ConnectedIndicesModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : 3;
}

void QReactiveProxyModelPrivate::clear()
{
    while (!m_lConnections.isEmpty()) {
        delete m_lConnections.first();
    }
}

void QReactiveProxyModelPrivate::slotMimeDestroyed()
{
    // collect the garbage
    m_hDraggedIndexCache.remove(static_cast<QMimeData*>(QObject::sender()));

}
