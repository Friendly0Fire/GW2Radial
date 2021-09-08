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
    ImGui::Checkbox("Autoscroll", &autoscroll_);

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
    filter("Debug", Severity::Debug);
    filter("Info", Severity::Info);
    filter("Warn", Severity::Warn);
    filter("Error", Severity::Error);

    ImGui::Separator();
    ImGui::BeginChild("logScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushFont(Core::i().fontMono());

    ImGuiListClipper clipper;
    clipper.Begin(lines_.size(), ImGui::GetTextLineHeightWithSpacing());
    while (clipper.Step()) {
        for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
        {
            cref l = lines_[line_no];
            if ((uint8_t(l.sev) & filter_) == 0)
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

    ImGui::PopFont();
    ImGui::PopStyleVar();

    if (autoscroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
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
