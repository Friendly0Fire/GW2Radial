#pragma once
#include <memory>
#include <concepts>

namespace GW2Radial
{

template<typename T, bool AutoInit = true>
class Singleton
{
public:
	static T* i()
	{
		if constexpr(AutoInit) {
		    if(!i_)
			    i_ = std::unique_ptr<T>(new T());
		}
		return i_.get();
	}

	template<typename T2> requires std::derived_from<T2, T>
	static T* i(std::unique_ptr<T2>&& i)
	{
	    if(!i_)
			i_ = std::move(i);
		return i_.get();
	}

	static T* iNoInit()
	{
		if(!i_)
			return nullptr;
		return i_.get();
	}

	virtual ~Singleton()
	{
		i_.release();
	}

protected:
	static std::unique_ptr<T> i_;
};

#define DEFINE_SINGLETON(x) std::unique_ptr<x> Singleton<x>::i_ = nullptr
#define DEFINE_SINGLETON_NOINIT(x) std::unique_ptr<x> Singleton<x, false>::i_ = nullptr

}