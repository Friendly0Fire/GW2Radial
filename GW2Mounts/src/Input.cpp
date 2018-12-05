#include <Input.h>
#include <imgui/imgui.h>
#include <Utility.h>

namespace GW2Addons
{

DEFINE_SINGLETON(InputHook);


InputHook::InputHook()
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

uint InputHook::ConvertHookedMessage(uint msg) const
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

InputHook::DelayedInput InputHook::TransformVKey(uint vk, bool down, mstime t)
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

void InputHook::SendKeybind(const std::set<uint> &vkeys)
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

void InputHook::SendQueuedInputs()
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
