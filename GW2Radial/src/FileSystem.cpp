#include <FileSystem.h>
#include <optional>
#include <fstream>

namespace fs = std::filesystem;

namespace GW2Radial
{
    ZipArchive::Ptr FileSystem::FindOrCache(const fs::path& p)
    {
        if(auto arc = archiveCache_.find(p); arc != archiveCache_.end())
            return arc->second;
        
        auto zip = ZipFile::Open(p.string());
        if(!zip)
            return nullptr;

        archiveCache_.insert({ p, zip });
        return zip;
    }

    std::pair<fs::path, fs::path> FileSystem::SplitZipPath(const fs::path& p, ZipArchive::Ptr* zip)
    {
        auto p2 = p;
        do
        {
            p2 = p2.parent_path();
        }
        while (p2.has_relative_path() && !fs::exists(p2));

        if (p2.has_extension() && p2.extension() == L".zip")
        {
            auto& fs = i();

            auto z = fs.FindOrCache(p2.string());
            if(z) {
                if(zip) *zip = z;
                return { p2, fs::relative(p, p2) };
            }
        }

        if(zip) *zip = nullptr;
        return { p, fs::path() };
    }

    bool FileSystem::Exists(const fs::path& p)
    {
        if (fs::exists(p))
            return true;
        if (!p.has_relative_path())
            return false;
        if (p.wstring().find(L".zip") == std::wstring::npos)
            return false;

        ZipArchive::Ptr zip;
        cref [base, sub] = SplitZipPath(p, &zip);
        if(!zip) return false;

        return zip->GetEntry(sub.string()) != nullptr;
    }

    std::vector<byte> FileSystem::ReadFile(std::istream& is)
    {
        std::vector<byte> output;
        output.reserve(1024);

        byte buffer[1024];
        size_t n = 0;
        do
        {
            is.read(reinterpret_cast<char*>(buffer), 1024);
            n = is.gcount();
            output.insert(output.end(), std::begin(buffer), std::begin(buffer) + n);
        } while(n == 1024);

        return output;
    }

    std::filesystem::path FileSystem::GetSystemPath(const KNOWNFOLDERID& id, DWORD flags) {
        wchar_t* path = nullptr;
        SHGetKnownFolderPath(id, flags, nullptr, &path);

        std::filesystem::path p(path);
        CoTaskMemFree(path);
        return p;
    }

    std::vector<byte> FileSystem::ReadFile(const fs::path& p)
    {
        if (fs::exists(p))
        {
            std::ifstream is(p.wstring().c_str(), std::ifstream::binary);
            if(is.bad())
                return {};

            return ReadFile(is);
        }
        
        ZipArchive::Ptr zip;
        cref [base, sub] = SplitZipPath(p, &zip);

        auto entry = zip->GetEntry(sub.string());
        if(!entry) return {};

        auto* decompress = entry->GetDecompressionStream();
        if(!decompress) return {};

        auto v = ReadFile(*decompress);

        entry->CloseDecompressionStream();

        return v;
    }
}
