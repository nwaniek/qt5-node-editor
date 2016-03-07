#ifndef __QOBJECTNODE_HPP__FF345507_3840_47A5_BE60_D24FFF0BEE7F
#define __QOBJECTNODE_HPP__FF345507_3840_47A5_BE60_D24FFF0BEE7F

#include "graphicsnode.hpp"

class qObjectnode: public GraphicsNode
{
public:
	qObjectnode(QObject* data, QGraphicsItem *parent = nullptr);

private:
	QObject* m_data;
};

#endif /* __QOBJECTNODE_HPP__FF345507_3840_47A5_BE60_D24FFF0BEE7F */

