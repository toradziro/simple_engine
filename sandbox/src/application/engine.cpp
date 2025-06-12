#include "engine.h"

#include <iostream>
#include <format>

struct TstSystem
{
	int i;
};

Engine::Engine()
{
	TstSystem tst = { 24 };
	m_systemHolder.addSystem(std::move(tst));
}

Engine::~Engine()
{
	std::cout << std::format("TST SYSTEM VALUE:  {}", m_systemHolder.getSystem<TstSystem>().i) << std::endl;
}
