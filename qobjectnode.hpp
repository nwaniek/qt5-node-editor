#include "graphicsnode.hpp"

class qObjectnode: public GraphicsNode
{
public:
    qObjectnode(QObject* data, QGraphicsItem *parent = nullptr);

private:
    QObject* m_data;
};
