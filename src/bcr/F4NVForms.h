
#pragma once
#include "f4se\GameForms.h"
#include "Globals.h"

bool GetForms() {
	bool toReturn = true;

	checkAvif = reinterpret_cast<ActorValueInfo*>(GetFormFromIdentifier("Fallout4.esm|300"));

	checkAvif2 = reinterpret_cast<ActorValueInfo*>(GetFormFromIdentifier("Fallout4.esm|2FC"));

	return toReturn;
}