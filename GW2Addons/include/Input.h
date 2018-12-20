#pragma once

#include <Main.h>
#include <Singleton.h>
#include <set>
#include <list>
#include <functional>
#include <algorithm>

namespace GW2Addons
{

enum class InputResponse : uint
{
	PASS_TO_GAME = 0,
	PREVENT_MOUSE = 1,
	PREVENT_ALL = 2
};

struct EventKey
{
	uint vk : 31;
	bool down : 1;
};

inline InputResponse operator|(InputResponse a, InputResponse b)
{
	return InputResponse(std::max(uint(a), uint(b)));
}

inline InputResponse operator|=(InputResponse& a, InputResponse b)
{
	return (a = a | b);
}

class Input : public Singleton<Input>
{
public:
	using MouseMoveCallback = std::function<void()>;
	using InputChangeCallback = std::function<InputResponse(bool changed, const std::set<uint>& keys, const std::list<EventKey>& changedKeys)>;
	Input();

	uint id_H_LBUTTONDOWN() const { return id_H_LBUTTONDOWN_; }
	uint id_H_LBUTTONUP() const { return id_H_LBUTTONUP_; }
	uint id_H_RBUTTONDOWN() const { return id_H_RBUTTONDOWN_; }
	uint id_H_RBUTTONUP() const { return id_H_RBUTTONUP_; }
	uint id_H_MBUTTONDOWN() const { return id_H_MBUTTONDOWN_; }
	uint id_H_MBUTTONUP() const { return id_H_MBUTTONUP_; }
	uint id_H_SYSKEYDOWN() const { return id_H_SYSKEYDOWN_; }
	uint id_H_SYSKEYUP() const { return id_H_SYSKEYUP_; }
	uint id_H_KEYDOWN() const { return id_H_KEYDOWN_; }
	uint id_H_KEYUP() const { return id_H_KEYUP_; }

	// Returns true to consume message
	bool OnInput(UINT& msg, WPARAM& wParam, LPARAM& lParam);
	void OnFocusLost();
	void OnUpdate();
	
	void AddMouseMoveCallback(MouseMoveCallback* cb) { mouseMoveCallbacks_.push_back(cb); }
	void AddInputChangeCallback(InputChangeCallback* cb) { inputChangeCallbacks_.push_back(cb); }
	void RemoveMouseMoveCallback(MouseMoveCallback* cb) { mouseMoveCallbacks_.remove(cb); }
	void RemoveInputChangeCallback(InputChangeCallback* cb) { inputChangeCallbacks_.remove(cb); }
	void SendKeybind(const std::set<uint> &vkeys);

protected:
	struct DelayedInput
	{
		uint msg;
		WPARAM wParam;
		LPARAM lParam;

		mstime t;
	};

	uint ConvertHookedMessage(uint msg) const;
	DelayedInput TransformVKey(uint vk, bool down, mstime t);
	void SendQueuedInputs();

	// ReSharper disable CppInconsistentNaming
	uint id_H_LBUTTONDOWN_;
	uint id_H_LBUTTONUP_;
	uint id_H_RBUTTONDOWN_;
	uint id_H_RBUTTONUP_;
	uint id_H_MBUTTONDOWN_;
	uint id_H_MBUTTONUP_;
	uint id_H_SYSKEYDOWN_;
	uint id_H_SYSKEYUP_;
	uint id_H_KEYDOWN_;
	uint id_H_KEYUP_;
	// ReSharper restore CppInconsistentNaming

	std::set<uint> DownKeys;
	std::list<DelayedInput> QueuedInputs;
	
	std::list<MouseMoveCallback*> mouseMoveCallbacks_;
	std::list<InputChangeCallback*> inputChangeCallbacks_;
};

}