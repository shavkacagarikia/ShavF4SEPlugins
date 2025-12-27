#include "PapyrusFunctions.h"

#include <functional>
#include <chrono>
#include <future>
#include <cstdio>



class later
{
public:
	template <class callable, class... arguments>
	later(int after, bool async, callable&& f, arguments&&... args)
	{
		std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));

		if (async)
		{
			std::thread([after, task]() {
				std::this_thread::sleep_for(std::chrono::milliseconds(after));
				task();
			}).detach();
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(after));
			task();
		}
	}

};

void SetMotionType(TESObjectREFR* ref, SInt32 type, bool allowActivate) {
	//SetMotionTypeInternal((*g_gameVM)->m_virtualMachine, 0, ref, type, allowActivate);
	VMArray<VMVariable> varr;
	VMVariable b1;
	b1.Set(&type);
	varr.Push(&b1);
	VMVariable b2;
	b2.Set(&allowActivate);
	varr.Push(&b2);
	CallFunctionNoWait(ref, "SetMotionType", varr);
}

void SetAngle(TESObjectREFR* ref, float x, float y, float z) {
	//SetAngleInternal((*g_gameVM)->m_virtualMachine, 0, ref, x, y, z);

	VMArray<VMVariable> varr;
	VMVariable b1;
	b1.Set(&x);
	varr.Push(&b1);
	VMVariable b2;
	b2.Set(&y);
	varr.Push(&b2);
	VMVariable b3;
	b3.Set(&y);
	varr.Push(&b3);
	CallFunctionNoWait(ref, "SetAngle", varr);
}


void SetPosition(TESObjectREFR* ref, float x, float y, float z) {
	x = 1;
	y = 1;
	z = 1;
	VMArray<VMVariable> varr;
	VMVariable b1;
	b1.Set(&x);
	varr.Push(&b1);
	VMVariable b2;
	b2.Set(&y);
	varr.Push(&b2);
	VMVariable b3;
	b3.Set(&y);
	varr.Push(&b3);
	VMVariable b4;
	float ii = 1;
	b4.Set(&ii);
	varr.Push(&b4);
	CallFunctionNoWait(ref, "ApplyHavokImpulse", varr);
}

void ShowNotification(std::string asNotificationText) {
	CallGlobalFunctionNoWait1<BSFixedString>("Debug", "Notification", BSFixedString(asNotificationText.c_str()));
}

void ShowMessagebox(std::string asText) {
	CallGlobalFunctionNoWait1<BSFixedString>("Debug", "Messagebox", BSFixedString(asText.c_str()));
}


//------------------------------------------------------------------------------------------------------------------------
//Non papyrus



// returns the current camera state
// -1 - unknown / there is no camera yet
// 0 - first person
// 1 - auto vanity
// 2 - VATS
// 3 - free
// 4 - iron sights
// 5 - transition
// 6 - tween menu
// 7 - third person 1
// 8 - third person 2
// 9 - furniture
// 10 - horse
// 11 - bleedout
// 12 - dialogue
SInt32 GetCameraStateInt()
{
	PlayerCamera * playerCamera = *g_playerCamera;
	if (playerCamera)
		return playerCamera->GetCameraStateId(playerCamera->cameraState);

	return -1;
}