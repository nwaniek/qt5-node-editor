#include "modelnode.hpp"

#include <QtCore/QAbstractItemModel>
#include <QtCore/QDebug>

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
		connect(d_ptr->_model, &QAbstractItemModel::dataChanged , d_ptr, &ModelnodePrivate::slotDataChanged );
		
		// Initial data
		d_ptr->slotRowsInserted({}, 0, model->rowCount()-1);
		
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

int Modelnode::connectedObjectRole() const
{
	return d_ptr->m_ConnectedObjectRole;
}

int Modelnode::connectedPropertyRole() const
{
	return d_ptr->m_ConnectedPropertyRole;
}

int Modelnode::connectedModelIndexRole() const
{
	return d_ptr->m_ConnectedModelIndexRole;
}

QByteArray Modelnode::connectedProperty() const
{
	return d_ptr->m_ConnectedProperty;
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

void Modelnode::setConnectedObjectRole(int role)
{
	d_ptr->m_ConnectedObjectRole = role;
}

void Modelnode::setConnectedPropertyRole(int role)
{
	d_ptr->m_ConnectedPropertyRole = role;
}

void Modelnode::setConnectedModelIndexRole(int role)
{
	d_ptr->m_ConnectedModelIndexRole = role;
}


void Modelnode::setObjectProperty(QString &prop)
{
	d_ptr->m_ObjectProperty = prop;
}

void Modelnode::setConnectedProperty(const QByteArray& a)
{
	d_ptr->m_ConnectedProperty = a;
}

void ModelnodePrivate::
slotConnected(GraphicsNodeSocket* other)
{
	Q_UNUSED(other)
	qDebug() << "\n\n\nCONNECTED TO MODEL";
}

void ModelnodePrivate::
slotRowsInserted(const QModelIndex &parent, int first, int last)
{
	// Only list models are supported
	if (parent.isValid()) return;
	
	for (int i = first; i <= last; i++) {
		const QModelIndex idx = _model->index(i, 0);
		
		GraphicsNodeSocket* node = q_ptr->addSource(
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
	Q_UNUSED(first)
	Q_UNUSED(last)
	// Only list models are supported
	if (parent.isValid()) return;
	//TODO
}

void ModelnodePrivate::
slotRowsMoved(const QModelIndex &parent, int first, int last)
{
	Q_UNUSED(first)
	Q_UNUSED(last)
	// Only list models are supported
	if (parent.isValid()) return;
	//TODO
}

void ModelnodePrivate::
slotDataChanged(const QModelIndex &tl, const QModelIndex &br)
{
	// Update the names
	for (int i=tl.row(); i <= br.row(); i++) {
		auto s = q_ptr->getSourceSocket(i);
		s->setText(tl.model()->index(i,0).data(m_TitleRole).toString());
	}
// 	GraphicsNodeSocket* q_ptr->get_sink_socket(const size_t id);
}

void ModelnodePrivate::
slotReset()
{
	
}

void ModelnodePrivate::
slotLayoutChanged()
{
	
}

// kate: space-indent off;; indent-width 4; replace-tabs off;
