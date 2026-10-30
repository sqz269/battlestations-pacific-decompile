# CC16: concrete C++ cleanup design for raw Lua variant retirement

This is a design for adding the two actual name/key cleanup actions to qualified
Source C++ execution. It does not implement them. The normal-only candidate
`17b77e69a79d82a5b89841edb23c2046cfe86cd1` remains immutable and independently
reviewable. Its catches retain metadata and rethrow; they do **not** perform
C970D0 or C830A0. Full host, payload, queue, vector, scalar and producer adoption
remain held. No Native FH3/SEH or secondary-failure terminal equivalence is claimed.

The smallest useful next Source packet is additive qualified entry points plus
bounded factoring of `native_lua_variant_retirement.hpp/.cpp`. The existing
normal-only signatures, metadata layouts and legacy Host implementations can
remain unchanged. A wrapper around the three current public functions is
insufficient: the internal recursive edges must select the qualified bodies so
that cleanup runs in actual C++ call-stack order.

## Frozen evidence and exact scopes

The frozen input is `reports/cc16_native_lua_variant_retirement.json` at 17b77,
including its explicit 83a63 readiness input. This design rechecks the installed
PE spans and exact Source blobs without a live Ghidra batch. Cached assembly
exports corroborate the normal arm/demotion epochs. The report records hashes
and distinguishes inherited compile receipts from checks performed here.

| Scope | Arm | Normal demotion | Actual exception action |
| --- | --- | --- | --- |
| 00884AF0..00884B54, 101 bytes | Saved V; numerical D0E6F4 store; state0 at 884B13 before clear at 884B1B | Capture CURRENT V+10h at 884B20; state -1 at 884B25, before optional length/getter/return | C970D0 loads saved V from the FH-adjusted frame, adds Ch, tail-jumps 41DD20 |
| 006EE890..006EE925, 150 bytes | After right return, capture current LEFT; K=N+Ch; rewrite incoming argument-home VALUE to K at 6EE8C7; state0 at 6EE8CE before value destructor | Capture CURRENT K+8 at 6EE8DB; state -1 at 6EE8E0, before optional length/getter/return | C830A0 loads the overwritten argument-home VALUE from the FH-adjusted frame and tail-jumps 6EE020 |

DC9464 and DB10B8 each contain one unwind-map entry: state0 -> -1, with
C970D0 and C830A0 respectively. Their FuncInfo records are DC946C/DB10C0;
handlers are C970DB/C830A8. The saved native frame offsets identify evidence,
not Source stack locations. The qualified Source implementation must demote its
local cleanup eligibility **before** invoking an exceptional action.

C970D0..C970DA is `mov ecx,[ebp-10h]; add ecx,Ch; jmp 41DD20`.
C830A0..C830A7 is `mov ecx,[ebp+4]; jmp 6EE020`. Neither action frees the
variant, node, map or descriptor, clears a header, restores links, or resumes a
normal body. D0E6F4 is identity data, never callable Source code.

## Saved homes, current fields, concrete providers

Name cleanup retains the entry receiver address V and derives V+Ch. It reads the
**current** buffer at V+10h only when the exceptional action is entered, then the
current length at V+Ch only if that buffer is nonnull. It must not reuse a name
buffer captured by a failed normal operation or infer V from a changed payload.

Key cleanup retains the K VALUE written into the argument home before the value
destructor. K is the original current node+Ch at that epoch. It reads CURRENT
K+8, then conditionally CURRENT K+4. It must not recompute K from a changed map
head, a later node, a saved LEFT link, or a failed value's buffer metadata. Right
recursion occurs while this caller's key cleanup is unarmed, including later
iterations whose old argument-home value may still be a key.

The complete existing raw action bodies are already Source:

- `destroy_native_string_header_0041dd20(void*, NativeStringRawPoolContext&)`
  in `native_string.hpp/.cpp`, for the name header. Its semantic storage overload
  has a different noexcept contract and is not the selected provider.
- `return_native_lua_variant_string_fields_006ee020` in
  `native_lua_variant_string_fields_cleanup.hpp/.cpp`, for K+4/K+8.

Both capture the buffer before the null test, skip the length and services for
null, otherwise capture the late length once, wrap length+1 as uint32, resolve
419CC0, then return the captured buffer/size through BD1510. They do not clear
fields. The native PUSH1 is an unused physical word, not a new Source argument.

Use the SAME application's actual AA8 pool publication, AA0 raw-manager
publication, and AA4 return gate from `NativeStringRawPoolContext`. Keep the real
pool deletion binding and manager/critical-section/storage lifetimes through
last use. Each nonnull action must still call the real getter, even for large
returns or a disabled small-return gate. Do not cache a previous pool or replace
actual publication cells. Existing BD1510 and singleton free noexcept/CRT
boundaries remain unchanged. These actions add no object/node/map free.

## Proposed bounded implementation surface

After the immutable 17b77 review and root decision, lease only:

- `include/bsp/native_lua_variant_retirement.hpp`
- `src/native_lua_variant_retirement.cpp`
- `docs/CC16_NATIVE_LUA_VARIANT_CPP_CLEANUP.md` (new implementation receipt)
- `reports/cc16_native_lua_variant_cpp_cleanup.json` (new implementation receipt)

No helper/provider Source edit is required for the selected plan. Declare three
additive ordinary C++ entry points, provisionally the current clear/destruct/
subtree names with `_cpp_cleanup` suffixes. They accept the same genuine raw
objects and pool context, the existing operation, and explicit passive cleanup
backing. These are qualified Source interfaces, with no Original ABI credit.

Factor the complete current cycle into private compile-time-selected kernels.
Normal public entries select retain/rethrow only; qualified entries select the
local C++ cleanup behavior. All four internal call edges select their caller's
mode: clear -> subtree, subtree -> right subtree, subtree -> value destructor,
and destructor -> clear. Do not call normal-only internals and try to repair
the missing actions at an outer wrapper. Do not implement a broad Host adapter.
The old broad-host files remain byte-for-byte Source inputs.

For failure custody, locally compose the two existing raw action read/service
contracts into small private action kernels in the retirement implementation.
Capture the action buffer/length/size/pool in the passive action record at their
actual read/call epochs, and use the same real getter/return definitions already
used by the normal cycle. Existing `return_buffer` may be factored with a passive
record type; it must introduce no callback or provider abstraction. This is
necessary contract composition, with zero new Original/vendor/duplicate credit.
Blindly calling the existing action helper and then inspecting the header after
it throws cannot recover a buffer captured before its callback; a pre-read plus
helper reread is also unnecessary. Do not modify those existing public helpers
or count their already-present bodies again.

## Local handlers and passive backing

Each qualified lexical scope holds its own trivial armed flag and saved home,
plus references to its own existing frame and cleanup sidecar. The local flag
follows the native arm/demotion schedule; retained progress records do not drive
execution. On a genuine escaping C++ exception:

1. Retain the existing incoming failure metadata. If this local action is
   unarmed, rethrow without performing it.
2. If armed, demote before action entry; record that the action was entered,
   and run the actual name or key action against the saved home/current fields.
3. If that action returns normally, record action completion and bare-rethrow
   the incoming exception. Do not mark the body completed or resume its suffix.
4. If the action itself throws, record its distinct failure stage and rethrow
   that escaping C++ exception without retrying this action. This is the Source
   C++ transport limit described below, not an Original terminal policy.

These are explicit local handlers, not a NativeFH3 interpreter or a walk of
recorded progress. Clear and qualified whole-range composition have no added
raw cleanup action. No action frees a node or detached tail on the way out.

A metadata/domain non-completion is an ordinary returned status, outside the
valid/sufficient-backing domain. It must propagate through every nested caller
before later reads/free/reset/normal return effects. It must **not** trigger an
exception cleanup, force a native demotion, or become a fabricated exception.
The retained native-state epoch may therefore remain0 on such a Source stop.

Keep the current Frame/Operation layout and normal signatures intact. Add a
separate caller-owned, trivially destructible sidecar array indexed by the same
frame slot. Pre-provide live typed/disjoint backing sufficient for the actual
maximum active Source call chain, including nested maps, for both spans. No
count-derived size, fixed maximum, object/address history, allocation, logging,
virtual adapter, callback or fallible capture is an admission substitute.
Prepare each sidecar at its corresponding entry; prepare range metadata before
88693F. The endpoint-to-predicates/root interval must still contain no CALL.

A sidecar records scope/action identity, saved receiver or key-home VALUE,
original throw site/state, arm/demotion/action-entry, incoming C++ observation,
cleanup buffer/late length/wrapped size, getter entry/captured pool, return
entry/normal return, and action completion or secondary C++ escape. The original
failure captures must not be overwritten by action captures. Sidecars are
passive records of the latest current scope, not a journal or retry program.

After failure, active_depth intentionally retains inner frames. A local handler
must use its saved frame/sidecar references, **not** frames[active_depth-1],
metadata replay, or speculative depth popping. Action completion does not pop
retained failure frames or imply node/map/object retirement. External callbacks
that reenter use independent operation/backing and cannot mutate active records.
Actual homes, current/captured buffers, node/LEFT/map, pool/context/provider
identities and all metadata remain independently owned through final disposition.
A returned buffer can leave a stale header: automatic retry risks double return.

## Qualified C++ domain and secondary-failure limits

The intended claim is actual action effects for a genuine C++ exception that
crosses these Source scopes, **when each invoked cleanup action returns
normally**. Bare rethrow then preserves that incoming exception. A concrete
sufficient pool subset has the genuine AA8 publication nonnull at every cleanup
getter, the corresponding owner/section/ring/backing live and valid, and the
existing BD1510/CRT/OS operations returning normally. Getter fast-path Source
then introduces no allocation or manager registration callback. This is a real
custody precondition, not a new cached-pool path, precheck or fallback. Lazy
creation may also return normally, but cannot supply a no-secondary-failure
promise: raw manager lookup, allocation/construction or registration can fail.

Normal C++ exception-only scope guards could run these real actions in the
returning-cleanup domain if armed/demoted at the exact epochs and disabled for
ordinary metadata returns. They are not needed for the selected implementation.
A destructor that is implicitly noexcept, or noexcept(false) while another
exception is unwinding, can terminate when cleanup throws. That language rule
establishes no Original terminal equivalence and obstructs retained-custody
handling. Do not add such an operation-abandonment or failure-cleanup destructor.

Explicit local handlers avoid imposing a new destructor termination boundary.
If a real getter throws while an incoming exception is handled, the escaping
cleanup exception can reach outer C++ handlers. Those handlers may apply their
own still-armed local action to their own saved home; this does not retry the
failed inner action. The incoming exception is not thereby promised to reach the
root or remain retained as an exception object. Without a separately reviewed
exception-retention contract, passive flags are not primary/secondary exception
identity storage. No nested_exception wrapping, replacement exception, catch-
and-success, fallback allocator, forced termination, or guessed native policy
is proposed. Original secondary-failure behavior remains unclosed.

An existing noexcept/CRT termination, Win32 SEH, access fault, longjmp, panic,
process termination or other nonlocal transfer need not cross these /EHsc C++
handlers. Metadata may remain active/partial and no disposition is inferred.
This packet cannot claim all failures are C++ exceptions or that a successful
normal cleanup implies pool-manager lifetime or host adoption is authenticated.

## Exception schedule examples and immutable review gate

- Clear throws while a value destructor is state0: clean its CURRENT name at
  saved V+Ch, rethrow; then the caller's armed K cleanup runs. Do not free N,
  inspect LEFT, reset map heads/count, or free the map/descriptor afterward.
- Normal name getter throws after its demotion: do not return that name again;
  an armed caller key scope can still run its own key action.
- Normal key getter throws after its demotion: do not return that key again or
  free N. An ancestor paused in right recursion is unarmed for its own key.
- A nested action returns its buffer normally: retain that completion separately
  from the still-failed body; enclosing native-state epochs determine any next
  local action, not tag/count/address history or a progress replay.

Before Source admission, pin the new immutable implementation commit and all
inputs. Re-run the required strict Win32 compile/COFF/native-PE checks and root
common admission after authorization. Compare old normal public/support code,
EH data and relocations against 17b77; any emitted drift needs explicit review
and fresh receipts, even if it follows from factoring. Do not inherit unchanged
object or common-build claims from this design.

Inspect every new exceptional edge, saved-home/current-field read and demotion,
all four normal nested-status exits, all four post-callback current-head reads
and reset/output ordering. Recheck zero CALLs from 88693F endpoint capture
through whole-range predicates/root capture and absence of new allocation or
callback imports there. Verify normal mode cannot reach an action and qualified
mode selects qualified recursion throughout. Require genuine pool/free imports,
trivial metadata, no action on Source rejection, no action after normal demotion,
no post-failure normal suffix, and no extra object/node/map free. Static evidence
still does not prove runtime exception delivery, Native ABI/FH3/SEH, secondary
terminal equivalence or gameplay. This design performed no C++ edit/build/test,
link, body execution, probe, DLL/game run, Ghidra mutation or shared metadata edit.
