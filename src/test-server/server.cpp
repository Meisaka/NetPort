
#include <network>

#include <thread>

int main(int argc, char *argv[])
{
	network::initialize();

	{
		network::Socket hs = network::Socket(4);
		network::MEISystem sys;
		sys.init(network::NETA_IPv4);
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}

	return 0;
}

