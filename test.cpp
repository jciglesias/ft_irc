#include <iostream>

int	main()
{
	time_t now = time(0);

	char *time = ctime(&now);
	std::string timee;

	timee = time;
	std::cout << timee << std::endl;
}
