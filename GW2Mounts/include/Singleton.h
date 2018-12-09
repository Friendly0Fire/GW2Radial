#pragma once
#include <memory>

namespace GW2Addons
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

protected:
	static std::unique_ptr<T> i_;
};

#define DEFINE_SINGLETON(x) std::unique_ptr<x> Singleton<x>::i_ = nullptr;

}