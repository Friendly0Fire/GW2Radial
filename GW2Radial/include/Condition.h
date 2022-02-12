#pragma once
#include <ConfigurationFile.h>
#include <Main.h>
#include <MumbleLink.h>
#include <list>
#include <charconv>
#include <functional>
#include <numeric>
#include <optional>
#include <imgui.h>
#include <Utility.h>

namespace GW2Radial
{

enum class MenuResult {
    NOTHING,
    DELETE_ITEM,
    MOVE_UP,
    MOVE_DOWN
};

struct ConditionContext {
    bool inCombat;
    bool inWvW;
    bool underwater;
    MumbleLink::Profession profession;
    MumbleLink::EliteSpec elitespec;
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

    [[nodiscard]] virtual bool DrawInnerMenu() = 0;

public:
    explicit Condition(uint id) : id_(id) {}
    virtual ~Condition() = default;
    
    [[nodiscard]] virtual std::string nickname() const = 0;

    [[nodiscard]] uint id() const { return id_; }

    [[nodiscard]] bool negate() const { return negate_; }
    Condition& negate(bool negate) { negate_ = negate; return *this; }

    [[nodiscard]] bool passes(const ConditionContext& cc) const { return test(cc) != negate_; }
    
    virtual void Save(const char* category) const {
        ConfigurationFile::i().ini().SetBoolValue(category, paramName("negate").c_str(), negate_);
    }
    virtual void Load(const char* category) {
        negate_ = ConfigurationFile::i().ini().GetBoolValue(category, paramName("negate").c_str(), false);
    }
    
    [[nodiscard]] bool DrawMenu(const char* category, MenuResult& mr, bool isFirst, bool isLast);
};

class IsInCombatCondition final : public Condition {
public:
    using Condition::Condition;
    inline static const char* Nickname = "in_combat";

private:
    [[nodiscard]] bool test(const ConditionContext& cc) const override { return cc.inCombat; }
    [[nodiscard]] std::string nickname() const override { return Nickname; }
    [[nodiscard]] bool DrawInnerMenu() override { ImGui::Text(" in combat"); return false; }
};

class IsWvWCondition final : public Condition {
public:
    using Condition::Condition;
    inline static const char* Nickname = "wvw";

private:
    [[nodiscard]] bool test(const ConditionContext& cc) const override { return cc.inWvW; }
    [[nodiscard]] std::string nickname() const override { return Nickname; }
    [[nodiscard]] bool DrawInnerMenu() override { ImGui::Text(" in WvW"); return false; }
};

class IsUnderwaterCondition final : public Condition {
public:
    using Condition::Condition;
    inline static const char* Nickname = "underwater";

private:
    [[nodiscard]] bool test(const ConditionContext& cc) const override { return cc.underwater; }
    [[nodiscard]] std::string nickname() const override { return Nickname; }
    [[nodiscard]] bool DrawInnerMenu() override { ImGui::Text(" underwater"); return false; }
};

class IsProfessionCondition final : public Condition {
public:
    using Condition::Condition;
    inline static const char* Nickname = "profession";

private:
    MumbleLink::Profession profession_;

    [[nodiscard]] bool test(const ConditionContext& cc) const override { return cc.profession == profession_; }
    [[nodiscard]] std::string nickname() const override { return Nickname; }
    [[nodiscard]] bool DrawInnerMenu() override;

public:
    [[nodiscard]] MumbleLink::Profession profession() const { return profession_; }
    void profession(MumbleLink::Profession id) { profession_ = id; }
    [[nodiscard]] bool operator==(const IsProfessionCondition& other) const {
        return profession_ == other.profession_;
    }

    void Save(const char* category) const override {
        Condition::Save(category);
        ConfigurationFile::i().ini().SetLongValue(category, paramName("id").c_str(), static_cast<long>(profession_));
    }
    void Load(const char* category) override {
        Condition::Load(category);
        profession_ = static_cast<MumbleLink::Profession>(ConfigurationFile::i().ini().GetLongValue(category, paramName("id").c_str(), 0));
    }
};

class IsEliteSpecCondition final : public Condition {
public:
    using Condition::Condition;
    inline static const char* Nickname = "elitespec";

private:
    MumbleLink::EliteSpec elitespec_;

    [[nodiscard]] bool test(const ConditionContext& cc) const override { return cc.elitespec == elitespec_; }
    [[nodiscard]] std::string nickname() const override { return Nickname; }
    [[nodiscard]] bool DrawInnerMenu() override;

public:
    [[nodiscard]] MumbleLink::EliteSpec elitespec() const { return elitespec_; }
    void elitespec(MumbleLink::EliteSpec id) { elitespec_ = id; }
    [[nodiscard]] bool operator==(const IsEliteSpecCondition& other) const {
        return elitespec_ == other.elitespec_;
    }

    void Save(const char* category) const override {
        Condition::Save(category);
        ConfigurationFile::i().ini().SetLongValue(category, paramName("id").c_str(), static_cast<long>(elitespec_));
    }
    void Load(const char* category) override {
        Condition::Load(category);
        elitespec_ = static_cast<MumbleLink::EliteSpec>(ConfigurationFile::i().ini().GetLongValue(category, paramName("id").c_str(), 0));
    }
};

class IsCharacterCondition final : public Condition {
public:
    using Condition::Condition;
    inline static const char* Nickname = "character";

private:
    std::wstring characterName_;
    
    [[nodiscard]] bool test(const ConditionContext& cc) const override { return ToCaseInsensitive(cc.character) == ToCaseInsensitive(characterName_); }
    [[nodiscard]] std::string nickname() const override { return Nickname; }
    [[nodiscard]] bool DrawInnerMenu() override;

public:
    [[nodiscard]] const std::wstring& characterName() const { return characterName_; }
    [[nodiscard]] bool operator==(const IsCharacterCondition& other) const {
        return ToCaseInsensitive(characterName_) == ToCaseInsensitive(other.characterName_);
    }

    void Save(const char* category) const override {
        Condition::Save(category);
        ConfigurationFile::i().ini().SetValue(category, paramName("charname").c_str(), utf8_encode(characterName_).c_str());
    }
    void Load(const char* category) override {
        Condition::Load(category);
        characterName_ = utf8_decode(ConfigurationFile::i().ini().GetValue(category, paramName("charname").c_str(), ""));
    }
};

enum class ConditionOp {
    NONE = 0,

    OR = 1,
    AND = 2
};

struct ConditionEntry {
    ConditionOp prevOp;
    std::unique_ptr<Condition> condition;
};

class ConditionSet {
    std::string category_;

    std::list<ConditionEntry> conditions_;

    int newConditionComboSel_ = 0;
    bool enabled_ = false;

    void Load();

    std::unique_ptr<Condition> CreateCondition(uint id) const;

    bool ConditionOperatorMenu(ConditionOp& op, uint id) const;
public:
    explicit ConditionSet(std::string category);

    void enable(bool e) { enabled_ = e; }

    [[nodiscard]] bool passes() const;
    [[nodiscard]] int score() const { return int(conditions_.size()); }
    void Save() const;
    void DrawMenu();
};
using ConditionSetPtr = std::shared_ptr<ConditionSet>;

}
