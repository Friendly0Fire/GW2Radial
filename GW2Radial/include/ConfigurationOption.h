#pragma once

#include <Main.h>
#include <ConfigurationFile.h>
#include <type_traits>

namespace GW2Radial
{

template<typename T>
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
	void value(const T &value) { value_ = value; ForceSave(); }

	void Reload()
	{
		LoadValue();
	}

	void ForceSave() const
	{
		SaveValue();
		ConfigurationFile::i()->Save();
	}

protected:

	// ReSharper disable once CppMemberFunctionMayBeStatic
	// ReSharper disable once CppMemberFunctionMayBeConst
	void LoadValue()
	{
		static_assert(false, "Unsupported value type");
	}

	// ReSharper disable once CppMemberFunctionMayBeStatic
	void SaveValue() const
	{
		static_assert(false, "Unsupported value type");
	}

	std::string displayName_, nickname_, category_;
	T value_;
};

template<>
inline void ConfigurationOption<int>::LoadValue()
{
	value_ = ConfigurationFile::i()->ini().GetLongValue(category_.c_str(), nickname_.c_str(), value());
}

template<>
inline void ConfigurationOption<double>::LoadValue()
{
	value_ = ConfigurationFile::i()->ini().GetDoubleValue(category_.c_str(), nickname_.c_str(), value());
}

template<>
inline void ConfigurationOption<float>::LoadValue()
{
	value_ = float(ConfigurationFile::i()->ini().GetDoubleValue(category_.c_str(), nickname_.c_str(), value()));
}

template<>
inline void ConfigurationOption<bool>::LoadValue()
{
	value_ = ConfigurationFile::i()->ini().GetBoolValue(category_.c_str(), nickname_.c_str(), value());
}

template<>
inline void ConfigurationOption<const char*>::LoadValue()
{
	value_ = ConfigurationFile::i()->ini().GetValue(category_.c_str(), nickname_.c_str(), value());
}

template<>
inline void ConfigurationOption<int>::SaveValue() const
{
	ConfigurationFile::i()->ini().SetLongValue(category_.c_str(), nickname_.c_str(), value());
}

template<>
inline void ConfigurationOption<double>::SaveValue() const
{
	ConfigurationFile::i()->ini().SetDoubleValue(category_.c_str(), nickname_.c_str(), value());
}

template<>
inline void ConfigurationOption<float>::SaveValue() const
{
	ConfigurationFile::i()->ini().SetDoubleValue(category_.c_str(), nickname_.c_str(), double(value()));
}

template<>
inline void ConfigurationOption<bool>::SaveValue() const
{
	ConfigurationFile::i()->ini().SetBoolValue(category_.c_str(), nickname_.c_str(), value());
}

template<>
inline void ConfigurationOption<const char*>::SaveValue() const
{
	ConfigurationFile::i()->ini().SetValue(category_.c_str(), nickname_.c_str(), value());
}

}