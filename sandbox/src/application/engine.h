#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <cassert>

template<typename System>
std::string systemId()
{
	const auto typeName = typeid(System).name();
	return typeName;
}

class System
{
public:
	template <typename T>
	System(T&& systemObject) : m_systemObject(std::make_unique<SystemObject<T>>(std::move(systemObject)))
	{}

	template <typename T>
	T& getUnderlyingSystem()
	{
		auto&& systemObject = static_cast<SystemObject<T>*>(m_systemObject.get());
		return systemObject->m_system;
	}

	std::string systemId() const
	{
		return m_systemObject->systemId();
	}

private:
	struct ISystem
	{
		virtual ~ISystem() = default;
		virtual std::string systemId() const = 0;
	};

	template <typename T>
	struct SystemObject final : ISystem
	{
		SystemObject(T&& system) : m_system(std::move(system)) {}

		virtual std::string systemId() const override
		{
			return ::systemId<T>();
		}

		T&&	m_system;
	};

private:
	std::unique_ptr<ISystem>	m_systemObject;
};

struct SystemHolder
{
	void addSystem(System&& system)
	{
		m_systems.insert({ system.systemId(), std::move(system) });
	}

	template<typename T>
	T& getSystem()
	{
		auto&& systemIdStr = systemId<T>();
		auto it = m_systems.find(systemIdStr);
		if (it == m_systems.end())
		{
			assert(false);
		}
		return it->second.getUnderlyingSystem<T>();
	}

private:
	std::unordered_map<std::string, System>	m_systems;
};

class Engine
{
public:
	Engine();
	~Engine();

private:
	SystemHolder	m_systemHolder;
};