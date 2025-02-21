#include <iostream>
#include <format>

int main(int argc, char** argv)
{
	std::cout << std::format("Hello in {}", argv[0]) << std::endl;
}