#pragma once
#include <Main.h>
#include <MumbleLink.h>
#include <list>

namespace GW2Radial
{

class Condition {
protected:
    bool negate_ = false;
    bool enable_ = false;
    [[nodiscard]] virtual bool test() const = 0;
public:
    Condition& negate(bool negate) { negate_ = negate; return *this; }
    Condition& enable(bool enable) { enable_ = enable; return *this; }

    virtual ~Condition() = default;
    [[nodiscard]] bool operator()() const { return enable_ ? true : test() != negate_; }

    friend struct ConditionSet;
};

class IsInCombatCondition final : public Condition {
    [[nodiscard]] bool test() const override;
};

class IsWvWCondition final : public Condition {
    [[nodiscard]] bool test() const override;
};

class IsProfessionCondition final : public Condition {
    uint32_t professionFlags_;

    [[nodiscard]] bool test() const override;

public:
    void profession(MumbleLink::Profession id, bool enabled);
    [[nodiscard]] uint32_t professionFlags() const { return professionFlags_; }
    [[nodiscard]] bool operator==(const IsProfessionCondition& other) const;
};

class IsCharacterCondition final : public Condition {
    std::list<std::wstring> characterNames_;
    
    [[nodiscard]] bool test() const override;

public:
    void character(wchar_t* name, bool add);
    [[nodiscard]] const std::list<std::wstring>& characterNames() const { return characterNames_; }
    [[nodiscard]] bool operator==(const IsCharacterCondition& other) const;
};

struct ConditionSet {
    IsInCombatCondition isInCombat;
    IsWvWCondition isWvW;
    IsProfessionCondition isProfession;
    IsCharacterCondition isCharacter;

    Condition* conditions[4] = {
        &isInCombat,
        &isWvW,
        &isProfession,
        &isCharacter
    };

    [[nodiscard]] bool operator()() const;
    [[nodiscard]] bool conflicts(const ConditionSet& other) const;
};

}
