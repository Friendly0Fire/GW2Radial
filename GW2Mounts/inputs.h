#pragma once
#include "main.h"
#include <set>

extern std::set<uint> DownKeys;

void SendQueuedInputs();
void SendKeybind(const std::set<uint>& vkeys);