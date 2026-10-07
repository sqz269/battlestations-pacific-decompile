# Initial land state entries — CC11

Packet `cc11_land_initial_state_entries` adds complete conditional ordinary
SOURCE control flow for the three initial entries reached by the existing
`construct_native_land_task_009b3240` caller. Its opt-in abstract calls adapter
dispatches the SAME borrowed task's actual matched state/profile identities to
those entries. All other constructor services remain abstract. This is source
reconstruction and fixture evidence; original profile/arena/provider binding,
binary ABI, exception handling, lifetimes and gameplay remain unproved.

The owned source paths are `include/bsp/native_land_state_entries.hpp` and
`src/native_land_state_entries.cpp`. Existing `GameUnitsHost` projections and
the outer constructor are unchanged. CMake inclusion and full integration build
belong to the primary agent.

## Native evidence and ABI

Primary recovery is recorded in
`reports/orchestrator_dispatch_cc11.json`, key
`land_state_entry_native_recovery`, on main `16fd96f5a`. Primary reviewed all four
complete bodies, matched disk/live bytes and the two constants, preserved/appended
names/comments, saved the project and refreshed exports. Worker analysis and
call verification are read-only in `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 LE32, image base `00400000`.

| Entry | End exclusive | Native ABI and coverage |
| --- | --- | --- |
| `009BED80` | `009BEE25` | ECX=Follow state, RET; preserves EBX/ESI and branch-saved EBP/EDI. Full ordinary entry caller. |
| `009B21A0` | `009B21BA` | ECX=Park state, RET. Full stores. |
| `007B3DB0` | `007B3DB1` | ECX=MoveTo state is unused; sole one-byte RET. |
| `009BE150` | `009BE28B` | ECX=existing parameter destination, stack=source, RET4. Complete reviewed dependency, REQUIRED external source provider. |

`009BE150` has forty ordered x87 FLD/FSTP binary32 lane copies: offsets `0..64h`
in four-byte steps, then bytes `68h/69h`, then offsets `6Ch..A0h` in four-byte
steps. It copies INTO the existing destination. It neither assigns state+6Ch
nor performs a raw struct/memory copy. This packet does not implement a
substitute for that required complete service.

`00CE3854` is verified binary32 `40400000` (3.0); `00D7A24C` is verified
`3F800000` (1.0). Their stable read-only values are admitted. Source literals
preserve the ordinary field values and store order, not the native constant
load addresses, SSE register state or fault timing.

## Follow entry order

1. `009BED84` calls actual singleton `0042E740`; obtain its actual +380h block.
   Read state+6Ch AFTER that call (`BED89`), and pass that EXISTING destination
   to COMPLETE `009BE150` at `BED92`. The entry never replaces +6Ch.
2. Reload +6Ch (`BED9A`) and capture state+4 (`BED9D`) before the field stores.
   Store positive zero to +94h then +90h. The primary fixed kernel loads from
   the actual CURRENT parameter+8 address with FLD and stores to actual
   state+88h with FSTP. Then clear +84h and store verified binary32 one to +8Ch.
3. Read the CAPTURED approach's +Ch after these stores (`BEDD1`). If NULL, skip
   all formation and observer work and proceed to the final +85h clear.
4. Otherwise store DWORD one to this squadron's +3E4h (`BEDDA`), then call
   COMPLETE `007ED260` (`BEDE4`). Detached rotation/index arrays alone cannot
   provide its actual plane+9D8h/+9D0h writes and their order.
5. AFTER reindex, freshly read state+4, then that approach's +Ch (`BEDE9/DEC`),
   then capture its +3D0h leader (`BEDEF`). The second +Ch has no NULL guard;
   it must name a valid live squadron in the admitted domain. Read old state+2Ch
   only after capturing this new leader (`BEDF5`).
6. If identities differ, unregister nonnull OLD leader with the SAME actual
   embedded state+18h callback owner (`BEE05`, `006952A0`). Store the CAPTURED
   new identity into +2Ch (`BEE0C`), then register it if nonnull (`BEE15`,
   `00694A60`). Do not re-read the leader after old unregister. Equal pointers
   skip this work; a null new leader still clears the old alias after unregister.
7. Clear byte +85h LAST (`BEE1C`) on every ordinary branch.

The actual existing observer register/unregister implementation is reused.
Required pure maps expose actual old/new observed endpoint prefixes and the
fully constructed embedded callback prefix; they do not create endpoints,
substitute a semantic owner, extend lifetime or synthesize callbacks.

## Park, MoveTo and connected adoption

Park writes positive zero at +1Ch, clears byte +18h, then writes verified
binary32 three at +28h. MoveTo performs the proved empty RET only.

`NativeLandTaskStateEntryConstructorCalls` borrows the SAME
`NativeLandTaskConstructorView` passed to the outer source constructor. Its
final `enter_state_04` admits only these actual member/profile pairs:

| Task member | Required executable table | Native profile+4 entry |
| --- | --- | --- |
| MoveTo +4C4h | `00D20AEC` | `007B3DB0` |
| Follow +500h | `00D20AB8` | `009BED80` |
| Park +620h | `00D1FF60` | `009B21A0` |

The adapter reads the actual state profile cell, checks the admitted pair, and
uses its complete source entry. Unsupported state/profile pairs report a source
logic error outside the admitted domain; there is no default/no-op entry.
The adapter supplies no actual profiles, owner/controller state, allocation or
base/composite implementation. `construct_base_0099c6f0`, faithful owner+50h
read, COMPLETE composite `009B2E50`, actual embedded-plane+72Ch virtual+38h and
actual fresh-plane leader probe remain required abstract services. The existing
outer source caller still publishes task+310h BEFORE entry and binds aliases
410h/414h/418h AFTER entry, preserving entry mutations.

## Identity, lifetime and normal-return domains

The outer task remains caller-owned stable nonnull storage. Its separate plane
3FCh cell and ONE canonical squadron 404h cell are retained; approach+Ch aliases
that same actual 404h storage for this task. State+4 may be freshly loaded after
the required service; any newly selected approach, squadron and leader must be
stable through the segment that uses them. No translated cached pointer is
published. Mappings from identity/address to borrowed fields are REQUIRED PURE:
no represented field reads, callbacks, native calls, allocation, default values
or hidden pointer caches. Field loads remain explicit in the source caller.

Parameter destination/source and amplitude address, state fields, executable
profiles, callback+18h, old and captured new observed endpoints, current
observer lock/dispatch/lifetime context must stay live. Observer arrays must
satisfy their existing actual storage/count/capacity domains. State+2Ch is the
alias beyond the callback-owner prefix, not a second ownership cache.
State+4, state+6Ch, singleton block+380h and every accessed field address are
required nonnull/valid. Only the explicitly guarded approach+Ch and leader
values admit NULL; none of these guards is a default storage provider.

The admitted numerical domain has masked FP exceptions, no pending unmasked
x87 exception, one free stack slot, valid binary32 addresses and invariant
verified constants. The fixed FLD/FSTP preserves ambient x87 control and its
native payload/quieting behavior; no float reinterpretation, normalization,
rounding policy or default tuning value is added. Native register ABI, unmasked
traps, faults, private EH/unwind, overflow/allocation failure, structural alias
invalidation, structural reentry, concurrent mutation and unsupported profiles
remain outside the ordinary source contract. A borrowed reference or observer
edge is not proof of cached task/plane/queue lifetime after death notification.

Follow uses cached approach+Ch, not current plane+9D4h, membership/census or a
current command getter. It does not read/drain the landing queue or deliver BE
messages. Queue+42Ch creation, record identity and deferred lifetime remain
the outer/composite's REQUIRED COMPLETE `006C0B50` dependency.

## Focused verification

MSVC x86 `19.51.36244.0`, `/EHsc /std:c++17 /MD /O2 /DNDEBUG /Iinclude`, compiled
the actual new source TU and the existing outer constructor TU. One unique
ignored probe (`local/cc11_land_state_entry_probe.cpp/.obj/.exe`) links those
objects with existing main Release core/Lua/zlib and Win32 libraries using
`/MANIFEST:EMBED /INCREMENTAL:NO /OPT:REF`. It passes a connected outer-to-Follow
scenario: same task/canonical cells, fresh post-base owner plane, ordered root
profiles, fresh plane after control probe, +310h before entry, existing parameter
destination read after singleton, reset/amplitude/formation ordering, fresh
state+4/+Ch after reindex, actual source observer pair replacement, final +85h
clear, aliases after entry, and x87 control/TOP preservation.

The same focused scenario then checks initial +Ch NULL skips reindex and leaves
the observer pair untouched, unchanged captured leader preserves the exact
existing pair/refcount, Park's positive-zero/byte/three stores, empty MoveTo and
the excluded source profile-mismatch guard. The distinct old/new branch and
the COFF captured-value spill/reload around unregister check captured-leader use;
there is no invented observer profile or artificial unregister callback to
pretend native reentrancy was observed.

Its abstract constructor, singleton/copy/reindex effects and source executable
tables are fixture-only. The copy provider models the tested amplitude effect;
it does NOT implement all forty native copies. Controlled state/parameter field
replacement tests source reload semantics, not observed native reentrancy or
actual service side effects. Existing observer prefix/register/unregister code
does execute on explicit fixture storage under its already-published lock.
The fixture proves neither native construction/profile/arena/runtime ABI nor
full required provider completeness. No new tracked tests were added.

The existing ignored hook/cruise/validity/canonical/outer fixture still passes.
COFF inspection confirms kernel FLD at +04/FSTP at +0A; Follow source singleton
call +13, destination read +1A/copy +21, reload +27/captured approach +2F,
zero stores +37/+40, amplitude call +4D, +84 clear +5A/+8C store +61, captured
approach+Ch read +6C, shape store +89/reindex +95, fresh approach +9B/+Ch +A7,
captured leader +B8/spill +BD, old unregister +F2/captured reload +F7,
captured alias store +104, new register
+127 and final +85 clear +135. These are compiled SOURCE offsets, not native
ABI proof. Five direct native call rows are checked live; the outer profile+4
entry row remains explicitly indirect. JSON parsing and staged diff checks pass.

## Remaining connected dependencies

The adapter advances the existing required outer state-entry call without
claiming an arena adapter. Actual 670h task arena/factory/scalar destruction,
base `0099C6F0`, composite `009B2E50`, lower `009F9CE0`/`009AFE70`, queue producer
`006C0B50`, full Follow constructor `009C2980` and callback profile/lifetime,
singleton block acquisition, forty-lane `009BE150`, complete live `007ED260`,
actual executable tables and faithful raw member mappings remain unbound.
The next packet should recover/adopt one of those complete construction/copy
services with its real storage and lifetime domain. Sparse `BotTaskHost`,
current registry/queue projections, fabricated profiles and Boolean task
presence cannot supply them.
