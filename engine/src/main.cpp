import simple_engine;

#include "absl/flags/flag.h"

int main(int argc, char** argv)
{
	Config config { .m_projectPath = "D:/dev/simple_engine/images" };
	Engine e{ config };
	e.run();
}
