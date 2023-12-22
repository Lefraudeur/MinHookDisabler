#include <iostream>
#include "Process/Process.hpp"
#include <fstream>


int main(int arc, char* argv[])
{
	Process p("Window Name Here");
	if (!p)
		return -1;
	p.disableHooks();
	return 0;
}