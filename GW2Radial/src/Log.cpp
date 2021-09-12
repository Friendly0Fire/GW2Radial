#include <Log.h>
#include <Utility.h>
#include <Core.h>

extern std::ofstream g_logStream;

namespace GW2Radial
{

Log::Log()
{
#ifdef _DEBUG
    isVisible_ = true;
#endif
}

void Log::Draw()
{
    if (!isVisible_)
        return;

    ImGui::SetNextWindowSize({ 800, 400 }, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("GW2Radial Log Window", &isVisible_))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear"))
        lines_.clear();
    ImGui::SameLine();

    bool scrollDown = false;
    if (ImGui::Checkbox("Autoscroll", &autoscroll_) && autoscroll_)
        scrollDown = true;

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
    ImGui::TextUnformatted("Filters: ");

    auto filter = [&](const char* name, Severity sev) {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ToColor(sev));
        bool v = (uint8_t(sev) & filter_) != 0;
        if (ImGui::Checkbox(name, &v)) {
            if (v)
                filter_ |= uint8_t(sev);
            else
                filter_ &= ~uint8_t(sev);
        }
        ImGui::PopStyleColor();
    };
#ifdef _DEBUG
    filter("Debug", Severity::Debug);
#endif
    filter("Info", Severity::Info);
    filter("Warn", Severity::Warn);
    filter("Error", Severity::Error);

    ImGui::Separator();
    ImGui::BeginChild("logScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushFont(Core::i().fontMono());

    int filtered_size = lines_.size();
    if ((filter_ & uint8_t(Severity::MaxVal)) != uint8_t(Severity::MaxVal)) {
        for (cref l : lines_)
            if ((uint8_t(l.sev) & filter_) == 0)
                filtered_size--;
    }

    if (filtered_size > 0) {
        ImGuiListClipper clipper;
        clipper.Begin(filtered_size);
        while (clipper.Step()) {
            int offset = 0;
            for (int line_no = 0; line_no < clipper.DisplayEnd; )
            {
                cref l = lines_[line_no + offset];
                if ((uint8_t(l.sev) & filter_) == 0) {
                    offset++;
                    continue;
                }

                line_no++;
                if (clipper.DisplayStart >= line_no)
                    continue;

                uint32_t col = (line_no & 1) == 0 ? 0xFFFFFFFF : 0xFFDDDDDD;

                ImGui::PushID(line_no);

                ImGui::PushStyleColor(ImGuiCol_Text, col);
                ImGui::TextUnformatted(l.time.c_str());
                ImGui::SameLine();

                ImGui::PushStyleColor(ImGuiCol_Text, ToColor(l.sev));
                ImGui::TextUnformatted(ToString(l.sev));
                ImGui::SameLine();

                ImGui::PushStyleColor(ImGuiCol_Text, col);
                ImGui::TextUnformatted(l.message.c_str());

                ImGui::PopStyleColor(3);
                ImGui::PopID();
            }
        }
        clipper.End();
    }

    ImGui::PopFont();
    ImGui::PopStyleVar();

    if (scrollDown || autoscroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}

void Log::PrintInternal(Severity sev, const std::string& line)
{
    {
        auto ts = ToString(Timestamp::clock::now());
        std::list<std::string> lines;
        SplitString(line.c_str(), "\n", std::back_inserter(lines));
        for(auto& l : lines)
            lines_.push_back({ sev, ts, l });
    }
    while (lines_.size() > maxLines_)
        lines_.pop_front();

    std::string l = std::format("{}{}{}\n", lines_.back().time, ToString(sev), line);
    OutputDebugStringA(l.c_str());
    logStream() << l.c_str();
}

std::string Log::ToString(const std::string& s)
{
    return s;
}

std::string Log::ToString(const std::wstring& s)
{
    return utf8_encode(s);
}

const char* Log::ToString(Severity sev)
{
    switch (sev)
    {
    default:
    case Severity::Debug:
        return "|DBG] ";
    case Severity::Info:
        return "|INF] ";
    case Severity::Warn:
        return "|WRN] ";
    case Severity::Error:
        return "|ERR] ";
    }
}

uint32_t Log::ToColor(Severity sev)
{
    switch (sev)
    {
    default:
    case Severity::Debug:
        return 0xFFAAAAAA;
    case Severity::Info:
        return 0xFFFFFFFF;
    case Severity::Warn:
        return 0xFF4FE0FF;
    case Severity::Error:
        return 0xFF0000E6;
    }
}

std::string Log::ToString(const Timestamp& t)
{
    return std::format("[{:%T}", t);
}

std::ofstream& Log::logStream()
{
    return g_logStream;
}

}
