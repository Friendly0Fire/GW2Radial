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

class Condition {
protected:
    bool negate_ = false;
    bool enable_ = false;
    [[nodiscard]] virtual bool test() const = 0;
    [[nodiscard]] virtual std::string nickname() const = 0;
public:
    bool negate() const { return negate_; }
    bool enable() const { return enable_; }
    Condition& negate(bool negate) { negate_ = negate; return *this; }
    Condition& enable(bool enable) { enable_ = enable; return *this; }

    virtual ~Condition() = default;
    [[nodiscard]] bool operator()() const { return enable_ ? test() != negate_ : true; }
    
    
    virtual void Save(const char* category) const {
        ConfigurationFile::i()->ini().SetBoolValue(category, ("cond_enable_" + nickname()).c_str(), enable_);
        ConfigurationFile::i()->ini().SetBoolValue(category, ("cond_negate_" + nickname()).c_str(), negate_);
    }
    virtual void Load(const char* category) {
        enable_ = ConfigurationFile::i()->ini().GetBoolValue(category, ("cond_enable_" + nickname()).c_str(), false);
        negate_ = ConfigurationFile::i()->ini().GetBoolValue(category, ("cond_negate_" + nickname()).c_str(), false);
    }
};

class IsInCombatCondition final : public Condition {
    [[nodiscard]] bool test() const override;
    [[nodiscard]] std::string nickname() const override { return "in_combat"; }
};

class IsWvWCondition final : public Condition {
    [[nodiscard]] bool test() const override;
    [[nodiscard]] std::string nickname() const override { return "wvw"; }
};

class IsUnderwaterCondition final : public Condition {
    [[nodiscard]] bool test() const override;
    [[nodiscard]] std::string nickname() const override { return "underwater"; }
};

class IsProfessionCondition final : public Condition {
    uint32_t professionFlags_ = 0;

    [[nodiscard]] bool test() const override;
    [[nodiscard]] std::string nickname() const override { return "profession"; }

public:
    bool profession(MumbleLink::Profession id) const;
    void profession(MumbleLink::Profession id, bool enabled);
    [[nodiscard]] uint32_t professionFlags() const { return professionFlags_; }
    [[nodiscard]] bool operator==(const IsProfessionCondition& other) const;

    void Save(const char* category) const override {
        Condition::Save(category);
        ConfigurationFile::i()->ini().SetLongValue(category, ("cond_id_" + nickname()).c_str(), static_cast<long>(professionFlags_));
    }
    void Load(const char* category) override {
        professionFlags_ = static_cast<long>(ConfigurationFile::i()->ini().GetLongValue(category, ("cond_id_" + nickname()).c_str(), 0));
    }
};

class IsCharacterCondition final : public Condition {
    std::list<std::wstring> characterNames_;
    
    [[nodiscard]] bool test() const override;
    [[nodiscard]] std::string nickname() const override { return "character"; }

public:
    void character(wchar_t* name, bool add);
    [[nodiscard]] const std::list<std::wstring>& characterNames() const { return characterNames_; }
    [[nodiscard]] bool operator==(const IsCharacterCondition& other) const;

    void Save(const char* category) const override {
        Condition::Save(category);
        auto names = utf8_encode(std::accumulate(characterNames_.begin(), characterNames_.end(), std::wstring{}, [](const auto& a, const auto& b) { return a + L", " + b; }));
        ConfigurationFile::i()->ini().SetValue(category, ("cond_names_" + nickname()).c_str(), names.c_str());
    }
    void Load(const char* category) override {
        auto names = utf8_decode(ConfigurationFile::i()->ini().GetValue(category, ("cond_names_" + nickname()).c_str(), ""));
        characterNames_.clear();
        SplitString(names.c_str(), L",", characterNames_.begin());
    }
};

class ConditionSet {
    std::string category_;

    void Load();
    bool DrawBaseMenuItem(Condition& c, const char* enableDesc, const char* disableDesc, std::optional<std::function<bool()>> extras = std::nullopt);
public:
    explicit ConditionSet(std::string category);

    IsInCombatCondition isInCombat;
    IsWvWCondition isWvW;
    IsUnderwaterCondition isUnderwater;
    IsProfessionCondition isProfession;
    IsCharacterCondition isCharacter;

    Condition* conditions[5] = {
        &isInCombat,
        &isWvW,
        &isUnderwater,
        &isProfession,
        &isCharacter
    };

    [[nodiscard]] bool passes() const;
    [[nodiscard]] bool conflicts(const ConditionSet* other) const;
    void Save() const;
    void DrawMenu();
};
using ConditionSetPtr = std::shared_ptr<ConditionSet>;

}
