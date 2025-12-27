#pragma once
#include "f4se\GameForms.h"
#include "f4se/GameTypes.h"
#include "f4se\PapyrusArgs.h"
#include "f4se\PapyrusEvents.h"
#include "f4se\GameObjects.h"
#include "f4se\GameReferences.h"
#include "f4se\GameRTTI.h"
#include "f4se/GameCamera.h"

#include "Globals.h"
#include "Utils.h"

void ShowNotification(std::string asNotificationText);

void ShowMessagebox(std::string asText);

void PlaySubgraphAnimation(Actor* target, BSFixedString asEventName);

void SetAnimationVariableBool(TESObjectREFR* ref, BSFixedString asVariableName, bool newVal);

void SetAngle(TESObjectREFR* ref, float x, float y, float z);

void SetMotionType(TESObjectREFR* ref, SInt32 type, bool allowActivate=true);

//bool IsWeaponDrawn(Actor* actor);


SInt32 GetCameraStateInt();

void SetPosition(TESObjectREFR* ref, float x, float y, float z);