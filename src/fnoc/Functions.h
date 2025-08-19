#pragma once
#include "ScaleformValue.h"


namespace Functions {

	void* GetMajorFrame(void* hudMarketData);

	void* DisplayObjectDtor(BSGFxDisplayObject* obj);

	bool IsInCombatWithActor(Actor* actor1, Actor* actor2);

	float GetDistanceFromReference(TESObjectREFR* caller, TESObjectREFR* a1, TESObjectREFR* a2, UInt8 a3);
	 
	bool GetHostileToActor(Actor* actor1, Actor* actor2);


}


