#include <Condition.h>
#include <MumbleLink.h>

#include <utility>
#include <sstream>

namespace GW2Radial {

bool IsInCombatCondition::test() const {
    return MumbleLink::i()->isInCombat();
}

bool IsWvWCondition::test() const {
    return MumbleLink::i()->isInWvW();
}

bool IsUnderwaterCondition::test() const {
    return MumbleLink::i()->isUnderwater();
}

bool IsProfessionCondition::test() const {
    return MumbleLink::i()->characterProfession() == profession_;
}

bool IsProfessionCondition::operator==(const IsProfessionCondition& other) const {
    return profession_ == other.profession_;
}

bool IsCharacterCondition::test() const {
    return characterName_ == std::wstring(MumbleLink::i()->characterName());
}

bool IsCharacterCondition::operator==(const IsCharacterCondition& other) const {
    return characterName_ == other.characterName_;
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
        std::vector<std::string> itemElems(3);
        SplitString(item.c_str(), "/", itemElems.begin());

        uint id = std::stol(itemElems[0]);
        auto op = static_cast<ConditionOp>(std::stol(itemElems[2]));
        std::unique_ptr<Condition> cond;

        // Use boolean operator to short-circuit evaluation
        MakeConditionIf<IsInCombatCondition>(itemElems[1], id, cond)   ||
        MakeConditionIf<IsWvWCondition>(itemElems[1], id, cond)        ||
        MakeConditionIf<IsUnderwaterCondition>(itemElems[1], id, cond) ||
        MakeConditionIf<IsProfessionCondition>(itemElems[1], id, cond) ||
        MakeConditionIf<IsCharacterCondition>(itemElems[1], id, cond);

        cond->Load(c);

        conditions_.emplace_back(cond, op);
    }
}

ConditionSet::ConditionSet(std::string category) : category_(std::move(category)) {
    Load();
}

bool ConditionSet::passes() const {
    bool finalResult = false;
    for(const auto& c : conditions_) {
        bool result = c.condition->passes();
        switch(c.prevOp) {
        case ConditionOp::AND:
            finalResult = finalResult && result;
            break;
        case ConditionOp::OR:
            finalResult = finalResult || result;
            break;
        }
    }

    return true;
}

bool ConditionSet::conflicts(const ConditionSet* otherPtr) const {
    if(!otherPtr)
        return true;

    const ConditionSet& other = *otherPtr;
    if(isInCombat.enable() && other.isInCombat.enable() && isInCombat.negate() != other.isInCombat.negate())
        return false;
    
    if(isWvW.enable() && other.isWvW.enable() && isWvW.negate() != other.isWvW.negate())
        return false;
    
    if(isUnderwater.enable() && other.isUnderwater.enable() && isUnderwater.negate() != other.isUnderwater.negate())
        return false;
    
    if(isProfession.enable() && other.isProfession.enable()) {
        if(isProfession.negate() != other.isProfession.negate()) {
            if((~isProfession.professionFlags() & other.isProfession.professionFlags()) == 0)
                return false;
        } else {
            if((isProfession.professionFlags() & other.isProfession.professionFlags()) == 0)
                return false;
        }
    }
    
    if(isCharacter.enable() && other.isCharacter.enable()) {
        auto otherB = other.isCharacter.characterNames().begin();
        auto otherE = other.isCharacter.characterNames().end();
        bool invert = isCharacter.negate() != other.isCharacter.negate();
        bool match = false;
        for(const auto& n : isCharacter.characterNames()) {
            auto find = std::find(otherB, otherE, n);
            if(invert && find == otherE || !invert && find != otherE) {
                match = true;
                break;
            }
        }

        if(!match)
            return false;
    }

    return true;
}

void ConditionSet::Save() const {
    const char* c = category_.c_str();

    std::stringstream set;

    for(const auto& cond : conditions_) {
        cond.condition->Save(c);
        set << cond.condition->id() << "/" << cond.condition->nickname() << "/" << uint(cond.prevOp) << ", ";
    }

    ConfigurationFile::i()->ini().SetValue(c, "condition_set", set.str().substr(0, set.str().size() - 2).c_str());
}

bool ConditionSet::DrawBaseMenuItem(Condition& c, const char* enableDesc, const char* disableDesc, std::optional<std::function<bool()>> extras) {
    bool dirty = false;

    std::string suffix = std::string("##") + enableDesc;

    bool b = c.enable();
    ImGui::Checkbox((" " + suffix).c_str(), &b);
    if(b != c.enable()) {
        c.enable(b);
        dirty = true;
    }
    ImGui::SameLine();

    if(c.enable()) {
        bool negate = c.negate();

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
    } else {
        ImGui::TextDisabled(disableDesc);
    }

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
