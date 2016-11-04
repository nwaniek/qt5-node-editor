#include "qmultimodeltree.h"

struct InternalItem
{
    enum class Mode {
        ROOT,
        PROXY
    };

    int                    m_Index;
    Mode                   m_Mode;
    QAbstractItemModel*    m_pModel;
    InternalItem*          m_pParent;
    QVector<InternalItem*> m_lChildren;
};

class QMultiModelTreePrivate : public QObject
{
public:
    QVector<InternalItem*> m_lRows;
    QHash<const QAbstractItemModel*, InternalItem*> m_hModels;

public Q_SLOTS:
    void slotRowsInserted(const QModelIndex& parent, int first, int last);
    void slotAddRows(const QModelIndex& parent, int first, int last, QAbstractItemModel* src);
};

QMultiModelTree::QMultiModelTree(QObject* parent) : QAbstractItemModel(parent),
    d_ptr(new QMultiModelTreePrivate)
{
}

QMultiModelTree::~QMultiModelTree()
{
    d_ptr->m_hModels.clear();
    while(!d_ptr->m_lRows.isEmpty()) {
        while(!d_ptr->m_lRows.first()->m_lChildren.isEmpty())
            delete d_ptr->m_lRows.first()->m_lChildren.first();
        delete d_ptr->m_lRows.first();
    }

    delete d_ptr;
}

QVariant QMultiModelTree::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};

    const auto i = static_cast<InternalItem*>(idx.internalPointer());

    if (i->m_Mode == InternalItem::Mode::PROXY)
        return i->m_pModel->data(mapToSource(idx), role);

    switch (role) {
        case Qt::DisplayRole:
            return i->m_pModel->objectName().isEmpty() ?
                QStringLiteral("N/A") : i->m_pModel->objectName();
    };

    return {};
}

bool QMultiModelTree::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return {};

    const auto i = static_cast<InternalItem*>(index.internalPointer());

    if (i->m_Mode == InternalItem::Mode::PROXY)
        return i->m_pModel->setData(mapToSource(index), value, role);

    return false;
}

int QMultiModelTree::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return d_ptr->m_lRows.size();

    const auto i = static_cast<InternalItem*>(parent.internalPointer());

    if (i->m_Mode == InternalItem::Mode::PROXY)
        return 0;

    return i->m_pModel->rowCount();
}

int QMultiModelTree::columnCount(const QModelIndex& parent) const
{
    return 1;
}

QModelIndex QMultiModelTree::index(int row, int column, const QModelIndex& parent) const
{
    if (
        (!parent.isValid())
        && row >= 0
        && row < d_ptr->m_lRows.size()
        && column == 0
    )
        return createIndex(row, column, d_ptr->m_lRows[row]);

    const auto i = static_cast<InternalItem*>(parent.internalPointer());

    return mapFromSource(i->m_pModel->index(row, column));
}

Qt::ItemFlags QMultiModelTree::flags(const QModelIndex &idx) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex QMultiModelTree::parent(const QModelIndex& idx) const
{
    if (!idx.isValid())
        return {};

    const auto i = static_cast<InternalItem*>(idx.internalPointer());

    if (i->m_Mode == InternalItem::Mode::ROOT)
        return {};

    return createIndex(i->m_pParent->m_Index, 0, i->m_pParent);
}

QModelIndex QMultiModelTree::mapFromSource(const QModelIndex& sourceIndex) const
{
    if ((!sourceIndex.isValid()) || sourceIndex.parent().isValid() || sourceIndex.column())
        return {};

    const auto i = d_ptr->m_hModels[sourceIndex.model()];

    if (!i && sourceIndex.row() >= i->m_lChildren.size())
        return {};

    return createIndex(sourceIndex.row(), 0, i->m_lChildren[sourceIndex.row()]);
}

QModelIndex QMultiModelTree::mapToSource(const QModelIndex& proxyIndex) const
{
    if ((!proxyIndex.isValid()) || proxyIndex.model() != this)
        return {};

    if (!proxyIndex.parent().isValid())
        return {};

    const auto i = static_cast<InternalItem*>(proxyIndex.internalPointer());

    return i->m_pModel->index(proxyIndex.row(), proxyIndex.column());
}

void QMultiModelTreePrivate::slotAddRows(const QModelIndex& parent, int first, int last, QAbstractItemModel* src)
{
    if (parent.isValid()) return;

    auto p = m_hModels[src];
    Q_ASSERT(p);

    for (int i = first; i <= last; i++) {
        p->m_lChildren << new InternalItem {
            p->m_lChildren.size(),
            InternalItem::Mode::PROXY,
            src,
            p,
            {}
        };
    }
}

void QMultiModelTreePrivate::slotRowsInserted(const QModelIndex& parent, int first, int last)
{
    const auto model = qobject_cast<QAbstractItemModel*>(QObject::sender());
    Q_ASSERT(model);

    slotAddRows(parent, first, last, model);
}

void QMultiModelTree::appendModel(QAbstractItemModel* model)
{
    if ((!model) || d_ptr->m_hModels[model]) return;

    d_ptr->m_hModels[model] = new InternalItem {
        d_ptr->m_lRows.size(),
        InternalItem::Mode::ROOT,
        model,
        Q_NULLPTR,
        {}
    };
    d_ptr->m_lRows << d_ptr->m_hModels[model];

    d_ptr->slotAddRows({}, 0, model->rowCount(), model);
}

