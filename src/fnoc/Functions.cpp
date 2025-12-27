#include "Functions.h"

namespace Functions {

	typedef void* (*_DisplayObjectDtor)(BSGFxDisplayObject* obj);
	RelocAddr <_DisplayObjectDtor> DisplayObjectDtorInternal(0x1B17790); //11.91

	typedef void* (*_GetMajorFrame)(void* obj, void* obj2);
	RelocAddr <_GetMajorFrame> GetMajorFrameInternal(0x09C9150); //11.91

	typedef bool(*_IsInCombatWithActorInternal)(Actor* actor1, Actor* actor2);
	RelocAddr <_IsInCombatWithActorInternal> IsInCombatWithActorInternal(0x0CD0420); //11.91

	typedef float(*_GetDistanceFromReference)(TESObjectREFR* caller, TESObjectREFR* a1, TESObjectREFR* a2, UInt8 a3);
	RelocAddr <_GetDistanceFromReference> GetDistanceFromReferenceInternal(0x0517330); //11.91

	typedef bool(*_GetHostileToActorInternal)(Actor* actor1, Actor* actor2);
	RelocAddr <_GetHostileToActorInternal> GetHostileToActorInternal(0x0C7B680); //11.91

	void* GetMajorFrame(void* hudMarketData) {
		return GetMajorFrameInternal(hudMarketData, hudMarketData);
	}

	void* DisplayObjectDtor(BSGFxDisplayObject* obj) {
		return DisplayObjectDtorInternal(obj);
	}

	bool IsInCombatWithActor(Actor* actor1, Actor* actor2)
	{
		return IsInCombatWithActorInternal(actor1, actor2);
	}

	float GetDistanceFromReference(TESObjectREFR* caller, TESObjectREFR* a1, TESObjectREFR* a2, UInt8 a3) {
		return GetDistanceFromReferenceInternal(caller, a1, a2, a3);
	}

	bool GetHostileToActor(Actor* actor1, Actor* actor2)
	{
		return GetHostileToActorInternal(actor1, actor2);
	}
}
