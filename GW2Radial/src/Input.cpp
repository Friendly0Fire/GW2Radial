#include <Input.h>
#include <imgui.h>
#include <Utility.h>
#include <Core.h>
#include <algorithm>
#include <SettingsMenu.h>

IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace GW2Radial
{

std::wstring GetScanCodeName(ScanCode scanCode) { return GetScanCodeName(static_cast<uint>(scanCode)); }

ScanCode GetScanCode(KeyLParam lParam) {
	if (lParam.extendedFlag) {
		if (lParam.scanCode != 0x45)
			lParam.scanCode |= 0xE000;
	} else {
		if (lParam.scanCode == 0x45)
			lParam.scanCode = 0xE11D45;
		else if (lParam.scanCode == 0x54)
			lParam.scanCode = 0xE037;
	}

	return ScanCode(lParam.scanCode);
}

DEFINE_SINGLETON(Input);

Input::Input()
{
	id_H_LBUTTONDOWN_ = RegisterWindowMessage(TEXT("H_LBUTTONDOWN"));
	id_H_LBUTTONUP_   = RegisterWindowMessage(TEXT("H_LBUTTONUP"));
	id_H_RBUTTONDOWN_ = RegisterWindowMessage(TEXT("H_RBUTTONDOWN"));
	id_H_RBUTTONUP_   = RegisterWindowMessage(TEXT("H_RBUTTONUP"));
	id_H_MBUTTONDOWN_ = RegisterWindowMessage(TEXT("H_MBUTTONDOWN"));
	id_H_MBUTTONUP_   = RegisterWindowMessage(TEXT("H_MBUTTONUP"));
	id_H_XBUTTONDOWN_ = RegisterWindowMessage(TEXT("H_XBUTTONDOWN"));
	id_H_XBUTTONUP_   = RegisterWindowMessage(TEXT("H_XBUTTONUP"));
	id_H_SYSKEYDOWN_  = RegisterWindowMessage(TEXT("H_SYSKEYDOWN"));
	id_H_SYSKEYUP_    = RegisterWindowMessage(TEXT("H_SYSKEYUP"));
	id_H_KEYDOWN_     = RegisterWindowMessage(TEXT("H_KEYDOWN"));
	id_H_KEYUP_       = RegisterWindowMessage(TEXT("H_KEYUP"));
	id_H_MOUSEMOVE_   = RegisterWindowMessage(TEXT("H_MOUSEMOVE"));
}

WPARAM MapLeftRightKeys(WPARAM vk, LPARAM lParam)
{
    WPARAM newVk = vk;
    const UINT scancode = (lParam & 0x00ff0000) >> 16;
    const int extended  = (lParam & 0x01000000) != 0;

    switch (vk) {
    case VK_SHIFT:
        newVk = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
        break;
    case VK_CONTROL:
        newVk = extended ? VK_RCONTROL : VK_LCONTROL;
        break;
    case VK_MENU:
        newVk = extended ? VK_RMENU : VK_LMENU;
        break;
    default:
        // not a key we map from generic to left/right specialized
        //  just return it.
        newVk = vk;
        break;    
    }

    return newVk;
}

bool IsRawInputMouse(LPARAM lParam)
{
	UINT dwSize = 40;
	static BYTE lpb[40];

	GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT,
		lpb, &dwSize, sizeof(RAWINPUTHEADER));

	auto* raw = reinterpret_cast<RAWINPUT*>(lpb);

	return raw->header.dwType == RIM_TYPEMOUSE;
}

bool Input::OnInput(UINT& msg, WPARAM& wParam, LPARAM& lParam)
{
	std::list<EventKey> eventKeys;

	// Generate our EventKey list for the current message
	{
		bool eventDown = false;
		switch (msg)
		{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			eventDown = true;
		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			auto& keylParam = KeyLParam::Get(lParam);
			const ScanCode sc = GetScanCode(keylParam);

			if ((msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP) && wParam != VK_F10)
			{
				if (keylParam.contextCode == 1)
					eventKeys.push_back({ sc, true });
				else
					eventKeys.push_back({ sc, false });
			}
			
			eventKeys.push_back({ sc, eventDown });
			break;
		}
		case WM_LBUTTONDOWN:
			eventDown = true;
		case WM_LBUTTONUP:
			eventKeys.push_back({ ScanCode::LBUTTON, eventDown });
			break;
		case WM_MBUTTONDOWN:
			eventDown = true;
		case WM_MBUTTONUP:
			eventKeys.push_back({ ScanCode::MBUTTON, eventDown });
			break;
		case WM_RBUTTONDOWN:
			eventDown = true;
		case WM_RBUTTONUP:
			eventKeys.push_back({ ScanCode::RBUTTON, eventDown });
			break;
		case WM_XBUTTONDOWN:
			eventDown = true;
		case WM_XBUTTONUP:
			eventKeys.push_back({ uint(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? ScanCode::X1BUTTON : ScanCode::X2BUTTON), eventDown });
			break;
		}
	}

	const auto isRawInputMouse = msg == WM_INPUT && IsRawInputMouse(lParam);

	bool preventMouseMove = false;
	if(msg == WM_MOUSEMOVE || isRawInputMouse)
		for(auto& cb : mouseMoveCallbacks_)
			preventMouseMove |= (*cb)();

	bool downKeysChanged = false;

	// Apply key events now
	for (const auto& k : eventKeys)
		if (k.down)
			downKeysChanged |= DownKeys.insert(k.sc).second;
		else
			downKeysChanged |= DownKeys.erase(k.sc) > 0;


	InputResponse response = InputResponse::PASS_TO_GAME;
	// Only run these for key down/key up (incl. mouse buttons) events
	if(!eventKeys.empty())
		for(auto& cb : inputChangeCallbacks_)
			response |= (*cb)(downKeysChanged, DownKeys, eventKeys);

	ImGui_ImplWin32_WndProcHandler(Core::i()->gameWindow(), msg, wParam, lParam);
	if (msg == WM_MOUSEMOVE)
	{
		auto& io = ImGui::GetIO();
		io.MousePos.x = static_cast<signed short>(lParam);
		io.MousePos.y = static_cast<signed short>(lParam >> 16);
	}

	if(response == InputResponse::PREVENT_ALL)
		return true;

	if(response == InputResponse::PREVENT_MOUSE || preventMouseMove)
	{
		switch (msg)
		{
		case WM_MOUSEMOVE:
			return true;
		case WM_INPUT:
			if(isRawInputMouse)
				return true;
			break;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDBLCLK:
		{
			const auto& io2 = ImGui::GetIO();

			short mx, my;
			mx = (short)io2.MousePos.x;
			my = (short)io2.MousePos.y;
			lParam = MAKELPARAM(mx, my);
			break;
		}
		}
	}

	// Prevent game from receiving input if ImGui requests capture
	const auto& io = ImGui::GetIO();
	switch (msg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
		if (io.WantCaptureMouse)
			return true;
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		if (io.WantCaptureKeyboard)
			return true;
		break;
	case WM_CHAR:
		if (io.WantTextInput)
			return true;
		break;
	}

	// Convert hook messages back into their original messages
	msg = ConvertHookedMessage(msg);

	return false;
}

void Input::OnFocusLost()
{
	DownKeys.clear();
}

void Input::OnUpdate()
{
	SendQueuedInputs();
}

uint Input::ConvertHookedMessage(uint msg) const
{
	if (msg == id_H_LBUTTONDOWN_)
		return WM_LBUTTONDOWN;
	if (msg == id_H_LBUTTONUP_)
		return WM_LBUTTONUP;
	if (msg == id_H_RBUTTONDOWN_)
		return WM_RBUTTONDOWN;
	if (msg == id_H_RBUTTONUP_)
		return WM_RBUTTONUP;
	if (msg == id_H_MBUTTONDOWN_)
		return WM_MBUTTONDOWN;
	if (msg == id_H_MBUTTONUP_)
		return WM_MBUTTONUP;
	if (msg == id_H_XBUTTONDOWN_)
		return WM_XBUTTONDOWN;
	if (msg == id_H_XBUTTONUP_)
		return WM_XBUTTONUP;
	if (msg == id_H_SYSKEYDOWN_)
		return WM_SYSKEYDOWN;
	if (msg == id_H_SYSKEYUP_)
		return WM_SYSKEYUP;
	if (msg == id_H_KEYDOWN_)
		return WM_KEYDOWN;
	if (msg == id_H_KEYUP_)
		return WM_KEYUP;
	if (msg == id_H_MOUSEMOVE_)
		return WM_MOUSEMOVE;

	return msg;
}

Input::DelayedInput Input::TransformScanCode(ScanCode sc, bool down, mstime t, const std::optional<Point>& cursorPos)
{
	DelayedInput i { };
	i.t = t;
	i.cursorPos = cursorPos;
	if (IsMouse(sc))
	{
		std::tie(i.wParam, i.lParamValue) = CreateMouseEventParams(cursorPos);
	}
	else
	{
		bool isUniversal = IsUniversalModifier(sc);
		if (isUniversal)
			sc = sc & ~ScanCode::UNIVERSAL_MODIFIER_FLAG;

		i.wParam = MapVirtualKey(uint(sc), isUniversal ? MAPVK_VSC_TO_VK : MAPVK_VSC_TO_VK_EX);
		assert(i.wParam != 0);
		i.lParamKey.repeatCount = 0;
		i.lParamKey.scanCode = uint(sc);
		i.lParamKey.extendedFlag = IsExtendedKey(sc) ? 1 : 0;
		i.lParamKey.contextCode = 0;
		i.lParamKey.previousKeyState = down ? 0 : 1;
		i.lParamKey.transitionState = down ? 0 : 1;
	}

	switch (sc)
	{
		case ScanCode::LBUTTON:
			i.msg = down ? id_H_LBUTTONDOWN_ : id_H_LBUTTONUP_;
			break;
		case ScanCode::MBUTTON:
			i.msg = down ? id_H_MBUTTONDOWN_ : id_H_MBUTTONUP_;
			break;
		case ScanCode::RBUTTON:
			i.msg = down ? id_H_RBUTTONDOWN_ : id_H_RBUTTONUP_;
			break;
		case ScanCode::X1BUTTON:
		case ScanCode::X2BUTTON:
			i.msg = down ? id_H_XBUTTONDOWN_ : id_H_XBUTTONUP_;
			i.wParam |= (WPARAM)(IsSame(sc, ScanCode::X1BUTTON) ? XBUTTON1 : XBUTTON2) << 16;
			break;
		case ScanCode::ALTLEFT:
		case ScanCode::ALTRIGHT:
		case ScanCode::F10:
			i.msg = down ? id_H_SYSKEYDOWN_ : id_H_SYSKEYUP_;
			break;
		default:
			i.msg = down ? id_H_KEYDOWN_ : id_H_KEYUP_;
			break;
	}

	return i;
}

std::tuple<WPARAM, LPARAM> Input::CreateMouseEventParams(const std::optional<Point>& cursorPos) const
{
	WPARAM wParam = 0;
	if (DownKeys.count(ScanCode::CONTROLLEFT) || DownKeys.count(ScanCode::CONTROLRIGHT))
		wParam += MK_CONTROL;
	if (DownKeys.count(ScanCode::SHIFTLEFT) || DownKeys.count(ScanCode::SHIFTRIGHT))
		wParam += MK_SHIFT;
	if (DownKeys.count(ScanCode::LBUTTON))
		wParam += MK_LBUTTON;
	if (DownKeys.count(ScanCode::RBUTTON))
		wParam += MK_RBUTTON;
	if (DownKeys.count(ScanCode::MBUTTON))
		wParam += MK_MBUTTON;
	if (DownKeys.count(ScanCode::X1BUTTON))
		wParam += MK_XBUTTON1;
	if (DownKeys.count(ScanCode::X2BUTTON))
		wParam += MK_XBUTTON2;

	const auto& io = ImGui::GetIO();

	LPARAM lParam = MAKELPARAM(cursorPos ? cursorPos->x : (static_cast<int>(io.MousePos.x)), cursorPos ? cursorPos->y : (static_cast<int>(io.MousePos.y)));
	return { wParam, lParam };
}

void Input::SendKeybind(const std::set<ScanCode> &scs, const std::optional<Point>& cursorPos)
{
	if (scs.empty())
		return;

	std::list<ScanCode> scsSorted(scs.begin(), scs.end());

	if (scsSorted.size() > 1) {
		auto removeGeneric = [&](ScanCode g, ScanCode r) {
			if (const auto it = std::find(scsSorted.begin(), scsSorted.end(), g); it != scsSorted.end()) {
				scsSorted.erase(it);
				scsSorted.push_back(r);
			}
		};
		removeGeneric(ScanCode::SHIFT, ScanCode::SHIFTLEFT);
		removeGeneric(ScanCode::CONTROL, ScanCode::CONTROLLEFT);
		removeGeneric(ScanCode::ALT, ScanCode::ALTLEFT);
		removeGeneric(ScanCode::META, ScanCode::METALEFT);
	}

	{
		std::set<ScanCode> modifiers{ ScanCode::CONTROLLEFT, ScanCode::CONTROLRIGHT, ScanCode::SHIFTLEFT, ScanCode::SHIFTRIGHT, ScanCode::ALTLEFT, ScanCode::ALTRIGHT };
		scsSorted.sort([&modifiers](ScanCode& a, ScanCode& b) {
			if (modifiers.count(a))
				return true;
			else
				return a < b;
			});
	}

	mstime currentTime = TimeInMilliseconds() + 10;

	// GW2 only distinguishes left/right modifiers when not used with other keys
	bool useUniversalModifiers = false;
	for (const auto& sc : scsSorted)
	{
		if (!IsModifier(sc)) {
			useUniversalModifiers = true;
			break;
		}
	}

	for (const auto &sc : scsSorted)
	{
		if (DownKeys.count(sc))
			continue;

		auto sc2 = useUniversalModifiers ? MakeUniversal(sc) : sc;

		DelayedInput i = TransformScanCode(sc2, true, currentTime, cursorPos);
		if(i.wParam != 0)
			QueuedInputs.push_back(i);
		currentTime += 20;
	}

	currentTime += 50;

	for (const auto &sc : reverse(scsSorted))
	{
		if (DownKeys.count(sc))
			continue;

		DelayedInput i = TransformScanCode(sc, false, currentTime, cursorPos);
		if (i.wParam != 0)
			QueuedInputs.push_back(i);
		currentTime += 20;
	}
}

void Input::SendQueuedInputs()
{
	if (QueuedInputs.empty())
		return;

	const auto currentTime = TimeInMilliseconds();

	auto &qi = QueuedInputs.front();

	if (currentTime < qi.t)
		return;

	if (qi.cursorPos)
		SetCursorPos(qi.cursorPos->x, qi.cursorPos->y);

	PostMessage(Core::i()->gameWindow(), qi.msg, qi.wParam, qi.lParamValue);

	QueuedInputs.pop_front();
}
}
