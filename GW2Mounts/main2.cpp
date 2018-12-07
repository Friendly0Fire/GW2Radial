#include "main.h"
#include <tchar.h>
#include <imgui.h>
#include <examples\imgui_impl_dx9.h>
#include <examples\imgui_impl_win32.h>
#include <sstream>
#include "UnitQuad.h"
#include <d3dx9.h>
#include "Config.h"
#include "Utility.h"
#include <functional>
#include "minhook/include/MinHook.h"
#include <Shlwapi.h>
#include <d3d9.h>
#include "inputs.h"
#include "imgui_ext.h"



/*std::vector<MountType> GetActiveMounts()
{
	std::vector<MountType> ActiveMounts;
	for (uint i = 0; i < MountTypeCount; i++)
	{
		if (!Cfg.MountKeybind(i).empty())
			ActiveMounts.push_back((MountType)i);
	}

	return ActiveMounts;
}

std::vector<MountType> GetAllMounts()
{
	std::vector<MountType> AllMounts;
	for (uint i = 0; i < MountTypeCount; i++)
	{
		AllMounts.push_back((MountType)i);
	}

	return AllMounts;
}*/

bool FirstFrame = true;
std::vector<const char*> FavoriteMountNames(MountTypeCount);
