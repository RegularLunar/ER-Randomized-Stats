#include <windows.h>
#include <thread>
#include <psapi.h>
#include <chrono>
#include <cstdint>

//-----------------------------------------------------------------------------
// xoshiro256** PRNG
// See: https://prng.di.unimi.it/
// Seeded via splitmix64 as recommended by the authors.
//-----------------------------------------------------------------------------

struct xoshiro256ss_state {
    uint64_t s[4];
};

static inline uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

uint64_t next(xoshiro256ss_state* state) {
    const uint64_t result = rotl(state->s[1] * 5, 7) * 9;
    const uint64_t t = state->s[1] << 17;

    state->s[2] ^= state->s[0];
    state->s[3] ^= state->s[1];
    state->s[1] ^= state->s[2];
    state->s[0] ^= state->s[3];

    state->s[2] ^= t;
    state->s[3] = rotl(state->s[3], 45);

    return result;
}

void seed_rng(xoshiro256ss_state* state, uint64_t seed) {
    // splitmix64 is used to expand a single 64-bit seed into 256 bits of state.
    // Passing the same seed value repeatedly is intentional - splitmix64 mutates x.
    auto splitmix64 = [&](uint64_t& x) {
        uint64_t z = (x += 0x9e3779b97f4a7c15);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        return z ^ (z >> 31);
        };
    state->s[0] = splitmix64(seed);
    state->s[1] = splitmix64(seed);
    state->s[2] = splitmix64(seed);
    state->s[3] = splitmix64(seed);
}

//-----------------------------------------------------------------------------
// Game memory layout
//-----------------------------------------------------------------------------

// AOB for the GameDataMan singleton pointer instruction (mov rax, [rip+offset]).
// Wildcards cover the 4-byte RIP-relative displacement.
const char* GAMEDATAMAN_AOB  = "\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x74\x05\x48\x8B\x40\x58\xC3\xC3";
const char* GAMEDATAMAN_MASK = "xxx????xxxxxxxxxxx";

// Offsets are relative to the GameDataMan instance, not the module base.
// StatBasePtr dereferences to a stats sub-object; stats are packed as int[8] from Vigor onward.
// Level lives in the same sub-object. DeathCount is on GameDataMan itself.
struct Offsets {
    static const uintptr_t StatBasePtr = 0x8;
    static const uintptr_t Vigor       = 0x3C;
    static const uintptr_t Level       = 0x68;
    static const uintptr_t DeathCount  = 0x94;
};

//-----------------------------------------------------------------------------
// Helpers
//-----------------------------------------------------------------------------

int GetRandomInt(xoshiro256ss_state* state, int min, int max) {
    // Biased toward low end for large ranges, acceptable here.
    return min + (int)(next(state) % (max - min + 1));
}

// Naive byte-pattern scan over the main module image.
// Returns 0 if not found. Not optimized; called once at startup so it's fine.
uintptr_t FindPattern(const char* pattern, const char* mask) {
    MODULEINFO modInfo = { 0 };
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &modInfo, sizeof(MODULEINFO));
    uintptr_t start = (uintptr_t)modInfo.lpBaseOfDll;
    uintptr_t size  = (uintptr_t)modInfo.SizeOfImage;
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

// Resolves a RIP-relative MOV instruction to the address of the pointed-to global.
// Assumes the displacement is at bytes [+3..+6] and the instruction is 7 bytes total.
uintptr_t GetRIPRelative(uintptr_t address) {
    if (!address) return 0;
    int32_t offset = *(int32_t*)(address + 3);
    return address + offset + 7;
}

//-----------------------------------------------------------------------------
// Core logic
//-----------------------------------------------------------------------------

// Writes random values to all 8 stats (Vigor through Arcane, contiguous int[8])
// and randomizes the player level. Called 2 seconds after each detected death
// to let the death animation and respawn sequence finish.
void RandomizeStatsAndLevel(uintptr_t gameDataManAddr, xoshiro256ss_state* rng) {
    uintptr_t gameDataMan = *(uintptr_t*)gameDataManAddr;
    if (!gameDataMan) return;

    uintptr_t statBase = *(uintptr_t*)(gameDataMan + Offsets::StatBasePtr);
    if (!statBase) return;

    for (int i = 0; i < 8; i++) {
        *(int*)(statBase + Offsets::Vigor + (i * 4)) = GetRandomInt(rng, 1, 99);
    }

    *(int*)(statBase + Offsets::Level) = GetRandomInt(rng, 1, 713); // 713 = max level
}

void ModThread() {
    uintptr_t gameDataManInst = FindPattern(GAMEDATAMAN_AOB, GAMEDATAMAN_MASK);
    if (!gameDataManInst) return;

    uintptr_t gameDataManAddr = GetRIPRelative(gameDataManInst);
    int lastDeathCount = -1; // -1 = not yet initialized; avoids triggering on the first read
    xoshiro256ss_state rng;
    uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    seed_rng(&rng, timeSeed);

    while (true) {
        uintptr_t gameDataMan = *(uintptr_t*)gameDataManAddr;
        if (gameDataMan) {
            int currentDeaths = *(int*)(gameDataMan + Offsets::DeathCount);

            if (lastDeathCount == -1) {
                lastDeathCount = currentDeaths;
            }

            if (currentDeaths > lastDeathCount) {
                // Wait for the game to finish its death/respawn state transition
                // before writing stats. Writing too early can be overwritten or crash.
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));

                RandomizeStatsAndLevel(gameDataManAddr, &rng);

                lastDeathCount = currentDeaths;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ModThread, nullptr, 0, nullptr);
    }
    return TRUE;
}
