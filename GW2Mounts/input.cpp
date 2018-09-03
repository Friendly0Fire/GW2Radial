#include "inputs.h"
#include <list>
#include "Utility.h"
#include "imgui/imgui.h"

std::set<uint> DownKeys;

uint id_H_LBUTTONDOWN;
uint id_H_LBUTTONUP;
uint id_H_RBUTTONDOWN;
uint id_H_RBUTTONUP;
uint id_H_MBUTTONDOWN;
uint id_H_MBUTTONUP;
uint id_H_SYSKEYDOWN;
uint id_H_SYSKEYUP;
uint id_H_KEYDOWN;
uint id_H_KEYUP;

void InitializeHookedMessages()
{
	id_H_LBUTTONDOWN = RegisterWindowMessage(TEXT("H_LBUTTONDOWN"));
	id_H_LBUTTONUP   = RegisterWindowMessage(TEXT("H_LBUTTONUP"));
	id_H_RBUTTONDOWN = RegisterWindowMessage(TEXT("H_RBUTTONDOWN"));
	id_H_RBUTTONUP   = RegisterWindowMessage(TEXT("H_RBUTTONUP"));
	id_H_MBUTTONDOWN = RegisterWindowMessage(TEXT("H_MBUTTONDOWN"));
	id_H_MBUTTONUP   = RegisterWindowMessage(TEXT("H_MBUTTONUP"));
	id_H_SYSKEYDOWN  = RegisterWindowMessage(TEXT("H_SYSKEYDOWN"));
	id_H_SYSKEYUP    = RegisterWindowMessage(TEXT("H_SYSKEYUP"));
	id_H_KEYDOWN     = RegisterWindowMessage(TEXT("H_KEYDOWN"));
	id_H_KEYUP       = RegisterWindowMessage(TEXT("H_KEYUP"));
}

uint ConvertHookedMessage(uint msg)
{
	if (msg == id_H_LBUTTONDOWN)
		return WM_LBUTTONDOWN;
	if (msg == id_H_LBUTTONUP)
		return WM_LBUTTONUP;
	if (msg == id_H_RBUTTONDOWN)
		return WM_RBUTTONDOWN;
	if (msg == id_H_RBUTTONUP)
		return WM_RBUTTONUP;
	if (msg == id_H_MBUTTONDOWN)
		return WM_MBUTTONDOWN;
	if (msg == id_H_MBUTTONUP)
		return WM_MBUTTONUP;
	if (msg == id_H_SYSKEYDOWN)
		return WM_SYSKEYDOWN;
	if (msg == id_H_SYSKEYUP)
		return WM_SYSKEYUP;
	if (msg == id_H_KEYDOWN)
		return WM_KEYDOWN;
	if (msg == id_H_KEYUP)
		return WM_KEYUP;

	return msg;
}

struct DelayedInput
{
	uint msg;
	WPARAM wParam;
	LPARAM lParam;

	mstime t;
};

DelayedInput TransformVKey(uint vk, bool down, mstime t)
{
	DelayedInput i;
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

		const auto& io = ImGui::GetIO();

		i.lParam = MAKELPARAM(((int)io.MousePos.x), ((int)io.MousePos.y));
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
		i.msg = down ? id_H_LBUTTONDOWN : id_H_LBUTTONUP;
		break;
	case VK_MBUTTON:
		i.msg = down ? id_H_MBUTTONDOWN : id_H_MBUTTONUP;
		break;
	case VK_RBUTTON:
		i.msg = down ? id_H_RBUTTONDOWN : id_H_RBUTTONUP;
		break;
	case VK_MENU:
	case VK_F10:
		i.msg = down ? id_H_SYSKEYDOWN : id_H_SYSKEYUP;

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
		i.msg = down ? id_H_KEYDOWN : id_H_KEYUP;

		// Set the correct transition state: 1 for KEYUP and 0 for KEYDOWN
		if(!down)
			i.lParam |= 1 << 31;

		break;
	}

	return i;
}


std::list<DelayedInput> QueuedInputs;

void SendKeybind(const std::set<uint>& vkeys)
{
	if (vkeys.empty())
		return;

	std::list<uint> vkeys_sorted(vkeys.begin(), vkeys.end());
	vkeys_sorted.sort([](uint& a, uint& b) {
		if (a == VK_CONTROL || a == VK_SHIFT || a == VK_MENU)
			return true;
		else
			return a < b;
	});

	mstime currentTime = timeInMS() + 10;

	for (const auto& vk : vkeys_sorted)
	{
		if (DownKeys.count(vk))
			continue;

		DelayedInput i = TransformVKey(vk, true, currentTime);
		QueuedInputs.push_back(i);
		currentTime += 20;
	}

	currentTime += 50;

	for (const auto& vk : reverse(vkeys_sorted))
	{
		if (DownKeys.count(vk))
			continue;

		DelayedInput i = TransformVKey(vk, false, currentTime);
		QueuedInputs.push_back(i);
		currentTime += 20;
	}
}

void SendQueuedInputs()
{
	if (QueuedInputs.empty())
		return;

	mstime currentTime = timeInMS();

	auto& qi = QueuedInputs.front();

	if (currentTime < qi.t)
		return;

	PostMessage(GameWindow, qi.msg, qi.wParam, qi.lParam);

	QueuedInputs.pop_front();
}