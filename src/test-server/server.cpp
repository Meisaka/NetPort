
#include "net/network"

#include <thread>

int main(int argc, char *argv[])
{
	network::Initialize();

	{
		network::MEISystem sys;
		sys.Init(network::ADDRTYPE::NETA_IPv4);
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}

	return 0;
}

