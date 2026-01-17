#include "pch.h"
#include <windows.h>
#include <random>
#include <thread>
#include <psapi.h>

const char* GAMEDATAMAN_AOB = "\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x74\x05\x48\x8B\x40\x58\xC3\xC3";
const char* GAMEDATAMAN_MASK = "xxx????xxxxxxxxxxx";

struct Offsets {
    static const uintptr_t StatBasePtr = 0x8;
    static const uintptr_t Vigor = 0x3C;
    static const uintptr_t Level = 0x68;
    static const uintptr_t DeathCount = 0x94;
};

uintptr_t FindPattern(const char* pattern, const char* mask) {
    MODULEINFO modInfo = { 0 };
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &modInfo, sizeof(MODULEINFO));
    uintptr_t start = (uintptr_t)modInfo.lpBaseOfDll;
    uintptr_t size = (uintptr_t)modInfo.SizeOfImage;
    size_t patternLen = strlen(mask);
    for (uintptr_t i = 0; i < size - patternLen; i++) {
        bool found = true;
        for (size_t j = 0; j < patternLen; j++) {
            if (mask[j] != '?' && pattern[j] != *(char*)(start + i + j)) {
                found = false; break;
            }
        }
        if (found) return start + i;
    }
    return 0;
}

uintptr_t GetRIPRelative(uintptr_t address) {
    if (!address) return 0;
    int32_t offset = *(int32_t*)(address + 3);
    return address + offset + 7;
}

void RandomizeStatsAndLevel(uintptr_t gameDataManAddr) {
    uintptr_t gameDataMan = *(uintptr_t*)gameDataManAddr;
    if (!gameDataMan) return;
    uintptr_t statBase = *(uintptr_t*)(gameDataMan + Offsets::StatBasePtr);
    if (!statBase) return;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> disStats(1, 99);
    std::uniform_int_distribution<> disLevel(1, 713);

    for (int i = 0; i < 8; i++) {
        *(int*)(statBase + Offsets::Vigor + (i * 4)) = disStats(gen);
    }

    *(int*)(statBase + Offsets::Level) = disLevel(gen);
}

void ModThread() {
    uintptr_t gameDataManInst = FindPattern(GAMEDATAMAN_AOB, GAMEDATAMAN_MASK);
    if (!gameDataManInst) return;

    uintptr_t gameDataManAddr = GetRIPRelative(gameDataManInst);
    int lastDeathCount = -1;

    while (true) {
        uintptr_t gameDataMan = *(uintptr_t*)gameDataManAddr;
        if (gameDataMan) {
            int currentDeaths = *(int*)(gameDataMan + Offsets::DeathCount);

            if (lastDeathCount == -1) {
                lastDeathCount = currentDeaths;
            }

            if (currentDeaths > lastDeathCount) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));

                RandomizeStatsAndLevel(gameDataManAddr);

                lastDeathCount = currentDeaths;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ModThread, nullptr, 0, nullptr);
    }
    return TRUE;
}