#include "modelnode.hpp"

#include <QtCore/QAbstractItemModel>

#include "graphicsnodesocket.hpp"
#include "connectmapper_p.hpp"

Modelnode::
Modelnode(QAbstractItemModel *model, QGraphicsItem *parent) : GraphicsNode(parent),
	d_ptr(new ModelnodePrivate())
{
	d_ptr->q_ptr  = this;
	d_ptr->_model = model;

	if(model == nullptr)
		qWarning("NULL Data Model!");
	else {
		
		connect(d_ptr->_model, &QAbstractItemModel::rowsInserted, d_ptr, &ModelnodePrivate::slotRowsInserted);
		connect(d_ptr->_model, &QAbstractItemModel::rowsRemoved , d_ptr, &ModelnodePrivate::slotRowsRemoved );
		connect(d_ptr->_model, &QAbstractItemModel::rowsMoved   , d_ptr, &ModelnodePrivate::slotRowsMoved   );
		
		// Initial data
		d_ptr->slotRowsInserted({}, 0, model->rowCount());
		
		//TODO watch for header changes
		setTitle(model->headerData(0, Qt::Horizontal).toString());
		
	}
}

int Modelnode::idRole() const
{
	return d_ptr->m_IdRole;
}

int Modelnode::titleRole() const
{
	return d_ptr->m_TitleRole;
}

int Modelnode::objectRole() const
{
	return d_ptr->m_ObjectRole;
}

QString Modelnode::objectProperty() const
{
	return d_ptr->m_ObjectProperty;
}

void Modelnode::setIdRole(int role)
{
	d_ptr->m_IdRole = role;
}

void Modelnode::setTitleRole(int role)
{
	d_ptr->m_TitleRole = role;
}

void Modelnode::setObjectRole(int role)
{
	d_ptr->m_ObjectRole = role;
}

void Modelnode::setObjectProperty(QString &prop)
{
	d_ptr->m_ObjectProperty = prop;
}

void ModelnodePrivate::
slotConnected(GraphicsNodeSocket* other)
{
	
}

void ModelnodePrivate::
slotRowsInserted(const QModelIndex &parent, int first, int last)
{
	// Only list models are supported
	if (parent.isValid()) return;
	
	for (int i = first; i < last; i++) {
		const QModelIndex idx = _model->index(i, 0);
		
		GraphicsNodeSocket* node = q_ptr->add_source(
			idx.data().toString(), _model, i
		);

		PropertyConnection* conn = new PropertyConnection(_model, PropertyConnection::Mode::MODEL);
		conn->_prop_id  = i;
		conn->d_ptr2 = this;
		node->setProperty("socketData", QVariant::fromValue(conn));
	}
}

void ModelnodePrivate::
slotRowsRemoved(const QModelIndex &parent, int first, int last)
{
	// Only list models are supported
	if (parent.isValid()) return;
	//TODO
}

void ModelnodePrivate::
slotRowsMoved(const QModelIndex &parent, int first, int last)
{
	// Only list models are supported
	if (parent.isValid()) return;
	//TODO
}

void ModelnodePrivate::
slotReset()
{
	
}

void ModelnodePrivate::
slotLayoutChanged()
{
	
}

// kate: space-indent off;; indent-width 8; replace-tabs off;
