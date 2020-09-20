#include <Condition.h>
#include <MumbleLink.h>

#include <utility>
#include <sstream>

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

bool Condition::DrawMenu() const {
    bool dirty = false;

    std::string suffix = std::string("##") + nickname() + std::to_string(id());

    if(ImGui::RadioButton(("is" + suffix).c_str(), !c.negate()))
        c.negate(false);
    ImGui::SameLine();

    if(ImGui::RadioButton(("isn't" + suffix).c_str(), c.negate()))
        c.negate(true);
    ImGui::SameLine();

    if(negate != c.negate())
        dirty = true;

    ImGui::Text(enableDesc);

    if(extras) dirty = dirty || (*extras)();

    if(dirty)
        c.Save(category_.c_str());

    return dirty;
}

void ConditionSet::DrawMenu() {
    bool dirty = false;
    ImGui::Text("Only enable keybinds if character...");
    
    dirty |= DrawBaseMenuItem(isInCombat, "in combat and", "ignore combat state and");
    dirty |= DrawBaseMenuItem(isWvW, "in WvW and", "ignore WvW state and");
    dirty |= DrawBaseMenuItem(isUnderwater, "underwater and", "ignore underwater state and");
    dirty |= DrawBaseMenuItem(isProfession, "one of these professions:", "ignore profession and", [&]() {
            auto flags = isProfession.professionFlags();

            ImGui::Indent();

            auto chk = [&](const char* name, MumbleLink::Profession id) {
                bool b = isProfession.profession(id);
                ImGui::Checkbox(name, &b);
                isProfession.profession(id, b);
            };
            
            chk("Elementalist", MumbleLink::Profession::ELEMENTALIST);
            chk("Engineer", MumbleLink::Profession::ENGINEER);
            chk("Guardian", MumbleLink::Profession::GUARDIAN);
            chk("Mesmer", MumbleLink::Profession::MESMER);
            chk("Necromancer", MumbleLink::Profession::NECROMANCER);
            chk("Ranger", MumbleLink::Profession::RANGER);
            chk("Revenant", MumbleLink::Profession::REVENANT);
            chk("Thief", MumbleLink::Profession::THIEF);
            chk("Warrior", MumbleLink::Profession::WARRIOR);

            ImGui::Text("and");

            ImGui::Unindent();

            return flags != isProfession.professionFlags();
       });
    dirty |= DrawBaseMenuItem(isCharacter, "one of these characters:", "ignore character name.", [&]() {
            
            bool dirty = false;
            char buf[255];
            uint id = 0;
            
            ImGui::Indent();

            for (auto it = isCharacter.characterNames().begin(); it != isCharacter.characterNames().end();) {
                auto c8 = utf8_encode(*it);
                if(c8.size() >= 255)
                    c8 = c8.substr(0, 254);
                std::copy(std::begin(c8), std::end(c8), buf);
                buf[c8.size()] = '\0';
                if(ImGui::InputText(("##char" + std::to_string(id++)).c_str(), buf, 255)) {
                    *it = utf8_decode(buf);
                    dirty = true;
                }

                ImGui::SameLine();
                if(ImGui::Button(("-##char" + std::to_string(id++)).c_str())) {
                    it = isCharacter.characterNames().erase(it);
                    dirty = true;
                } else
                    ++it;
            }
            if(ImGui::Button("+##chars")) {
                isCharacter.characterNames().emplace_back(L"");
                dirty = true;
            }

            ImGui::Unindent();

            return dirty;
       });

    if(dirty)
        ConfigurationFile::i()->Save();
}

}
