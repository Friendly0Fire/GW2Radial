#pragma once

#include <Main.h>
#include <Singleton.h>

#include <filesystem>
#include <map>

#include <ZipFile.h>

#include <ShlObj.h>

namespace GW2Radial
{
    class FileSystem : public Singleton<FileSystem>
    {
        std::map<std::filesystem::path, ZipArchive::Ptr> archiveCache_;

        ZipArchive::Ptr FindOrCache(const std::filesystem::path& p);

        static std::pair<std::filesystem::path, std::filesystem::path> SplitZipPath(const std::filesystem::path& p, ZipArchive::Ptr* zip = nullptr);
    public:

        static bool Exists(const std::filesystem::path& p);
        static std::vector<byte> ReadFile(const std::filesystem::path& p);
        static std::vector<byte> ReadFile(std::istream& is);
        static std::filesystem::path GetSystemPath(REFKNOWNFOLDERID id, DWORD flags = KF_FLAG_DEFAULT);
        static std::vector<std::filesystem::path> IterateZipFolders(const std::filesystem::path& p);
    };
}
