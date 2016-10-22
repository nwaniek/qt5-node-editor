#include "qobjectnode.hpp"
#include <QMetaProperty>
#include <QWidget>

#include "graphicsnodesocket.hpp"
#include "connectmapper_p.hpp"

#include <QtCore/QAbstractItemModel>

QObjectnode::
QObjectnode(QObject *data, QGraphicsItem *parent) : GraphicsNode(parent),
	d_ptr(new QObjectnodePrivate())
{
	d_ptr->_data = data;

	if(data==0)
		qWarning("NULL Data Object!");
	else {
		QWidget* tst = dynamic_cast<QWidget*>(data);
		if(tst!=0)
			setCentralWidget(tst);

		const QMetaObject* m = data->metaObject();
		int property_count = m->propertyCount()-1;

		for(;property_count>=0;property_count--) {
			QMetaProperty prop = m->property(property_count);
			if(prop.isConstant() || !prop.isUser())
				continue;
			if(prop.isReadable() && prop.hasNotifySignal())
			{
				GraphicsNodeSocket* node = add_source(QString(prop.name()) + "[" +QString(prop.typeName())  +"]",data,property_count);

                                PropertyConnection* conn = new PropertyConnection(data, PropertyConnection::Mode::OBJECT);
                                conn->_prop_id  = property_count;
                                conn->_source_mo = m;
                                conn->d_ptr = d_ptr;
                                node->setProperty("socketData", QVariant::fromValue(conn));

			}
			if(prop.isWritable()) {
				GraphicsNodeSocket* node = add_sink(QString(prop.name()) + "[" +QString(prop.typeName())  +"]",data,property_count);

                                PropertyConnection* conn = new PropertyConnection(data, PropertyConnection::Mode::OBJECT);
                                conn->_prop_id  = property_count;
                                conn->_source_mo = m;
                                conn->d_ptr = d_ptr;
                                node->setProperty("socketData", QVariant::fromValue(conn));

                                QObject::connect(node, &GraphicsNodeSocket::connectedTo, d_ptr, &QObjectnodePrivate::slotConnected);
                        }
		}
	}
}


void QObjectnodePrivate::
slotConnected(GraphicsNodeSocket* source_node)
{
	const GraphicsNodeSocket* sink_node = qobject_cast<GraphicsNodeSocket*>(QObject::sender());

	if (sink_node && source_node) {
		const PropertyConnection* srcMO  = qvariant_cast<PropertyConnection*>(source_node->property("socketData"));
		const PropertyConnection* sinkMO = qvariant_cast<PropertyConnection*>(sink_node->property("socketData"));

		// Connect 2 QObjectnode
		if (srcMO && sinkMO) {
			const QMetaProperty sinkProp = sinkMO->_source_mo->property(sinkMO->_prop_id);
			const QByteArray sinkName   = sinkProp.name();

			// Set the target node property to the source current value
			switch(srcMO->_mode) {
				case PropertyConnection::Mode::OBJECT: {
					const QMetaProperty sourceProp = srcMO->_source_mo->property(srcMO->_prop_id);

					const QByteArray sourceName = sourceProp.name();

					sinkMO->d_ptr->_data->setProperty(
						sinkName,
						srcMO->d_ptr->_data->property(sourceName)
					);
				}
					break;
				case PropertyConnection::Mode::MODEL: {
					const auto idx = srcMO->d_ptr2->_model->index(sinkMO->_prop_id, 0);
					sinkMO->d_ptr->_data->setProperty(
						sinkName,
						idx.data(srcMO->d_ptr2->m_ObjectRole)
					);
				}
					break;
			}
		//TODO connect to the signal
		}
	}
}
// kate: space-indent off;; indent-width 8; replace-tabs off;
