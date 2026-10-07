# JM06 class-id 8 torpedo-director pair (CC11)

Both pinned builds and both JM06 host runs completed successfully. The primary
integrator reviewed the changed torpedo masks, ranges, and candidate gates and
approved enabling only `kShipDirectorClass8Bound`; the tracked default is now
`true`. The preserved OFF/ON captures still represent the original exact pin.

The candidate binding is documented in [TORPEDO_DIRECTOR_CLASS8_CC11.md](TORPEDO_DIRECTOR_CLASS8_CC11.md).
At `0082394A`, shared initializer `00822C20` invokes the receiver's vtable
slot `+5Ch` with query `8` only after finding an explicit false
`TorpedoDirector`. The submarine object resolves that slot to
`00853050`; the native predicate accepts query 8. An absent property already
takes the true branch and does not require this exception.

The host candidate uses the resolved bag `Type` integer, installed
`VehicleClass[Type].Type`, the existing actual leaf descriptor, and the
verified kind helper. It retains explicit false for unknown descriptors.
Scene labels and group names do not establish class membership. The existing
group merge followed by authored property writes remains the property order.

Both fresh exports are pinned to
`86d596624d90d49fed79d34cf8ef393b7ee92dd7`:

| Variant | Export | Switch |
| --- | --- | --- |
| OFF | `local/cc11_director_jm06_off` | `false` |
| ON | `local/cc11_director_jm06_on` | `true` |

All 11,335 exported source files were compared by SHA-256. The only byte
difference is the exact switch declaration in
`include/bsp/game_hosts_scene_contents.hpp`; both manifests record the same
commit and sole requested flip. The original tracked header matches OFF.
Full per-file receipts remain under the ignored pair directory.

The installed input root is
`I:/SteamLibrary/steamapps/common/Battlestations Pacific`. The pair hashes
34 mission, group, enum, and autoload table files and rechecks each before
launch. It uses
`universe/scenes/missions/COTP-IJN/PRCPIJN/ijn_06_prelude_to_midway.scn`
and `scripts/missions/COTP-IJN/PRCPIJN/jm06.lua`.

Installed `ship.props` lines 2-9 define `Ship(Common)` with explicit
`TorpedoDirector=false`; lines 199-203 define `Sub(Ship)` and retain that
value. Enum IDs 30/31/93 resolve through `vehicleclasses.lua` lines
18468/18768/43772 to actual leaf type `Submarine`. The group provides the
raw property; the resolved descriptor provides class admission.

| Installed submarine rows | Resolved type | Scene admission / Lua setup |
| --- | --- | --- |
| Narwhal-class Submarine 01 | Narwhal, id 31 | TorpedoEnable true at Lua 204 |
| Gato-class Submarine 01 | Gato, id 30 | Enabled at difficulty 2; killed otherwise, Lua 208-213 |
| PlayerSub 01, 02, 03 | TypeB_Jake, id 93 | Enabled by loop at Lua 220-227 |
| Submarine TypeB w Jake 04-09 | TypeB_Jake, id 93 | Authored Hidden true; enabling loops are commented |
| Submarine TypeB w Jake 01-03 | TypeB_Jake, id 93 | Authored GenerateInGame false; actually admitted in this mode-9 host run |

These 14 asset rows are an input catalogue. Both logs confirm effective Lua
difficulty 1 and effective scene mode 9 (`raw=0 forced=0 multiplayer=0`). Eight
submarine unit rows were actually created: Narwhal, Gato, PlayerSub 01-03,
and the three tutorial TypeB rows. Gato was killed at time 0 by the difficulty
branch, leaving seven active submarine rows. Hidden TypeB 04-09 were deferred.
The overall scene summary is identical: 96 registered, 96 instantiated,
76 generated, 20 rejected, 26 created, and 20 Hidden rows held back. The actual
host admission is recorded; normal original-game JM06 admission is not proved.

Common launch schedule: 3,000 mission frames at 0.05 seconds, outer frame
limit 3,200, press-start frame 30, mission JM06, jitter 0 percent / seed 1,
present interval immediate, and no custom orders. Gunnery RNG streams are
enabled with their fixed seed `0x9E3779B9`; script-orders and AI seeds are
`0x13579BDF` and `0x2545F491`. Both sequential launches claim slot 2/core 6
through an ignored copy of the existing launcher. Its two changes restrict
the slot search and preserve the repository config path; source exports are
unchanged.

Both fresh Win32 builds exited 0 and passed the two configured tests,
`reconstructed_math` and `tool_tests`. Each post-build audit rechecked all
11,335 source files without a source mutation. The native differential test is
not configured in these fresh git-archive exports. Both executables are PE x86,
5,032,448 bytes; hashes, build logs, and DLL receipts are in the report.
Both runs exited 0, released slot 2, and reached requested/ran/simulated mission
frame 3000 with `t=150.00`. Runtime handles were OFF session 60374 and ON session
5703; both had finished before PID sampling and were not restarted.

| Existing diagnostic | OFF | ON |
| --- | ---: | ---: |
| Class8 property override rows | 0 | 14 |
| Lua torpedo enables / changed writes | 4 / 4 | 4 / 0 |
| Tutorial TypeB 01-03 final torpedo masks, each | 0 | 3 |
| Tutorial TypeB 01-03 scored / masked / accepted, each | 56 / 56 / 0 | 56 / 0 / 0 |
| Tutorial TypeB 01-03 unit-table range, each | 1600 | 1852 |
| Torpedo candidate scored / accepted | 335 / 96 | 335 / 96 |
| Torpedo rejects: mask / range | 168 / 71 | 0 / 239 |
| Director-disabled pushes | 1036 | 814 |
| Gunnery candidate native score calls | 5205 | 5373 |

JM06 enables Narwhal and PlayerSub 01-03 in Lua. Those four writes explain the
changed-write difference and matching later state. PlayerSub 01/03/02 retain
final torpedo mask 3 in both runs, with accepted candidates 0/51/45. Narwhal
does not produce a scored-candidate mask row. The three tutorial submarines
receive no Lua enable, so their final masks and candidate gate changes supply
an unmasked host-gunnery witness of the class8 exception. None of their
candidates passes the range check in either run.

The existing `pair_diff` correctly returns **3**: three unit ranges and the
torpedo-mask/candidate fields changed. It reports 21/21 unit-table rows with
no additions or removals, nine changed summary lines, and one non-noise native
count change. Free-bearing empty/refill counters also move 37906/106 to
36779/108. Headline shots 233, hit records 149, hull hits 140, damage 3587.2,
deaths 2, and first hit 10.10 seconds remain equal; clock offset and first-hit
delta are both zero. Equality of those headlines does not establish full
gameplay identity. Death-row tables were not enabled by `BSP_DEATH_TABLE`.

The primary review accepted these bounded changes as consequences of the
recovered native class8 exception and authorized only its default promotion.
The source comment now makes clear that property projection occurs before
generation/Hidden gates, so 14 override logs do not mean 14 live units. Unknown
descriptor kinds still retain false. Sparse shipyard and absent-property
behavior is preserved by the existing source contract; this pair specifically
exercises explicit false. Original native predicate receipts remain in the
class8 evidence; host runtime, fixture checks, and recovered ABI are separate.
This is not an original-game or native ABI replacement validation.

Exact manifests, SHA-256 inputs, artifact/log hashes, unit rows, native call
receipts, and comparison fields are recorded in
`reports/torpedo_director_jm06_pair_cc11.json`. Large source and runtime captures
remain ignored under `local/cc11_director_jm06_pair`.
Live `verify_report_calls` passed eight direct class8 rows and one direct pair
row with zero failures; three indirect receipts were explicitly excluded.
`git diff --check` passed. The existing `C:/Users/sqz269/bsp.gpr` target and live
`/battlestationspacific.exe` program were verified; this packet made no Ghidra
or ledger mutation.
