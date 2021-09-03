#pragma once
#include <Main.h>
#include <Input.h>

namespace GW2Radial
{
struct KeyCombo
{
public:
	ScanCode& key = key_;
	Modifier& mod = mod_;

	KeyCombo() {
		key = ScanCode::NONE;
		mod = Modifier::NONE;
	}
	KeyCombo(ScanCode k, Modifier m) : key(k), mod(m) { }
	explicit KeyCombo(const std::set<ScanCode>& keys) : KeyCombo() {
		for (auto sc : keys) {
			if (IsModifier(sc))
				mod = mod | ToModifier(sc);
			else
				key = sc;
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