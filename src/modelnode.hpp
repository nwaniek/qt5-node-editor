#ifndef MODEL_NODE_H
#define MODEL_NODE_H

#include "graphicsnode.hpp"

class QAbstractItemModel;

class ModelnodePrivate;

class Modelnode : public GraphicsNode
{
public:
	Modelnode(QAbstractItemModel *model, QGraphicsItem *parent = nullptr);
	
	int idRole() const;
	int titleRole() const;
	int objectRole() const;

	QString objectProperty() const;
	
	
	void setIdRole(int role);
	void setTitleRole(int role);
	void setObjectRole(int role);

	void setObjectProperty(QString &prop);

private:
	ModelnodePrivate* d_ptr;
	Q_DECLARE_PRIVATE(Modelnode)
};

#endif /* MODEL_NODE_H */

// kate: space-indent off;; indent-width 8; replace-tabs off;
