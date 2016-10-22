#ifndef CONNECT_MAPPER_H
#define CONNECT_MAPPER_H

#include <QtCore/QObject>
#include "graphicsnode.hpp"

class QAbstractItemModel;

class Modelnode;

/*
 * Private structure shared by internal Qt introspection and model based
 * implementations
 */

class QObjectnodePrivate : public QObject
{
	Q_OBJECT
public:
	QObject* _data;
	QHash<int, QMetaObject::Connection> _source_connections;
	QHash<int, QMetaObject::Connection> _sink_connections;

public Q_SLOTS:
	void slotConnected(GraphicsNodeSocket* other);
};

class ModelnodePrivate : public QObject
{
	Q_OBJECT
public:
	QAbstractItemModel* _model;
	QVector<QMetaObject::Connection> _source_connections;
	QVector<QMetaObject::Connection> _sink_connections;
	
	int m_IdRole {Qt::DisplayRole};
	int m_TitleRole {Qt::DisplayRole};
	int m_ObjectRole {Qt::UserRole};

	QString m_ObjectProperty {QString()}; //The object itself
	
	Modelnode *q_ptr;

public Q_SLOTS:
	void slotConnected(GraphicsNodeSocket* other);
	void slotRowsInserted(const QModelIndex &parent, int first, int last);
	void slotRowsRemoved(const QModelIndex &parent, int first, int last);
	void slotRowsMoved(const QModelIndex &parent, int first, int last);
	void slotReset();
	void slotLayoutChanged();
};


class PropertyConnection final : public QObject //TODO make private
{
	Q_OBJECT
public:
	enum class Mode {
		MODEL,
		OBJECT
	};

	explicit PropertyConnection(QObject* parent, Mode mode);
	~PropertyConnection();

	const QMetaObject* _source_mo;
	QMetaObject::Connection _conn;
	int _prop_id;
	Mode _mode;
	QObjectnodePrivate* d_ptr;
	ModelnodePrivate* d_ptr2;
};
Q_DECLARE_METATYPE(PropertyConnection*)

#endif
// kate: space-indent off;; indent-width 8; replace-tabs off;
