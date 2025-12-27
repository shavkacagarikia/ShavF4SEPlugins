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
#include "f4se\NiNodes.h"
#include "f4se\NiObjects.h"
#include "f4se\NiCloningProcess.h"
#include "f4se\NiTypes.h"
#include "f4se\NiExtraData.h"
#include "f4se\BSGeometry.h"
#include "f4se\NiProperties.h"
#include "f4se\BSCollision.h"
#include "f4se\BSSkin.h"
#include "f4se\BSModelDB.h"
#include "VisibleHolster.h"
#include "AttachHandler.h"
#include <functional>
#include <chrono>
#include <future>
#include <cstdio>
#include <sstream>
#include "f4se\GameThreads.h"

std::string mName = "ClassicHolsteredWeapons";



IDebugLog	gLog;


int ammoCount = 0;
int ammoCapacity = 0;
int totalAmmoCount = 0;

BGSKeyword* ActorTypeNPC;

void* action = nullptr;
TESObjectWEAP::InstanceData* GWeaponInstance = nullptr;
TESObjectWEAP* GWeaponForm = nullptr;
ActorValueInfo* CheckAvif = nullptr;
BGSTerminal* CheckTerminal = nullptr;

bool IsGameDataReady = false;

BSTEventDispatcher<TESObjectLoadedEvent>* eventDispatcherGlobal = nullptr;

bool logEnabled = false;

PluginHandle			    g_pluginHandle = kPluginHandle_Invalid;
F4SEMessagingInterface* g_messaging = nullptr;
F4SEPapyrusInterface* g_papyrus = NULL;

F4SEScaleformInterface* g_scaleform = NULL;
F4SESerializationInterface* g_serialization = NULL;
F4SETaskInterface* g_task = nullptr;

const char* ActorSkeletonPath = "meshes\\actors\\character\\characterassets\\skeleton.nif";
const char* WeaponsPath = "meshes";

ActorValueInfo* TypeAlwaysChecker;
ActorValueInfo* TypeOnDrawChecker;
ActorValueInfo* TypeOnHolsterChecker;

TESRace* HumanRace;
TESRace* GhoulRace;
TESRace* VertibirdRace;

BGSInventoryItem::Stack* LatestInvItem = nullptr;
//const char* ActorSkeletonPath = "skeleton.nif";


const char* VHWMark = "VHWMark";

std::string mAbbrv = "VHW";

BGSKeyword* WeaponTypeRifle;
BGSKeyword* WeaponTypeBigGun;
BGSKeyword* WeaponTypePistol;
BGSKeyword* WeaponType1HMelee;
BGSKeyword* WeaponType2HMelee;

bool IsWeaponDrawn = false;
bool IsWeaponHolstered = false;


class HandleDisplayDelegate : public ITaskDelegate
{
public:
	HandleDisplayDelegate(bool isWeaponDrawn, NiAVObject* weaponNodeRef, Actor* actor, TESObjectWEAP* weap, TESObjectWEAP::InstanceData* inst, bool isManual);
	virtual ~HandleDisplayDelegate() { };
	virtual void Run() override;

protected:
	NiAVObject* _weaponNodeRef;
	bool _isWeaponDrawn;
	Actor* _refr;
	TESObjectWEAP* _weap;
	TESObjectWEAP::InstanceData* _inst;
	bool _isManual;
};


class laterMain
{
public:
	template <class callable, class... arguments>
	laterMain(int after, bool async, callable&& f, arguments&&... args)
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


typedef void(*_PlaySubgraphAnimation)(VirtualMachine* vm, UInt32 stackId, Actor* target, BSFixedString asEventName);
RelocAddr <_PlaySubgraphAnimation> PlaySubgraphAnimationInternal(0x138A250);//NG

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


bool isPipboyOpen() {
	BSFixedString bf = "PipboyMenu";
	if ((*g_ui) && (*g_ui)->IsMenuOpen(bf)) {

		_MESSAGE("pipboy is open");
		return true;
	}
	else
		return false;

}

typedef bool(*_getHasMandatoryAnimUpdate)(TESObjectREFR* arg1);
RelocAddr <_getHasMandatoryAnimUpdate> GetHasMandatoryAnimUpdate_HookTarget(0x0524C30);
//04D0CB0
//0BD79F0
//04BB9E0??
//04A6820

//RelocAddr <_actorState> actorState_HookTarget(0x2D3F4E0);
_getHasMandatoryAnimUpdate GetHasMandatoryAnimUpdate_Original;

//void* actorStateProcess_Hook(Actor * arg1, void* arg2) {
bool GetHasMandatoryAnimUpdate_Hook(TESObjectREFR* arg1) {

	Actor* actor = (Actor*)arg1;
	if (arg1 && arg1->baseForm && arg1->baseForm->formType == kFormType_NPC_ && actor && actor->baseForm && actor->baseForm->formType == kFormType_NPC_) {
		/*counter--;
		if (counter <= 0) {*/
		/*auto name = CALL_MEMBER_FN(actor, GetReferenceName)();
		_MESSAGE(name);
		if (actor != (*g_player)) return GetHasMandatoryAnimUpdate_Original(arg1);*/
		if (!IsHolsterVisibleOnNPCs) {
			if (actor != (*g_player)) {
				return GetHasMandatoryAnimUpdate_Original(arg1);
			}
		}
		/*if (g_task)
			g_task->AddTask(new HandleDisplayDelegate(arg1->actorState.IsWeaponDrawn(), nullptr, arg1, nullptr, nullptr, false));*/
			/*if (arg1 != (*g_player) && arg1->actorState.IsWeaponDrawn()) {
				VisibleHolster::ClearAllHolsters(arg1);
			}*/
		if (!actor->IsDead()) {
			VisibleHolster::HandleWeaponDisplay(actor->actorState.IsWeaponDrawn(), nullptr, actor);

			/*_MESSAGE("----------------------------------");
			_MESSAGE("dead actor:");
			const char* n = CALL_MEMBER_FN(arg1, GetReferenceName)();
			_MESSAGE(n);
			_MESSAGE(std::to_string(arg1->formID).c_str());*/
			//VisibleHolster::ClearAllHolsters(arg1); 
		}
		/*if (arg1 == (*g_player)) {

		}*/
		//counter = 5;
	}
	//}
	return GetHasMandatoryAnimUpdate_Original(arg1);
}

typedef char(*_AttachWeaponHook)(void* handler, Actor* actor, BSFixedString* str);
RelocAddr <_AttachWeaponHook> AttachWeaponHook_HookTarget(0x0F6F770);

_AttachWeaponHook AttachWeaponHook_Original;


char AttachWeaponHook_Hook(void* handler, Actor* actor, BSFixedString* str) {
	_MESSAGE("AAAAAAA");

	return AttachWeaponHook_Original(handler, actor, str);

}

//class BGSEquipIndex
//{
//public:
//
//								  // members
//	UInt32 index;  // 0
//};
////STATIC_ASSERT(sizeof(BGSEquipIndex) == 0x4);

//typedef void(*_AttachWeaponHook)(TESObjectREFR * arg1, ExtraInstanceData* weap, UInt32 index);
//RelocAddr <_AttachWeaponHook> AttachWeaponHook_HookTarget(0x0409B40);
//
////RelocAddr <_actorState> actorState_HookTarget(0x2D3F4E0);
//_AttachWeaponHook AttachWeaponHook_Original;
//
////void* actorStateProcess_Hook(Actor * arg1, void* arg2) {
//void AttachWeaponHook_Hook(TESObjectREFR * arg1, ExtraInstanceData* weap, UInt32 index) {
//
//	Actor* actor = (Actor*)arg1;
//
//	if (arg1->baseForm->formType == kFormType_NPC_ && actor && actor->baseForm && actor->baseForm->formType == kFormType_NPC_) {
//		if (!IsHolsterVisibleOnNPCs) {
//			if (actor != (*g_player)) {
//				return AttachWeaponHook_Original(arg1, weap, index);
//			}
//		}
//
//		if (!actor->IsDead()) {
//			VisibleHolster::HandleWeaponDisplay(false, nullptr, actor);
//		}
//		/*if (arg1 == (*g_player)) {
//
//		}*/
//	}
//
//	return AttachWeaponHook_Original(arg1, weap, index);
//}


void InitActorStateHook() {
	//aiProcess_Original = HookUtil::SafeWrite64(aiProcess_HookTarget.GetUIntPtr(), &aiProcess_Hook);
	//_aiProcess aiProcess_Original = RelocAddr<_aiProcess> 0x0E28120
	struct AiProcess_Code : Xbyak::CodeGenerator {
		AiProcess_Code(void* buf) : Xbyak::CodeGenerator(4096, buf)
		{
			Xbyak::Label retnLabel;

			//mov(ptr[rsp + 0x08], rbx);
			mov(rcx, ptr[rcx + 0x0E0]);



			jmp(ptr[rip + retnLabel]);




			L(retnLabel);
			dq(GetHasMandatoryAnimUpdate_HookTarget.GetUIntPtr() + 7);
		}
	};
	void* codeBuf = g_localTrampoline.StartAlloc();
	AiProcess_Code code(codeBuf);
	g_localTrampoline.EndAlloc(code.getCurr());

	GetHasMandatoryAnimUpdate_Original = (_getHasMandatoryAnimUpdate)codeBuf;

	g_branchTrampoline.Write6Branch(GetHasMandatoryAnimUpdate_HookTarget.GetUIntPtr(), (uintptr_t)GetHasMandatoryAnimUpdate_Hook);

	/*{

		struct AttachWeaponHook_Code : Xbyak::CodeGenerator {
			AttachWeaponHook_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;

				mov(ptr[rsp + 0x08], rbx);

				jmp(ptr[rip + retnLabel]);


				L(retnLabel);
				dq(AttachWeaponHook_HookTarget.GetUIntPtr() + 5);
			}
		};
		void * codeBuf = g_localTrampoline.StartAlloc();
		AttachWeaponHook_Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		AttachWeaponHook_Original = (_AttachWeaponHook)codeBuf;

		g_branchTrampoline.Write5Branch(AttachWeaponHook_HookTarget.GetUIntPtr(), (uintptr_t)AttachWeaponHook_Hook);
	}*/
}




struct TESEquipEvent
{
	Actor* owner;        // 00
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

DECLARE_EVENT_DISPATCHER(TESEquipEvent, 0x00531570);//NG




//Equip event
class EquipEventSink : public BSTEventSink<TESEquipEvent>
{
public:
	virtual	EventResult	ReceiveEvent(TESEquipEvent* evn, void* dispatcher) override
	{
		//if (evn->owner && evn->owner == (*g_player) && !evn->status) {
		//_MESSAGE(std::to_string(evn->owner->formID).c_str());
		if (evn->owner && !evn->status) {
			TESForm* ff = LookupFormByID(evn->FormID);
			if (ff && ff->formType == kFormType_WEAP) {
				/*BSFixedString nam = CALL_MEMBER_FN(evn->owner, GetReferenceName)();
				_MESSAGE(nam.c_str());*/

				//evn->owner->actorValueOwner.SetBase(TypeAlwaysChecker, 0);
				VisibleHolster::ClearAllHolsters(evn->owner, true);
			}
		}

		return kEvent_Continue;
	}
};
EquipEventSink equipEventSink;


bool RegisterAfterLoadEvents() {

	/*auto eventDispatcher4 = GET_EVENT_DISPATCHER(PlayerAmmoCountEvent);
	if (eventDispatcher4) {
	eventDispatcher4->AddEventSink(&playerAmmoCountEventSink);
	}
	else {
	logMessage("cant register PlayerAmmoCountEvent:");
	return false;
	}*/

	/*auto eventDispatcher1 = GET_EVENT_DISPATCHER(PlayerUpdateEvent);
	if (eventDispatcher1) {
	eventDispatcher1->AddEventSink(&playerUpdateEventSink);
	}
	else {
	logMessage("cant register playerUpdateEventSink");
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

	/*auto eventDispatcher4 = GetEventDispatcher<TESActivateEvent>();
	if (eventDispatcher4) {
	eventDispatcher4->eventSinks.Push(&activateEventSink);
	}
	else {
	logMessage("cant register TESEquipEvent:");
	return false;
	}*/


	//auto d = (*g_ActorEquipManager);



	//auto eventDispatcher6 = GetEventDispatcher<ActorEquipManagerEvent::Event>();
	//if (eventDispatcher6) {
	//	eventDispatcher6->eventSinks.Push(&actorEquipManagerEventSink);
	//}
	//else {
	//	logMessage("cant register ActorItemEquipped:");
	//	//return false;
	//}

	//bool a = (*g_ActorEquipManager)->eventDispatcher.AddEventSink(&actorEquipManagerEventSink);

	/*auto eventDispatcher7 = GetEventDispatcher<ActorItemEquipped::Event>();
	if (eventDispatcher7) {
	eventDispatcher7->eventSinks.Push(&actorItemEquippedSink);
	}
	else {
	logMessage("cant register ActorItemEquipped:");
	return false;
	}*/

	return true;
}

HandleDisplayDelegate::HandleDisplayDelegate(bool isWeaponDrawn, NiAVObject* weaponNodeRef, Actor* actor, TESObjectWEAP* weap, TESObjectWEAP::InstanceData* inst, bool isManual) {
	_refr = actor;
	_isWeaponDrawn = isWeaponDrawn;
	_weaponNodeRef = weaponNodeRef;
	_inst = inst;
	_weap = weap;
	_isManual = isManual;
	//_MESSAGE("CONSTRUCTOR");
}

void HandleDisplayDelegate::Run() {
	//_MESSAGE("DELEGATE");
	if (_refr) {
		/*float original = GetScale(refr);
		SetScale(refr, original + 0.01);
		SetScale(refr, original);*/
		VisibleHolster::HandleWeaponDisplay(_refr->actorState.IsWeaponDrawn(), nullptr, _refr);

		/*VisibleHolster::ClearAllHolsters(refr);
		VisibleHolster::HandleWeaponDisplay(refr->actorState.IsWeaponDrawn(), nullptr, refr);*/

	}
}


bool oncePerSession = false;

class TESLoadGameHandler : public BSTEventSink<TESLoadGameEvent>
{
public:
	virtual ~TESLoadGameHandler() { };
	virtual    EventResult    ReceiveEvent(TESLoadGameEvent* evn, void* dispatcher) override
	{
		Positioner::SetSkeletonNode(nullptr);
		GWeaponForm = nullptr;
		GWeaponInstance = nullptr;
		/*if (!oncePerSession) {

		if (!RegisterAfterLoadEvents()) {
		logMessage("unable to register for events");
		}
		oncePerSession = true;

		}*/
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
	if (!RegisterAfterLoadEvents()) {
		logMessage("unable to register for events");
	}
	//RegisterForObjectLoaded();

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

			HandleIniFiles();

			VisibleHolster::SetModelProcessor();



			VisibleHolster::SetArraysAfterFormsLoad();

			GameDataReady();
			ReceiveKeys();
			VisibleHolster::CollectWeaponOMODModels();
			//VisibleHolster::CollectRacesForSkeleton();


			IsGameDataReady = true;

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
	g_serialization->SetUniqueID(g_pluginHandle, 'CHW');
	g_serialization->SetRevertCallback(g_pluginHandle, F4NVSerialization::RevertCallback);
	g_serialization->SetLoadCallback(g_pluginHandle, F4NVSerialization::LoadCallback);
	g_serialization->SetSaveCallback(g_pluginHandle, F4NVSerialization::SaveCallback);
}



extern "C"
{
	__declspec(dllexport) F4SEPluginVersionData F4SEPlugin_Version = {
		F4SEPluginVersionData::kVersion,
		108,
		"ClassicHolsteredWeapons",
		"Shavkacagarikia",
		0,
		F4SEPluginVersionData::kAddressIndependence_AddressLibrary_1_11_137,
		CURRENT_RELEASE_RUNTIME,
		0,
		{ RUNTIME_VERSION_1_11_191 },
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
			//_MESSAGE("got it");
		}

		g_papyrus = (F4SEPapyrusInterface*)f4se->QueryInterface(kInterface_Papyrus);
		if (!g_papyrus) {
			_FATALERROR("couldn't get papyrus interface");
			return false;
		}
		else {
			//_MESSAGE("got it papyrus");
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
		g_task = (F4SETaskInterface*)f4se->QueryInterface(kInterface_Task);
		if (!g_task)
		{
			_WARNING("couldn't get task interface");
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
		//aiProcess_Original = HookUtil::SafeWrite64(aiProcess_HookTarget.GetUIntPtr(), &aiProcess_Hook);
		VisibleHolster::InitMainVec();
		InitActorStateHook();
		logMessage("load");
		Hooks_Handlers_Commit();
		DoSerialization();
		return true;

	}


};


