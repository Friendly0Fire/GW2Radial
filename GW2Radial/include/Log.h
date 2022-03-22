#pragma once
#include <string>
#include <deque>
#include <format>
#include <fstream>
#include <chrono>

#include <Win.h>
#include <Singleton.h>
#include <mutex>

namespace GW2Radial
{

enum class Severity : uint8_t {
    Debug = 1,
    Info = 2,
    Warn = 4,
    Error = 8,

    MaxVal = Debug | Info | Warn | Error
};

class Log : public Singleton<Log>
{
public:
    using Timestamp = std::chrono::time_point<std::chrono::system_clock>;

    Log();
    ~Log() = default;

    bool isVisible() const { return isVisible_; }
    void isVisible(bool v) { isVisible_ = v; }

    template<typename T, typename... Args>
    void Print(Severity sev, const T& fmt, Args&& ...args) {
#ifndef _DEBUG
        if (sev == Severity::Debug)
            return;
#endif
        std::string l;
        if constexpr (sizeof...(args) > 0)
            l = ToString(std::vformat(fmt, MakeFormatArgs(fmt[0], std::forward<Args>(args)...)));
        else
            l = ToString(fmt);

        PrintInternal(sev, l);
    }

    void Draw();

private:
    void PrintInternal(Severity sev, const std::string& line);

    std::string ToString(const std::string& s);
    std::string ToString(const std::wstring& s);
    std::string ToString(const Timestamp& t);
    const char* ToString(Severity sev);
    uint32_t ToColor(Severity sev);

    template<typename... Args>
    auto MakeFormatArgs(char, Args&& ...args) {
        return std::make_format_args(std::forward<Args>(args)...);
    }
    template<typename... Args>
    auto MakeFormatArgs(wchar_t, Args&& ...args) {
        return std::make_wformat_args(std::forward<Args>(args)...);
    }

    std::ofstream& logStream();

    bool isVisible_ = false;
    bool autoscroll_ = true;
    size_t maxLines_ = 500;
    uint8_t filter_ = uint8_t(Severity::MaxVal);
    struct Line {
        Severity sev;
        std::string time;
        std::string message;
    };
    std::deque<Line> lines_;
    std::mutex linesMutex_;
};

#define DEFINE_LOG(sev, name) \
template<typename T, typename... Args> \
void name(const T& fmt, Args&& ...args) \
{ \
    Log::i().Print(sev, fmt, std::forward<Args>(args)...); \
}

#ifdef _DEBUG
DEFINE_LOG(Severity::Debug, LogDebug);
#else
#define LogDebug(...)
#endif

DEFINE_LOG(Severity::Info, LogInfo);
DEFINE_LOG(Severity::Warn, LogWarn);
DEFINE_LOG(Severity::Error, LogError);

struct LogPtr_
{
    template<typename T>
    const void* operator|(const T* ptr) const
    {
        return static_cast<const void*>(ptr);
    }
};

inline static const LogPtr_ LogPtr;

template<typename T>
auto LogGUID(const GUID& guid)
{
    std::basic_stringstream<T> ss;
    ss << std::hex << std::setw(2) << std::setfill<T>('0');

    for (int i = 0; i < 2; i++)
        ss << guid.Data4[i];
    ss << "-";
    for (int i = 2; i < 8; i++)
        ss << guid.Data4[i];

    if constexpr (std::is_same_v<char, T>)
        return std::format("{{{:x}-{:x}-{:x}-{}}}", guid.Data1, guid.Data2, guid.Data3, ss.str());
    else
        return std::format(L"{{{:x}-{:x}-{:x}-{}}}", guid.Data1, guid.Data2, guid.Data3, ss.str());
}

}