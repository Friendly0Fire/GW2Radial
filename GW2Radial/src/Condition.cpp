#include <Condition.h>
#include <MumbleLink.h>

#include <utility>
#include <sstream>
#include <array>
#include <ranges>

namespace GW2Radial {

void ConditionContext::Populate() {
    auto mlPtr = MumbleLink::i();
    if(!mlPtr)
        return;

    const auto& ml = *mlPtr;
    
    inCombat = ml.isInCombat();
    inWvW = ml.isInWvW();
    underwater = ml.isUnderwater();
    profession = ml.characterProfession();
    character = std::wstring(ml.characterName());
}

template<typename T>
bool MakeConditionIf(const std::string& type, uint id, std::unique_ptr<Condition>& cond) {
    if(type == T::Nickname) {
        cond = std::make_unique<T>(id);
        return true;
    }

    return false;
}

bool IsProfessionCondition::DrawInnerMenu() {
    auto suffix = "##condition_profession_" + std::to_string(id_);

    int comboVal = static_cast<int>(profession_) - 1;
    const char* items = "Guardian\0Warrior\0Engineer\0Ranger\0Thief\0Elementalist\0Mesmer\0Necromancer\0Revenant\0";
    if(ImGui::Combo(suffix.c_str(), &comboVal, items)) {
        profession_ = static_cast<MumbleLink::Profession>(comboVal + 1);
        return true;
    }

    return false;
}

bool IsCharacterCondition::DrawInnerMenu() {
    auto suffix = "##condition_profession_" + std::to_string(id_);

    auto utf8name = utf8_encode(characterName_);
    std::array<char, 255> buf;
    std::copy(utf8name.begin(), utf8name.end(), buf.begin());
    if(ImGui::InputText(suffix.c_str(), buf.data(), buf.size())) {
        characterName_ = utf8_decode(std::string(buf.data()));
        return true;
    }

    return false;
}

void ConditionSet::Load() {
    const char* c = category_.c_str();
    const auto& ini = ConfigurationFile::i()->ini();

    const char* set = ini.GetValue(c, "condition_set", "");
    std::list<std::string> setList;
    SplitString(set, ",", std::back_inserter(setList));
    for(const auto& item : setList) {
        if(item.find('/') == std::string::npos) {
        } else {
            std::vector<std::string> itemElems(3);
            SplitString(item.c_str(), "/", itemElems.begin());
            ConditionOp op = ConditionOp::NONE;
            
            if(itemElems[0] == "OR")
                op = ConditionOp::OR;
            else if(itemElems[0] == "AND")
                op = ConditionOp::AND;

            uint id = std::stol(itemElems[1]);
            std::unique_ptr<Condition> cond;

            // Use boolean operator to short-circuit evaluation
            MakeConditionIf<IsInCombatCondition>(itemElems[2], id, cond)   ||
            MakeConditionIf<IsWvWCondition>(itemElems[2], id, cond)        ||
            MakeConditionIf<IsUnderwaterCondition>(itemElems[2], id, cond) ||
            MakeConditionIf<IsProfessionCondition>(itemElems[2], id, cond) ||
            MakeConditionIf<IsCharacterCondition>(itemElems[2], id, cond);

            cond->Load(c);

            conditions_.push_back({ op, std::move(cond) });
        }
    }
}

ConditionSet::ConditionSet(std::string category) : category_(std::move(category)) {
    Load();
}

bool ConditionSet::passes() const {
    ConditionContext cc;
    cc.Populate();

    bool result = true;

    for(const auto& c : conditions_) {
        if(c.prevOp == ConditionOp::AND)
            result = result && c.condition->passes(cc);
        else if(c.prevOp == ConditionOp::OR)
            result = result || c.condition->passes(cc);
        else
            result = c.condition->passes(cc);
    }

    return result;
}

void ConditionSet::Save() const {
    const char* cat = category_.c_str();

    std::stringstream set;

    for(const auto& c : conditions_) {
        c.condition->Save(cat);
        switch(c.prevOp) {
        case ConditionOp::OR:
            set << "OR/";
            break;
        case ConditionOp::AND:
            set << "AND/";
            break;
        case ConditionOp::NONE:
        default:
            set << "NONE/";
        }
        set << c.condition->id() << "/" << c.condition->nickname() << ", ";
    }

    ConfigurationFile::i()->ini().SetValue(cat, "condition_set", set.str().substr(0, set.str().size() - 2).c_str());
}

bool Condition::DrawMenu(const char* category, MenuResult& mr, bool isFirst, bool isLast) {
    bool dirty = false;

    std::string suffix = std::string("##") + nickname() + std::to_string(id());

    bool prevNegate = negate();

    if(ImGui::RadioButton(("is" + suffix).c_str(), !negate_))
        negate_ = false;
    ImGui::SameLine();

    if(ImGui::RadioButton(("isn't" + suffix).c_str(), negate_))
        negate_ = true;
    ImGui::SameLine();

    if(prevNegate != negate_)
        dirty = true;

    dirty = dirty || DrawInnerMenu();

    mr = MenuResult::NOTHING;

    ImGui::SameLine();

    if(ImGui::Button(("⮽" + suffix).c_str()))
        mr = MenuResult::DELETE_ITEM;

    if(!isFirst) {
        if(ImGui::Button(("⭡" + suffix).c_str()))
            mr = MenuResult::MOVE_UP;
    }

    if(!isLast) {
        if(ImGui::Button(("⭣" + suffix).c_str()))
            mr = MenuResult::MOVE_DOWN;
    }

    if(dirty)
        Save(category);

    return dirty;
}

bool ConditionSet::ConditionOperatorMenu(ConditionOp& op, uint id) const {
    auto suffix = "##condition_operator_" + std::to_string(id);

    int comboVal = static_cast<int>(op) - 1;
    if(ImGui::Combo(suffix.c_str(), &comboVal, "OR\0AND\0")) {
        op = static_cast<ConditionOp>(comboVal + 1);
        return true;
    }

    return false;
}

std::unique_ptr<Condition> ConditionSet::CreateCondition(uint id) const {
    switch(newConditionComboSel_) {
    case 0:
        return std::make_unique<IsInCombatCondition>(id);
    case 1:
        return std::make_unique<IsWvWCondition>(id);
    case 2:
        return std::make_unique<IsUnderwaterCondition>(id);
    case 3:
        return std::make_unique<IsProfessionCondition>(id);
    case 4:
        return std::make_unique<IsCharacterCondition>(id);
    default:
        return nullptr;
    }
}

void ConditionSet::DrawMenu() {
    bool dirty = false;
    ImGui::Text("Only enable keybinds if character...");

    uint id = 0;

    for(auto it = conditions_.begin(); it != conditions_.end();) {
        if(id > 0)
            dirty |= ConditionOperatorMenu(it->prevOp, id);

        bool isFirst = it == conditions_.begin();
        bool isLast = conditions_.size() <= 1 || it == --conditions_.end();

        MenuResult mr;
        dirty |= it->condition->DrawMenu(category_.c_str(), mr, isFirst, isLast);

        switch(mr) {
        case MenuResult::DELETE_ITEM:
            it = conditions_.erase(it);
            dirty = true;
            break;
        case MenuResult::MOVE_UP:
            if(!isFirst) {
                auto itPrev = it;
                --itPrev;
                std::iter_swap(it, itPrev);
                dirty = true;
            }
            break;
        case MenuResult::MOVE_DOWN:
            if(!isLast) {
                auto itNext = it;
                ++itNext;
                std::iter_swap(it, itNext);
                dirty = true;
            }
            break;
        default:
            break;
        }

        if(mr != MenuResult::DELETE_ITEM)
            ++it;

        id++;
    }

    const char* items = "In combat\0In WvW\0Underwater\0Is profession\0Is character\0";
    ImGui::Combo("##NewConditionCombo", &newConditionComboSel_, items);

    if(ImGui::Button("Add Condition")) {
        conditions_.push_back({ ConditionOp::OR, CreateCondition(id) });
        dirty = true;
    }

    if(dirty) {
        Save();
        ConfigurationFile::i()->Save();
    }
}

}
