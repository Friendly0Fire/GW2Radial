#include <Input.h>
#include <imgui/imgui.h>
#include <Utility.h>

namespace GW2Addons
{

DEFINE_SINGLETON(Input);


Input::Input()
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
}

bool Input::OnInput(UINT& msg, WPARAM& wParam, LPARAM& lParam)
{
	struct EventKey
	{
		uint vk : 31;
		bool down : 1;
	};

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
			if ((msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP) && wParam != VK_F10)
			{
				if (((lParam >> 29) & 1) == 1)
					eventKeys.push_back({ VK_MENU, true });
				else
					eventKeys.push_back({ VK_MENU, false });
			}

			eventKeys.push_back({ (uint)wParam, eventDown });
			break;

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
			eventKeys.push_back({ (uint)(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2), eventDown });
			break;
		}
	}

	// Apply key events now
	for (const auto& k : eventKeys)
		if (k.down)
			DownKeys.insert(k.vk);
		else
			DownKeys.erase(k.vk);

	// Detect hovered section of the radial menu, if visible
	if (DisplayMountOverlay && msg == WM_MOUSEMOVE)
		DetermineHoveredMount();

	bool isMenuKeybind = false;

	// Only run these for key down/key up (incl. mouse buttons) events
	if (!eventKeys.empty())
	{
		// Very exclusive test: *only* consider the menu keybind to be activated if they're the *only* keys currently down
		// This minimizes the likelihood of the menu randomly popping up when it shouldn't
		isMenuKeybind = DownKeys == Cfg.SettingsKeybind();

		if (isMenuKeybind)
			DisplayOptionsWindow = true;
		else
		{
			bool oldMountOverlay = DisplayMountOverlay;

			bool mountOverlay = !Cfg.MountOverlayKeybind().empty() && std::includes(DownKeys.begin(), DownKeys.end(), Cfg.MountOverlayKeybind().begin(), Cfg.MountOverlayKeybind().end());
			bool mountOverlayLocked = !Cfg.MountOverlayLockedKeybind().empty() && std::includes(DownKeys.begin(), DownKeys.end(), Cfg.MountOverlayLockedKeybind().begin(), Cfg.MountOverlayLockedKeybind().end());

			DisplayMountOverlay = mountOverlayLocked || mountOverlay;

			if (DisplayMountOverlay && !oldMountOverlay)
			{
				// Mount overlay is turned on

				if (mountOverlayLocked)
				{
					OverlayPosition.x = OverlayPosition.y = 0.5f;

					// Attempt to move the cursor to the middle of the screen
					if (Cfg.ResetCursorOnLockedKeybind())
					{
						RECT rect = { };
						if (GetWindowRect(GameWindow, &rect))
						{
							if (SetCursorPos((rect.right - rect.left) / 2 + rect.left, (rect.bottom - rect.top) / 2 + rect.top))
							{
								auto& io = ImGui::GetIO();
								io.MousePos.x = ScreenWidth * 0.5f;
								io.MousePos.y = ScreenHeight * 0.5f;
							}
						}
					}
				}
				else
				{
					const auto& io = ImGui::GetIO();
					OverlayPosition.x = io.MousePos.x / (float)ScreenWidth;
					OverlayPosition.y = io.MousePos.y / (float)ScreenHeight;
				}

				OverlayTime = TimeInMilliseconds();
				MountHoverTime = OverlayTime + Cfg.OverlayDelayMilliseconds();

				DetermineHoveredMount();
			}
			else if (!DisplayMountOverlay && oldMountOverlay)
			{
				// Check for special behavior if no mount is hovered
				CurrentMountHovered = ModifyMountNoneBehavior(CurrentMountHovered);

				// Mount overlay is turned off, send the keybind
				if (CurrentMountHovered != MountType::NONE)
					SendKeybind(Cfg.MountKeybind((uint)CurrentMountHovered));

				PreviousMountUsed = CurrentMountHovered;
				CurrentMountHovered = MountType::NONE;
			}

			{
				// If a key was lifted, we consider the key combination *prior* to this key being lifted as the keybind
				bool keyLifted = false;
				auto fullKeybind = DownKeys;
				for (const auto& ek : eventKeys)
				{
					if (!ek.down)
					{
						fullKeybind.insert(ek.vk);
						keyLifted = true;
					}
				}

				// Explicitly filter out M1 (left mouse button) from keybinds since it breaks too many things
				fullKeybind.erase(VK_LBUTTON);

				MainKeybind.UpdateKeybind(fullKeybind, keyLifted);
				MainLockedKeybind.UpdateKeybind(fullKeybind, keyLifted);

				for (uint i = 0; i < MountTypeCount; i++)
					MountKeybinds[i].UpdateKeybind(fullKeybind, keyLifted);
			}
		}
	}

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

	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	if (msg == WM_MOUSEMOVE)
	{
		auto& io = ImGui::GetIO();
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
	}

	// Prevent game from receiving the settings menu keybind
	if (!eventKeys.empty() && isMenuKeybind)
		return true;

	// Prevent game cursor/camera from moving when the overlay is displayed
	if (DisplayMountOverlay && Cfg.LockCameraWhenOverlayed())
	{
		switch (msg)
		{
		case WM_MOUSEMOVE:
			return true;
		case WM_INPUT:
		{
			UINT dwSize = 40;
			static BYTE lpb[40];

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
				lpb, &dwSize, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE)
				return true;

			break;
		}
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

	return msg;
}

Input::DelayedInput Input::TransformVKey(uint vk, bool down, mstime t)
{
	DelayedInput i { };
	i.t = t;
	if (vk == VK_LBUTTON || vk == VK_MBUTTON || vk == VK_RBUTTON)
	{
		i.wParam = i.lParam = 0;
		if (DownKeys.count(VK_CONTROL))
			i.wParam += MK_CONTROL;
		if (DownKeys.count(VK_SHIFT))
			i.wParam += MK_SHIFT;
		if (DownKeys.count(VK_LBUTTON))
			i.wParam += MK_LBUTTON;
		if (DownKeys.count(VK_RBUTTON))
			i.wParam += MK_RBUTTON;
		if (DownKeys.count(VK_MBUTTON))
			i.wParam += MK_MBUTTON;

		const auto &io = ImGui::GetIO();

		i.lParam = MAKELPARAM((static_cast<int>(io.MousePos.x)), (static_cast<int>(io.MousePos.y)));
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
		case VK_MENU:
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

void Input::SendKeybind(const std::set<uint> &vkeys)
{
	if (vkeys.empty())
		return;

	std::list<uint> vkeysSorted(vkeys.begin(), vkeys.end());
	vkeysSorted.sort([](uint &a, uint &b)
	{
		if (a == VK_CONTROL || a == VK_SHIFT || a == VK_MENU)
			return true;
		else
			return a < b;
	});

	mstime currentTime = TimeInMilliseconds() + 10;

	for (const auto &vk : vkeysSorted)
	{
		if (DownKeys.count(vk))
			continue;

		DelayedInput i = TransformVKey(vk, true, currentTime);
		QueuedInputs.push_back(i);
		currentTime += 20;
	}

	currentTime += 50;

	for (const auto &vk : reverse(vkeysSorted))
	{
		if (DownKeys.count(vk))
			continue;

		DelayedInput i = TransformVKey(vk, false, currentTime);
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

	PostMessage(GameWindow, qi.msg, qi.wParam, qi.lParam);

	QueuedInputs.pop_front();
}
}
