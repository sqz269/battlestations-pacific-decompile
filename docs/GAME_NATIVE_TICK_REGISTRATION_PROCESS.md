# Canonical process tick registration lists

`GameNativeTickRegistrationProcess` supplies one permanent Source authority for
the Original mutable groups at `00F876C0`, pending head/tail at
`00E0B6D0`/`00E0B704`, and run-pass byte at `00E0B6CE`. It is an independently
compiled prerequisite. **Normal fixed-step attachment remains false.** It
creates no unit, descriptor, callable unit table, tick callback or frame pool.

## Original initialization and process identity

The verified installed PE and current GPR agree on both retained byte spans:

| Data | Original image state | Source owner |
| --- | --- | --- |
| `F876C0..F878C7` | 208h zero bytes, five groups at stride 68h | One aligned 208h backing, initially zero |
| `E0B6D0..E0B737` | 68h bytes; head+8 = E0B704, tail+4 = E0B6D0; all other bytes zero | One aligned 68h backing with the same two pointer roles |
| `E0B6CE` | Byte 1 | One mutable byte, initially 1 |

The Original PE has no base-relocation directory. The two pending-list words
are absolute image pointers, not newly proved HIGHLOW relocation records.
Source initializes the corresponding pointers to its permanent backing; it
does not use the Original numeric addresses as Source pointers.

`initialize_groups_crt_once()` calls the already accepted borrowed
`00CD27C0..00CD2858` twenty-write initializer exactly once. Accessing the
process facade does not run that initializer. Repeated calls return false and
preserve live links. This is a Source admission guard under quiescent startup;
it does not recover an Original CRT symbol, register ABI or synchronization
policy. The original fragment's 1E0h direct-access bound is unchanged; the full
208h backing separately derives from the five-group consumers and image bytes.

There was no existing mutable-data owner for these addresses in the inspected
Source. The new facade follows the existing intentionally retained
`GameNativeStringProcess` lifetime. It borrows that SAME process's `F878CC`
pending-registry publication and raw `01090AA0` manager cell. It adds no manager,
registry publication or lifetime dispatcher. The accepted raw registry getter,
section lock and deletion resolver remain responsible for that registry owner.

## Real producer entrypoints and run-pass writers

The facade routes `construct_registration_00875890` to the complete existing
raw constructor with the actual 34h node, actual opaque payload, unchanged raw
group word, canonical cells, permanent pending tail and required current timer
word. It neither chooses a group nor owns the receiver. The constructor's
original unchecked group semantics, link rereads, timer capture and failure
behavior remain in the existing body.

`register_parent_constructed_unit_0087b699` routes the qualified existing
`0087B699..0087B728` fragment through those same identities. Its caller must
have completed `0077EED0`, provide actual receiver backing through +38Bh and
the required timer/initial words, and retain the real parent lifetime. It does
not finish `0087B670` or install callable Source replacements for its opaque
numeric profiles. Semantic `GameUnitSlot` is not such a receiver.

The mutable accessor `run_pass_00e0b6ce()` returns the SAME byte referenced by
`Lists::run_pass_00e0b6ce` and therefore by the bound raw waves. For real writer
attachment, the command-line arm at `004E354F` must write zero to this accessor,
and `DebugSwitchBytes::job_pool_run_pass_00e0b6ce` must point to this accessor.
The existing `lua_set_single_thread` performs the Original `!arg0` store from
`008A199D`. The facade does not parse command-line options, invoke Lua or copy
another flag into this byte. Those writer hooks remain explicit. The driver
does not own a different frame-counter-derived run-pass flag.

## Required next parent attachment

Register this translation unit in `bsp_game`, alongside the existing canonical
string process. At genuine startup, invoke explicit group initialization once
at the recovered CRT ordering boundary. Initialize/borrow the existing entity
registry process rather than creating different ID tables or pending-init lists.

The next receiver work remains a specialist readiness/closure packet for
`00925CE0 -> 00928630 -> 0077EED0 -> 0087B670`. Existing raw observer/string,
entity-registry/ID services and qualified tick-registration fragment can be
composed only after actual storage preimage, world publication, callable virtual
slots, list/peer-holder and iterator callback pairs, exceptional ownership and
root destruction are established. `GameNativeGameRuntime` currently supplies
the 71A0h game/physics owner, not this unit receiver chain. Existing semantic
construction functions do not close that gap. This packet recovers no new
register-ABI, x87 or large parent body.

When that parent supplies genuine raw receivers and complete current
`NativeFixedStepTickBindings` plus the application's real remaining frame-job
dispatcher, it may obtain `lists()`, pass its exact four identities to
`GameNativeFrameJobApplication::bind_actual_fixed_step_groups`, and attach the
returned waves to `GameFixedStepHost`. Keep receivers and process backing live
through pending splice, caller dispatch and worker waits. Preserve the existing
application retention check and canonical manager drain before graph retirement.
No normal attachment call was added here.

## Validation and limits

Strict Win32 MSVC `/std:c++20 /EHsc /O2 /MD /W4 /WX` compilation passed.
`scripts/build.ps1` passed Release and both configured existing suites at base
`88a3bcd1d`; that build includes the accepted application TU but predates this
new TU's pending primary-owned CMake registration. The focused probe explicitly
compiled this TU and the existing executable-layer string process and linked
the fresh matching 200-byte deletion-binding core.

The inherited controlled-bootstrap normal/failure child probe uses the Original
read-only metadata and real raw manager, registry getter, registration body,
splice, frame pool, Win32 worker and raw deletion path. It verifies canonical
cell aliases, initial links/run-pass, explicit initialization refusal, repeated
initialization preserving populated pending/group links, actual worker execution
and interruption retention. Raw fixture receivers and instrumented downstream
tick/remaining bodies are fixture inputs, not production units or jobs. The
child's two-processor setting forces worker participation only in the probe.

Immutable local receipts and their hashes are listed in the companion report.
This is Source/build/fixture evidence. Original ABI/EH/fault equivalence, native
differential coverage, normal game attachment, startup and gameplay remain
unproved. No CMake/shared host or existing frame metadata/random/deletion
lifetime implementation was changed by this packet.
