
#include <network>

#include <thread>

int main(int argc, char *argv[])
{
	net::initialize();

	{
		net::Socket hs = net::Socket(4);
		net::MEISystem sys;
		sys.init(net::NETA_IPv4);
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}

	return 0;
}

