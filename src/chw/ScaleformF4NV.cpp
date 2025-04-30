#include "f4se/ScaleformValue.h"
#include "f4se/ScaleformMovie.h"
#include "f4se/ScaleformCallbacks.h"
#include "f4se/PapyrusScaleformAdapter.h"

#include "f4se/PapyrusEvents.h"
#include "f4se/PapyrusUtilities.h"
#include "main.h"
#include "f4se/GameData.h"
#include "f4se/GameRTTI.h"
#include "f4se/GameMenus.h"
#include "f4se/GameInput.h"
#include "f4se/InputMap.h"
#include "f4se/ScaleformLoader.h"
#include "HookUtil.h"
#include "ScaleformF4NV.h"
#include "f4se\GameReferences.h"
#include "Globals.h"
#include "PapyrusFunctions.h"
#include "Positioner.h"






void HandleButtonEvent(ButtonEvent * inputEvent)
{
	UInt32	keyCode;
	UInt32	deviceType = inputEvent->deviceType;
	UInt32	keyMask = inputEvent->keyMask;

	// Mouse
	if (deviceType == InputEvent::kDeviceType_Mouse)
		keyCode = InputMap::kMacro_MouseButtonOffset + keyMask;
	// Gamepad
	else if (deviceType == InputEvent::kDeviceType_Gamepad)
		keyCode = InputMap::GamepadMaskToKeycode(keyMask);
	// Keyboard
	else
		keyCode = keyMask;

	// Valid scancode?
	if (keyCode >= InputMap::kMaxMacros)
		return;

	BSFixedString	control = *inputEvent->GetControlID();
	float			timer = inputEvent->timer;

	bool isDown = inputEvent->isDown == 1.0f && timer == 0.0f;
	bool isUp = inputEvent->isDown == 0.0f && timer != 0.0f;

	if (isDown)
	{
		if (!IsGameDataReady) {
			return;
		}

		if (keyCode == PositionerKeyToggle && IsPositioningToolEnabled) {
			Positioner::SetPositioningEnabled(!Positioner::GetPositioningEnabled());
		}

		if (Positioner::GetPositioningEnabled()) {
			Positioner::SetActiveDimension(keyCode);
			Positioner::SetOperationType(keyCode);
			if (keyCode == PositionerKeyAdd) {
				Positioner::DoOperation(true);
			}
			if (keyCode == PositionerKeyMarkForSave) {
				if (IsHolsterPositionSaveLogginEnabled) {
					Positioner::LogCurrentPositions();
				}
				Positioner::SaveCurrentPositions();
			}
			else if (keyCode == PositionerKeySubtract) {
				Positioner::DoOperation(false);
			}
			if (keyCode == PositionerKeyShowCurrent) {
				Positioner::ShowCurrentState();
			}
			if (keyCode == PositionerKeyReset) {
				Positioner::ResetAllChanges();
			}
		}
		//make system

	}
	else if (isUp)
	{
	}
}

class F4SEInputHandler2 : public PlayerInputHandler
{
public:
	F4SEInputHandler2() : PlayerInputHandler() { }

	virtual void OnButtonEvent(ButtonEvent * inputEvent) override
	{
		if ((*g_ui)->numPauseGame)
		{
			return;
		}

		HandleButtonEvent(inputEvent);
	}


};

F4SEInputHandler2 ehandler;


void ReceiveKeys()
{
	ehandler.enabled = true;
	tArray<PlayerInputHandler*>* inputEvents = &((*g_playerControls)->inputEvents1);
	PlayerInputHandler* inputHandler = &ehandler;
	int idx = inputEvents->GetItemIndex(inputHandler);
	if (idx == -1)
	{
		inputEvents->Insert(0, inputHandler);
		_MESSAGE("Registered for input events.");
	}
}