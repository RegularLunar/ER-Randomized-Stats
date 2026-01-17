# ER-StatRandomizer

A lightweight C++ DLL middleware for **Elden Ring** that monitors player death events to inject randomized attributes and character levels.

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CMake](https://img.shields.io/badge/CMake-Build-green.svg)](https://cmake.org/)

## Warning
**Do not use this mod while playing online.** Elden Ring utilizes Easy Anti-Cheat (EAC). Modifying memory while connected to FromSoftware's servers will result in an account ban. Always play in **Offline Mode** with EAC disabled via Mod Engine 2 or a similar launcher.
**BACK UP YOUR SAVES!** This mod **permanently** alters your attributes.

## Features
- **Death-Triggered**: Stats are randomized only upon death detection.
- **Attribute Randomization**: All 8 primary attributes (Vigor through Arcane) scaled 1–99.
- **Level Scaling**: Character level randomized between 1–713.

## Prerequisites
- **Visual Studio 2022** (with "Desktop development with C++" workload)
- **CMake 3.16** or higher
- **Windows SDK**

## Building from Source
1. Clone the repository:
   ```powershell
   git clone https://github.com/regularlunar/ER-Randomized-Stats.git
   cd ER-Randomized-Stats
   ```
2. Configure the project:
   ```powershell
   cmake -B build -S . -A x64
   ```
3. Build the DLL:
   ```powershell
   cmake --build build --config Release
   ```
The compiled `StatRandomizer.dll` will be in the `build/Release` folder.

## Installation

### Via Mod Engine 2
1. Place `StatRandomizer.dll` into your `ModEngine2/mod` folder.
2. Open `config_eldenring.toml` and add the DLL to the `external_dlls` array:
   ```toml
   external_dlls = [ "mod/StatRandomizer.dll" ]
   ```

### Via Elden Mod Loader
1. Navigate to the game's `/Game/` directory.
2. Place `StatRandomizer.dll` into the `mods` folder.

## Technical Details

### Signature Scanning (AOB)
The module identifies the `GameDataManager` singleton dynamically. This ensures compatibility across game versions by searching for a specific byte pattern in the `.text` section rather than relying on static offsets.
- **Pattern:** `48 8B 05 ? ? ? ? 48 85 C0 74 05 48 8B 40 58 C3 C3`

### RIP-Relative Addressing
The module resolves the absolute address of the `GameDataManager` by extracting the 32-bit signed displacement from the `MOV` instruction and adding it to the current Instruction Pointer (RIP) plus the instruction length (7 bytes).

### Mutation Logic
The background thread implements a simple state machine:
1. **Pointer Traversal**: Follows `GameDataManager → StatBasePtr (0x8)`.
2. **State Monitoring**: Tracks the `DeathCount` value at offset `0x94`.
3. **Execution Delay**: Upon detecting an increment, the thread sleeps for 2000ms. This prevents memory corruption during the game's internal cleanup/loading transition.
4. **Entropy Injection**: Uses `std::mt19937` to iterate through the attribute block starting at `Vigor (0x3C)`, applying 4-byte step increments to overwrite the 8-integer stat array.

### Data Map
| Offset | Field | Description |
| :--- | :--- | :--- |
| `0x08` | `StatBasePtr` | Base pointer to character stats. |
| `0x3C` | `Vigor` | Start of 32-byte attribute block. |
| `0x68` | `Level` | Total character level. |
| `0x94` | `DeathCount` | Incremental player death tracker. |

## Credits
- Developed by **[RegularLunar]**
- Research and Signatures via **[The Grand Archives](https://github.com/The-Grand-Archives/Elden-Ring-CT-TGA)**
