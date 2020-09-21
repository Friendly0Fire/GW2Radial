#include <Condition.h>
#include <MumbleLink.h>

#include <utility>
#include <sstream>
#include <array>

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
    if(type == T::nickname()) {
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
        characterName_ = utf8_decode(std::string(buf));
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
            if(item == "OR")
                conditions_.emplace_back(ConditionOp::OR);
            else if(item == "AND")
                conditions_.emplace_back(ConditionOp::AND);
            else if(item == "OPEN_PAREN")
                conditions_.emplace_back(ConditionOp::OPEN_PAREN);
            else if(item == "CLOSE_PAREN")
                conditions_.emplace_back(ConditionOp::CLOSE_PAREN);
        } else {
            std::vector<std::string> itemElems(2);
            SplitString(item.c_str(), "/", itemElems.begin());

            uint id = std::stol(itemElems[0]);
            std::unique_ptr<Condition> cond;

            // Use boolean operator to short-circuit evaluation
            MakeConditionIf<IsInCombatCondition>(itemElems[1], id, cond)   ||
            MakeConditionIf<IsWvWCondition>(itemElems[1], id, cond)        ||
            MakeConditionIf<IsUnderwaterCondition>(itemElems[1], id, cond) ||
            MakeConditionIf<IsProfessionCondition>(itemElems[1], id, cond) ||
            MakeConditionIf<IsCharacterCondition>(itemElems[1], id, cond);

            cond->Load(c);

            conditions_.emplace_back(cond);
        }
    }
}

ConditionSet::ConditionSet(std::string category) : category_(std::move(category)) {
    Load();
}

bool ConditionSet::ConditionIteration(const ConditionContext& cc, std::list<ConditionEntry>::const_iterator& it, std::optional<bool> prevResult) const {
    if(it == conditions_.end())
        return prevResult ? *prevResult : false;

    const auto& e = *it;

    if(std::holds_alternative<ConditionOp>(e)) {
        GW2_ASSERT(prevResult.has_value());

        auto op = std::get<ConditionOp>(e);
        switch(op) {
        case ConditionOp::OR:
            return *prevResult || ConditionIteration(cc, ++it);
        case ConditionOp::AND:
            return *prevResult && ConditionIteration(cc, ++it);
        case ConditionOp::OPEN_PAREN:
            bool subexpression = ConditionIteration(cc, ++it);
            return ConditionIteration(cc, ++it, subexpression);
        case ConditionOp::CLOSE_PAREN:
            return *prevResult;
        default:
            return ConditionIteration(cc, ++it, prevResult);
        }
    } else {
        GW2_ASSERT(!prevResult.has_value());

        const Condition& c = std::get<Condition>(e);
        return ConditionIteration(cc, ++it, c.passes(cc));
    }
}

bool ConditionSet::passes() const {
    ConditionContext cc;
    cc.Populate();

    auto it = conditions_.begin();
    return ConditionIteration(cc, it);
}

void ConditionSet::Save() const {
    const char* c = category_.c_str();

    std::stringstream set;

    for(const auto& ce : conditions_) {
        if(std::holds_alternative<Condition>(ce)) {
            const auto& cond = std::get<Condition>(ce);
            cond.Save(c);
            set << cond.id() << "/" << cond.nickname() << ", ";
        } else {
            switch(std::get<ConditionOp>(ce)) {
            case ConditionOp::OR:
                set << "OR, ";
                break;
            case ConditionOp::AND:
                set << "AND, ";
                break;
            case ConditionOp::OPEN_PAREN:
                set << "OPEN_PAREN, ";
                break;
            case ConditionOp::CLOSE_PAREN:
                set << "CLOSE_PAREN, ";
                break;
            default: ;
            }
        }
    }

    ConfigurationFile::i()->ini().SetValue(c, "condition_set", set.str().substr(0, set.str().size() - 2).c_str());
}

bool Condition::DrawMenu(const char* category) {
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

    if(dirty)
        Save(category);

    return dirty;
}

bool ConditionSet::ConditionOperatorMenu(ConditionOp& op, uint id, int& indentCount) const {
    auto suffix = "##condition_operator_" + std::to_string(id);

    if(op == ConditionOp::CLOSE_PAREN && indentCount <= 0) {
        op = ConditionOp::OR;
        indentCount = 0;
    }

    int comboVal = static_cast<int>(op) - 1;
    const char* items = indentCount > 0 ? "OR\0AND\0(\0)\0" : "OR\0AND\0(\0";
    if(ImGui::Combo(suffix.c_str(), &comboVal, items)) {
        op = static_cast<ConditionOp>(comboVal + 1);
        return true;
    }

    if(op == ConditionOp::OPEN_PAREN) {
        ImGui::Indent();
        indentCount++;
    } else if(op == ConditionOp::CLOSE_PAREN && indentCount > 0) {
        ImGui::Unindent();
        indentCount--;
    }

    return false;
}

void ConditionSet::DrawMenu() {
    bool dirty = false;
    ImGui::Text("Only enable keybinds if character...");

    uint id = 0;
    int indentCount = 0;

    for(auto& ce : conditions_) {
        if(std::holds_alternative<ConditionOp>(ce))
            dirty |= ConditionOperatorMenu(std::get<ConditionOp>(ce), id, indentCount);
        else
            dirty |= std::get<Condition>(ce).DrawMenu(category_.c_str());
        id++;
    }

    while(indentCount > 0) {
        indentCount--;
        ImGui::Unindent();
    }



    if(dirty)
        ConfigurationFile::i()->Save();
}

}
