#include "qnodewidget.h"
#include "qobjectmodel.h"
#include "graphicsnode.hpp"
#include "qmultimodeltree.h"

#include "qreactiveproxymodel.h"

#include <QtCore/QDebug>

class QNodeWidgetPrivate : public QObject
{
public:
    explicit QNodeWidgetPrivate(QObject* p) : QObject(p) {}

    QMultiModelTree m_Model{this};

    QNodeWidget* q_ptr;

public Q_SLOTS:
    void slotRemoveRows(const QModelIndex& parent, int first, int last);
};

QNodeWidget::QNodeWidget(QWidget* parent) : QNodeView(parent),
    d_ptr(new QNodeWidgetPrivate(this))
{
    d_ptr->m_Model.setTopLevelIdentifierRole(Qt::UserRole);

    setModel(&d_ptr->m_Model);

    connect(reactiveModel(), &QAbstractItemModel::rowsAboutToBeRemoved,
        d_ptr, &QNodeWidgetPrivate::slotRemoveRows);
}

QNodeWidget::~QNodeWidget()
{
    delete d_ptr;
}

GraphicsNode* QNodeWidget::addObject(QObject* o, const QString& title, QNodeWidget::ObjectFlags f, const QVariant& uid)
{
    Q_UNUSED(f)
    d_ptr->q_ptr = this;

    auto m = new QObjectModel({o}, Qt::Vertical, QObjectModel::Role::PropertyNameRole, this);

    return addModel(m, title, uid);
}

/**
 * Add a new node based on a model to the canvas.
 * 
 * For the model to behave properly, it has to comply to the following thing:
 *
 *  1) Only QModelIndex with Qt::ItemIsDragEnabled flags will be added as source
 *  2) Only QModelIndex with Qt::ItemIsEditable and Qt::ItemIsDropEnable
 *    flags will be added as sinks
 *  3) All model ::data Qt::EditRole need to set the role that changes when
 *     dropped. It need to **always** return a typed QVariant, even if the
 *     value is NULL. This is used to check if the connection is valid.
 *  4) The following roles are supported:
 *     * Qt::DisplayRole for the label text
 *     * Qt::TooltipRole for the socket tooltip
 *     * Qt::BackgroundRole for the socket and edge background
 *     * Qt::DecorationRole for the socket icon
 *     * Qt::ForegroundRole for the label color
 *
 * Also note that mimeData() need to include the `application/x-qabstractitemmodeldatalist`
 * MIME type. For models that re-implement it, make sure to use the QMimeData*
 * returned from QAbstractItemModel::mimeData() as a base instead of creating
 * a blank one.
 */
GraphicsNode* QNodeWidget::addModel(QAbstractItemModel* m, const QString& title, const QVariant& uid)
{
    const auto idx = d_ptr->m_Model.appendModel(m);

    if (!title.isEmpty())
        d_ptr->m_Model.setData(idx, title, Qt::EditRole);

    d_ptr->m_Model.setData(idx, uid, Qt::UserRole);

    Q_ASSERT(d_ptr->m_Model.data(idx, Qt::UserRole) == uid);
    Q_ASSERT(reactiveModel()->data(idx, Qt::UserRole) == uid);
    Q_ASSERT(idx.isValid());
    Q_ASSERT(getNode(idx));

    return getNode(idx);
}

void QNodeWidgetPrivate::slotRemoveRows(const QModelIndex& parent, int first, int last)
{
    if (parent.isValid())
        return;

    for (int i = first; i <= last; i++) {
        auto idx = m_Model.index(i, 0);

        Q_ASSERT(idx.isValid());

        auto m = m_Model.getModel(idx);

        if (auto qom = qobject_cast<QObjectModel*>(m)) {
            if (qom->objectCount() == 1)
                Q_EMIT q_ptr->objectRemoved(qom->getObject(qom->index(0,0)));
            else
                Q_EMIT q_ptr->modelRemoved(m);
        }
        else
            Q_EMIT q_ptr->modelRemoved(m);
    }
}
