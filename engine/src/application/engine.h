#pragma once

#include <iostream>
#include <format>
#include <chrono>
#include <ctime>
#include <print>
#include <absl/time/time.h>

#include <absl/time/clock.h>

#include <application/core/system_interface.h>
#include <application/engine_context.h>

struct Config
{
	std::string m_projectPath;
};

class Engine
{
public:
	Engine(const Config& config);
	~Engine() {}

	void run();

private:
	EngineContext m_context;
	SystemHolder  m_systemHolder;

	bool m_running = true;
};
