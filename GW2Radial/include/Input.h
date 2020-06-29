#pragma once

#include <Main.h>
#include <Singleton.h>
#include <set>
#include <list>
#include <functional>
#include <algorithm>
#include <optional>
#include "ConfigurationOption.h"

namespace GW2Radial
{

/*
The scancode values come from:
- http://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/scancode.doc (March 16, 2000).
- http://www.computer-engineering.org/ps2keyboard/scancodes1.html
- using MapVirtualKeyEx( VK_*, MAPVK_VK_TO_VSC_EX, 0 ) with the english us keyboard layout
- reading win32 WM_INPUT keyboard messages.
*/
enum class ScanCode : uint {
    ESCAPE = 0x01,
    NUMROW_1 = 0x02,
    NUMROW_2 = 0x03,
    NUMROW_3 = 0x04,
    NUMROW_4 = 0x05,
    NUMROW_5 = 0x06,
    NUMROW_6 = 0x07,
    NUMROW_7 = 0x08,
    NUMROW_8 = 0x09,
    NUMROW_9 = 0x0A,
    NUMROW_0 = 0x0B,
    MINUS = 0x0C,
    EQUALS = 0x0D,
    BACKSPACE = 0x0E,
    TAB = 0x0F,
    Q = 0x10,
    W = 0x11,
    E = 0x12,
    R = 0x13,
    T = 0x14,
    Y = 0x15,
    U = 0x16,
    I = 0x17,
    O = 0x18,
    P = 0x19,
    BRACKETLEFT = 0x1A,
    BRACKETRIGHT = 0x1B,
    ENTER = 0x1C,
    CONTROLLEFT = 0x1D,
    A = 0x1E,
    S = 0x1F,
    D = 0x20,
    F = 0x21,
    G = 0x22,
    H = 0x23,
    J = 0x24,
    K = 0x25,
    L = 0x26,
    SEMICOLON = 0x27,
    APOSTROPHE = 0x28,
    GRAVE = 0x29,
    SHIFTLEFT = 0x2A,
    BACKSLASH = 0x2B,
    Z = 0x2C,
    X = 0x2D,
    C = 0x2E,
    V = 0x2F,
    B = 0x30,
    N = 0x31,
    M = 0x32,
    COMMA = 0x33,
    PREIOD = 0x34,
    SLASH = 0x35,
    SHIFTRIGHT = 0x36,
    NUMPAD_MULTIPLY = 0x37,
    ALTLEFT = 0x38,
    SPACE = 0x39,
    CAPSLOCK = 0x3A,
    F1 = 0x3B,
    F2 = 0x3C,
    F3 = 0x3D,
    F4 = 0x3E,
    F5 = 0x3F,
    F6 = 0x40,
    F7 = 0x41,
    F8 = 0x42,
    F9 = 0x43,
    F10 = 0x44,
    NUMLOCK = 0x45,
    SCROLLLOCK = 0x46,
    NUMPAD_7 = 0x47,
    NUMPAD_8 = 0x48,
    NUMPAD_9 = 0x49,
    NUMPAD_MINUS = 0x4A,
    NUMPAD_4 = 0x4B,
    NUMPAD_5 = 0x4C,
    NUMPAD_6 = 0x4D,
    NUMPAD_PLUS = 0x4E,
    NUMPAD_1 = 0x4F,
    NUMPAD_2 = 0x50,
    NUMPAD_3 = 0x51,
    NUMPAD_0 = 0x52,
    NUMPAD_PERIOD = 0x53,
    ALT_PRINTSCREEN = 0x54, /* Alt + print screen. MapVirtualKeyEx( VK_SNAPSHOT, MAPVK_VK_TO_VSC_EX, 0 ) returns scancode 0x54. */
    BRACKETANGLE = 0x56, /* Key between the left shift and Z. */
    F11 = 0x57,
    F12 = 0x58,
    OEM_1 = 0x5A, /* VK_OEM_WSCTRL */
    OEM_2 = 0x5B, /* VK_OEM_FINISH */
    OEM_3 = 0x5C, /* VK_OEM_JUMP */
    ERASEEOF = 0x5D,
    OEM_4 = 0x5E, /* VK_OEM_BACKTAB */
    OEM_5 = 0x5F, /* VK_OEM_AUTO */
    ZOOM = 0x62,
    HELP = 0x63,
    F13 = 0x64,
    F14 = 0x65,
    F15 = 0x66,
    F16 = 0x67,
    F17 = 0x68,
    F18 = 0x69,
    F19 = 0x6A,
    F20 = 0x6B,
    F21 = 0x6C,
    F22 = 0x6D,
    F23 = 0x6E,
    OEM_6 = 0x6F, /* VK_OEM_PA3 */
    KATAKANA = 0x70,
    OEM_7 = 0x71, /* VK_OEM_RESET */
    F24 = 0x76,
    SBCSCHAR = 0x77,
    CONVERT = 0x79,
    NONCONVERT = 0x7B, /* VK_OEM_PA1 */

    MEDIA_PREVIOUS = 0xE010,
    MEDIA_NEXT = 0xE019,
    NUMPAD_ENTER = 0xE01C,
    CONTROLRIGHT = 0xE01D,
    VOLUME_MUTE = 0xE020,
    LAUNCH_APP2 = 0xE021,
    MEDIA_PLAY = 0xE022,
    MEDIA_STOP = 0xE024,
    VOLUME_DOWN = 0xE02E,
    VOLUME_UP = 0xE030,
    BROWSER_HOME = 0xE032,
    NUMPAD_DIVIDE = 0xE035,
    PRINTSCREEN = 0xE037,
    /*
    PRINTSCREEN:
    - make: 0xE02A 0xE037
    - break: 0xE0B7 0xE0AA
    - MapVirtualKeyEx( VK_SNAPSHOT, MAPVK_VK_TO_VSC_EX, 0 ) returns scancode 0x54;
    - There is no VK_KEYDOWN with VK_SNAPSHOT.
    */
    ALTRIGHT = 0xE038,
    CANCEL = 0xE046, /* CTRL + PAUSE */
    HOME = 0xE047,
    ARROWUP = 0xE048,
    PAGEUP = 0xE049,
    ARROWLEFT = 0xE04B,
    ARROWRIGHT = 0xE04D,
    END = 0xE04F,
    ARROWDOWN = 0xE050,
    PAGEDOWN = 0xE051,
    INSERT = 0xE052,
    DELETE_ = 0xE053,
    METALEFT = 0xE05B,
    METARIGHT = 0xE05C,
    APPLICATION = 0xE05D,
    POWER = 0xE05E,
    SLEEP = 0xE05F,
    WAKE = 0xE063,
    BROWSER_SEARCH = 0xE065,
    BROWSER_FAVORITES = 0xE066,
    BROWSER_REFRESH = 0xE067,
    BROWSER_STOP = 0xE068,
    BROWSER_FORWARD = 0xE069,
    BROWSER_BACK = 0xE06A,
    LAUNCH_APP1 = 0xE06B,
    LAUNCH_EMAIL = 0xE06C,
    LAUNCH_MEDIA = 0xE06D,

    PAUSE = 0xE11D45,
    /*
    PAUSE:
    - make: 0xE11D 45 0xE19D C5
    - make in raw input: 0xE11D 0x45
    - break: none
    - No repeat when you hold the key down
    - There are no break so I don't know how the key down/up is expected to work. Raw input sends "keydown" and "keyup" messages, and it appears that the keyup message is sent directly after the keydown message (you can't hold the key down) so depending on when GetMessage or PeekMessage will return messages, you may get both a keydown and keyup message "at the same time". If you use VK messages most of the time you only get keydown messages, but some times you get keyup messages too.
    - when pressed at the same time as one or both control keys, generates a 0xE046 (sc_cancel) and the string for that scancode is "break".
    */
        
    MOUSE_FLAG = 0xF0000,
    LBUTTON = MOUSE_FLAG | VK_LBUTTON,
    RBUTTON = MOUSE_FLAG | VK_RBUTTON,
    MBUTTON = MOUSE_FLAG | VK_MBUTTON,
    X1BUTTON = MOUSE_FLAG | VK_XBUTTON1,
    X2BUTTON = MOUSE_FLAG | VK_XBUTTON2,
    MOUSE_MASK = LBUTTON | RBUTTON | MBUTTON | X1BUTTON | X2BUTTON,

    UNIVERSAL_MODIFIER_FLAG = 0xF00000,
    SHIFT = UNIVERSAL_MODIFIER_FLAG | 1,
    CONTROL = UNIVERSAL_MODIFIER_FLAG | 2,
    ALT = UNIVERSAL_MODIFIER_FLAG | 3,
    META = UNIVERSAL_MODIFIER_FLAG | 4,
    UNIVERSAL_MODIFIER_MASK = SHIFT | CONTROL | ALT | META,

    MAX_VAL = (1u << 31) - 1 // 31 bits to fit in EventKey struct
};
std::wstring GetScanCodeName(ScanCode scanCode);

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

struct KeyLParam {
    uint repeatCount : 16 = 1;
    uint scanCode : 8 = 0;
    uint extendedFlag : 1 = 0;
    uint reserved : 4 = 0;
    uint contextCode : 1 = 0;
    uint previousKeyState : 1 = 0;
    uint transitionState : 1 = 0;

    static KeyLParam& Get(LPARAM& lp) {
        return *(KeyLParam*)&lp;
    }
};

struct Point
{
	int x;
	int y;
};

ScanCode GetScanCode(KeyLParam lParam);
inline bool IsScanCodeMouse(ScanCode sc) {
    return (uint(sc) & uint(ScanCode::MOUSE_MASK)) != 0;
}

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
