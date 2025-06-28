#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <cassert>
#include <algorithm>
#include <ranges>
#include <concepts>

template<typename Manager>
std::string managerId()
{
	const auto typeName = typeid(Manager).name();
	return typeName;
}

class Manager
{
private:
	struct IManager
	{
		virtual ~IManager() = default;
		virtual std::string managerId() const = 0;
	};

	template <typename T>
	struct ManagerObject final : IManager
	{
		template<typename... Args>
		ManagerObject(Args&&... args) : m_manager(std::forward<Args>(args)...)
		{
		}

		virtual std::string managerId() const override
		{
			return ::managerId<T>();
		}

		T m_manager;
	};

public:
	template <typename T, typename... Args>
	explicit Manager(std::in_place_type_t<T>, Args&&... args)
		: m_managerObject(std::make_unique<ManagerObject<T>>(std::forward<Args>(args)...))
	{
	}

	template <typename T>
	T& getUnderlyingManager()
	{
		auto managerObject = static_cast<ManagerObject<T>*>(m_managerObject.get());
		return managerObject->m_manager;
	}

	std::string managerId() const
	{
		return m_managerObject->managerId();
	}

private:
	std::unique_ptr<IManager> m_managerObject;
};

struct ManagerHolder
{
	void addManager(Manager&& manager)
	{
		m_managers.insert({ manager.managerId(), std::move(manager) });
	}

	template<typename ManagerType, typename... Args>
	void addManager(Args&&... args)
	{
		m_managers.insert({
			managerId<ManagerType>(),
			Manager{ std::in_place_type<ManagerType>, std::forward<Args>(args)... }
		});
	}

	template<typename T>
	T& getManager()
	{
		auto managerIdStr = managerId<T>();
		auto it = std::ranges::find_if(m_managers, [&managerIdStr](const auto& manager)
			{
				return manager.first == managerIdStr;
			});

		if (it == m_managers.end())
		{
			assert(false);
		}

		return it->second.getUnderlyingManager<T>();
	}

private:
	std::unordered_map<std::string, Manager> m_managers;
};
