#include <Input.h>
#include <imgui.h>
#include <Utility.h>
#include <Core.h>
#include <algorithm>
#include <SettingsMenu.h>

IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace GW2Radial
{

DEFINE_SINGLETON(Input);

Input::Input()
	: distinguishLeftRight_("Distinguish between left and right Shift/Ctrl/Alt", "distinguish_lr", "Core", false)
{
	id_H_LBUTTONDOWN_ = RegisterWindowMessage(TEXT("H_LBUTTONDOWN"));
	id_H_LBUTTONUP_   = RegisterWindowMessage(TEXT("H_LBUTTONUP"));
	id_H_RBUTTONDOWN_ = RegisterWindowMessage(TEXT("H_RBUTTONDOWN"));
	id_H_RBUTTONUP_   = RegisterWindowMessage(TEXT("H_RBUTTONUP"));
	id_H_MBUTTONDOWN_ = RegisterWindowMessage(TEXT("H_MBUTTONDOWN"));
	id_H_MBUTTONUP_   = RegisterWindowMessage(TEXT("H_MBUTTONUP"));
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
			const uint key = distinguishLeftRight_.value() ? uint(MapLeftRightKeys(wParam, lParam)) : uint(wParam);
			if ((msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP) && wParam != VK_F10)
			{
				if (((lParam >> 29) & 1) == 1)
					eventKeys.push_back({ key, true });
				else
					eventKeys.push_back({ key, false });
			}

			eventKeys.push_back({ key, eventDown });
			break;
		}
		case WM_LBUTTONDOWN:
			eventDown = true;
		case WM_LBUTTONUP:
			eventKeys.push_back({ VK_LBUTTON, eventDown });
			break;
		case WM_MBUTTONDOWN:
			eventDown = true;
		case WM_MBUTTONUP:
			eventKeys.push_back({ VK_MBUTTON, eventDown });
			break;
		case WM_RBUTTONDOWN:
			eventDown = true;
		case WM_RBUTTONUP:
			eventKeys.push_back({ VK_RBUTTON, eventDown });
			break;
		case WM_XBUTTONDOWN:
			eventDown = true;
		case WM_XBUTTONUP:
			eventKeys.push_back({ uint(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2), eventDown });
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
			downKeysChanged |= DownKeys.insert(k.vk).second;
		else
			downKeysChanged |= DownKeys.erase(k.vk) > 0;


	InputResponse response = InputResponse::PASS_TO_GAME;
	// Only run these for key down/key up (incl. mouse buttons) events
	if(!eventKeys.empty())
		for(auto& cb : inputChangeCallbacks_)
			response |= (*cb)(downKeysChanged, DownKeys, eventKeys);

#if 0
	if (input_key_down || input_key_up)
	{
		std::string keybind = "";
		for (const auto& k : DownKeys)
		{
			keybind += GetKeyName(k) + std::string(" + ");
		}
		keybind = keybind.substr(0, keybind.size() - 2) + "\n";

		OutputDebugStringA(("Current keys down: " + keybind).c_str());

		char buf[1024];
		sprintf_s(buf, "msg=%u wParam=%u lParam=%u\n", msg, (uint)wParam, (uint)lParam);
		OutputDebugStringA(buf);
	}
#endif

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

Input::DelayedInput Input::TransformVKey(uint vk, bool down, mstime t, const std::optional<Point>& cursorPos)
{
	DelayedInput i { };
	i.t = t;
	i.cursorPos = cursorPos;
	if (vk == VK_LBUTTON || vk == VK_MBUTTON || vk == VK_RBUTTON)
	{
		std::tie(i.wParam, i.lParam) = CreateMouseEventParams(cursorPos);
	}
	else
	{
		i.wParam = vk;
		i.lParam = 1;
		i.lParam += (MapVirtualKeyEx(vk, MAPVK_VK_TO_VSC, 0) & 0xFF) << 16;
		if (!down)
			i.lParam += (1 << 30) + (1 << 31);
	}

	switch (vk)
	{
		case VK_LBUTTON:
			i.msg = down ? id_H_LBUTTONDOWN_ : id_H_LBUTTONUP_;
			break;
		case VK_MBUTTON:
			i.msg = down ? id_H_MBUTTONDOWN_ : id_H_MBUTTONUP_;
			break;
		case VK_RBUTTON:
			i.msg = down ? id_H_RBUTTONDOWN_ : id_H_RBUTTONUP_;
			break;
		case VK_LMENU:
		case VK_RMENU:
		case VK_F10:
			i.msg = down ? id_H_SYSKEYDOWN_ : id_H_SYSKEYUP_;

			// Set the correct transition state: 1 for KEYUP and 0 for KEYDOWN
			if (!down)
				i.lParam |= 1 << 31;

			break;
		case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // Arrow keys
		case VK_PRIOR: case VK_NEXT: // Page up and page down
		case VK_END: case VK_HOME:
		case VK_INSERT: case VK_DELETE:
		case VK_DIVIDE: // Numpad slash
		case VK_NUMLOCK:
			i.lParam |= 1 << 24; // Set extended bit
		default:
			i.msg = down ? id_H_KEYDOWN_ : id_H_KEYUP_;

			// Set the correct transition state: 1 for KEYUP and 0 for KEYDOWN
			if (!down)
				i.lParam |= 1 << 31;

			break;
	}

	return i;
}

std::tuple<WPARAM, LPARAM> Input::CreateMouseEventParams(const std::optional<Point>& cursorPos) const
{
	WPARAM wParam = 0;
	if (DownKeys.count(VK_LCONTROL) || DownKeys.count(VK_RCONTROL))
		wParam += MK_CONTROL;
	if (DownKeys.count(VK_LSHIFT) || DownKeys.count(VK_RSHIFT))
		wParam += MK_SHIFT;
	if (DownKeys.count(VK_LBUTTON))
		wParam += MK_LBUTTON;
	if (DownKeys.count(VK_RBUTTON))
		wParam += MK_RBUTTON;
	if (DownKeys.count(VK_MBUTTON))
		wParam += MK_MBUTTON;

	const auto& io = ImGui::GetIO();

	LPARAM lParam = MAKELPARAM(cursorPos ? cursorPos->x : (static_cast<int>(io.MousePos.x)), cursorPos ? cursorPos->y : (static_cast<int>(io.MousePos.y)));
	return { wParam, lParam };
}

void Input::SendKeybind(const std::set<uint> &vkeys, const std::optional<Point>& cursorPos)
{
	if (vkeys.empty())
		return;

	std::list<uint> vkeysSorted(vkeys.begin(), vkeys.end());
	vkeysSorted.sort([](uint &a, uint &b)
	{
		if (std::set<uint>{ VK_LCONTROL, VK_RCONTROL, VK_LSHIFT, VK_RSHIFT, VK_LMENU, VK_RMENU }.count(a))
			return true;
		else
			return a < b;
	});

	mstime currentTime = TimeInMilliseconds() + 10;

	for (const auto &vk : vkeysSorted)
	{
		if (DownKeys.count(vk))
			continue;

		DelayedInput i = TransformVKey(vk, true, currentTime, cursorPos);
		QueuedInputs.push_back(i);
		currentTime += 20;
	}

	currentTime += 50;

	for (const auto &vk : reverse(vkeysSorted))
	{
		if (DownKeys.count(vk))
			continue;

		DelayedInput i = TransformVKey(vk, false, currentTime, cursorPos);
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
	{
		auto [wParam, lParam] = CreateMouseEventParams(qi.cursorPos);
		PostMessage(Core::i()->gameWindow(), id_H_MOUSEMOVE_, wParam, lParam);
	}

	PostMessage(Core::i()->gameWindow(), qi.msg, qi.wParam, qi.lParam);

	QueuedInputs.pop_front();
}
}
