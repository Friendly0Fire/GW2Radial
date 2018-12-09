#pragma once

#include <Main.h>
#include <ConfigurationFile.h>

namespace GW2Addons
{

template<typename T, typename FallbackT = T>
class ConfigurationOption
{
public:
	ConfigurationOption(std::string displayName, std::string nickname, std::string category, T defaultValue = T())
		: displayName_(std::move(displayName)), nickname_(std::move(nickname)), category_(std::move(category)), value_(defaultValue)
	{
		LoadValue();
	}

	const std::string & displayName() const { return displayName_; }
	void displayName(const std::string &displayName) { displayName_ = displayName; }

	const std::string & category() const { return category_; }
	void category(const std::string &category) { category_ = category; }
	
	const T & value() const { return value_; }
	T & value() { return value_; }
	void value(const T &value) { value_ = value; SaveValue(); }
	
	const FallbackT& fallbackValue() const { return static_cast<FallbackT&>(value_); }
	FallbackT& fallbackValue() { return static_cast<FallbackT&>(value_); }
	void fallbackValue(const FallbackT &value) { value_ = T(value); SaveValue(); }

	void Reload()
	{
		LoadValue();
	}

	void ForceSave()
	{
		SaveValue();
	}

protected:

	// ReSharper disable once CppMemberFunctionMayBeStatic
	// ReSharper disable once CppMemberFunctionMayBeConst
	void LoadValue()
	{
		throw std::invalid_argument("Unsupported value type");
	}

	// ReSharper disable once CppMemberFunctionMayBeStatic
	void SaveValue() const
	{
		throw std::invalid_argument("Unsupported value type");
	}

	std::string displayName_, nickname_, category_;
	T value_;
};

template<>
inline void ConfigurationOption<int>::LoadValue()
{
	value_ = ConfigurationFile::i()->ini().GetLongValue(category_.c_str(), nickname_.c_str(), value_);
}

template<>
inline void ConfigurationOption<double>::LoadValue()
{
	value_ = ConfigurationFile::i()->ini().GetDoubleValue(category_.c_str(), nickname_.c_str(), value_);
}

template<>
inline void ConfigurationOption<bool>::LoadValue()
{
	value_ = ConfigurationFile::i()->ini().GetBoolValue(category_.c_str(), nickname_.c_str(), value_);
}

template<>
inline void ConfigurationOption<const char*>::LoadValue()
{
	value_ = ConfigurationFile::i()->ini().GetValue(category_.c_str(), nickname_.c_str(), value_);
}

template<>
inline void ConfigurationOption<int>::SaveValue() const
{
	ConfigurationFile::i()->ini().SetLongValue(category_.c_str(), nickname_.c_str(), value_);
}

template<>
inline void ConfigurationOption<double>::SaveValue() const
{
	ConfigurationFile::i()->ini().SetDoubleValue(category_.c_str(), nickname_.c_str(), value_);
}

template<>
inline void ConfigurationOption<bool>::SaveValue() const
{
	ConfigurationFile::i()->ini().SetBoolValue(category_.c_str(), nickname_.c_str(), value_);
}

template<>
inline void ConfigurationOption<const char*>::SaveValue() const
{
	ConfigurationFile::i()->ini().SetValue(category_.c_str(), nickname_.c_str(), value_);
}

}