
#include "net/network"

#include <thread>

int main(int argc, char *argv[])
{
	network::MEISystem sys;

	network::Initialize();

	sys.Init(network::ADDRTYPE::NETA_IPv4);

	std::this_thread::sleep_for(std::chrono::seconds(20));
	return 0;
}

