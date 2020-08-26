#include <Condition.h>
#include <MumbleLink.h>

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
    return (uint32_t(MumbleLink::i()->characterProfession()) << 1) & professionFlags_;
}

void IsProfessionCondition::profession(MumbleLink::Profession id, bool enabled) {
    if(enabled)
        professionFlags_ |= 1 << uint32_t(id);
    else
        professionFlags_ &= ~(1 << uint32_t(id));
}

bool IsProfessionCondition::operator==(const IsProfessionCondition& other) const {
    return professionFlags_ == other.professionFlags_;
}

bool IsCharacterCondition::test() const {
    return std::find(characterNames_.begin(), characterNames_.end(), std::wstring(MumbleLink::i()->characterName())) != characterNames_.end();
}

void IsCharacterCondition::character(wchar_t* name, bool add) {
    auto it = std::find(characterNames_.begin(), characterNames_.end(), std::wstring(name));
    if(add && it == characterNames_.end())
        characterNames_.emplace_back(name);
    else if(!add && it != characterNames_.end())
        characterNames_.erase(it);
}

bool IsCharacterCondition::operator==(const IsCharacterCondition& other) const {
    return std::equal(characterNames_.begin(), characterNames_.end(), other.characterNames_.begin(), other.characterNames_.end());
}

bool ConditionSet::operator()() const {
    for(const auto* c : conditions)
        if(!(*c)())
            return false;

    return true;
}

bool ConditionSet::conflicts(const ConditionSet& other) const {
    if(isInCombat.enable_ && other.isInCombat.enable_ && isInCombat.negate_ != other.isInCombat.negate_)
        return false;
    
    if(isWvW.enable_ && other.isWvW.enable_ && isWvW.negate_ != other.isWvW.negate_)
        return false;
    
    if(isUnderwater.enable_ && other.isUnderwater.enable_ && isUnderwater.negate_ != other.isUnderwater.negate_)
        return false;
    
    if(isProfession.enable_ && other.isProfession.enable_) {
        if(isProfession.negate_ != other.isProfession.negate_) {
            if((~isProfession.professionFlags() & other.isProfession.professionFlags()) == 0)
                return false;
        } else {
            if((isProfession.professionFlags() & other.isProfession.professionFlags()) == 0)
                return false;
        }
    }
    
    if(isCharacter.enable_ && other.isCharacter.enable_) {
        auto otherB = other.isCharacter.characterNames().begin();
        auto otherE = other.isCharacter.characterNames().end();
        bool invert = isCharacter.negate_ != other.isCharacter.negate_;
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

}
