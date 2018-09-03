#pragma once
#include "main.h"
#include <set>

extern std::set<uint> DownKeys;

// Full list of hooked message IDs
extern uint id_H_LBUTTONDOWN;
extern uint id_H_LBUTTONUP;
extern uint id_H_RBUTTONDOWN;
extern uint id_H_RBUTTONUP;
extern uint id_H_MBUTTONDOWN;
extern uint id_H_MBUTTONUP;
extern uint id_H_SYSKEYDOWN;
extern uint id_H_SYSKEYUP;
extern uint id_H_KEYDOWN;
extern uint id_H_KEYUP;

void InitializeHookedMessages();
uint ConvertHookedMessage(uint msg);
void SendQueuedInputs();
void SendKeybind(const std::set<uint>& vkeys);