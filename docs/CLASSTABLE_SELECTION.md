# Which classtables folder a mission loads (packet `cc9_classtable_selection`)

**Answer: `classtables/arcade/`, chosen by this installation's Lua, not by the image. The host
loads the same table. So USN02's failed-mission outcome at 44.6 s (`docs/USN02_SAMESIDE_TORPEDOES.md`
5.1) is the image's outcome for an idle player on this installation, not a host artefact.**

## 1. The image has no selector

The executable on disk (`battlestationspacific.exe`) contains none of these strings:
`classtables`, `realistic` (in any case), `arcade`, `gamemode.lua`, `RealisticTable` or
`ArcadeTable`. It names only the autoload files, for example `autoload/DeviceClasses.lua` and
`autoload/VehicleClasses.lua`. It runs `scripts/datatables/autoload/*.lua` and reads the globals
they leave: `Bullets`, `DeviceClass`, `ReconClass`, `VehicleClass`. Nothing native chooses
between two folders.

The class readers then take their fields from whatever those globals hold. The Type 93's
`WaterTravelSpeed` reaches the torpedo class through `BSP_TorpedoClass_ReadLuaFields`
(`008566B0`) into `classDesc+0E4h`, which `00855A9A` and the torpedo bot's intercept
(`0090022B`) read (`docs/TORPEDO_CATEGORY_ADMISSION.md`, `docs/BULLET_ENGAGEMENT_RANGE.md`).

## 2. The selector is this installation's (modded) autoload wrapper

`scripts/datatables/autoload/bulletclasses.lua` (536 bytes) in this installation:

```
DoFile("gamemode.lua")
Bullets = {}
if GameMode == 1 then
    DoFile("Scripts/datatables/classtables/realistic/bulletclasses.lua")
    Bullets = RealisticTable
else
    DoFile("Scripts/datatables/classtables/arcade/bulletclasses.lua")
    Bullets = ArcadeTable
end
```

`deviceclasses.lua`, `reconclasses.lua` and `effects.lua` (the effect path) in the same folder
have the same shape.

The installation root carries `gamemode.lua` (229 bytes, 2024-07-13):

```
-- Note from Breeze
-- Kantai Kessen does not support BSPRM 1.0s Realistic mode, however this file is still conntected to several others
-- Do not bother changing this, certain things probably will not work anyway
GameMode = 0
```

`GameMode = 0` takes the `else` arm. If `DoFile` did not resolve the root file, `GameMode` would
be `nil` and still take the `else` arm. **Both readings load `classtables/arcade/`.** The mod's
own note says its Realistic mode is not supported in this build. Nothing is set per mission:
the wrappers run once from autoload.

The files, for the record:

| file | size | modified |
| --- | --- | --- |
| `classtables/arcade/bulletclasses.lua` | 75048 | 2026-05-09 23:04 (locally modified) |
| `classtables/arcade/deviceclasses.lua` | 433194 | 2026-05-09 22:37 (locally modified) |
| `classtables/arcade/reconclasses.lua` | 15982 | 2024-07-13 |
| `classtables/realistic/bulletclasses.lua` | 64887 | 2025-06-02 |
| `classtables/realistic/deviceclasses.lua` | 392736 | 2024-07-13 |
| `classtables/realistic/reconclasses.lua` | 16031 | 2024-07-13 |

## 3. The host

`GameMissionLuaHost::read_bullet_class_number` reads the global `Bullets` the same wrapper
fills (`src/game_hosts_lua.cpp`). The Type 93 row reads `WaterTravelSpeed` 170.444 there, as the
launch diagnostic showed (`water_speed=170.4`, `local/stB_usn02.log`). The realistic table has
51.444 for the same row, "24. Type 93 Mod1 Long Lance ship torpedo". The host's recon table
is hard-wired to `arcade` (`kReconVariant`, `src/game_hosts_gunnery.cpp`). That is the same
answer, although its comment says the tree has no `gamemode.lua`. It has one at the
installation root; the result does not change.

**No divergence, no switch.**

## 4. What the table choice would change

This does not apply to this installation, because the choice is fixed. The realistic folder
differs from arcade in:
- the bullet classes: all torpedoes top out at 51.444, and ranges, fly times and damages differ;
- the device classes: gun `Throw`, reload and windows;
- the recon classes;
- the effect path.

`vehicleclasses.lua` is a single autoload file. Its `HP_Realistic`, `MaxSpeed_Realistic` and
`MaxTorpedoStock_Realistic` keys are mod data. The executable has no `Realistic` string, so no
native reader takes them.

## 5. Consequence

USN02's reference outcome under the landed torpedo steering (`kTorpedoGyroHeadingBound`) stands:
the arcade Type 93 at 170.444 m/s, swimming at about 102 m/s, steers onto its lead point. The
opening IJN salvos sink Exeter and Perth by 47 s, and the mission ends failed at 44.6 s with the
player idle. Re-baseline USN02 on that.
