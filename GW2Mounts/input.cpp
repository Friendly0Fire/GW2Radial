#include "inputs.h"
#include <list>
#include "Utility.h"
#include "imgui/imgui.h"

std::set<uint> DownKeys;

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
		i.msg = down ? WM_LBUTTONDOWN : WM_LBUTTONUP;
		break;
	case VK_MBUTTON:
		i.msg = down ? WM_MBUTTONDOWN : WM_MBUTTONUP;
		break;
	case VK_RBUTTON:
		i.msg = down ? WM_RBUTTONDOWN : WM_RBUTTONUP;
		break;
	case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
	case VK_PRIOR: case VK_NEXT: // page up and page down
	case VK_END: case VK_HOME:
	case VK_INSERT: case VK_DELETE:
	case VK_DIVIDE: // numpad slash
	case VK_NUMLOCK:
		i.lParam |= 1 << 24; // set extended bit
	default:
		i.msg = down ? WM_KEYDOWN : WM_KEYUP;
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