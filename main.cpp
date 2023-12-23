#include <iostream>
#include "Process/Process.hpp"
#include <fstream>
#include <string>


int main(int argc, char* argv[])
{
	std::string window_name{};
	if (argc >= 2)
		window_name = argv[1];

	while (window_name.empty())
	{
		std::cout << "Enter target process window name: ";
		std::getline(std::cin, window_name);
	}

	Process p(window_name.c_str());
	if (!p)
		return -1;

	p.disableHooks();

	return 0;
}