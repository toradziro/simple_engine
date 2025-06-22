#pragma once
#include <memory>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include <ranges>
#include <concepts>

template<typename System>
std::string systemId()
{
	const auto typeName = typeid(System).name();
	return typeName;
}

template <typename T>
concept SystemConcept = requires(T obj, float dt)
{
	obj.update(dt);
};

class System
{
private:
	struct ISystem
	{
		virtual ~ISystem() = default;
		virtual std::string systemId() const = 0;
		virtual void update(float dt) = 0;
	};

	template <SystemConcept T>
	struct SystemObject final : ISystem
	{
		template<typename... Args>
		SystemObject(Args&&... args) : m_system(std::forward<Args>(args)...)
		{
		}

		virtual std::string systemId() const override
		{
			return ::systemId<T>();
		}

		virtual void update(float dt) override
		{
			m_system.update(dt);
		}

		T m_system;
	};

public:
	template <SystemConcept T, typename... Args>
	explicit System(std::in_place_type_t<T>, Args&&... args)
		: m_systemObject(std::make_unique<SystemObject<T>>(std::forward<Args>(args)...))
	{
	}

	template <typename T>
	T& getUnderlyingSystem()
	{
		auto systemObject = static_cast<SystemObject<T>*>(m_systemObject.get());
		return systemObject->m_system;
	}

	void update(float dt)
	{
		m_systemObject->update(dt);
	}

	std::string systemId() const
	{
		return m_systemObject->systemId();
	}

private:
	std::unique_ptr<ISystem> m_systemObject;
};

struct SystemHolder
{
	void addSystem(System&& system)
	{
		m_systems.push_back({ system.systemId(), std::move(system) });
	}

	template<typename SystemType, typename... Args>
	void addSystem(Args&&... args)
	{
		m_systems.emplace_back(
			systemId<SystemType>(),
			System{ std::in_place_type<SystemType>, std::forward<Args>(args)... }
		);
	}

	template<typename T>
	T& getSystem()
	{
		auto systemIdStr = systemId<T>();
		auto it = std::ranges::find_if(m_systems, [&systemIdStr](const auto& system)
			{
				return system.first == systemIdStr;
			});

		if (it == m_systems.end())
		{
			assert(false);
		}

		return it->second.getUnderlyingSystem<T>();
	}

	auto begin()
	{
		return m_systems.begin();
	}
	auto end()
	{
		return m_systems.end();
	}

private:
	std::vector<std::pair<std::string, System>> m_systems;
};