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
	ConfigurationOption(ConfigurationOption&&) = default;

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
		ConfigurationFile::i().Save();
	}

protected:
	void LoadValue()
	{
		if constexpr (std::is_same_v<T, int>)
			value_ = ConfigurationFile::i().ini().GetLongValue(category_.c_str(), nickname_.c_str(), value());
		else if constexpr (std::is_same_v<T, double>)
			value_ = ConfigurationFile::i().ini().GetDoubleValue(category_.c_str(), nickname_.c_str(), value());
		else if constexpr (std::is_same_v<T, float>)
			value_ = float(ConfigurationFile::i().ini().GetDoubleValue(category_.c_str(), nickname_.c_str(), value()));
		else if constexpr (std::is_same_v<T, bool>)
			value_ = ConfigurationFile::i().ini().GetBoolValue(category_.c_str(), nickname_.c_str(), value());
		else if constexpr (std::is_same_v<T, const char*>)
			value_ = ConfigurationFile::i().ini().GetValue(category_.c_str(), nickname_.c_str(), value());
		else
			static_assert(!sizeof(T), "Unsupported value type");
	}

	void SaveValue() const
	{
		if constexpr (std::is_same_v<T, int>)
			ConfigurationFile::i().ini().SetLongValue(category_.c_str(), nickname_.c_str(), value());
		else if constexpr (std::is_same_v<T, double>)
			ConfigurationFile::i().ini().SetDoubleValue(category_.c_str(), nickname_.c_str(), value());
		else if constexpr (std::is_same_v<T, float>)
			ConfigurationFile::i().ini().SetDoubleValue(category_.c_str(), nickname_.c_str(), double(value()));
		else if constexpr (std::is_same_v<T, bool>)
			ConfigurationFile::i().ini().SetBoolValue(category_.c_str(), nickname_.c_str(), value());
		else if constexpr (std::is_same_v<T, const char*>)
			ConfigurationFile::i().ini().SetValue(category_.c_str(), nickname_.c_str(), value());
		else
			static_assert(!sizeof(T), "Unsupported value type");
	}

	std::string displayName_, nickname_, category_;
	T value_;
};

}