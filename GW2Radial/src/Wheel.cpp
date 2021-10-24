#include <Main.h>
#include <Wheel.h>
#include <Core.h>
#include <Utility.h>
#include <UnitQuad.h>
#include <ImGuiExtensions.h>
#include <imgui.h>
#include <utility>
#include <Input.h>
#include "../imgui/imgui_internal.h"
#include <algorithm>
#include <GFXSettings.h>
#include <MumbleLink.h>
#include "../shaders/registers.h"
#include <Effect.h>

namespace GW2Radial
{

Wheel::Wheel(uint bgResourceId, uint wipeMaskResourceId, std::string nickname, std::string displayName, IDirect3DDevice9 * dev)
	: nickname_(std::move(nickname)), displayName_(std::move(displayName)),
	  keybind_(nickname_, "Show on mouse", nickname_), centralKeybind_(nickname_ + "_cl", "Show in center", nickname_),
	  centerBehaviorOption_("Center behavior", "center_behavior", "wheel_" + nickname_),
	  centerFavoriteOption_("Favorite choice##Center", "center_favorite.2", "wheel_" + nickname_),
	  delayFavoriteOption_("Favorite choice##Delay", "delay_favorite", "wheel_" + nickname_),
	  scaleOption_("Scale", "scale", "wheel_" + nickname_, 1.f),
	  centerScaleOption_("Center scale", "center_scale", "wheel_" + nickname_, 0.2f),
	  displayDelayOption_("Pop-up delay", "delay", "wheel_" + nickname_),
	  animationTimeOption_("Fade-in time", "anim_time", "wheel_" + nickname_, 750),
	  resetCursorOnLockedKeybindOption_("Reset cursor to center with Center Locked keybind", "reset_cursor_cl", "wheel_" + nickname_, true),
	  lockCameraWhenOverlayedOption_("Lock camera when overlay is displayed", "lock_camera", "wheel_" + nickname_, true),
	  showOverGameUIOption_("Show on top of game UI", "show_over_ui", "wheel_" + nickname_, true),
	  noHoldOption_("Activate first hovered option without holding down", "no_hold", "wheel_" + nickname_, false),
	  clickSelectOption_("Require click on option to select", "click_select", "wheel_" + nickname_, false),
	  behaviorOnReleaseBeforeDelay_("Behavior when released before delay has lapsed", "behavior_before_delay", "wheel_" + nickname_),
	  resetCursorAfterKeybindOption_("Move cursor to original location after release", "reset_cursor_after", "wheel_" + nickname_, true),
	maximumConditionalWaitTimeOption_("Expiration time of queued input", "max_wait_cond", "wheel_" + nickname_, 30),
	conditionalDelayDelayOption_("Delay before sending queued input", "min_delay_cond", "wheel_" + nickname_, 200),
	showDelayTimerOption_("Show timer next to skill bar when waiting to send input", "timer_ooc", "wheel_" + nickname_, true),
    centerCancelDelayedInputOption_("Cancel queued input with center region", "queue_center_cancel", "wheel_" + nickname_, false),
	enableConditionsOption_("Enable conditional keybinds", "conditions_enabled", "wheel_" + nickname_, false),
	enableQueuingOption_("Enable input queuing", "queuing_enabled", "wheel_" + nickname_, true),
    visibleInMenuOption_(displayName_ + "##Visible", "menu_visible", "wheel_" + nickname_, true),
	opacityMultiplierOption_("Opacity multiplier", "opacity", "wheel_" + nickname_, 100)
{
	conditions_ = std::make_shared<ConditionSet>("wheel_" + nickname_);
	conditions_->enable(enableConditionsOption_.value());
	keybind_.conditions(conditions_);
	centralKeybind_.conditions(conditions_);
	keybind_.callback([&](Activated a) {
		return KeybindEvent(false, a);
	});
	centralKeybind_.callback([&](Activated a) {
		return KeybindEvent(true, a);
	});

	aboveWater_.test = []() { return MumbleLink::i().isUnderwater(); };
	aboveWater_.toggleOffTest = outOfCombat_.toggleOffTest = []() { return MumbleLink::i().isMounted(); };
	outOfCombat_.test = []() { return MumbleLink::i().isInCombat(); };

	backgroundTexture_ = CreateTextureFromResource(dev, Core::i().dllModule(), bgResourceId);
	wipeMaskTexture_ = CreateTextureFromResource(dev, Core::i().dllModule(), wipeMaskResourceId);

	mouseMoveCallback_ = std::make_unique<Input::MouseMoveCallback>([this](bool& rv) { OnMouseMove(rv); });
	Input::i().AddMouseMoveCallback(mouseMoveCallback_.get());

	mouseButtonCallback_ = std::make_unique<Input::MouseButtonCallback>([this](EventKey ek, bool& rv) { OnMouseButton(ek.sc, ek.down, rv); });
	Input::i().AddMouseButtonCallback(mouseButtonCallback_.get());

	SettingsMenu::i().AddImplementer(this);
}

Wheel::~Wheel()
{
	Input::i([&](auto& i) {
		i.RemoveMouseMoveCallback(mouseMoveCallback_.get());
		i.RemoveMouseButtonCallback(mouseButtonCallback_.get());
	});
	SettingsMenu::i([&](auto& i) { i.RemoveImplementer(this); });
}

void Wheel::UpdateHover()
{
	const auto io = ImGui::GetIO();

	fVector2 mousePos;
	mousePos.x = io.MousePos.x / float(Core::i().screenWidth());
	mousePos.y = io.MousePos.y / float(Core::i().screenHeight());
	mousePos.x -= currentPosition_.x;
	mousePos.y -= currentPosition_.y;

	mousePos.y *= float(Core::i().screenHeight()) / float(Core::i().screenWidth());

	WheelElement* lastHovered = currentHovered_;

	auto activeElements = GetActiveElements();

	float mpLenSq = mousePos.x * mousePos.x + mousePos.y * mousePos.y;

	// Middle circle does not count as a hover event
	if (!activeElements.empty() && mpLenSq > SQUARE(scaleOption_.value() * 0.125f * 0.8f * centerScaleOption_.value()))
	{
		float mouseAngle = atan2(-mousePos.y, -mousePos.x) - 0.5f * float(M_PI);
		if (mouseAngle < 0)
			mouseAngle += float(2 * M_PI);

		const float elementAngle = float(2 * M_PI) / activeElements.size();
		const int elementId = int((mouseAngle - elementAngle / 2) / elementAngle + 1) % int(activeElements.size());

		currentHovered_ = activeElements[elementId];
	}
	else
		currentHovered_ = GetCenterHoveredElement();

	if (lastHovered != currentHovered_)
	{
		const auto time = TimeInMilliseconds();

		if(lastHovered) lastHovered->currentExitTime(time);
		if(currentHovered_) currentHovered_->currentHoverTime(time);
	}
}

void Wheel::DrawMenu(Keybind** currentEditedKeybind)
{
	ImGui::PushID((nickname_ + "Elements").c_str());

	ImGuiTitle("In-game Keybinds");

	ImGui::TextUnformatted("Set the following to your in-game keybinds (F11, Control Options).");

	for (auto& we : wheelElements_)
		ImGuiKeybindInput(we->keybind(), currentEditedKeybind, nullptr);

	ImGuiTitle("Keybinds");

	ImGuiKeybindInput((Keybind&)keybind_, currentEditedKeybind, "Pressing this key combination will open the radial menu at your cursor's current location.");
	ImGuiKeybindInput((Keybind&)centralKeybind_, currentEditedKeybind, "Pressing this key combination will open the radial menu in the middle of the screen. Your cursor will be moved to the middle of the screen and moved back after you have selected an option.");

	if (ImGuiConfigurationWrapper(&ImGui::Checkbox, enableConditionsOption_) && conditions_)
		conditions_->enable(enableConditionsOption_.value());

	{
		ImGuiDisabler disable(!conditions_ || !enableConditionsOption_.value());

		ImGui::Indent();

		conditions_->DrawMenu();

		ImGui::Unindent();
	}

	ImGuiTitle("Display Options");

	ImGuiConfigurationWrapper(&ImGui::SliderInt, opacityMultiplierOption_, 0, 100, "%d %%", ImGuiSliderFlags_AlwaysClamp);
	ImGuiHelpTooltip("Transparency of the entire overlay. Setting to 0% hides it entirely.");

	ImGuiConfigurationWrapper(&ImGui::SliderInt, animationTimeOption_, 0, 2000, "%d ms", ImGuiSliderFlags_AlwaysClamp);
	ImGuiHelpTooltip("Amount of time, in milliseconds, for the radial menu to fade in.");

	ImGuiConfigurationWrapper(&ImGui::SliderFloat, scaleOption_, 0.25f, 4.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGuiHelpTooltip("Scale factor for the size of the whole radial menu.");

	ImGuiConfigurationWrapper(&ImGui::SliderFloat, centerScaleOption_, 0.05f, 0.5f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGuiHelpTooltip("Scale factor for the size of just the central region of the radial menu.");

	ImGuiConfigurationWrapper(&ImGui::SliderInt, displayDelayOption_, 0, 3000, "%d ms", ImGuiSliderFlags_AlwaysClamp);
	ImGuiHelpTooltip("Amount of time, in milliseconds, to wait before displaying the radial menu. The input bound to the central region can still be sent by releasing the key, even before the menu is visible.");

	ImGuiConfigurationWrapper(&ImGui::Checkbox, showOverGameUIOption_);
	ImGuiHelpTooltip("Either show the radial menu over or under the game's UI.");

	if (extraUI_ && extraUI_->display)
		extraUI_->display();

	ImGuiTitle("Interaction Options");

	{
		ImGuiDisabler disable(clickSelectOption_.value());
		if (clickSelectOption_.value())
			noHoldOption_.value() = false;

		ImGuiConfigurationWrapper(&ImGui::Checkbox, noHoldOption_);
		ImGuiHelpTooltip("This option will activate whichever option is hovered first. Any key pressed to activate the menu can be released immediately without dismissing it. Mutually exclusive with \"click to select\".");
	}

	{
		ImGuiDisabler disable(noHoldOption_.value());
		if (noHoldOption_.value())
			clickSelectOption_.value() = false;

		ImGuiConfigurationWrapper(&ImGui::Checkbox, clickSelectOption_);
		ImGuiHelpTooltip("This option will only activate an option when it is clicked on. Any key pressed to activate the menu can be released immediately without dismissing it. Mutually exclusive with \"hover to select\".");
	}

	{
		ImGuiDisabler disable(displayDelayOption_.value() == 0);

		ImGui::Text((behaviorOnReleaseBeforeDelay_.displayName() + ":").c_str());

		bool (*rb)(const char*, int*, int) = &ImGui::RadioButton;
		ImGuiConfigurationWrapper(rb, "Nothing##ReleaseBeforeDelay", behaviorOnReleaseBeforeDelay_, int(BehaviorBeforeDelay::NOTHING));
		ImGui::SameLine();
		ImGuiConfigurationWrapper(rb, "Previous##ReleaseBeforeDelay", behaviorOnReleaseBeforeDelay_, int(BehaviorBeforeDelay::PREVIOUS));
		ImGui::SameLine();
		ImGuiConfigurationWrapper(rb, "Favorite##ReleaseBeforeDelay", behaviorOnReleaseBeforeDelay_, int(BehaviorBeforeDelay::FAVORITE));
		ImGui::SameLine();
		ImGuiConfigurationWrapper(rb, "As if opened##ReleaseBeforeDelay", behaviorOnReleaseBeforeDelay_, int(BehaviorBeforeDelay::DIRECTION));
		ImGuiHelpTooltip("Determines the behavior of the menu if it is dismissed before it becomes visible. By default, it does nothing, but it can also (1) trigger the last selected item; (2) trigger a fixed \"favorite\" option; (3) function exactly as if the menu was opened, making it possible to select an item with the mouse without seeing them.");

		if (BehaviorBeforeDelay(behaviorOnReleaseBeforeDelay_.value()) == BehaviorBeforeDelay::FAVORITE)
		{
			const auto textSize = ImGui::CalcTextSize(delayFavoriteOption_.displayName().c_str());
			const auto itemSize = ImGui::CalcItemWidth() - textSize.x - ImGui::GetCurrentWindowRead()->WindowPadding.x;

			std::vector<const char*> potentialNames(wheelElements_.size());
				for (uint i = 0; i < wheelElements_.size(); i++)
					potentialNames[i] = wheelElements_[i]->displayName().c_str();

			bool (*cmb)(const char*, int*, const char* const*, int, int) = &ImGui::Combo;
			ImGuiConfigurationWrapper(cmb, delayFavoriteOption_, potentialNames.data(), int(potentialNames.size()), -1);
			ImGuiHelpTooltip("If the \"favorite\" option is selected, this determine what the favorite is.");
		}
	}

	{
		ImGuiDisabler disable(noHoldOption_.value());

		ImGui::Text((centerBehaviorOption_.displayName() + ":").c_str());
		ImGui::SameLine();

		bool (*rb)(const char*, int*, int) = &ImGui::RadioButton;
		ImGuiConfigurationWrapper(rb, "Nothing##CenterBehavior", centerBehaviorOption_, int(CenterBehavior::NOTHING));
		ImGui::SameLine();
		ImGuiConfigurationWrapper(rb, "Previous##CenterBehavior", centerBehaviorOption_, int(CenterBehavior::PREVIOUS));
		ImGui::SameLine();
		ImGuiConfigurationWrapper(rb, "Favorite##CenterBehavior", centerBehaviorOption_, int(CenterBehavior::FAVORITE));
		ImGuiHelpTooltip("Determines the behavior of the central region of the menu. By default, it does nothing, but it can also (1) trigger the last selected item; (2) trigger a fixed \"favorite\" option.");

		if (CenterBehavior(centerBehaviorOption_.value()) == CenterBehavior::FAVORITE)
		{
			const auto textSize = ImGui::CalcTextSize(centerFavoriteOption_.displayName().c_str());
			const auto itemSize = ImGui::CalcItemWidth() - textSize.x - ImGui::GetCurrentWindowRead()->WindowPadding.x;

			std::vector<const char*> potentialNames(wheelElements_.size());
			for (uint i = 0; i < wheelElements_.size(); i++)
				potentialNames[i] = wheelElements_[i]->displayName().c_str();

			bool (*cmb)(const char*, int*, const char* const*, int, int) = &ImGui::Combo;
			ImGuiConfigurationWrapper(cmb, centerFavoriteOption_, potentialNames.data(), int(potentialNames.size()), -1);
			ImGuiHelpTooltip("If the \"favorite\" option is selected, this determine what the favorite is.");
		}
	}

	ImGuiConfigurationWrapper(&ImGui::Checkbox, resetCursorOnLockedKeybindOption_);
	ImGuiHelpTooltip("Moves the cursor to the center of the screen when the \"show in center\" keybind is used.");

	ImGuiConfigurationWrapper(&ImGui::Checkbox, lockCameraWhenOverlayedOption_);
	ImGuiHelpTooltip("Prevents the camera from being affected by mouse movements while the menu is displayed.");

	if (!alwaysResetCursorPositionBeforeKeyPress_)
	{
		ImGuiConfigurationWrapper(&ImGui::Checkbox, resetCursorAfterKeybindOption_);
		ImGuiHelpTooltip("Once the menu is dismissed, moves the cursor to where it was on screen before the menu was displayed.");
	}

	if(extraUI_ && extraUI_->interaction)
		extraUI_->interaction();

	ImGuiTitle("Queuing Options");

	ImGuiConfigurationWrapper(&ImGui::Checkbox, enableQueuingOption_);
	ImGuiHelpTooltip("If sending a keybind now would be ignored by the game (e.g., mounting while in combat), enabling queuing will \"queue\" the input until all necessary conditions are satisfied.");

	{
		ImGuiDisabler disable(!enableQueuingOption_.value());

		ImGui::PushItemWidth(0.33f * ImGui::GetWindowWidth());
		ImGuiConfigurationWrapper(&ImGuiInputIntFormat, conditionalDelayDelayOption_, "%d ms", 1, 100, 0);
		ImGuiHelpTooltip("Time, in milliseconds, to wait before sending a queued input once all conditions pass. Increasing this value can make activation more reliable.");
		ImGuiConfigurationWrapper(&ImGuiInputIntFormat, maximumConditionalWaitTimeOption_, "%d s", 1, 10, 0);
		ImGuiHelpTooltip("Maximum amount of time, in seconds, to wait with a queued input before dismissing it.");
		ImGui::PopItemWidth();

		{
			ImGuiDisabler disable2(noHoldOption_.value());
			ImGuiConfigurationWrapper(&ImGui::Checkbox, centerCancelDelayedInputOption_);
			ImGuiHelpTooltip("If the center region has no bound behavior, this option makes it cancel any queued input.");
		}

		if (extraUI_ && extraUI_->queuing)
			extraUI_->queuing();
	}

	ImGuiTitle("Visibility & Ordering");

	ImGui::Text("Ordering top to bottom is clockwise starting at noon.");

	for(auto it = sortedWheelElements_.begin(); it != sortedWheelElements_.end(); ++it)
	{
		const auto extremum = it == sortedWheelElements_.begin() ? 1 : it == std::prev(sortedWheelElements_.end()) ? -1 : 0;
		auto& e = *it;
		if(const auto dir = e->DrawPriority(extremum); dir != 0)
		{
			if(dir == 1 && e == sortedWheelElements_.front() ||
				dir == -1 && e == sortedWheelElements_.back())
				continue;

			auto& eOther = dir == 1 ? *std::prev(it) : *std::next(it);
			std::swap(e, eOther);
			const auto tempPriority = eOther->sortingPriority();
			eOther->sortingPriority(e->sortingPriority());
			e->sortingPriority(tempPriority);
		}
	}

	if(extraUI_ && extraUI_->misc)
		extraUI_->misc();

	ImGui::PopID();
}

void Wheel::OnUpdate() {
	if (conditionallyDelayed_) {
		const auto currentTime = TimeInMilliseconds();
		if(currentTime <= conditionallyDelayedTime_ + maximumConditionalWaitTimeOption_.value() * 1000ull) {
			cref mumble = MumbleLink::i();
		    if (mumble.gameHasFocus() && !mumble.isMapOpen()
				&& aboveWater_.passes() && outOfCombat_.passes()) {
				if (!conditionallyDelayedTestPasses_)
					conditionallyDelayedPassesTime_ = currentTime;
				else if(currentTime >= conditionallyDelayedPassesTime_ + conditionalDelayDelayOption_.value()) {
					Input::i().SendKeybind(conditionallyDelayed_->keybind().keyCombo(), std::nullopt);
					conditionallyDelayed_ = nullptr;
					conditionallyDelayedTime_ = currentTime;
					conditionallyDelayedTestPasses_ = false;
				}
				conditionallyDelayedTestPasses_ = true;
		    } else
				conditionallyDelayedTestPasses_ = false;
		} else {
		    conditionallyDelayed_ = nullptr;
			conditionallyDelayedTime_ = currentTime;
			conditionallyDelayedTestPasses_ = false;
		}
	}
}

void Wheel::OnMapChange(uint prevId, uint newId)
{
	conditionallyDelayed_ = nullptr;
	conditionallyDelayedTime_ = TimeInMilliseconds();
	conditionallyDelayedTestPasses_ = false;
}

void Wheel::OnCharacterChange(const std::wstring& prevCharacterName, const std::wstring& newCharacterName)
{
	conditionallyDelayed_ = nullptr;
	conditionallyDelayedTime_ = TimeInMilliseconds();
	conditionallyDelayedTestPasses_ = false;
}


void Wheel::Draw(IDirect3DDevice9* dev, Effect* fx, UnitQuad* quad)
{
	if (opacityMultiplierOption_.value() == 0)
		return;

	const int screenWidth = Core::i().screenWidth();
	const int screenHeight = Core::i().screenHeight();

	fVector4 screenSize = { float(screenWidth), float(screenHeight), 1.f / screenWidth, 1.f / screenHeight };

	const auto currentTime = TimeInMilliseconds();

	const auto resetCursorPositionToCenter = [&]() {
		RECT rect = { };
		if (GetWindowRect(Core::i().gameWindow(), &rect))
		{
			if (SetCursorPos((rect.right - rect.left) / 2 + rect.left, (rect.bottom - rect.top) / 2 + rect.top))
			{
				auto& io = ImGui::GetIO();
				io.MousePos.x = screenWidth * 0.5f;
				io.MousePos.y = screenHeight * 0.5f;
			}
		}
		resetCursorPositionToCenter_ = false;
	};

	if (behaviorOnReleaseBeforeDelay_.value() == int(BehaviorBeforeDelay::DIRECTION) && resetCursorPositionToCenter_)
		resetCursorPositionToCenter();

	if (isVisible_)
	{
		if (currentTime >= currentTriggerTime_ + displayDelayOption_.value())
		{
			if (resetCursorPositionToCenter_)
				resetCursorPositionToCenter();

			fx->Begin();
			quad->Bind(fx);
			fx->SetShader(ShaderType::VERTEX_SHADER, L"Shader_vs.hlsl", "ScreenQuad_VS");

			// Setup viewport
			D3DVIEWPORT9 vp;
			vp.X = vp.Y = 0;
			vp.Width = screenWidth;
			vp.Height = screenHeight;
			vp.MinZ = 0.0f;
			vp.MaxZ = 1.0f;
			dev->SetViewport(&vp);

			auto activeElements = GetActiveElements();
			if (!activeElements.empty())
			{
				fVector4 baseSpriteDimensions;
				baseSpriteDimensions.x = currentPosition_.x;
				baseSpriteDimensions.y = currentPosition_.y;
				baseSpriteDimensions.z = scaleOption_.value() * 0.5f * screenSize.y * screenSize.z;
				baseSpriteDimensions.w = scaleOption_.value() * 0.5f;

				const float fadeTimer = std::min(1.f, (currentTime - (currentTriggerTime_ + displayDelayOption_.value())) / float(animationTimeOption_.value() * 0.5f));

				std::vector<float> hoveredFadeIns;
				std::transform(activeElements.begin(), activeElements.end(), std::back_inserter(hoveredFadeIns),
					[&](const WheelElement* elem) { return elem->hoverFadeIn(currentTime, this); });

				switch(CenterBehavior(centerBehaviorOption_.value()))
				{
				case CenterBehavior::PREVIOUS:
					if(previousUsed_)
						hoveredFadeIns.push_back(previousUsed_->hoverFadeIn(currentTime, this));
					break;
				case CenterBehavior::FAVORITE:
				{
					auto fav = centerFavoriteOption_.value();
					if (fav >= 0 && fav < wheelElements_.size()) {
						hoveredFadeIns.push_back(wheelElements_[fav]->hoverFadeIn(currentTime, this));
						break;
					}
				}
				[[fallthrough]];
				default:
					hoveredFadeIns.push_back(0.f);
					break;
				}

				fx->SetShader(ShaderType::PIXEL_SHADER, L"Shader_ps.hlsl", "BgImage_PS");
				fx->SetRenderStates({
					{ D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA },
					{ D3DRS_SRCBLEND, D3DBLEND_ONE },
				});
				{
				    using namespace ShaderRegister::ShaderPS;
				    using namespace ShaderRegister::ShaderVS;

				    fx->SetVariable(ShaderType::PIXEL_SHADER, float3_fWipeMaskData, wipeMaskData_);
				    fx->SetVariable(ShaderType::VERTEX_SHADER, float4_fSpriteDimensions, baseSpriteDimensions);
				    fx->SetVariable(ShaderType::PIXEL_SHADER, float_fWheelFadeIn, fadeTimer);
				    fx->SetVariable(ShaderType::PIXEL_SHADER, float_fAnimationTimer, fmod(currentTime / 1010.f, 55000.f));
				    fx->SetVariable(ShaderType::PIXEL_SHADER, float_fCenterScale, centerScaleOption_.value());
				    fx->SetVariable(ShaderType::PIXEL_SHADER, int_iElementCount, activeElements.size());
					fx->SetVariable(ShaderType::PIXEL_SHADER, float_fGlobalOpacity, opacityMultiplierOption_.value() * 0.01f);
				    fx->SetVariableArray(ShaderType::PIXEL_SHADER, array_float4_fHoverFadeIns, (const std::span<float>&)hoveredFadeIns);

					fx->SetSamplerStates(sampler2D_texMainSampler, {});
					fx->SetSamplerStates(sampler2D_texWipeMaskImageSampler, {});
				    fx->SetTexture(sampler2D_texMainSampler, backgroundTexture_.Get());
				    fx->SetTexture(sampler2D_texWipeMaskImageSampler, wipeMaskTexture_.Get());
				}

				fx->ApplyStates();
				quad->Draw();

				fx->SetShader(ShaderType::PIXEL_SHADER, L"Shader_ps.hlsl", "MountImage_PS");
				fx->SetRenderStates({
					{ D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA },
					{ D3DRS_SRCBLEND, D3DBLEND_ONE },
				});

				int n = 0;
				for (auto it : activeElements)
				{
					it->Draw(n, baseSpriteDimensions, activeElements.size(), currentTime, currentHovered_, this);
					n++;
				}

			}

			{
				cref io = ImGui::GetIO();

				fx->SetShader(ShaderType::PIXEL_SHADER, L"Shader_ps.hlsl", "Cursor_PS");
				fx->SetRenderStates({
					{ D3DRS_DESTBLEND, D3DBLEND_ONE },
					{ D3DRS_SRCBLEND, D3DBLEND_ONE },
				});

				fVector4 spriteDimensions = { io.MousePos.x * screenSize.z, io.MousePos.y * screenSize.w, 0.08f  * screenSize.y * screenSize.z, 0.08f };
				fx->SetVariable(ShaderType::VERTEX_SHADER, ShaderRegister::ShaderVS::float4_fSpriteDimensions, spriteDimensions);

				fx->ApplyStates();
				quad->Draw();
			}

			fx->End();
		}
	} else if(showDelayTimerOption_.value() && (conditionallyDelayed_ != nullptr || currentTime - conditionallyDelayedTime_ < 500)) {
		float dt = float(currentTime - conditionallyDelayedTime_) / 1000.f;
		float absDt = dt;
		float timeLeft = 0.f;
		if(conditionallyDelayed_ == nullptr) absDt = 0.5f - dt;
		else
			timeLeft = 1.f - (currentTime - conditionallyDelayedTime_) / (float(maximumConditionalWaitTimeOption_.value()) * 1000.f);
	    cref io = ImGui::GetIO();

		fx->Begin();
		quad->Bind(fx);
		fx->SetShader(ShaderType::VERTEX_SHADER, L"Shader_vs.hlsl", "ScreenQuad_VS");
		fx->SetShader(ShaderType::PIXEL_SHADER, L"Shader_ps.hlsl", "TimerCursor_PS");
		fx->SetRenderStates({
			{ D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA },
			{ D3DRS_SRCBLEND, D3DBLEND_ONE },
		});

		float dpiScale = 1.f;
		if(GFXSettings::i().dpiScaling())
		    dpiScale = float(Core::i().GetDpiForWindow(Core::i().gameWindow())) / 96.f;

		float uiScale = float(MumbleLink::i().uiScale());

		fVector2 topLeftCorner;
		topLeftCorner.y = 77.f + 10.f * uiScale;
		if(uiScale == 0)
			topLeftCorner.y += 2.f;

		float bottom = 13.f + 1.f * uiScale;
		topLeftCorner.x = 450.f / 107.f * topLeftCorner.y;

		float widthMinScale = std::min(1.f, screenSize.x / 1024.f);
		float heightMinScale = std::min(1.f, screenSize.y / 768.f);

		float minScale = std::min(widthMinScale, heightMinScale);

		topLeftCorner.x *= dpiScale * minScale;
		topLeftCorner.y *= dpiScale * minScale;
		bottom *= dpiScale * minScale;

		topLeftCorner.x += screenSize.x * 0.5f;
		topLeftCorner.y = screenSize.y - topLeftCorner.y;
		bottom = screenSize.y - bottom;

		float spriteSize = bottom - topLeftCorner.y;

		if(dt < 0.3333f && conditionallyDelayed_ != nullptr) {
			float sizeInterpolant = SmoothStep(dt * 3);
			spriteSize = std::lerp(spriteSize * 3, spriteSize, sizeInterpolant);
		    topLeftCorner.x = std::lerp(0.5f * screenSize.x, topLeftCorner.x, 0.25f + sizeInterpolant * 0.75f);
		    topLeftCorner.y = std::lerp(0.5f * screenSize.y, topLeftCorner.y, 0.25f + sizeInterpolant * 0.75f);
		}

		fVector4 spriteDimensions
	        { topLeftCorner.x * screenSize.z,
		      topLeftCorner.y * screenSize.w,
	          spriteSize * screenSize.z,
	          spriteSize * screenSize.w };

		spriteDimensions.x += spriteDimensions.z * 0.5f;
		spriteDimensions.y += spriteDimensions.w * 0.5f;

		{
			using namespace ShaderRegister::ShaderPS;
			using namespace ShaderRegister::ShaderVS;
		    fx->SetVariable(ShaderType::PIXEL_SHADER, float_fAnimationTimer, fmod(currentTime / 1010.f, 55000.f));
		    fx->SetVariable(ShaderType::VERTEX_SHADER, float4_fSpriteDimensions, spriteDimensions);
		    fx->SetVariable(ShaderType::PIXEL_SHADER, float_fWheelFadeIn, std::min(absDt * 2, 1.f));
		    fx->SetVariable(ShaderType::PIXEL_SHADER, float_fTimeLeft, timeLeft);
			fx->SetVariable(ShaderType::PIXEL_SHADER, bool_bShowIcon, conditionallyDelayed_ != nullptr);
			fx->SetVariable(ShaderType::PIXEL_SHADER, float_fGlobalOpacity, opacityMultiplierOption_.value() * 0.01f);

			fx->SetSamplerStates(sampler2D_texMainSampler, {});
			fx->SetSamplerStates(sampler2D_texSecondarySampler, {
				{ D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER },
				{ D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER },
				{ D3DSAMP_BORDERCOLOR, D3DCOLOR(0) }
			});
			fx->SetTexture(sampler2D_texMainSampler, backgroundTexture_.Get());

			if(conditionallyDelayed_) {
				fx->SetTexture(sampler2D_texSecondarySampler, conditionallyDelayed_->appearance());
				conditionallyDelayed_->SetShaderState();
			}
		}

		fx->ApplyStates();
		quad->Draw();

		fx->End();
	}
}

void Wheel::OnFocusLost()
{
	currentHovered_ = nullptr;
	isVisible_ = false;
	currentTriggerTime_ = 0;

	conditionallyDelayed_ = nullptr;
	conditionallyDelayedTime_ = TimeInMilliseconds();
	conditionallyDelayedTestPasses_ = false;
}

void Wheel::Sort()
{
	sortedWheelElements_.resize(wheelElements_.size());
	std::transform(wheelElements_.begin(), wheelElements_.end(), sortedWheelElements_.begin(), [](auto& p) { return p.get(); });

	std::sort(sortedWheelElements_.begin(), sortedWheelElements_.end(),
		[](const WheelElement* a, const WheelElement* b) { return a->sortingPriority() < b->sortingPriority(); });
	minElementSortingPriority_ = sortedWheelElements_.front()->sortingPriority();
}

WheelElement* Wheel::GetCenterHoveredElement()
{
	if(noHoldOption_.value())
		return nullptr;

	if(centerCancelDelayedInputOption_.value() && conditionallyDelayed_)
		return nullptr;

	switch (CenterBehavior(centerBehaviorOption_.value()))
	{
	case CenterBehavior::PREVIOUS:
		return previousUsed_;
	case CenterBehavior::FAVORITE:
		return GetFavorite(centerFavoriteOption_.value());
	default:
		return nullptr;
	}
}

WheelElement* Wheel::GetFavorite(int favoriteId)
{
	if (favoriteId < 0 || favoriteId >= wheelElements_.size())
		return nullptr;

	auto* elem = wheelElements_[favoriteId].get();

	return elem->isBound() ? elem : nullptr;
}

std::vector<WheelElement*> Wheel::GetActiveElements(bool sorted)
{
	std::vector<WheelElement*> elems;
	if (sorted) {
		for (auto& we : sortedWheelElements_)
			if (we->isActive())
				elems.push_back(we);
	} else {
		for (auto& we : wheelElements_)
			if (we->isActive())
				elems.push_back(we.get());
	}

	return elems;
}

void Wheel::OnMouseMove(bool& rv)
{
	if(isVisible_)
	{
		UpdateHover();

		// If holding down the button is not necessary, modify behavior
		if(noHoldOption_.value() && isVisible_ && currentHovered_ != nullptr)
			DeactivateWheel();
	}

	rv |= isVisible_ && lockCameraWhenOverlayedOption_.value();
}

void Wheel::OnMouseButton(ScanCode sc, bool down, bool& rv)
{
	if (clickSelectOption_.value() && isVisible_) {
		const bool previousVisibility = isVisible_;
		isVisible_ = isVisible_ && sc != ScanCode::LBUTTON;
		if (!isVisible_ && previousVisibility) {
			rv = true;
			DeactivateWheel();
		}
	}
}

PreventPassToGame Wheel::KeybindEvent(bool center, bool activated)
{
	const bool previousVisibility = isVisible_;

	if (MumbleLink::i().isMapOpen())
		isVisible_ = false;
	else {
		WheelElement* bypassElement = nullptr;
		if (activated && doBypassWheel_(bypassElement) && !waitingForBypassComplete_) {
			isVisible_ = false;
			waitingForBypassComplete_ = true;
			SendKeybindOrDelay(bypassElement, std::nullopt);
		}

		if (waitingForBypassComplete_) {
			if (!activated)
				waitingForBypassComplete_ = false;
		}
		else {
			isVisible_ = activated;

			// If holding down the button is not necessary, modify behavior
			if (noHoldOption_.value() && previousVisibility && currentHovered_ == nullptr)
				isVisible_ = true;

			if (isVisible_ && !previousVisibility)
				ActivateWheel(center);
		}
	}

	if (!isVisible_ && previousVisibility)
		DeactivateWheel();

	return isVisible_ != previousVisibility;
}

#if 0
void Wheel::OnInputChange(bool changed, const ScanCodeSet& scs, const std::list<EventKey>& changedKeys, InputResponse& response)
{
	if (SettingsMenu::i().isVisible()) {
		bool isAnyElementBeingModified = keybind_.isBeingModified() || centralKeybind_.isBeingModified() ||
			std::any_of(wheelElements_.begin(), wheelElements_.end(),
													[](cref we) { return we->keybind().isBeingModified(); });

		if (isAnyElementBeingModified) 	{
			// If a key was lifted, we consider the key combination *prior* to this key being lifted as the keybind
			bool keyLifted = false;
			auto fullKeybind = scs;

			// Explicitly filter out M1 (left mouse button) from keybinds since it breaks too many things
			fullKeybind.erase(ScanCode::LBUTTON);

			for (cref ek : changedKeys) {
				if (IsSame(ek.sc, ScanCode::LBUTTON))
					continue;

				if (!ek.down) {
					fullKeybind.insert(ek.sc);
					keyLifted = true;
				}
			}


			keybind_.scanCodes(fullKeybind);
			centralKeybind_.scanCodes(fullKeybind);

			if (keyLifted) {
				keybind_.isBeingModified(false);
				centralKeybind_.isBeingModified(false);
			}

			for (auto& we : wheelElements_) {
				we->keybind().scanCodes(fullKeybind);
				if (keyLifted)
					we->keybind().isBeingModified(false);
			}

			response = InputResponse::PREVENT_ALL;
		}

		return;
	}


}
#endif

void Wheel::ActivateWheel(bool isMountOverlayLocked)
{
	auto& io = ImGui::GetIO();

	if (alwaysResetCursorPositionBeforeKeyPress_ || resetCursorAfterKeybindOption_.value())
	{
		cursorResetPosition_ = { static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y) };
		Log::i().Print(Severity::Debug, "Storing cursor position ({}, {}) for restore...", cursorResetPosition_->x, cursorResetPosition_->y);
	}
	else
		cursorResetPosition_.reset();

	// Mount overlay is turned on
	if (isMountOverlayLocked)
	{
		currentPosition_.x = currentPosition_.y = 0.5f;

		if (resetCursorOnLockedKeybindOption_.value())
			resetCursorPositionToCenter_ = true;
	}
	else
	{
		resetCursorPositionToCenter_ = false;
		currentPosition_.x = io.MousePos.x / float(Core::i().screenWidth());
		currentPosition_.y = io.MousePos.y / float(Core::i().screenHeight());
	}

	currentTriggerTime_ = TimeInMilliseconds();

	wipeMaskData_ = { frand() * 0.20f + 0.40f, frand() * 0.20f + 0.40f, frand() * 2 * float(M_PI) };

	UpdateHover();
}

void Wheel::DeactivateWheel()
{
	isVisible_ = false;
	resetCursorPositionToCenter_ = false;

	if(currentHovered_ == nullptr && centerCancelDelayedInputOption_.value()) {
	    conditionallyDelayed_ = nullptr;
	    conditionallyDelayedTime_ = TimeInMilliseconds();
		conditionallyDelayedTestPasses_ = false;

		return;
	}

	// If keybind release was done before the wheel is visible, check our behavior
	if(currentTriggerTime_ + displayDelayOption_.value() > TimeInMilliseconds())
	{
		switch(BehaviorBeforeDelay(behaviorOnReleaseBeforeDelay_.value()))
		{
		default:
			currentHovered_ = nullptr;
			break;
		case BehaviorBeforeDelay::FAVORITE:
			currentHovered_ = GetFavorite(delayFavoriteOption_.value());
			break;
		case BehaviorBeforeDelay::PREVIOUS:
			currentHovered_= previousUsed_;
			break;
		case BehaviorBeforeDelay::DIRECTION:
			if(!currentHovered_)
				currentHovered_ = GetCenterHoveredElement();
			break;
		}
	} else if(!currentHovered_)
		currentHovered_ = GetCenterHoveredElement();

	// Mount overlay is turned off, send the keybind
	if (currentHovered_!= nullptr) {
		SendKeybindOrDelay(currentHovered_, cursorResetPosition_);
	    previousUsed_ = currentHovered_;
	    currentHovered_ = nullptr;
	}
	else
		SendKeybindOrDelay(nullptr, cursorResetPosition_);
}

void Wheel::SendKeybindOrDelay(WheelElement* we, std::optional<Point> mousePos) {
	if (we == nullptr) {
		if (mousePos) {
			Log::i().Print(Severity::Debug, "Restoring cursor position ({}, {}).", cursorResetPosition_->x, cursorResetPosition_->y);
			Input::i().SendKeybind({}, mousePos);
		}
		return;
	}

	if (aboveWater_.delayed() || outOfCombat_.delayed()) {
		if (mousePos)
			Log::i().Print(Severity::Debug, "Restoring cursor position ({}, {}) and delaying keybind.", cursorResetPosition_->x, cursorResetPosition_->y);
		else
			Log::i().Print(Severity::Debug, "Delaying keybind.");
		Input::i().SendKeybind({}, mousePos);
		if (enableQueuingOption_.value())
		{
			conditionallyDelayed_ = we;
			conditionallyDelayedTime_ = TimeInMilliseconds();
			conditionallyDelayedTestPasses_ = false;
		}
	}
	else {
		if (mousePos)
			Log::i().Print(Severity::Debug, "Restoring cursor position ({}, {}) and sending keybind.", cursorResetPosition_->x, cursorResetPosition_->y);
		else
			Log::i().Print(Severity::Debug, "Sending keybind.");
		Input::i().SendKeybind(we->keybind().keyCombo(), mousePos);
	}
}


}
