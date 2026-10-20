# Larp

Local client-side larp tool for Roblox. Theme and clothing-item swaps,
rendered through an ImGui overlay. External read/write via a kernel driver.

## Layout

```
driver/        WDM kernel driver exposing \\.\Larp (r/w primitives)
loader/        Notes on mapping Larp.sys (BYOVD or dev testsigning)
controller/    Usermode controller: offsets, roblox walker, UI
presets/       JSON preset libraries (themes, items)
third_party/   imgui + nlohmann/json — fetched manually
```

## Build

**Driver (one-time):**
- Visual Studio 2022 + WDK installed
- Open `driver/` as a Kernel Mode Driver project (WDM, Empty)
- Build x64 Release → `Larp.sys`

**Controller:**
- Drop `imgui/` and `nlohmann/` headers under `third_party/`
- `cmake -B build -A x64`
- `cmake --build build --config Release`

## Run

1. Map `Larp.sys` (see `loader/README.md`)
2. Launch Roblox, join a place
3. `LarpController.exe`
4. INSERT toggles mouse interactivity on the overlay

## Offsets

`controller/offsets.h` is pinned to Roblox `version-2366ba214ec740ca`
(dumped 2026-09-22). Re-dump after each Roblox update; source is noted
at the top of the file.
