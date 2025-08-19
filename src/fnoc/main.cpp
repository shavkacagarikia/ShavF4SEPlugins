#include "main.h"

std::string mName = "FriendlyNPCsOnCompass";
IDebugLog	gLog; 

PluginHandle			    g_pluginHandle = kPluginHandle_Invalid;
F4SEMessagingInterface* g_messaging = nullptr;



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
				_FATALERROR("FNOC: Cannot load one of the forms, game will be closed");
				MessageBox(NULL, (LPCSTR)"FNOC: Cannot load one of the forms, game will be closed", (LPCSTR)mName.c_str(), MB_OK | MB_ICONEXCLAMATION);
				killProcessByName("Fallout4.exe");
				exit(EXIT_FAILURE);
			}


		}
		break;
	}
	}
}




extern "C"
{
	__declspec(dllexport) F4SEPluginVersionData F4SEPlugin_Version = {
		F4SEPluginVersionData::kVersion,
		100,
		"FriendlyNPCsOnCompass",
		"Shavkacagarikia",
		0,
		F4SEPluginVersionData::kAddressIndependence_AddressLibrary_1_10_980,
		CURRENT_RELEASE_RUNTIME,
		0,
		{ RUNTIME_VERSION_1_10_984 },
		0,
	};


	bool F4SEPlugin_Load(const F4SEInterface* f4se)
	{

		gLog.OpenRelative(CSIDL_MYDOCUMENTS, (const char*)("\\My Games\\Fallout4\\F4SE\\" + mName + ".log").c_str());
		logMessage(std::to_string(F4SEPlugin_Version.pluginVersion));

		g_pluginHandle = f4se->GetPluginHandle();


		g_messaging = (F4SEMessagingInterface*)f4se->QueryInterface(kInterface_Messaging);
		if (!g_messaging)
		{
			_FATALERROR("couldn't get messaging interface");
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
		HandleIniFile();
		Compass::InitHooks();
		logMessage("load");
		return true;

	}

};
