#include "json.hpp"
#include <fstream>
#include <iostream>
#include <API/ARK/ARK.h>

#pragma comment(lib, "ArkApi.lib")

nlohmann::json config;

std::string GetConfigPath()
{
	return ArkApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/AntiStructureDamage/config.json";
}

FString GetText(const std::string& str)
{
	return FString(ArkApi::Tools::Utf8Decode(config["Messages"].value(str, "No message")).c_str());
}

DECLARE_HOOK(APrimalStructure_TakeDamage, float, APrimalStructure*, float, FDamageEvent*, AController*, AActor*);

float Hook_APrimalStructure_TakeDamage(APrimalStructure* _this, float Damage, FDamageEvent* DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (_this->TargetingTeamField() != 0)
	{
		if (EventInstigator)
		{
			ACharacter* character = EventInstigator->CharacterField();

			if (character != nullptr && character->IsA(APrimalDinoCharacter::GetPrivateStaticClass()))
			{
				APrimalDinoCharacter* dino = static_cast<APrimalDinoCharacter*>(character);

				FString descr;
				dino->GetDinoDescriptiveName(&descr);

				const bool mek_damage = config.value("PreventMekDamage", true);
				const bool extinction_titans_damage = config.value("PreventExtinctionTitansDamage", true);
				const bool corrupted_damage = config.value("PreventCorruptedDamage", true);

				/*Log::GetLog()->warn(descr.ToString());*/

				if (mek_damage) {
					if (descr.Contains(L"Mek"))
					{
						return 0;
					}
				}
				if (extinction_titans_damage) {
					if (descr.EndsWith(L" Titan") || descr.Contains(L"Titßn"))
					{
						return 0;
					}
				}
				if (corrupted_damage) {
					if (descr.Contains(L"Corrupt"))
					{
						return 0;
					}
				}
			}
		}
	}

	return APrimalStructure_TakeDamage_original(_this, Damage, DamageEvent, EventInstigator, DamageCauser);
}

void ReadConfig()
{
	const std::string config_path = GetConfigPath();
	std::ifstream file{config_path};
	if (!file.is_open())
		throw std::runtime_error("Can't open config.json");

	file >> config;

	file.close();
}

void Load()
{
	Log::Get().Init("AntiStructureDamage");

	try
	{
		ReadConfig();
	}
	catch (const std::exception& error)
	{
		Log::GetLog()->error(error.what());
		throw;
	}

	ArkApi::GetHooks().SetHook("APrimalStructure.TakeDamage", &Hook_APrimalStructure_TakeDamage,
		&APrimalStructure_TakeDamage_original);
}

void Unload()
{
	ArkApi::GetHooks().DisableHook("APrimalStructure.TakeDamage", &Hook_APrimalStructure_TakeDamage);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Load();
		break;
	case DLL_PROCESS_DETACH:
		Unload();
		break;
	}
	return TRUE;
}