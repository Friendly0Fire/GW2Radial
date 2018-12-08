#pragma once

#include <Main.h>
#include <Singleton.h>
#include <set>
#include <list>

namespace GW2Addons
{

class Input : public Singleton<Input>
{
public:
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
	void SendKeybind(const std::set<uint> &vkeys);
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
};

}