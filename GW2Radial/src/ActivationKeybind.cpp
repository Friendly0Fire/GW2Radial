#include <ActivationKeybind.h>

namespace GW2Radial
{
void ActivationKeybind::CheckForConflict(bool recurse)
{
	isConflicted_ = false;
	for(auto* kb : keybinds_)
	{
		auto* other = dynamic_cast<ActivationKeybind*>(kb);
		if(!other)
			continue;

		if(other != this && other->category_ == category_ && !scanCodes_.empty() && kb->scanCodes() == scanCodes_ && conditions_.conflicts(conditions_))
			isConflicted_ = true;
		
		if(recurse) other->CheckForConflict(false);
	}
}

bool ActivationKeybind::conflicts(const std::set<ScanCode>& scanCodes) const
{
	for(auto* kb : keybinds_)
	{
		auto* other = dynamic_cast<ActivationKeybind*>(kb);
		if(!other)
			continue;

		if(other != this && scanCodes_.size() < kb->scanCodes().size()
			&& std::includes(scanCodes.begin(), scanCodes.end(), kb->scanCodes().begin(), kb->scanCodes().end()) && conditions_.conflicts(other->conditions_))
			return true;
	}

	return false;
}

void ActivationKeybind::ApplyKeys() {
    CheckForConflict();
	Keybind::ApplyKeys();
}

};