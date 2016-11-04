#pragma once

#include <QtCore/QAbstractItemModel>

class QMultiModelTreePrivate;

/**
 * Create a tree model with external models as top level indices and their
 * content as children.
 * 
 * The use case for this is either views like side panels or as a node in a
 * longer proxy model chain.
 * 
 * Note that this code currently doesn't tree models as children. If anybody
 * has a solution that's both fast and reliable, please tell.
 */
class QMultiModelTree : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QMultiModelTree(QObject* parent = Q_NULLPTR);
    virtual ~QMultiModelTree();

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent ={}) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;
    virtual QModelIndex parent(const QModelIndex& idx) const override;
    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

    void appendModel(QAbstractItemModel* model);

public:
    QMultiModelTreePrivate* d_ptr;
    Q_DECLARE_PRIVATE(QMultiModelTree)
};
