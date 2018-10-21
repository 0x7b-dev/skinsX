#include <stdio.h>
#include "nbqmemory.h"

/*
* necessary offsets and netvars for our knife / skin changer
* some of these need to be updated after each game update
*/
#define m_dwLocalPlayer 0xC5F89C
#define m_dwEntityList 0x4C3C4F4
#define m_hViewModel 0x32DC
#define m_iViewModelIndex 0x3210
#define m_flFallbackWear 0x31B0
#define m_nFallbackPaintKit 0x31A8
#define m_iItemIDHigh 0x2FB0
#define m_iEntityQuality 0x2F9C
#define m_iItemDefinitionIndex 0x2F9A
#define m_hActiveWeapon 0x2EE8
#define m_hMyWeapons 0x2DE8
#define m_nModelIndex 0x254

/*
* viewmodel index offsets located in the sv_precacheinfo list
* these usually change after new knives are introduced to the game
*/
#define precache_bayonet_ct 64 // v_knife_bayonet.mdl - v_knife_default_ct.mdl
#define precache_bayonet_t 40 // v_knife_bayonet.mdl - v_knife_default_t.mdl

/*
* nbq's memory class
*/
NBQMemory mem;

enum knifeDefinitionIndex               // id
{
	WEAPON_KNIFE = 42,
	WEAPON_KNIFE_T = 59,
	WEAPON_KNIFE_BAYONET = 500,         // 0
	WEAPON_KNIFE_FLIP = 505,            // 1
	WEAPON_KNIFE_GUT = 506,             // 2
	WEAPON_KNIFE_KARAMBIT = 507,        // 3
	WEAPON_KNIFE_M9_BAYONET = 508,      // 4
	WEAPON_KNIFE_TACTICAL = 509,        // 5
	WEAPON_KNIFE_FALCHION = 512,        // 6
	WEAPON_KNIFE_SURVIVAL_BOWIE = 514,  // 7
	WEAPON_KNIFE_BUTTERFLY = 515,       // 8
	WEAPON_KNIFE_PUSH = 516,            // 9
	WEAPON_KNIFE_URSUS = 519,           // 10
	WEAPON_KNIFE_GYPSY_JACKKNIFE = 520, // 11
	WEAPON_KNIFE_STILETTO = 522,        // 12
	WEAPON_KNIFE_WIDOWMAKER = 523       // 13
};
int GetKnifeID(const short itemDef)
{
	switch (itemDef)
	{
		default:
		case WEAPON_KNIFE_BAYONET:
			return 0;
		case WEAPON_KNIFE_FLIP:
			return 1;
		case WEAPON_KNIFE_GUT:
			return 2;
		case WEAPON_KNIFE_KARAMBIT:
			return 3;
		case WEAPON_KNIFE_M9_BAYONET:
			return 4;
		case WEAPON_KNIFE_TACTICAL:
			return 5;
		case WEAPON_KNIFE_FALCHION:
			return 6;
		case WEAPON_KNIFE_SURVIVAL_BOWIE:
			return 7;
		case WEAPON_KNIFE_BUTTERFLY:
			return 8;
		case WEAPON_KNIFE_PUSH:
			return 9;
		case WEAPON_KNIFE_URSUS:
			return 10;
		case WEAPON_KNIFE_GYPSY_JACKKNIFE:
			return 11;
		case WEAPON_KNIFE_STILETTO:
			return 12;
		case WEAPON_KNIFE_WIDOWMAKER:
			return 13;
	}
}
void skinsX(HANDLE csgo, DWORD client, short itemDef, DWORD paintKit)
{
	const int itemIDHigh = -1;
	const int entityQuality = 3;
	const float fallbackWear = 0.0001f;

	int knifeID = GetKnifeID(itemDef);
	int knifeIDOffset = knifeID < 10 ? 0 : 1; /* precache offset id by 1 for new knives */

	DWORD cachedPlayer = 0;
	DWORD modelIndex = 0;

	while (!GetAsyncKeyState(VK_F9))
	{
		DWORD localPlayer = mem.ReadMemory<DWORD>(csgo, client + m_dwLocalPlayer);
		if (localPlayer == 0) /* localplayer is not connected to any server (unreliable method) */
		{
			modelIndex = 0;
			continue;
		}
		else if (localPlayer != cachedPlayer) /* localplayer changed by server switch / demo record */
		{
			modelIndex = 0;
			cachedPlayer = localPlayer;
		}

		if (paintKit > 0 && modelIndex > 0)
		{
			for (int i = 0; i < 8; i++)
			{
				/* handle to weapon entity in the current slot */
				DWORD myWeapons = mem.ReadMemory<DWORD>(csgo, localPlayer + m_hMyWeapons + i * 0x4) & 0xfff;
				DWORD weaponEntity = mem.ReadMemory<DWORD>(csgo, client + m_dwEntityList + (myWeapons - 1) * 0x10);
				if (weaponEntity == 0) { continue; }

				/* id of the weapon in the current slot */
				short weaponID = mem.ReadMemory<short>(csgo, weaponEntity + m_iItemDefinitionIndex);

				DWORD fallbackPaint = paintKit;
				if (weaponID == 1) { fallbackPaint = 37; } /* Desert Eagle | Blaze */
				else if (weaponID == 4) { fallbackPaint = 38; } /* Glock-18 | Fade */
				else if (weaponID == 7) { fallbackPaint = 180; } /* AK-47 | Fire Serpent */
				else if (weaponID == 9) { fallbackPaint = 344; } /* AWP | Dragon Lore */
				else if (weaponID == 60) { fallbackPaint = 445; } /* M4A1-S | Hot Rod */
				else if (weaponID == 61) { fallbackPaint = 653; } /* USP-S | Neo-Noir */
				else if (weaponID != WEAPON_KNIFE && weaponID != WEAPON_KNIFE_T && weaponID != itemDef) { continue; }
				else
				{
					/* knife-specific memory writes */
					mem.WriteMemory<short>(csgo, weaponEntity + m_iItemDefinitionIndex, itemDef);
					mem.WriteMemory<DWORD>(csgo, weaponEntity + m_nModelIndex, modelIndex);
					mem.WriteMemory<DWORD>(csgo, weaponEntity + m_iViewModelIndex, modelIndex);
					mem.WriteMemory<int>(csgo, weaponEntity + m_iEntityQuality, entityQuality);
				}

				/* memory writes necessary for both knives and other weapons in slots */
				mem.WriteMemory<int>(csgo, weaponEntity + m_iItemIDHigh, itemIDHigh);
				mem.WriteMemory<DWORD>(csgo, weaponEntity + m_nFallbackPaintKit, fallbackPaint);
				mem.WriteMemory<float>(csgo, weaponEntity + m_flFallbackWear, fallbackWear);
			}
		}

		/* handle to weapon entity we are currently holding in hands */
		DWORD activeWeapon = mem.ReadMemory<DWORD>(csgo, localPlayer + m_hActiveWeapon) & 0xfff;
		DWORD weaponEntity = mem.ReadMemory<DWORD>(csgo, client + m_dwEntityList + (activeWeapon - 1) * 0x10);
		if (weaponEntity == 0) { continue; }

		/* id of weapon we are currently holding in hands */
		short weaponID = mem.ReadMemory<short>(csgo, weaponEntity + m_iItemDefinitionIndex);

		/* viewmodel id of the weapon in our hands (default ct knife should usually be around 300) */
		int weaponViewModelID = mem.ReadMemory<int>(csgo, weaponEntity + m_iViewModelIndex);

		/* a way to calculate the correct modelindex for different servers / maps / teams */
		if (weaponID == WEAPON_KNIFE && weaponViewModelID > 0)
		{
			modelIndex = weaponViewModelID + precache_bayonet_ct + 3 * knifeID + knifeIDOffset;
		}
		else if (weaponID == WEAPON_KNIFE_T && weaponViewModelID > 0)
		{
			modelIndex = weaponViewModelID + precache_bayonet_t + 3 * knifeID + knifeIDOffset;
		}
		else if (weaponID != itemDef || modelIndex == 0) { continue; }

		/* handle to viewmodel entity we will use to change the knife viewmodel index */
		DWORD knifeViewmodel = mem.ReadMemory<DWORD>(csgo, localPlayer + m_hViewModel) & 0xfff;
		DWORD knifeEntity = mem.ReadMemory<DWORD>(csgo, client + m_dwEntityList + (knifeViewmodel - 1) * 0x10);
		if (knifeEntity == 0) { continue; }

		/* change our current viewmodel but only if localplayer is holding a knife in hands */
		mem.WriteMemory<DWORD>(csgo, knifeEntity + m_nModelIndex, modelIndex);
	}
}

int main()
{
	printf("************************************\n");
	printf("skinsX | External Knife/Skin Changer\n");
	printf("************************************\n");

	DWORD dwPID = 0;
	DWORD dwClient = 0;

	printf("Looking for csgo.exe... ");
	do {
		dwPID = mem.GetProcessIdByProcessName("csgo.exe");
		Sleep(500);
	} while (!dwPID);
	printf("%d\n", dwPID);

	printf("Module client_panorama.dll... ");
	do {
		dwClient = mem.GetModuleBaseAddress(dwPID, "client_panorama.dll");
		Sleep(500);
	} while (!dwClient);
	printf("0x%X\n", dwClient);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	printf("Handle to csgo.exe process... 0x%X\n", (DWORD)hProcess);

	if (hProcess != INVALID_HANDLE_VALUE)
	{
		/* give ourselves a karambit sapphire */
		skinsX(hProcess, dwClient, WEAPON_KNIFE_KARAMBIT, 416);
	}

	if (hProcess) { CloseHandle(hProcess); }

	return 0;
}