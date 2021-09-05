#pragma once
#include <Main.h>
#include <Input.h>

namespace GW2Radial
{
struct KeyCombo
{
public:
	ScanCode& key() { return key_; }
	Modifier& mod() { return mod_; }
	ScanCode key() const { return key_; }
	Modifier mod() const { return mod_; }

	KeyCombo() {
		key_ = ScanCode::NONE;
		mod_ = Modifier::NONE;
	}
	KeyCombo(ScanCode k, Modifier m) : key_(k), mod_(m) { }
	explicit KeyCombo(const std::set<ScanCode>& keys) : KeyCombo() {
		for (auto sc : keys) {
			if (IsModifier(sc))
				mod_ = mod_ | ToModifier(sc);
			else
				key_ = sc;
		}
	}

private:
	union {
		uint64_t storage_;
		struct {
			ScanCode key_;
			Modifier mod_;
		};
	};

	friend std::partial_ordering operator<=>(const KeyCombo& a, const KeyCombo& b);
};

inline std::partial_ordering operator<=>(const KeyCombo& a, const KeyCombo& b)
{
	return a.storage_ <=> b.storage_;
}
}