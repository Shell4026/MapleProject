#include "World/MapleServer.h"

namespace sh::game
{
	MapleServer::MapleServer(GameObject& owner) :
		UdpServer(owner)
	{
	}
}//namespace