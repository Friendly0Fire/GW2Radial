#include <Input.h>
#include <imgui.h>
#include <Utility.h>
#include <Core.h>
#include <algorithm>
#include <SettingsMenu.h>

#include <MumbleLink.h>

IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace GW2Radial
{
    Input::Input()
    {
        // While WM_USER+n is recommended by MSDN, we do not know if the game uses special
        // window events, so avoid any potential conflict using explicit registration
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
        const UINT scancode = (lParam & 0x00ff0000) >> 16;
        const int extended  = (lParam & 0x01000000) != 0;

        switch (vk)
        {
        case VK_SHIFT:
            return MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
        case VK_CONTROL:
            return extended ? VK_RCONTROL : VK_LCONTROL;
        case VK_MENU:
            return extended ? VK_RMENU : VK_LMENU;
        default:
            // Not a key we map from generic to left/right,
            // just return it.
            return vk;
        }
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
        EventKey eventKey = { ScanCode::NONE, false };
        {
            bool eventDown = false;
            switch (msg)
            {
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
                eventDown = true;
            [[fallthrough]];
            case WM_SYSKEYUP:
            case WM_KEYUP:
                {
                    auto& keylParam = KeyLParam::Get(lParam);
                    const ScanCode sc = GetScanCode(keylParam);

                    eventKey = { sc, eventDown };
                    break;
                }
            case WM_LBUTTONDOWN:
                eventDown = true;
            [[fallthrough]];
            case WM_LBUTTONUP:
                eventKey = { ScanCode::LBUTTON, eventDown };
                break;
            case WM_MBUTTONDOWN:
                eventDown = true;
            [[fallthrough]];
            case WM_MBUTTONUP:
                eventKey = { ScanCode::MBUTTON, eventDown };
                break;
            case WM_RBUTTONDOWN:
                eventDown = true;
            [[fallthrough]];
            case WM_RBUTTONUP:
                eventKey = { ScanCode::RBUTTON, eventDown };
                break;
            case WM_XBUTTONDOWN:
                eventDown = true;
            [[fallthrough]];
            case WM_XBUTTONUP:
                eventKey = { GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? ScanCode::X1BUTTON : ScanCode::X2BUTTON, eventDown };
                break;
            }
        }

        const auto isRawInputMouse = msg == WM_INPUT && IsRawInputMouse(lParam);

        bool preventMouseMove = false;
        if (msg == WM_MOUSEMOVE || isRawInputMouse) {
            bool interrupt = false;
            for (auto& cb : mouseMoveCallbacks_) {
                cb->callback(preventMouseMove);
            }
        }

        bool preventMouseButton = false;
        if (eventKey.sc != ScanCode::NONE &&
            (eventKey.sc == ScanCode::LBUTTON || eventKey.sc == ScanCode::MBUTTON || eventKey.sc == ScanCode::RBUTTON || eventKey.sc == ScanCode::X1BUTTON || eventKey.sc == ScanCode::X2BUTTON)) {
            bool interrupt = false;
            for (auto& cb : mouseButtonCallbacks_) {
                cb->callback(eventKey, preventMouseButton);
            }
        }

        InputResponse response = preventMouseButton ? InputResponse::PREVENT_MOUSE : InputResponse::PASS_TO_GAME;
        if (inputRecordCallback_) {
            response |= InputResponse::PREVENT_KEYBOARD;
            if (!eventKey.down && eventKey.sc != ScanCode::LBUTTON) {
                (*inputRecordCallback_)(KeyCombo(eventKey.sc, downModifiers_));
                inputRecordCallback_ = std::nullopt;
            }
        }

        // Only run these for key down/key up (incl. mouse buttons) events
        if (eventKey.sc != ScanCode::NONE && !MumbleLink::i().textboxHasFocus())
            response |= TriggerKeybinds(eventKey) ? InputResponse::PREVENT_KEYBOARD : InputResponse::PASS_TO_GAME;

        if (IsModifier(eventKey.sc)) {
            auto mod = ToModifier(eventKey.sc);
            if (eventKey.down)
                downModifiers_ |= mod;
            else
                downModifiers_ &= ~mod;
        }

        ImGui_ImplWin32_WndProcHandler(Core::i().gameWindow(), msg, wParam, lParam);
        if (msg == WM_MOUSEMOVE)
        {
            auto& io = ImGui::GetIO();
            io.MousePos.x = static_cast<signed short>(lParam);
            io.MousePos.y = static_cast<signed short>(lParam >> 16);
        }

        if(response == InputResponse::PREVENT_ALL)
            return true;

        if(response == InputResponse::PREVENT_KEYBOARD)
        {
            switch (msg)
            {
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
                return true;
            }
        }

        if(response == InputResponse::PREVENT_MOUSE || preventMouseMove)
        {
            switch (msg)
            {
            // Outright prevent those two events
            case WM_MOUSEMOVE:
                return true;
            case WM_INPUT:
                if(isRawInputMouse)
                    return true;
                break;

            // All other mouse events pass mouse position as well, so revert that
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
                    cref io2 = ImGui::GetIO();

                    short mx, my;
                    mx = (short)io2.MousePos.x;
                    my = (short)io2.MousePos.y;
                    lParam = MAKELPARAM(mx, my);
                    break;
                }
            }
        }

        // Prevent game from receiving input if ImGui requests capture
        cref io = ImGui::GetIO();
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
    }

    void Input::OnUpdate()
    {
        SendQueuedInputs();
    }

    PreventPassToGame Input::TriggerKeybinds(const EventKey& ek)
    {
#ifdef _DEBUG
        std::wstring dbgkeys = L"";
        if (!ek.down && isNone(downModifiers_)) {
            dbgkeys = L"<NONE>";
        } else {
            if (notNone(downModifiers_ & Modifier::CTRL))
                dbgkeys += L"CTRL + ";
            if (notNone(downModifiers_ & Modifier::SHIFT))
                dbgkeys += L"SHIFT + ";
            if (notNone(downModifiers_ & Modifier::ALT))
                dbgkeys += L"ALT + ";
            if(ek.down)
                dbgkeys += GetScanCodeName(ek.sc);
            else
                dbgkeys[dbgkeys.size() - 3] = L'\0';
        }
        FormattedOutputDebugString(L"Triggering keybinds, active keys: %s\n", dbgkeys.c_str());
#endif

        KeyCombo kc(ek.sc, downModifiers_);
        if (ek.down) {
            for (auto& kb : keybinds_[kc]) {
                if (kb->conditionsFulfilled() && kb != activeKeybind_) {
                    if (activeKeybind_ != nullptr)
                        activeKeybind_->callback()(false);
                    activeKeybind_ = kb;
#ifdef _DEBUG
                    FormattedOutputDebugString(L"Active keybind is now '%s'\n", utf8_decode(kb->displayName()).c_str());
#endif
                    return kb->callback()(true);
                }
            }
        } else if(activeKeybind_ != nullptr) {
            if (!activeKeybind_->matches(kc)) {
                activeKeybind_->callback()(false);
                activeKeybind_ = nullptr;
#ifdef _DEBUG
                OutputDebugString(L"Active keybind is now null\n");
#endif
            }
        }

        return false;
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
            bool isUniversal = IsUniversal(sc);
            if (isUniversal)
                sc = sc & ~ScanCode::UNIVERSAL_MODIFIER_FLAG;

            i.wParam = MapVirtualKey(uint(sc), isUniversal ? MAPVK_VSC_TO_VK : MAPVK_VSC_TO_VK_EX);
            GW2_ASSERT(i.wParam != 0);
            i.lParamKey.repeatCount = 0;
            i.lParamKey.scanCode = uint(sc) & 0xFF; // Only take the first octet; there's a possibility the value won't fit in the bit field otherwise
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
        if (notNone(downModifiers_ & Modifier::CTRL))
            wParam += MK_CONTROL;
        if (notNone(downModifiers_ & Modifier::SHIFT))
            wParam += MK_SHIFT;
#if 0
        if (downKeys_.count(ScanCode::LBUTTON))
            wParam += MK_LBUTTON;
        if (downKeys_.count(ScanCode::RBUTTON))
            wParam += MK_RBUTTON;
        if (downKeys_.count(ScanCode::MBUTTON))
            wParam += MK_MBUTTON;
        if (downKeys_.count(ScanCode::X1BUTTON))
            wParam += MK_XBUTTON1;
        if (downKeys_.count(ScanCode::X2BUTTON))
            wParam += MK_XBUTTON2;
#endif

        cref io = ImGui::GetIO();

        LPARAM lParam = MAKELPARAM(cursorPos ? cursorPos->x : (static_cast<int>(io.MousePos.x)), cursorPos ? cursorPos->y : (static_cast<int>(io.MousePos.y)));
        return { wParam, lParam };
    }

    void Input::SendKeybind(const KeyCombo& ks, const std::optional<Point>& cursorPos, KeybindAction action)
    {
        if (ks.key() == ScanCode::NONE)
        {
            if (cursorPos.has_value())
            {
                DelayedInput i { };
                i.t = TimeInMilliseconds() + 10;
                std::tie(i.wParam, i.lParamValue) = CreateMouseEventParams(cursorPos);
                i.msg = id_H_MOUSEMOVE_;
                queuedInputs_.push_back(i);
            }
            return;
        }

        mstime currentTime = TimeInMilliseconds() + 10;

        std::list<ScanCode> codes;
        if (notNone(ks.mod() & Modifier::SHIFT))
            codes.push_back(ScanCode::SHIFTLEFT);
        if (notNone(ks.mod() & Modifier::CTRL))
            codes.push_back(ScanCode::CONTROLLEFT);
        if (notNone(ks.mod() & Modifier::ALT))
            codes.push_back(ScanCode::ALTLEFT);

        codes.push_back(ks.key());

        auto sendKeys = [&](ScanCode sc, bool down) {
            DelayedInput i = TransformScanCode(sc, down, currentTime, cursorPos);
            if (i.wParam != 0)
                queuedInputs_.push_back(i);
            currentTime += 20;
        };

        if (notNone(action & KeybindAction::DOWN)) {
            for (auto sc : codes)
                sendKeys(sc, true);
        }

        if(action == KeybindAction::BOTH)
            currentTime += 50;

        if (notNone(action & KeybindAction::UP)) {
            for (const auto& sc : reverse(codes))
                sendKeys(sc, false);
        }
    }

    void Input::SendQueuedInputs()
    {
        if (queuedInputs_.empty())
            return;

        const auto currentTime = TimeInMilliseconds();

        auto &qi = queuedInputs_.front();

        if (currentTime < qi.t)
            return;

        // Only send inputs that aren't too old
        if(currentTime < qi.t + 1000 && !MumbleLink::i().textboxHasFocus())
        {
            if (qi.cursorPos)
            {
                FormattedOutputDebugString(L"Moving cursor to (%d, %d)...\n", qi.cursorPos->x, qi.cursorPos->y);
                SetCursorPos(qi.cursorPos->x, qi.cursorPos->y);
            }

            if (qi.msg != id_H_MOUSEMOVE_)
            {
#ifdef _DEBUG
                FormattedOutputDebugString(L"Sending keybind 0x%x...\n", qi.wParam);
#endif
                PostMessage(Core::i().gameWindow(), qi.msg, qi.wParam, qi.lParamValue);
            }
        }

        queuedInputs_.pop_front();
    }

    void Input::RegisterKeybind(ActivationKeybind* kb)
    {
        keybinds_[kb->keyCombo()].push_back(kb);
    }

    void Input::UpdateKeybind(ActivationKeybind* kb)
    {
        UnregisterKeybind(kb);
        RegisterKeybind(kb);
    }

    void Input::UnregisterKeybind(ActivationKeybind* kb)
    {
        for (auto& [kc, vec] : keybinds_)
        {
            auto it = std::remove(vec.begin(), vec.end(), kb);
            if (it != vec.end())
                vec.erase(it);
        }
    }
}
