#include "Compass.h"
#include "xbyak/xbyak.h"

ActorValueInfo* AffectingAvif;
ActorValueInfo* HasImprovedDetection;
ActorValueInfo* Invisibility;


namespace Compass {

	float GetCappedCompareDistance() {
		auto maxDistance = fMaxActorCompassDetectionDistance;
		auto perceptionMult = fActorCompassDetectionActorValueMult;
		auto minDistance = fMinActorCompassDetectionDistance;


		auto perception = (*g_player)->actorValueOwner.GetValue(AffectingAvif);

		auto calculatedDistance = minDistance * (perception * perceptionMult);


		auto cappedDistance = min(maxDistance, max(minDistance, calculatedDistance));
		return cappedDistance;
	}


	typedef bool(*_OuterIsInCombatHook)(Actor* a1, Actor* player);
	RelocAddr <_OuterIsInCombatHook> OuterIsInCombat_HookTarget(0x0DA51C1); //11.191
	//4C 8B DC 55 41 56 48 83 EC ? 8B 41


	bool OuterIsInCombat_Hook(Actor* ref, Actor* player) {

		if (ref && (ref->formType == kFormType_REFR || ref->formType == kFormType_ACHR)) {

			if (ref->baseForm->formType == kFormType_NPC_) {
				if (ref->IsDead()) return 0;
				if (ref->flags & TESForm::kFlag_IsDeleted) return 0;
				if (ref->flags & TESForm::kFlag_IsDisabled) return 0;
				if (ref->baseForm->flags & ActorBaseFlags::kSimpleActor) return 0;
				if (ref->GetAlpha() == 0) return 0;



				TESActorBase* abase = (TESActorBase*)ref->baseForm;
				if (abase) {
					if (abase->actorData.GetIsGhost()) return 0;
					
				}

				auto actor = (Actor*)ref;
				//if (actor && actor->GetAlpha() == 0) return 0;

				auto dist = Functions::GetDistanceFromReference((*g_player), ref, (*g_player), 0);
				auto maxDistance = fMaxActorCompassDetectionDistance;

				if ((*g_player)->actorValueOwner.GetValue(HasImprovedDetection) > 0) {

					if (dist > maxDistance) return 0;

				}
				else {

					auto perceptionMult = fActorCompassDetectionActorValueMult;
					auto minDistance = fMinActorCompassDetectionDistance;


					auto perception = (*g_player)->actorValueOwner.GetValue(AffectingAvif);

					auto calculatedDistance = minDistance * (perception * perceptionMult);

					if (Functions::IsInCombatWithActor(actor, (*g_player))) {
						auto hostileMult = fActorCompassDetectionHostileMult;
						calculatedDistance *= hostileMult;

						auto cappedDistance = min(maxDistance, max(minDistance, calculatedDistance));

						if (dist > cappedDistance) return 0;
					}
					else {
						if (ref->actorValueOwner.GetValue(Invisibility) > 0) return 0;

						auto cappedDistance = min(maxDistance, max(minDistance, calculatedDistance));

						if (dist > cappedDistance) return 0;
					}
				}
				return 1;
			}
			return 0;
		}

		return 0;

	}


	typedef void* (*_HUDMarkerDataHook)(void* a1, TESObjectREFR* ref, int markerType, UInt8 IsReconScopeTarget, UInt8 hostile);
	RelocAddr <_HUDMarkerDataHook> HUDMarkerData_HookTarget(0x09C8EE0); //11.91
	_HUDMarkerDataHook HUDMarkerDataHook_Original;

	void* HUDMarkerData_Hook(void* a1, TESObjectREFR* ref, int markerType, UInt8 isReconScopeTarget, UInt8 hostile) {
		if (ref) {
			if (markerType == HUDMarkerData::MarkerIconTypes::MIT_MARKER_ENEMY) {
				if (ref->baseForm->formType == kFormType_NPC_) {
					if (Functions::GetHostileToActor((Actor*)ref, (*g_player))) {
						hostile = 1; 
					}
					else {
						hostile = 0;
					}
				}
			}
		}

		return HUDMarkerDataHook_Original(a1, ref, markerType, isReconScopeTarget, hostile);
	}


	float CalculateMarkerDistanceDisplay(float distanceSqr, float max)
	{
		float val = max;
		float fMarkerRevealDistSqr = val * val;
		float DefaultScale = fDefaultNPCMarkerScale;
		float MinScale = fMinNPCMarkerScale;
		constexpr float HUDScale = 18.0f;

		float markerScale;

		if (fMarkerRevealDistSqr == 0.0f)
		{
			markerScale = DefaultScale;
		}
		else
		{
			float dist = distanceSqr;
			if (dist >= fMarkerRevealDistSqr)
				dist = fMarkerRevealDistSqr;

			// Linear interpolation from DefaultScale (when close) to MinScale (when far)
			markerScale = ((dist - fMarkerRevealDistSqr) / -fMarkerRevealDistSqr) * (DefaultScale - MinScale) + MinScale;
		}

		//return (HUDScale * 0.0099999998f) * markerScale; // engine does this we dont need it
		return markerScale;

	}

	typedef void* (*_BSGFxDisplayObjectDtor)(BSGFxDisplayObject* obj);
	RelocAddr <_BSGFxDisplayObjectDtor> BSGFxDisplayObjectDtor_HookTarget(0x09BBCCC); //11.91


	HUDMarkerData* ActiveMarker = nullptr;

	void* BSGFxDisplayObjectDtor_Hook(BSGFxDisplayObject* obj) {
		if (!obj) {
			ActiveMarker = nullptr;
			return Functions::DisplayObjectDtor(obj);
		}

		auto type = obj->GetType();
		if (type == GFxValue::kType_DisplayObject) {
			GFxValue::DisplayInfo info = {};
			obj->GetDisplayInfo(&info);
			if (ActiveMarker) {
				auto markerType = ActiveMarker->markerIconType;
				if (markerType == HUDMarkerData::MarkerIconTypes::MIT_MARKER_ENEMY || markerType == HUDMarkerData::MarkerIconTypes::MIT_MARKER_ENEMY_TARGETED) {

					auto cappedDist = GetCappedCompareDistance();
					auto value = CalculateMarkerDistanceDisplay(ActiveMarker->distanceToPlayerSqr, cappedDist);
					info.SetScale(value, value);
					info.SetAlpha(value);

				}
			}

			obj->SetDisplayInfo(&info);

		}
		ActiveMarker = nullptr;
		return Functions::DisplayObjectDtor(obj);
	}


	typedef float (*GetHostileEnemyMaxDistanceHook)();
	RelocAddr <GetHostileEnemyMaxDistanceHook> GetHostileEnemyMaxDistance_HookTarget(0x09D0110);//11.91
	GetHostileEnemyMaxDistanceHook GetHostileEnemyMaxDistanceHook_Original;


	float GetHostileEnemyMaxDistance_Hook() {
		//float orig = GetHostileEnemyMaxDistanceHook_Original();
		if (fMaxActorCompassDetectionDistance) {
			return fMaxActorCompassDetectionDistance;
		}

		return GetHostileEnemyMaxDistanceHook_Original();
	}

	typedef void* (*_GetMajorFrame)(BSGFxDisplayObject* obj);
	RelocAddr <_GetMajorFrame> GetMajorFrame_HookTarget(0x09BB72C); //11.91


	void* GetMajorFrame_Hook(HUDMarkerData* hud, void* data2) {
		ActiveMarker = hud;

		return Functions::GetMajorFrame(hud);
	}


	void InitHooks() {

		if (bFriendlyNPCMarkers) {
			{
				struct HUDMarkerDataHook_Code : Xbyak::CodeGenerator {
					HUDMarkerDataHook_Code(void* buf) : Xbyak::CodeGenerator(4096, buf)
					{
						Xbyak::Label retnLabel;
						//Xbyak::Label foo;

						mov(ptr[rsp + 0x08], rbx);

						/*movss(xmm1, ptr[rip + foo]);
						L(foo);
						dq(Test.GetUIntPtr());*/


						jmp(ptr[rip + retnLabel]);
						L(retnLabel);
						dq(HUDMarkerData_HookTarget.GetUIntPtr() + 5);
					}
				};

				void* codeBuf = g_localTrampoline.StartAlloc();
				HUDMarkerDataHook_Code code(codeBuf);
				g_localTrampoline.EndAlloc(code.getCurr());

				HUDMarkerDataHook_Original = (_HUDMarkerDataHook)codeBuf;

				g_branchTrampoline.Write5Branch(HUDMarkerData_HookTarget.GetUIntPtr(), (uintptr_t)HUDMarkerData_Hook);
			}


			if (!g_branchTrampoline.Write5Call(OuterIsInCombat_HookTarget.GetUIntPtr(), (uintptr_t)&OuterIsInCombat_Hook))
			{
				_ERROR("Failed to hook IsInCombatWithActor");
			}
		}
		


		//No distance based marker scaling and fadings, so dont hook any related stuff
		if (!bDistanceBasedMarkers) {
			_MESSAGE("Skipping marker scaling hooks");
			return;
		}

		if (!g_branchTrampoline.Write5Call(BSGFxDisplayObjectDtor_HookTarget.GetUIntPtr(), (uintptr_t)BSGFxDisplayObjectDtor_Hook)) {
			_ERROR("Failed to hook BSGFxDisplayObjectDtor");

		}
		if (!g_branchTrampoline.Write5Call(GetMajorFrame_HookTarget.GetUIntPtr(), (uintptr_t)GetMajorFrame_Hook)) {
			_ERROR("Failed to hook BSGFxDisplayObjectDtor");
		}


		{
			struct GetHostileEnemyMaxDistanceHook_Code : Xbyak::CodeGenerator {
				GetHostileEnemyMaxDistanceHook_Code(void* buf) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					//Xbyak::Label foo;

					mov(ptr[rsp + 0x08], rbx);

					jmp(ptr[rip + retnLabel]);
					L(retnLabel);
					dq(GetHostileEnemyMaxDistance_HookTarget.GetUIntPtr() + 5);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			GetHostileEnemyMaxDistanceHook_Code code(codeBuf);
			g_localTrampoline.EndAlloc(code.getCurr());

			GetHostileEnemyMaxDistanceHook_Original = (GetHostileEnemyMaxDistanceHook)codeBuf;

			g_branchTrampoline.Write5Branch(GetHostileEnemyMaxDistance_HookTarget.GetUIntPtr(), (uintptr_t)GetHostileEnemyMaxDistance_Hook);
		}
	}
}

