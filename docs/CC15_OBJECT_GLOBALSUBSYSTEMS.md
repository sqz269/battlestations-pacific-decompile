# cc15-ob1: 004DC6A0's owners (traffic, panel, warnings, weather)

Lane L1 of the cc15 single-player object bring-up. Switch: `kGlobalSubsystemsCc15Bound` in
`src/game_hosts_mission_frame.cpp`. Host object: `GameGlobalSubsystemsHost`
(`include/bsp/game_hosts_global_subsystems.hpp`, `src/game_hosts_global_subsystems.cpp`).
Names are hypotheses, not recovered symbols.

## What 004DC6A0 builds, and what this switch builds

| Owner | Allocation | Constructor | Store | Loader | Script | Host |
| --- | --- | --- | --- | --- | --- | --- |
| traffic | 004DC80F `push 58h` | 004A43C0 at 004DC82E | +21D0h at 004DC83E | 0049D690 at 004DC844 | `Scripts\datatables\TrafficGlobals.lua` | built |
| panel | 004DC849 `push 38h` | 00452660 at 004DC862 | +21E4h at 004DC872 | 0044FA30 at 004DC878 | `Scripts/datatables/DialogGlobals.lua` | built |
| power-up | 004DC87D `push 1C4h` | 008EDC60 at 004DC899 | [00F88C30] | 008ECEC0 at 004DC8AE | PowerupClasses.lua | **not built here**: cc13-powerup's `kPowerupGrantBound` path, which runs at this position |
| warnings | 004DC8B3 `push 1B0h` | 0098A020 at 004DC8CF | +21E0h at 004DC8DF | 009870A0 at 004DC8E5 | `Scripts/datatables/Warnings.lua` | constructor and Init's table fragment and scalar resets |
| weather | 004DC8EA `push 7Ch` | 00445B10 at 004DC903 | +21C4h at 004DC913 | 00444D20 at 004DC919 | `Scripts\datatables\Globals.lua` | built |

The rest of 004DC6A0 is not hosted by this switch, and the step stays a record:
- the `Game_Global` FileBlock;
- the global script folders 00886900 at 004DC72F (the host runs these on its own path);
- races 00800160;
- robots 00901610;
- recon values;
- marker classes;
- the collectgarbage string;
- the global effects vector.

Each Lua loader runs in a Lua 5.1.1 state of its own. The loader is
`PcStorageLuaOwner`, standing in for 00B6A020 mask 1, over `VfsLuaScriptFiles` on
the mounted VFS. The inputs are:
- **fundamentals:** `Scripts\fundamentals.lua` read out of the VFS (the unbound
  path of 00884770);
- **content suffixes:** none, because the host registers none (GameMissionLuaHost
  does the same);
- **x360comp:** 0;
- **region:** empty (0108FF20 / 0108FF24).

operator new's words that the constructors leave alone are zeroed. The image's heap
leaves them unspecified; zeroing gives a null traffic group and zero weather scalars
until the loaders write them.

## One warning owner

The host already ran a warning manager made of two fields:
- `Impl::warning`, the +D0h..+19Ch state;
- `Impl::warning_tables`, the +108h tables that cc15-wB4 loads.

These are now `warning_host_state` / `warning_host_tables`. Every use goes through
`Impl::warning()` / `warning_tables()`:
- With the switch on, once the owner exists, these return the owner's
  `state` and `*tables_108`. The owner is the manager.
- Otherwise they return the two host fields, so OFF is unchanged.

009870A0 (`BSP_WarningManager_Init`, called only from 004DC6A0) splits like this:

| Part | Host |
| --- | --- |
| publication to [00F8A0C4] | not published; the host reaches the owner through `global_subsystems` |
| +174h..+180h, +D0h, +198h, clocks +4h..+20h resets | run (`build_warnings`) |
| Warnings.lua through 00B6A020 / 00B69D40 | loaded once per owner through the existing cc15-wB4 fragment (`load_warning_message_bank_009870a0`); the loaded flag is cleared when a new owner is built |
| roster 00973F20 | not run: it walks the raw world [00E188A8]+6B4h; `roster_28` stays empty, and nothing in the host reads it |
| EnvironmentUSHorn / EnvironmentAirRaid effects 00871BA0 | not run; +190h / +194h stay null |
| hooks [00F8BBCC]+230h = 00987080, [00F8BF4C] = 00987090 | not published |

## Teardown

004D2BB0 calls each owner's vtable slot 0 with 1, in this order:
1. +21D0h at 004D2C65;
2. +21C4h at 004D2CBC;
3. +21DCh (the MessageManager) at 004D2D04;
4. +21E0h at 004D2D1C;
5. +21E4h at 004D2D34.

The host has no DestroyWorld, so `teardown_004d2bb0` runs in two places:
- before a later load builds a new set;
- when the mission frame host is destroyed.

It releases traffic, weather, warnings and then panel, logging one line each
(`global subsystems teardown: +21D0 traffic (004D2C65)` and so on). The panel uses
the recovered 00452720 with 1. The others use the typed bodies' host-storage
releases. The MessageManager has its own host and its own teardown.

## Seams

| Seam | Image | Host |
| --- | --- | --- |
| power-up manager | 008EDC60 / 008ECEC0 | cc13-powerup |
| warning roster, effects, hooks | 00973F20, 00871BA0, 00987080/90 | not run, see above |
| allocation words | operator new's unspecified words | zero |
| validation callbacks | 00BF6713 | throw (none is expected to be reached) |
| consumers | traffic, panel and weather readers elsewhere in the image | none yet; the owners are built and loaded, and nothing in the host reads them |

## Pairs

The runs are 2026-10-19, 4800 mission frames at 0.05 s. The baseline is origin/main
ee7ced46d. OFF is 1b4bc5613. ON is 1b4bc5613 with `kGlobalSubsystemsCc15Bound=true`; the
USN04 contents line comes from 50221743b with the same flip.

| Pair | pair_diff | Gameplay |
| --- | --- | --- |
| USN04 baseline vs this tree OFF | exit 0, identical apart from noise | identical |
| USN04 OFF vs ON | exit 1 | identical: 62 plane death modes, 90-row unit table |
| USN01 OFF vs ON | exit 1 | identical: 30 plane death modes, 104-row unit table |

Rows that move from OFF to ON, the same in both missions:
- Seven native rows are added, each concrete with calls=1:
  - `GlobalSubsystems::construct_traffic 004a43c0`;
  - `load_traffic 0049d690`;
  - `construct_panel 00452660`;
  - `load_dialog_config 0044fa30`;
  - `construct_warnings 0098a020`;
  - `construct_weather 00445b10`;
  - `load_weather 00444d20`.
- A `global subsystems built` line is added: four owners, three loads, failures=0.
- A `global subsystems contents` line is added (USN04, 50221743b):
  - traffic: soldiers=3, vehicles=8;
  - panel: characters=29, default_pause=1.000;
  - weather: start_day_time=12, morning 4..8, evening 18..22, recon_modifiers=5,
    view_mode_amps=6, weather_amps=5.

  The installed Globals.lua has StartDayTimeDefault 12, MorningStart 4,
  MorningEnd 8, EveningStart 18 and EveningEnd 22.
- Four teardown lines are added, in 004D2BB0's order: +21D0, +21C4, +21E0, +21E4.
  They come from the mission frame host's destruction, after the summary.
- No summary line changes. In particular, `mission warning activate` (USN04: 9
  activations; USN01: 23) and the `warning table` line (messages=122 entity=36
  playerunit_section=5) are the same. The warning path now runs on the owner's state
  and tables, with the same contents and results.

The switch stays OFF in this tree. Turning it ON is the lead's call; the pairs
above are the evidence. With both this switch and `kMessageManagerCc15Bound` on, the
two owners are independent: the MessageManager reads the entity escape map
through `warning_tables()`, which is the owner's +11Ch then, as 0097B5E0 reads
manager+11Ch.
