#include <ScanCode.h>

namespace GW2Radial
{
ScanCode GetScanCode(KeyLParam lParam) {
    uint scanCode = lParam.scanCode;
    if (lParam.extendedFlag)
    {
        if (scanCode != 0x45)
            scanCode |= 0xE000;
    } else
    {
        if (scanCode == 0x45)
            scanCode = 0xE11D45;
        else if (scanCode == 0x54)
            scanCode = 0xE037;
    }

    return ScanCode(scanCode);
}

std::wstring GetScanCodeName(ScanCode scanCode) {
	if(IsMouse(scanCode))
	{
	    switch(scanCode)
	    {
		case ScanCode::LBUTTON:
			return L"M1";
		case ScanCode::RBUTTON:
			return L"M2";
		case ScanCode::MBUTTON:
			return L"M3";
		case ScanCode::X1BUTTON:
			return L"M4";
		case ScanCode::X2BUTTON:
			return L"M5";
		default:
			return L"[Error]";
	    }
	}

	if (scanCode >= ScanCode::F13 && scanCode <= ScanCode::F23)
		return L"F" + std::to_wstring(uint(scanCode) - uint(ScanCode::F13) + 13);
	if (scanCode == ScanCode::F24)
		return L"F24";

	if (scanCode >= ScanCode::NUMROW_1 && scanCode <= ScanCode::NUMROW_9) {
		wchar_t c = wchar_t(scanCode) - 1 + 0x30;
		return std::wstring(1, c);
	}
	if (scanCode == ScanCode::NUMROW_0)
		return L"0";

	if (IsUniversal(scanCode)) {
		switch (scanCode) {
		case ScanCode::SHIFT:
			return L"SHIFT";
		case ScanCode::CONTROL:
			return L"CONTROL";
		case ScanCode::ALT:
			return L"ALT";
		case ScanCode::META:
			return L"META";
		}
	}

	wchar_t keyName[50];
	LPARAM lParam = 0;
	auto& lp = KeyLParam::Get(lParam);
	lp.scanCode = uint(scanCode);
	lp.extendedFlag = IsExtendedKey(scanCode) ? 1 : 0;
	// = (uint(scanCode) & 0xFF) << 16 | (IsExtendedKey(scanCode) ? 1 : 0) << 24;
	if (GetKeyNameTextW(lParam, keyName, int(std::size(keyName))) != 0)
		return keyName;
	else {
		auto err = GetLastError();
		Log::i().Print(Severity::Warn, L"Could not get key name for scan code {}, error 0x{:x}.", uint(scanCode), err);
	}

	return L"[Error]";
}
}