#include "qobjectnode.hpp"
#include <QMetaProperty>
#include <QWidget>

qObjectnode::qObjectnode(QObject *data, QGraphicsItem *parent):GraphicsNode(parent)
{
    m_data=data;
    if(m_data==0)
        qWarning("NULL Data Object!");
    else
    {
        QWidget* tst = dynamic_cast<QWidget*>(data);
        if(tst!=0)
            setCentralWidget(tst);

        const QMetaObject* m = m_data->metaObject();
        int property_count = m->propertyCount()-1;

        for(;property_count>=0;property_count--)
        {
            QMetaProperty prop = m->property(property_count);
            if(prop.isConstant() || !prop.isUser())
                continue;
            if(prop.isReadable() && prop.hasNotifySignal())
                add_source(prop.name());
            if(prop.isWritable())
                add_sink(prop.name());
        }
    }
}
