#include "f4se/PluginAPI.h"
#include "f4se/GameAPI.h"
#include "f4se/GameData.h"
#include "f4se/GameReferences.h"
#include "f4se/GameForms.h"
#include "f4se/GameMenus.h"
#include "f4se/GameRTTI.h"
#include "f4se_common\SafeWrite.h"
#include <shlobj.h>
#include <string>
#include <vector>
#include "xbyak/xbyak.h"
#include "f4se_common\BranchTrampoline.h"
#include "main.h"
#include "f4se_common/f4se_version.h"
#include "f4se/PapyrusVM.h"
#include "f4se/PapyrusNativeFunctions.h"
#include "f4se/PapyrusEvents.h"
#include "f4se\GameExtraData.h"
#include "Globals.h"
#include "Utils.h"
#include "f4se/Translation.h"
#include "f4se/ScaleformLoader.h"
#include "f4se/CustomMenu.h"
#include "f4se\GameTypes.h"
#include "F4NVSerialization.h"
#include "F4NVForms.h"
#include "HookUtil.h"
#include "PapyrusFunctions.h"
#include "ScaleformF4NV.h"
#include "INIReader.h"

#include <functional>
#include <chrono>
#include <future>
#include <cstdio>

std::string esmName = "BulletCountedReload.esm";
std::string mName = "BulletCountedReload";
IDebugLog	gLog;


int ammoCount = 0;
int ammoCapacity = 0;
int totalAmmoCount = 0;

TESIdleForm* idle = nullptr;
TESIdleForm* idle3rd = nullptr;
TESIdleForm* idlePA = nullptr;
TESIdleForm* idlePA3rd = nullptr;
void* action = nullptr;
TESObjectWEAP::InstanceData* weapInstance = nullptr;
ActorValueInfo* checkAvif = nullptr;
ActorValueInfo* checkAvif2 = nullptr;

BSTEventDispatcher<TESObjectLoadedEvent>* eventDispatcherGlobal = nullptr;

bool logEnabled = false;

PluginHandle			    g_pluginHandle = kPluginHandle_Invalid;
F4SEMessagingInterface* g_messaging = nullptr;
F4SEPapyrusInterface* g_papyrus = NULL;

F4SEScaleformInterface* g_scaleform = NULL;
F4SESerializationInterface* g_serialization = NULL;


enum ReloadStates {
	Default = 0,
	BulletInsert,
	UncullBone
};


typedef void(*_PlaySubgraphAnimation)(VirtualMachine* vm, UInt32 stackId, Actor* target, BSFixedString asEventName);
RelocAddr <_PlaySubgraphAnimation> PlaySubgraphAnimationInternal(0x138A130);//NG

void PlaySubgraphAnimation(Actor* target, BSFixedString asEventName) {
	PlaySubgraphAnimationInternal((*g_gameVM)->m_virtualMachine, 0, target, asEventName);
}

BSTEventDispatcher<void*>* GetGlobalEventDispatcher(BSTGlobalEvent* globalEvents, const char* dispatcherName)
{
	for (int i = 0; i < globalEvents->eventSources.count; i++) {
		const char* name = GetObjectClassName(globalEvents->eventSources[i]) + 15;    // ?$EventSource@V
		if (strstr(name, dispatcherName) == name) {
			return &globalEvents->eventSources[i]->eventDispatcher;
		}
	}
	return nullptr;
}
#define GET_EVENT_DISPATCHER(EventName) (BSTEventDispatcher<EventName>*) GetGlobalEventDispatcher(*g_globalEvents, #EventName);
//
//STATIC_ASSERT(sizeof(BSTEventSource<void*>) == 0x70);


class BSAnimationGraphEvent {
public:
	TESForm* unk00;
	BSFixedString eventName;
	//etc
};

typedef UInt8(*_tf1)(void* thissink, BSAnimationGraphEvent* evnstruct, void** dispatcher);
RelocAddr <_tf1> tf1_HookTarget(0x236A4D0);
_tf1 tf1_Original;



bool reloadStarted = false;
bool reloadEnd = true;
bool cullBone = false;
bool uncullbone = false;
int incrementor = 0;
int toAdd = 0;
bool animationGoingToPlay = true;
bool stopPressed = false;
bool readyForStopPress = false;
int reloadState = 0;

//Play stop idle based on camera mode and handle reload end
void PlayNeededIdle() {
	bool hasPostReload = false;
	/*if (weapInstance && weapInstance->flags & TESObjectWEAP::InstanceData::kFlag_BoltAction) {
		hasPostReload = true;
	}*/
	if (weapInstance && weapInstance->skill == checkAvif2) {
		hasPostReload = true;
	}
	animationGoingToPlay = SendStopSignal(hasPostReload);

	if (animationGoingToPlay) {
		if (stopPressed) {
			reloadEndHandle(true);
		}
		else {
			reloadEndHandle();
		}
	}

	logIfNeeded("is animation going to play: " + std::to_string(animationGoingToPlay));
}

//Plays stop idle if R or LMB pressed
void StopIfNeeded() {
	if (stopPressed && incrementor - 1 < toAdd) {
		PlayNeededIdle();
	}
}

void StopLesserAmmo() {
	if ((totalAmmoCount + ammoCount) - ammoCapacity < 0) {
		toAdd = totalAmmoCount;
	}
}

//Animation event hook
UInt8 tf1_Hook(void* arg1, BSAnimationGraphEvent* arg2, void** arg3) {
	//if (weapInstance && weapInstance->skill == checkAvif) {
	if (weapInstance) {
		const char* evnam = arg2->eventName.c_str();
		//reload ended by engine, do reload end logic
		if (!_strcmpi("reloadEnd", evnam)) {
			reloadEndHandle();
			reloadState = ReloadStates::Default;
			logIfNeeded("reload end");
		}
		//reload state enter, initialize everything
		if (!_strcmpi("Event00", evnam)) {
			reloadStartHandle();
			StopLesserAmmo();
			reloadState = ReloadStates::Default;
			logIfNeeded("reload start");
		}
		//Bone hide
		if (reloadStarted && !reloadEnd && uncullbone && reloadState == ReloadStates::BulletInsert && !_strcmpi("CullBone", evnam)) {
			cullBone = true;
			uncullbone = false;
			logIfNeeded("cullbone");
			if (toAdd != ammoCapacity) {
				incrementor++;
				if (incrementor >= toAdd) {
					PlayNeededIdle();
				}
			}
			StopIfNeeded();
			reloadState = ReloadStates::Default;

		}
		//Bone show
		if (reloadStarted && !reloadEnd && reloadState == ReloadStates::Default && !_strcmpi("UncullBone", evnam)) {
			cullBone = false;
			uncullbone = true;
			logIfNeeded("uncullbone");
			SetWeapAmmoCapacity(ammoCount + 1);

			if (incrementor >= toAdd) {
				PlayNeededIdle();
			}
			//with 3rd person faster speeds, game unable to play reload end, so making sure its played once more in first uncullbone
			if (!animationGoingToPlay) {
				PlayNeededIdle();

			}
			reloadState = ReloadStates::UncullBone;
		}
		//shell insert
		if (reloadStarted && !reloadEnd && !_strcmpi("ReloadComplete", evnam)) {
			readyForStopPress = true;
			reloadState = ReloadStates::BulletInsert;
			//StopIfNeeded();
		}

		//Manually handle relaod end for various situations
		if (reloadStarted && !reloadEnd && !_strcmpi("pipboyOpened", evnam)) {
			reloadEndHandle();
			reloadState = ReloadStates::Default;
			logIfNeeded("pipboy opened");
		}
		if (reloadStarted && !reloadEnd && !_strcmpi("weaponSwing", evnam)) {
			reloadEndHandle();
			reloadState = ReloadStates::Default;
			logIfNeeded("weapon swing");
		}
		if (reloadStarted && !reloadEnd && !_strcmpi("throwEnd", evnam)) {
			reloadEndHandle();
			reloadState = ReloadStates::Default;
			logIfNeeded("throw end");
		}
		if (!_strcmpi("weaponDraw", evnam) && reloadStarted && !reloadEnd) {
			reloadEndHandle();
			logIfNeeded("Weapon Draw");
		}
		if (reloadStarted && !reloadEnd && !_strcmpi("impactLandEnd", evnam)) {
			reloadEndHandle();
			reloadState = ReloadStates::Default;
			logIfNeeded("Weapon Draw");
		}

		/*if (reloadStarted && !reloadEnd) {
		_MESSAGE("%s", evnam);
		}*/
		//_MESSAGE("%s", evnam);

	}
	return tf1_Original(arg1, arg2, arg3);
}


struct TESEquipEvent
{
	TESObjectREFR* owner;        // 00
	UInt32            FormID;        // 08
	UInt32				refFormID;			// 0C
	UInt8				unk10;			// 10
	UInt8				unk11;			// 11
	UInt8				status;
	char              padding2[94]; 

	BGSInventoryItem::Stack* invItem; //78
	char			  padding3[8];
	TESObject* item;
	TESObjectWEAP::InstanceData* instanceData;
};
STATIC_ASSERT(offsetof(TESEquipEvent, instanceData) == 0x90);
STATIC_ASSERT(offsetof(TESEquipEvent, invItem) == 0x78);
STATIC_ASSERT(offsetof(TESEquipEvent, item) == 0x88);



DECLARE_EVENT_DISPATCHER(TESEquipEvent, 0x004DD5F0);//NG


//Equip Handler
void HanldeWeaponEquip(TESObjectWEAP* f, BGSInventoryItem::Stack* stack) {

	TESObjectWEAP::InstanceData* equippedWeaponInstance = GetInstanceDataFromExtraDataList(stack->extraData, f, true);
	if (equippedWeaponInstance && (equippedWeaponInstance->skill == checkAvif || equippedWeaponInstance->skill == checkAvif2)) {
		ammoCapacity = equippedWeaponInstance->ammoCapacity;
		weapInstance = equippedWeaponInstance;
		logIfNeeded("weapon instance equipped with ammo capacity of:" + std::to_string(weapInstance->ammoCapacity));
	}
	else {
		logIfNeeded("problem with equip");
	}
}


//Equip event
class EquipEventSink : public BSTEventSink<TESEquipEvent>
{
public:
	virtual	EventResult	ReceiveEvent(TESEquipEvent* evn, void* dispatcher) override
	{
		if (evn->owner && evn->owner == (*g_player) && evn->FormID && evn->status) {

			TESForm* ff = LookupFormByID(evn->FormID);
			if (ff && ff->formType == kFormType_WEAP) {

				TESObjectWEAP* f = (TESObjectWEAP*)ff;
				if (f && evn->item && evn->invItem) {

					reloadEndHandle();
					HanldeWeaponEquip(f, evn->invItem);
				}
			}
		}


		return kEvent_Continue;
	}
};
EquipEventSink equipEventSink;

//Called after game load to fill weapon instance if its equipped
void FillEquipDataFromEquippedItem() {
	auto inventory = (*g_player)->inventoryList;
	if (inventory)
	{
		inventory->inventoryLock.LockForWrite();
		for (size_t i = 0; i < inventory->items.count; i++)
		{
			TESForm* form = inventory->items[i].form;
			BGSInventoryItem it = inventory->items[i];
			BGSInventoryItem::Stack* stack = inventory->items[i].stack;

			if (form && form->formType == kFormType_WEAP && stack)
			{
				TESObjectWEAP* weap = (TESObjectWEAP*)form;

				stack->Visit([&](BGSInventoryItem::Stack* stackx)
					{
						if (stackx) {
							if (stackx->flags & 7)
							{
								HanldeWeaponEquip(weap, stackx);
							}

						}
						return true;
					});
			}
		}
		inventory->inventoryLock.UnlockWrite();
	}

}


struct PlayerAmmoCountEvent
{
	UInt32							ammoCount;        // 00
	UInt32							totalAmmoCount;    // 04
	UInt64							unk08;            // 08
	TESObjectWEAP* weapon;            // 10
	TESObjectWEAP::InstanceData* weaponInstance;
	//...
};
STATIC_ASSERT(offsetof(PlayerAmmoCountEvent, weapon) == 0x10);

//Ammo count handle
class PlayerAmmoCountEventSink : public BSTEventSink<PlayerAmmoCountEvent>
{
public:
	virtual ~PlayerAmmoCountEventSink() { };

	virtual    EventResult    ReceiveEvent(PlayerAmmoCountEvent* evn, void* dispatcher) override
	{

		if (evn->weapon || evn->weaponInstance || evn->unk08 == 1) {
			if (evn->unk08 == 1 && evn->weapon && evn->weaponInstance && evn->weaponInstance != weapInstance) {
				weapInstance = evn->weaponInstance;
				ammoCapacity = weapInstance->ammoCapacity;
			}
			ammoCount = evn->ammoCount;
			totalAmmoCount = evn->totalAmmoCount;
			logIfNeeded("ammo count: " + std::to_string(ammoCount));
		}
		return kEvent_Continue;
	}
};
PlayerAmmoCountEventSink playerAmmoCountEventSink;


struct PlayerSetWeaponStateEvent
{
	UInt8 unk00;
	UInt8 unk08;
	UInt8 unk10;
	UInt8 unk18;
	UInt8 unk20;
	UInt8 unk28;
	UInt8 unk30;
	UInt8 unk38;
	UInt8 unk40;
	UInt8 unk48;
};
//STATIC_ASSERT(offsetof(InventoryUpdateEvent, weapon) == 0x10);

//Ammo count handle
class PlayerSetWeaponStateEventSink : public BSTEventSink<PlayerSetWeaponStateEvent>
{
public:
	virtual ~PlayerSetWeaponStateEventSink() { };

	virtual    EventResult    ReceiveEvent(PlayerSetWeaponStateEvent* evn, void* dispatcher) override
	{
		_MESSAGE("weapon state");
		return kEvent_Continue;
	}
};
PlayerSetWeaponStateEventSink playerSetWeaponStateEventSink;


typedef void* (*_ActorProcessEvent)(void* arg1, BSAnimationGraphEvent* arg2, void** arg3);
RelocAddr <_ActorProcessEvent> ActorProcessEvent_HookTarget(0x0DBD4A0);

//RelocAddr <_actorState> actorState_HookTarget(0x2D3F4E0);
_ActorProcessEvent ActorProcessEvent_Original;

void* Hook(void* arg1, BSAnimationGraphEvent* arg2, void** arg3) {
	tf1_Hook(arg1, arg2, arg3);
	return ActorProcessEvent_Original(arg1, arg2, arg3);
}

void InstallHook() {
	struct AiProcess_Code : Xbyak::CodeGenerator {
		AiProcess_Code(void* buf) : Xbyak::CodeGenerator(4096, buf)
		{
			Xbyak::Label retnLabel;

			add(bl, dh);
			movups(xmm2, ptr[rsp + 0x44]);



			jmp(ptr[rip + retnLabel]);




			L(retnLabel);
			dq(ActorProcessEvent_HookTarget.GetUIntPtr() + 7);
		}
	};
	void* codeBuf = g_localTrampoline.StartAlloc();
	AiProcess_Code code(codeBuf);
	g_localTrampoline.EndAlloc(code.getCurr());

	ActorProcessEvent_Original = (_ActorProcessEvent)codeBuf;

	g_branchTrampoline.Write6Branch(ActorProcessEvent_HookTarget.GetUIntPtr(), (uintptr_t)Hook);
}

bool RegisterAfterLoadEvents() {


	auto eventDispatcher4 = GET_EVENT_DISPATCHER(PlayerAmmoCountEvent);
	if (eventDispatcher4) {
		eventDispatcher4->AddEventSink(&playerAmmoCountEventSink);
	}
	else {
		logMessage("cant register PlayerAmmoCountEvent:");
		return false;
	}

	/*auto eventDispatcher5 = GET_EVENT_DISPATCHER(PlayerSetWeaponStateEvent);
	if (eventDispatcher5) {
		eventDispatcher5->AddEventSink(&playerSetWeaponStateEventSink);
	}
	else {
		logMessage("cant register InventoryUpdateEvent");
		return false;
	}*/

	auto eventDispatcher3 = GetEventDispatcher<TESEquipEvent>();
	if (eventDispatcher3) {
		eventDispatcher3->eventSinks.Push(&equipEventSink);
	}
	else {
		logMessage("cant register TESEquipEvent:");
		return false;
	}


	return true;
}


bool oncePerSession = false;

class TESLoadGameHandler : public BSTEventSink<TESLoadGameEvent>
{
public:
	virtual ~TESLoadGameHandler() { };
	virtual    EventResult    ReceiveEvent(TESLoadGameEvent* evn, void* dispatcher) override
	{
		FillEquipDataFromEquippedItem();
		if (!oncePerSession) {
			if (!RegisterAfterLoadEvents()) {
				logMessage("unable to register for events");
			}
			oncePerSession = true;
		}
		return kEvent_Continue;
	}
};




void logMessage(std::string aString)
{
	_MESSAGE(("[" + mName + "] " + aString).c_str());
}
void GameDataReady()
{
	static auto pLoadGameHandler = new TESLoadGameHandler();
	GetEventDispatcher<TESLoadGameEvent>()->AddEventSink(pLoadGameHandler);

	HandleIniFiles();
}





void F4SEMessageHandler(F4SEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case F4SEMessagingInterface::kMessage_GameDataReady:
	{
		bool isReady = reinterpret_cast<bool>(msg->data);
		if (isReady)
		{

			if (!GetForms()) {
				logMessage("Unable to get one of the form, game wont work");
			}

			GameDataReady();
			ReceiveKeys();
		}
		break;
	}
	case F4SEMessagingInterface::kMessage_NewGame:
	{
		if (!oncePerSession) {
			if (!RegisterAfterLoadEvents()) {
				logMessage("unable to register for events");
			}
			oncePerSession = true;
		}
		break;
	}
	}
}



void DoSerialization() {
	g_serialization->SetUniqueID(g_pluginHandle, 'F4NV');
	g_serialization->SetRevertCallback(g_pluginHandle, F4NVSerialization::RevertCallback);
	g_serialization->SetLoadCallback(g_pluginHandle, F4NVSerialization::LoadCallback);
	g_serialization->SetSaveCallback(g_pluginHandle, F4NVSerialization::SaveCallback);
}

extern "C"
{
	__declspec(dllexport) F4SEPluginVersionData F4SEPlugin_Version = {
		F4SEPluginVersionData::kVersion,
		202,
		"BulletCountedReload",
		"Shavkacagarikia",
		0,
		F4SEPluginVersionData::kAddressIndependence_AddressLibrary_1_10_980,
		CURRENT_RELEASE_RUNTIME,
		0,
		{ RUNTIME_VERSION_1_10_984 },
		0,
	};

	bool F4SEPlugin_Query(const F4SEInterface* f4se, PluginInfo* info)
	{

		return true;
	}

	bool F4SEPlugin_Load(const F4SEInterface* f4se)
	{

		gLog.OpenRelative(CSIDL_MYDOCUMENTS, (const char*)("\\My Games\\Fallout4\\F4SE\\" + mName + ".log").c_str());
		logMessage(std::to_string(F4SEPlugin_Version.pluginVersion));

		g_pluginHandle = f4se->GetPluginHandle();

		g_scaleform = (F4SEScaleformInterface*)f4se->QueryInterface(kInterface_Scaleform);
		if (!g_scaleform) {
			_FATALERROR("couldn't get scaleform interface");
			return false;
		}
		else {
			_MESSAGE("got it");
		}

		g_papyrus = (F4SEPapyrusInterface*)f4se->QueryInterface(kInterface_Papyrus);
		if (!g_papyrus) {
			_FATALERROR("couldn't get papyrus interface");
			return false;
		}
		else {
			_MESSAGE("got it papyrus");
		}

		g_messaging = (F4SEMessagingInterface*)f4se->QueryInterface(kInterface_Messaging);
		if (!g_messaging)
		{
			_FATALERROR("couldn't get messaging interface");
			return false;
		}
		g_serialization = (F4SESerializationInterface*)f4se->QueryInterface(kInterface_Serialization);
		if (!g_serialization) {
			_MESSAGE("couldn't get serialization interface");
			return false;
		}
		if (g_messaging)
			g_messaging->RegisterListener(g_pluginHandle, "F4SE", F4SEMessageHandler);

		if (!g_localTrampoline.Create(1024 * 64, nullptr)) {
			_ERROR("couldn't create codegen buffer. this is fatal. skipping remainder of init process.");
			return false;
		}

		if (!g_branchTrampoline.Create(1024 * 64)) {
			_ERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
			return false;
		}
		if (!g_localTrampoline.Create(1024 * 64, nullptr))
		{
			_ERROR("couldn't create codegen buffer. this is fatal. skipping remainder of init process.");
			return false;
		}

		tf1_Original = HookUtil::SafeWrite64(tf1_HookTarget.GetUIntPtr(), &tf1_Hook);
		//InstallHook();
		logMessage("load");
		return true;

	}

};
