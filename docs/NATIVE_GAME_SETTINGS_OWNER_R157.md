# Native game settings lifecycle — R157

The native player-profile constructor/reset requires the raw settings object
at F88980. `GameSettingsBlock` is a C++ projection containing host containers;
it cannot be passed to that constructor. This packet recovers the raw BCh
settings lifecycle needed before connecting the native profile to startup.
The existing projected settings path remains separate.

## Recovered bodies

| Entry | Native contract | Source behavior |
| --- | --- | --- |
| 8D7710–8D78CD | ECX owner, EAX same, RET | Exact partial byte/DWORD stores; native clan-string construction; current online-manager state and selected-user import |
| 8D4950–8D49F3 | No arguments, RET | Release current input settings through the captured raw-manager lock, unregister, reload, scalar delete, clear publication |
| 8D78D0–8D797A | ECX owner, RET | Input release, clan string, A4 vector then98 vector; free backing after each resize0 |
| 8D7980–8D799D | ECX owner, stack flags, EAX same, RET4 | Full destruction then BF65AC free when bit0 is set |
| CD2D80–CD2D95 | No arguments, RET | Construct static F88980 then register CDEEC0 through atexit |
| CDEEC0–CDEEC9 | No arguments, tail jump | Set ECX to F88980 and jump to the complete destructor |

These are explicit C++ service interfaces, not drop-in binary replacements.
Names are descriptive hypotheses. The existing constructor/static-initializer
names are retained; their earlier projection is not treated as raw storage.
The caller supplies static storage, contexts and a stable shutdown thunk; this
packet does not install another application-wide settings owner.

The original static storage has BCh loader-zero bytes. General construction
preserves bytes outside the observed store set. Constants CE3800 and CE7D20
are captured at the original MOVSS load points and copied as bits. The online
manager is reloaded for the selected-user query. Successful selected-user
import uses the existing complete 8D45D0 source body and SDK boundary.

8D4950 preserves the first manager's critical section while reloading the
current publication for unregister and deletion. Source scalar dispatch handles
the existing CF81CC input-settings and CE3818 base profiles; unknown profiles
raise a contract error. It uses the existing real raw-manager and input-lifetime
services, without a replacement input-settings object.

Ghidra originally ended 8D78D0 at 8D7944 after BF6989. Disk instructions prove
the second vector cleanup and RET through 8D797A. Locked flow repair plus an
explicit function-body repair restored the full 171-byte extent, preserving
prior function metadata. The scalar helper also had a three-byte stack-fixup
gap after BF65AC. Both repairs, prior values, exports and readback are archived.
All 1,048 live/PE bytes across twelve body/data spans were compared. The six
bodies contain 843 bytes and nineteen direct call/tail-jump edges.

## Validation

- Strict MSVC Win32 build and the three existing CTests passed.
- Thirteen copied-original/source cases matched 3,076 observed bytes. Full
  BCh owner snapshots include opaque bytes; three allocation-pointer fields
  are normalized to null/non-null. Boundary traces distinguish A4/98 backing
  frees and capture both live vector counts, flags and manager receivers.
- Cases cover null manager, wrong state, no selection, selected-user success
  and failure, manager replacement between state/selection calls, empty and
  populated cleanup, CE3818 and CF81CC input deletion, high flag bits and
  static registration/shutdown order. Profile SDK and backing-free callbacks
  are controlled boundaries. Original FH3 paths are not executed.
- Actual native string pools, raw singleton registration/locking and existing
  input scalar cleanup execute in both lanes. CF81CC input members are built
  by the existing constructor prefix; this is not a new full table-loader test.
  The raw string-pool publication clears during final manager drain.
- One source second-backing-free failure retains the operation and rejects
  replay; explicit diagnostic cleanup releases the remaining backing storage.
- A separate real `std::atexit` registration executes the complete source
  static shutdown at process exit. Its borrowed fixture contexts remain alive.

`reports/native_game_settings_owner_r157.json` records evidence and hashes.
Local fixture inputs, Ghidra repair records and binaries are sealed under
`local/evidence-r157/`. No permanent test cases or test framework were added.

## Remaining work

The application's settings loader, native profile contexts and process owner
still need to share this raw storage. The ordinary application continues to
use `GameSettingsBlock`; native game admission is not enabled here. Native
FH3/SEH behavior, arbitrary virtual profiles, whole-game ABI and gameplay
validation remain open.
