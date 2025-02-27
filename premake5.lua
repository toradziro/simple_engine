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

local vulkan_path = os.getenv("VULKAN_SDK_PATH")

if not vulkan_path or #vulkan_path == 0 then
	error("Set VULKAN_SDK_PATH variable")
else
	print(string.format("Vulkan path is: %s", vulkan_path))
end

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "sandbox/vendor/GLFW/include"
IncludeDir["glm"] = "sandbox/vendor/glm"
IncludeDir["vulkan"] = string.format("%s/%s", vulkan_path, "Include")

group "Dependencies"
	include "sandbox/vendor/GLFW"
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
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.vulkan}"
	}

	local vulkan_lib_path = string.format("%s/%s", vulkan_path, "Lib")
	libdirs
	{
		vulkan_lib_path
	}

	links
	{
		"GLFW",
		"vulkan-1"
	}


	filter "system:windows"
		systemversion "latest"
		defines
		{
			"_CRT_SECURE_NO_WARNINGS",
		}

	AddCommonConfigurations()