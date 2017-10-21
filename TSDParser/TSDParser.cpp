#include "stdafx.h"

int main(int ac, char **av)
{
	for (int i = 1; i < ac; ++i)
	{
		nope::TSDFile file(av[i]);

		std::cout << file.json() << std::endl;
	}
    return 0;
}

