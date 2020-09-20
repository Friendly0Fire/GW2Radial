#pragma once
#include <ConfigurationFile.h>
#include <Main.h>
#include <MumbleLink.h>
#include <list>
#include <charconv>
#include <functional>
#include <numeric>
#include <optional>

namespace GW2Radial
{

struct ConditionContext {
    bool inCombat;
    bool inWvW;
    bool underwater;
    MumbleLink::Profession profession;
    std::wstring character;

    void Populate();
};

class Condition {
protected:
    uint id_ = 0;
    bool negate_ = false;
    [[nodiscard]] virtual bool test(const ConditionContext& cc) const = 0;

    std::string paramName(const char* param) const {
        return "condition_" + std::to_string(id_) + "_" + nickname() + "_" + param;
    }

public:
    explicit Condition(uint id) : id_(id) {}
    virtual ~Condition() = default;
    
    [[nodiscard]] virtual std::string nickname() const = 0;

    [[nodiscard]] uint id() const { return id_; }

    [[nodiscard]] bool negate() const { return negate_; }
    Condition& negate(bool negate) { negate_ = negate; return *this; }

    [[nodiscard]] bool passes(const ConditionContext& cc) const { return test(cc) != negate_; }
    
    virtual void Save(const char* category) const {
        ConfigurationFile::i()->ini().SetBoolValue(category, paramName("negate").c_str(), negate_);
    }
    virtual void Load(const char* category) {
        negate_ = ConfigurationFile::i()->ini().GetBoolValue(category, paramName("negate").c_str(), false);
    }
    
    [[nodiscard]] virtual bool DrawMenu() const;
};

class IsInCombatCondition final : public Condition {
    using Condition::Condition;

    [[nodiscard]] bool test(const ConditionContext& cc) const override { return cc.inCombat; }
    [[nodiscard]] std::string nickname() const override { return "in_combat"; }
};

class IsWvWCondition final : public Condition {
    using Condition::Condition;

    [[nodiscard]] bool test(const ConditionContext& cc) const override { return cc.inWvW; }
    [[nodiscard]] std::string nickname() const override { return "wvw"; }
};

class IsUnderwaterCondition final : public Condition {
    using Condition::Condition;

    [[nodiscard]] bool test(const ConditionContext& cc) const override { return cc.underwater; }
    [[nodiscard]] std::string nickname() const override { return "underwater"; }
};

class IsProfessionCondition final : public Condition {
    using Condition::Condition;
    MumbleLink::Profession profession_;

    [[nodiscard]] bool test(const ConditionContext& cc) const override { return cc.profession == profession_; }
    [[nodiscard]] std::string nickname() const override { return "profession"; }

public:
    [[nodiscard]] MumbleLink::Profession profession() const { return profession_; }
    void profession(MumbleLink::Profession id) { profession_ = id; }
    [[nodiscard]] bool operator==(const IsProfessionCondition& other) const {
        return profession_ == other.profession_;
    }

    void Save(const char* category) const override {
        Condition::Save(category);
        ConfigurationFile::i()->ini().SetLongValue(category, paramName("id").c_str(), static_cast<long>(profession_));
    }
    void Load(const char* category) override {
        Condition::Load(category);
        profession_ = static_cast<MumbleLink::Profession>(ConfigurationFile::i()->ini().GetLongValue(category, paramName("id").c_str(), 0));
    }
};

class IsCharacterCondition final : public Condition {
    using Condition::Condition;
    std::wstring characterName_;
    
    [[nodiscard]] bool test(const ConditionContext& cc) const override { return cc.character == characterName_; }
    [[nodiscard]] std::string nickname() const override { return "character"; }

public:
    [[nodiscard]] const std::wstring& characterName() const { return characterName_; }
    [[nodiscard]] bool operator==(const IsCharacterCondition& other) const {
        return characterName_ == other.characterName_;
    }

    void Save(const char* category) const override {
        Condition::Save(category);
        ConfigurationFile::i()->ini().SetValue(category, paramName("charname").c_str(), utf8_encode(characterName_).c_str());
    }
    void Load(const char* category) override {
        Condition::Load(category);
        characterName_ = utf8_decode(ConfigurationFile::i()->ini().GetValue(category, paramName("charname").c_str(), ""));
    }
};

enum class ConditionOp {
    NONE = 0,

    OR,
    AND,

    OPEN_PAREN,
    CLOSE_PAREN
};

using ConditionEntry = std::variant<Condition, ConditionOp>;

class ConditionSet {
    std::string category_;

    std::list<ConditionEntry> conditions_;

    void Load();
    bool DrawBaseMenuItem(Condition& c, const char* enableDesc, const char* disableDesc, std::optional<std::function<bool()>> extras = std::nullopt);
    bool ConditionIteration(const ConditionContext& cc, std::list<ConditionEntry>::const_iterator& it, std::optional<bool> prevResult = std::nullopt) const;
public:
    explicit ConditionSet(std::string category);

    [[nodiscard]] bool passes() const;
    void Save() const;
    void DrawMenu();
};
using ConditionSetPtr = std::shared_ptr<ConditionSet>;

}
