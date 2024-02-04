#include "connection.h"

#include <engine/shared/protocol.h>

IDbConnection::IDbConnection(const char *pPrefix)
{
	str_copy(m_aPrefix, pPrefix);
}
