#include <application/engine.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

ABSL_FLAG(std::string, projectPath, "../simple_project/", "Path to project");

int main(int argc, char** argv)
{
	absl::ParseCommandLine(argc, argv);

	Config config{ .m_projectPath = absl::GetFlag(FLAGS_projectPath) };
	Engine e{ config };
	e.run();
}
