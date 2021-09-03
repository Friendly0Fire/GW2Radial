#pragma once
#include <memory>
#include <concepts>
#include <stack>
#include <functional>

namespace GW2Radial
{

class BaseSingleton
{
public:
	virtual ~BaseSingleton() { }
protected:
	static BaseSingleton* Store(std::unique_ptr<BaseSingleton>&& ptr);
};

class SingletonManager
{
public:
	SingletonManager() = default;
	void Shutdown() {
		while (!singletons_.empty())
			singletons_.pop();
	}

private:
	std::stack<std::unique_ptr<BaseSingleton>> singletons_;

	friend class BaseSingleton;
};
inline static SingletonManager SingletonManagerInstance;

inline BaseSingleton* BaseSingleton::Store(std::unique_ptr<BaseSingleton>&& ptr)
{
	SingletonManagerInstance.singletons_.push(std::move(ptr));
	return SingletonManagerInstance.singletons_.top().get();
}

template<typename T, bool AutoInit = true>
class Singleton : public BaseSingleton
{
public:
	static T& i()
	{
		if constexpr (AutoInit) {
			if (!init_) {
				init_ = true;
				i_ = (T*)Store(std::make_unique<T>());
			}
		}
		else {
			if (!i_)
				throw std::logic_error("Singleton is not Auto Init and was not initialized.");
		}
		return *i_;
	}

	template<typename T2 = T> requires std::derived_from<T2, T>
	static T& i(std::unique_ptr<T2>&& i)
	{
		if (!init_) {
			init_ = true;
			i_ = (T*)Store(std::move(i));
		}
		return *i_;
	}

	template<typename T2 = T> requires std::derived_from<T2, T>
	static void i(std::function<void(T&)> action)
	{
		if (i_)
			action(*i_);
	}

	virtual ~Singleton() {
		i_ = nullptr;
	}

private:
	inline static bool init_ = false;
	inline static T* i_ = nullptr;
};

}