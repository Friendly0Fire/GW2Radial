#pragma once

#include <Main.h>
#include <Singleton.h>
#include <set>
#include <list>
#include <functional>
#include <algorithm>
#include <optional>
#include "ConfigurationOption.h"
#include <ScanCode.h>

namespace GW2Radial
{

enum class InputResponse : uint
{
	PASS_TO_GAME = 0, // Do not prevent any input from reaching the game
	PREVENT_MOUSE = 1, // Prevent mouse movement only from reaching the game
	PREVENT_ALL = 2 // Prevent all input from reaching the game
};

inline InputResponse operator|(InputResponse a, InputResponse b) {
    return InputResponse(std::max(uint(a), uint(b)));
}

inline InputResponse operator|=(InputResponse& a, InputResponse b) {
    return (a = a | b);
}

struct EventKey
{
    ScanCode sc : 31;
	bool down : 1;
};

struct Point
{
	int x;
	int y;
};

class Input : public Singleton<Input>
{
public:
	using MouseMoveCallback = std::function<bool()>;
	using InputChangeCallback = std::function<InputResponse(bool changed, const std::set<ScanCode>& sc, const std::list<EventKey>& changedKeys)>;
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
	void SendKeybind(const std::set<ScanCode> &sc, std::optional<Point> const& cursorPos = { });

protected:
	struct DelayedInput
	{
		uint msg;
		WPARAM wParam;
        union {
            KeyLParam lParamKey;
            LPARAM lParamValue;
        };

		mstime t;
		std::optional<Point> cursorPos;
	};

	uint ConvertHookedMessage(uint msg) const;
	DelayedInput TransformScanCode(ScanCode sc, bool down, mstime t, const std::optional<Point>& cursorPos);
	std::tuple<WPARAM, LPARAM> CreateMouseEventParams(const std::optional<Point>& cursorPos) const;
	void SendQueuedInputs();

	// ReSharper disable CppInconsistentNaming
	uint id_H_LBUTTONDOWN_;
	uint id_H_LBUTTONUP_;
	uint id_H_RBUTTONDOWN_;
	uint id_H_RBUTTONUP_;
	uint id_H_MBUTTONDOWN_;
	uint id_H_MBUTTONUP_;
	uint id_H_XBUTTONDOWN_;
	uint id_H_XBUTTONUP_;
	uint id_H_SYSKEYDOWN_;
	uint id_H_SYSKEYUP_;
	uint id_H_KEYDOWN_;
	uint id_H_KEYUP_;
	uint id_H_MOUSEMOVE_;
	// ReSharper restore CppInconsistentNaming

	std::set<ScanCode> DownKeys;
	std::list<DelayedInput> QueuedInputs;
	
	std::list<MouseMoveCallback*> mouseMoveCallbacks_;
	std::list<InputChangeCallback*> inputChangeCallbacks_;

	friend class MiscTab;
};

}