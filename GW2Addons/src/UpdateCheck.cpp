#include <UpdateCheck.h>
#include <Tag.h>
#include <nlohmann/json.hpp>
#include <Utility.h>
#include <ImGuiExtensions.h>
#include <WinInet.h>
#include <sstream>

namespace GW2Addons
{
DEFINE_SINGLETON(UpdateCheck);

UpdateCheck::UpdateCheck()
	: checkEnabled_("Automatically check for update", "check_for_updates", "Core", true)
{
	SettingsMenu::i()->AddImplementer(this);
}

UpdateCheck::~UpdateCheck()
{
	if(auto i = SettingsMenu::iNoInit(); i)
		i->RemoveImplementer(this);
}

void UpdateCheck::CheckForUpdates()
{
	using json = nlohmann::json;

	const auto currentTime = TimeInMilliseconds();
	if(lastCheckTime_ + checkTimeSpan_ > currentTime)
		return;

	lastCheckTime_ = currentTime;
	
	if(checkSucceeded_ || checkAttempts_ >= maxCheckAttempts_ || !checkEnabled_.value())
		return;
	
	checkAttempts_++;

	try
	{
		auto data = FetchReleaseData();
		if(data.empty())
			return;

		auto j = json::parse(data);

		auto tagName = j["tag_name"].get<std::string>();

		if(std::lexicographical_compare(CurrentVersion.begin(), CurrentVersion.end(), tagName.begin(), tagName.end()))
			updateAvailable_ = true;

		checkSucceeded_ = true;
	}
	catch(...)
	{
		checkSucceeded_ = false;
	}
}

void UpdateCheck::DrawMenu()
{
	ImGuiConfigurationWrapper(ImGui::Checkbox, "Automatically check for updates", checkEnabled_);
}

// Retrieving Headers Using a Constant
BOOL DumpHeaders(HINTERNET hRequest)
{
   char* lpOutBuffer=NULL;
   DWORD dwSize = 0;

retry:

   // This call will fail on the first pass, because
   // no buffer is allocated.
   if(!HttpQueryInfo(hRequest,HTTP_QUERY_RAW_HEADERS_CRLF,
      (LPVOID)lpOutBuffer,&dwSize,NULL))
   {
      if (GetLastError()==ERROR_HTTP_HEADER_NOT_FOUND)
      {
         // Code to handle the case where the header isn't available.
         return TRUE;
      }
      else
      {
        // Check for an insufficient buffer.
        if (GetLastError()==ERROR_INSUFFICIENT_BUFFER)
        {
            // Allocate the necessary buffer.
            lpOutBuffer = new char[dwSize];

            // Retry the call.
            goto retry;
        }
        else
        {
            // Error handling code.
            if (lpOutBuffer)
            {
               delete [] lpOutBuffer;
            }
            return FALSE;
        }
      }
   }

	OutputDebugStringW((wchar_t*)lpOutBuffer);

   if (lpOutBuffer)
   {
      delete [] lpOutBuffer;
   }

   return TRUE;
}

std::string UpdateCheck::FetchReleaseData() const
{
	std::string retVal;
    if(const auto hInternet = InternetOpen(L"GW2Radial", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0); hInternet)
    {
        if(const auto hConnection = InternetConnect(hInternet, L"api.github.com", INTERNET_DEFAULT_HTTPS_PORT, L"", L"", INTERNET_SERVICE_HTTP, 0, 0); hConnection)
        {
            const auto hRequest = HttpOpenRequest(hConnection, L"GET", L"repos/Friendly0Fire/GW2Addons/releases/latest",
				nullptr,
				nullptr,
				nullptr,
                INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_UI | INTERNET_FLAG_SECURE, 0);
            if(hRequest)
            {
                if(HttpSendRequest(hRequest, nullptr, 0, nullptr, 0))
                {
					DWORD statusCode = 0;
					DWORD statusCodeLen = sizeof(statusCode);
					const auto httpQueryInfoReceived = HttpQueryInfo(hRequest,
				        HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
				        &statusCode, &statusCodeLen, nullptr);
					if(httpQueryInfoReceived && statusCode == 200)
					{
						std::stringstream ss;

	                    DWORD availDataLen;
	                    while(InternetQueryDataAvailable(hRequest, &availDataLen, 0, 0) && availDataLen > 0)
	                    {
							DWORD readCount;
							std::vector<char> data;
							data.resize(availDataLen);
		                    InternetReadFile(hRequest, data.data(), data.size(), &readCount);

		                    ss.write(data.data(), data.size());
	                    }

						retVal = ss.str();
					}
                }
                InternetCloseHandle(hRequest);
            }
            InternetCloseHandle(hConnection);
        }
        InternetCloseHandle(hInternet);
    }

    return retVal;
}

}
