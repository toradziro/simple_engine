#include <iostream>
#include <format>
#include <glm/glm.hpp>

int main(int argc, char** argv)
{
	glm::vec2 vec = { 1.0f, 10.0f };
	std::cout << std::format("Hello in {}, {} {}", argv[0], vec.x, vec.y) << std::endl;
}