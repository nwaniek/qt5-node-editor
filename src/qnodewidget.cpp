#include "qnodewidget.h"
#include "qobjectmodel.h"
#include "graphicsnode.hpp"
#include "qmultimodeltree.h"

#include <QtCore/QDebug>

class QNodeWidgetPrivate : public QObject
{
public:
    explicit QNodeWidgetPrivate(QObject* p) : QObject(p) {}

    QMultiModelTree m_Model{this};
};

QNodeWidget::QNodeWidget(QWidget* parent) : QNodeView(parent),
    d_ptr(new QNodeWidgetPrivate(this))
{
    setModel(&d_ptr->m_Model);
}

QNodeWidget::~QNodeWidget()
{
    delete d_ptr;
}

GraphicsNode* QNodeWidget::addObject(QObject* o, const QString& title)
{
    auto m = new QObjectModel({o}, Qt::Vertical, QObjectModel::Role::PropertyNameRole, this);

    return addModel(m, title);
}

GraphicsNode* QNodeWidget::addModel(QAbstractItemModel* m, const QString& title)
{
    const auto idx = d_ptr->m_Model.appendModel(m);

    Q_ASSERT(idx.isValid());

    Q_ASSERT(getNode(idx));

    return getNode(idx);
}
