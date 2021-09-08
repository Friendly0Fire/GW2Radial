#pragma once
#include <string>
#include <deque>
#include <format>
#include <fstream>
#include <chrono>

#include <Win.h>
#include <Singleton.h>

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

    template<typename T, typename... Args>
    void Print(Severity sev, const T& fmt, Args&& ...args) {
        std::string l;
        if constexpr (sizeof...(args) > 0)
            l = ToString(std::vformat(fmt, MakeFormatArgs(fmt[0], std::forward<Args>(args)...)));
        else
            l = ToString(fmt);

        lines_.push_back({ sev, ToString(Timestamp::clock::now()), l });
        while (lines_.size() > maxLines_)
            lines_.pop_front();

        l = std::format("{}{}{}\n", lines_.back().time, ToString(sev), lines_.back().message);
        OutputDebugStringA(l.c_str());
        logStream() << l.c_str();
    }

    void Draw();

private:
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
};

}