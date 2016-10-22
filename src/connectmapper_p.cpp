#include "connectmapper_p.hpp"


PropertyConnection::PropertyConnection(QObject* parent, PropertyConnection::Mode mode) : QObject(parent),
_mode(mode)
{

}

PropertyConnection::~PropertyConnection()
{
	QObject::disconnect(_conn);
}
