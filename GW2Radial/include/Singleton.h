#pragma once
#include <memory>

namespace GW2Radial
{

template<typename T>
class Singleton
{
public:
	static T* i()
	{
		if(!i_)
			i_ = std::unique_ptr<T>(new T());
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

}