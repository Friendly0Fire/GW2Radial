#pragma once
#include <memory>
#include <concepts>

namespace GW2Radial
{

template<typename T, bool AutoInit = true>
class Singleton
{
public:
	template<typename T2 = T> requires std::derived_from<T2, T>
	static T& i(std::optional<std::unique_ptr<T2>&&> i = std::nullopt)
	{
		return *InstanceInternal<T2>(i);
	}

	virtual ~Singleton()
	{
		InstanceInternal().release();
	}

protected:
	template<typename T2 = T> requires std::derived_from<T2, T>
	static std::unique_ptr<T>& InstanceInternal(std::optional<std::unique_ptr<T2>&&> i = std::nullopt)
	{
		static std::unique_ptr<T> i_;

		if constexpr (AutoInit) {
			if (!i_)
				i_ = std::unique_ptr<T>(new T());
		}
		else {
			if (!i_ && i)
				i_ = std::move(*i);
		}

		return i_.get();
	}
};

}