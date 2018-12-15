#include <stdio.h>
#include "nbqmemory.h"

/*
* necessary offsets and netvars for our knife / skin changer
* some of these need to be updated after each game update
*/
#define m_dwLocalPlayer 0xCB3694
#define m_dwEntityList 0x4CC3564
#define m_hViewModel 0x32F8
#define m_iViewModelIndex 0x3220
#define m_flFallbackWear 0x31C0
#define m_nFallbackPaintKit 0x31B8
#define m_iItemIDHigh 0x2FC0
#define m_iEntityQuality 0x2FAC
#define m_iItemDefinitionIndex 0x2FAA
#define m_hActiveWeapon 0x2EF8
#define m_hMyWeapons 0x2DF8
#define m_nModelIndex 0x258

/*
* offsets between viewmodel indexes located in the sv_precacheinfo list
* these usually change after new knives are introduced to the game
*/
#define precache_bayonet_ct 84 // = v_knife_bayonet.mdl - v_knife_default_ct.mdl
#define precache_bayonet_t 60 // = v_knife_bayonet.mdl - v_knife_default_t.mdl

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

void skinsX(HANDLE csgo, DWORD client, int knifeID, short itemDef, DWORD paintKit)
{
	const int itemIDHigh = -1;
	const int entityQuality = 3;
	const float fallbackWear = 0.0001f;

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
				DWORD currentWeapon = mem.ReadMemory<DWORD>(csgo, localPlayer + m_hMyWeapons + i * 0x4) & 0xfff;
				currentWeapon = mem.ReadMemory<DWORD>(csgo, client + m_dwEntityList + (currentWeapon - 1) * 0x10);
				if (currentWeapon == 0) { continue; }

				/* id of the weapon in the current slot */
				short weaponID = mem.ReadMemory<short>(csgo, currentWeapon + m_iItemDefinitionIndex);

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
					mem.WriteMemory<short>(csgo, currentWeapon + m_iItemDefinitionIndex, itemDef);
					mem.WriteMemory<DWORD>(csgo, currentWeapon + m_nModelIndex, modelIndex);
					mem.WriteMemory<DWORD>(csgo, currentWeapon + m_iViewModelIndex, modelIndex);
					mem.WriteMemory<int>(csgo, currentWeapon + m_iEntityQuality, entityQuality);
				}

				/* memory writes necessary for both knives and other weapons in slots */
				mem.WriteMemory<int>(csgo, currentWeapon + m_iItemIDHigh, itemIDHigh);
				mem.WriteMemory<DWORD>(csgo, currentWeapon + m_nFallbackPaintKit, fallbackPaint);
				mem.WriteMemory<float>(csgo, currentWeapon + m_flFallbackWear, fallbackWear);
			}
		}

		/* handle to weapon entity we are currently holding in hands */
		DWORD activeWeapon = mem.ReadMemory<DWORD>(csgo, localPlayer + m_hActiveWeapon) & 0xfff;
		activeWeapon = mem.ReadMemory<DWORD>(csgo, client + m_dwEntityList + (activeWeapon - 1) * 0x10);
		if (activeWeapon == 0) { continue; }

		/* id of weapon we are currently holding in hands */
		short weaponID = mem.ReadMemory<short>(csgo, activeWeapon + m_iItemDefinitionIndex);

		/* viewmodel id of the weapon in our hands (default ct knife should usually be around 300) */
		int weaponViewModelID = mem.ReadMemory<int>(csgo, activeWeapon + m_iViewModelIndex);

		/* calculate the correct modelindex using the index of default knives and the precache list */
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
		DWORD knifeViewModel = mem.ReadMemory<DWORD>(csgo, localPlayer + m_hViewModel) & 0xfff;
		knifeViewModel = mem.ReadMemory<DWORD>(csgo, client + m_dwEntityList + (knifeViewModel - 1) * 0x10);
		if (knifeViewModel == 0) { continue; }

		/* change our current viewmodel but only if localplayer is holding a knife in hands */
		mem.WriteMemory<DWORD>(csgo, knifeViewModel + m_nModelIndex, modelIndex);
	}
}
void skinsPrint(const char* title, char* name[], DWORD sz, DWORD x)
{
	Sleep(100);
	printf("%s %s %s %s\t\t\t\r", title, x > 0 ? "<" : "|", name[x], x < sz ? ">" : "|");
}
DWORD skinsSelect(const char* title, char* name[], DWORD sz)
{
	DWORD x = 0; // index of current item
	skinsPrint(title, name, sz, x);

	while (!GetAsyncKeyState(VK_RETURN))
	{
		if (GetAsyncKeyState(VK_RIGHT) && x < sz)
		{
			skinsPrint(title, name, sz, ++x);
		}
		else if (GetAsyncKeyState(VK_LEFT) && x > 0)
		{
			skinsPrint(title, name, sz, --x);
		}
	}

	return x;
}
DWORD skinsLoad(char** names[], DWORD* ids[])
{
	DWORD i = 0;
	FILE* fp;

	if (fopen_s(&fp, "skins.txt", "r") == 0)
	{
		char line[64];

		while (fgets(line, 64, fp) != 0)
		{
			DWORD id;
			char name[64];

			if (sscanf_s(line, "%03d %63[^\n]", &id, &name, sizeof(name)) != 2) { continue; }

			*ids = (DWORD*)realloc(*ids, (i + 1) * sizeof(DWORD));
			(*ids)[i] = id;

			*names = (char**)realloc(*names, (i + 1) * sizeof(char*));
			(*names)[i] = _strdup(name);

			i++;
		}

		fclose(fp);
	}

	return i;
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

	/* arrays for our menu */

	char* knifeNames[] = { "Bayonet",
		"Flip Knife",
		"Gut Knife",
		"Karambit",
		"M9 Bayonet",
		"Huntsman Knife",
		"Falchion Knife",
		"Bowie Knife",
		"Butterfly Knife",
		"Shadow Daggers",
		"Ursus Knife",
		"Navaja Knife",
		"Stiletto Knife",
		"Talon Knife" };

	short knifeIDs[] = { WEAPON_KNIFE_BAYONET,
		WEAPON_KNIFE_FLIP,
		WEAPON_KNIFE_GUT,
		WEAPON_KNIFE_KARAMBIT,
		WEAPON_KNIFE_M9_BAYONET,
		WEAPON_KNIFE_TACTICAL,
		WEAPON_KNIFE_FALCHION,
		WEAPON_KNIFE_SURVIVAL_BOWIE,
		WEAPON_KNIFE_BUTTERFLY,
		WEAPON_KNIFE_PUSH,
		WEAPON_KNIFE_URSUS,
		WEAPON_KNIFE_GYPSY_JACKKNIFE,
		WEAPON_KNIFE_STILETTO,
		WEAPON_KNIFE_WIDOWMAKER };

	char** skinNames = 0;
	DWORD* skinIDs = 0;

	DWORD count = skinsLoad(&skinNames, &skinIDs);
	if (count)
	{
		printf("Loaded %d skins from skins.txt.\n", count);

		DWORD knifeID = skinsSelect("Select your knife model:", knifeNames, sizeof(knifeIDs) / sizeof(knifeIDs[0]) - 1);
		printf("\n");
		DWORD skinID = skinsSelect("Select your knife skin:", skinNames, count - 1);
		printf("\n");

		printf("Selected knife: %s | %s\n", knifeNames[knifeID], skinNames[skinID]);

		if (hProcess != INVALID_HANDLE_VALUE)
		{
			skinsX(hProcess, dwClient, knifeID, knifeIDs[knifeID], skinIDs[skinID]);
		}
	}
	else
	{
		printf("Error loading skins from skins.txt!\n");
	}

	if (hProcess) { CloseHandle(hProcess); }
	if (skinIDs) { free(skinIDs); }
	if (skinNames) { free(skinNames); }

	return 0;
}