workspace "sandbox"
	architecture "x64"
	startproject "sandbox"

	configurations
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}

group "Dependencies"
group ""

defines
{
}

-- Function to add common configurations
function AddCommonConfigurations()
	filter "configurations:Debug"
		symbols "on"
		runtime "Debug"
		
	filter "configurations:Release"
		symbols "on"
		optimize "on"
		runtime "Release"
end

-- JourneyEngine Project
project "sandbox"
	location "sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"%{prj.name}/src",
	}

	filter "system:windows"
		systemversion "latest"
		defines
		{
			"_CRT_SECURE_NO_WARNINGS",
		}

	AddCommonConfigurations()