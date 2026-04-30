# Elden Ring Attribute Randomizer
> A lightweight C++ DLL middleware for Elden Ring that monitors player death events to inject randomized attributes and character levels.

![Release](https://img.shields.io/github/v/release/RegularLunar/ER-Randomized-Stats?style=for-the-badge&color=916cd9) 
![License](https://img.shields.io/badge/License-ARR-10b981?style=for-the-badge)
![Stars](https://img.shields.io/github/stars/RegularLunar/ER-Randomized-Stats?style=for-the-badge&color=f59e0b)
![Downloads](https://img.shields.io/github/downloads/RegularLunar/ER-Randomized-Stats/total?style=for-the-badge&color=0ea5e9&label=Downloads)
![Last Commit](https://img.shields.io/github/last-commit/RegularLunar/ER-Randomized-Stats?style=for-the-badge&color=6366f1)

---

## Warning
- **Do not use this mod while playing online.** Elden Ring utilizes Easy Anti-Cheat (EAC). Modifying memory while connected to FromSoftware's servers will result in an account ban. Always play in **Offline Mode** with EAC disabled via Mod Engine 2 or a similar launcher. 
- **I am not responsible for any harm to your account. You have been warned.**
- **BACK UP YOUR SAVES!** This mod **permanently** alters your attributes.

### Features
- **Death Trigger:** Stats and Level re-roll 2s after every death.
- **xoshiro256\*\*:** PRNG gold-standard randomness (no repeating patterns).
- **AOB Scanning:** Patch-resistant; works across different game versions.

---

### Building From Source
- **Visual Studio 2022** (with "Desktop development with C++" workload)
- **CMake 3.16** or higher
- **Windows SDK**

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
The compiled `ER-RandomizedStats.dll` will be in the `build/Release` folder.

---

### Support
Issues and PRs are welcome. For major changes, please open an issue first.

### Acknowledgements / Credits
- **[The Grand Archives](https://github.com/The-Grand-Archives/Elden-Ring-CT-TGA)** - Signatures, Pointers

---
<sub>Made with 💜 by [RegularLunar](https://github.com/RegularLunar/)</sub>
