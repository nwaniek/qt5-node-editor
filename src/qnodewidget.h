#pragma once

#include "qnodeview.h"

class QNodeWidgetPrivate;

class QAbstractItemModel;
class GraphicsNode;

class QNodeWidget : public QNodeView
{
    Q_OBJECT
public:
    explicit QNodeWidget(QWidget* parent = Q_NULLPTR);
    virtual ~QNodeWidget();

    GraphicsNode* addObject(QObject* o, const QString& title = QString());
    GraphicsNode* addModel(QAbstractItemModel* m, const QString& title = QString());

private:
    QNodeWidgetPrivate* d_ptr;
    Q_DECLARE_PRIVATE(QNodeWidget);
};
