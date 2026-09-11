# Plain node terminal lifetime

Addresses: `00B6F8D0`, `00B6F440`, `00BD30E0`, `00B6F310`, `00B6E490`, `00D62C88`.

Packet `orch3_plain_node_lifetime_r`, 2026-09-11. Reconstructs the complete
plain-node scalar destructor and connects the existing physical node, scene
binding and actual reference count to terminal pool return. Descriptive names
are hypotheses, not recovered symbols. This adds no replacement node storage,
transform, reference count, scene registry, string pool or node pool.

## Scalar destructor and dispatch

`B6F8D0..B6F8EF` is 32 bytes. Native ECX is the physical node, its one stack
DWORD contains flags, EAX returns the captured original address, and RET4
starts at B6F8ED. It calls B6F440 unconditionally, then tests only flags bit 0.
When set, it returns the captured slot through B6E490 using the actual static
pool at 0108FF58. It performs no additional logical release or reference-count
decrement. There is no local exception handler and no raw return when direct
destruction fails.

The supplied current D62C88 table has virtual +00 = BD30E0 and +04 = B6F8D0.
The existing BD30E0 name is retained: it skips null and otherwise calls current
virtual +04 with flag 1, without decrementing references. The new companion
validates these identities and calls reconstructed C++ directly. Native integer
vtable words are never interpreted as executable host pointers.

The scalar source binding forgets the dead scene association after direct
destruction and before physical return. It also forgets that association after
the direct destructor's member unwind on a C++ exception; it rethrows without
returning storage. This is host association bookkeeping, not another native
instruction or a replacement ownership transaction. Direct callers must supply
a live, registered binding and remove any remaining host companions themselves.
Once the reference companion owns terminal dispatch, direct scalar calls are
outside its contract.

## Same string owner through destruction

The existing complete B6F440 body now has an overload accepting the same
`NativeStringStorage&` used to construct the node. Actual native pool users pass
`ActualNativeStringPoolStorage`, so name release uses the current 419CC0 owner,
BD1510 sized return, and the real shutdown gate. Canonical 41DD20 captures the
nonnull buffer and its size and leaves both name words unchanged. Existing
callers retain the runtime's `SizedStoragePool` adapter through the original
overload. No global allocator is replaced and no new pool is created here.

The shared `NativeNodeDestructionRuntime` still serves older GUI/light/model
callers and carries their semantic allocator member. The new plain-node path
passes its actual name storage explicitly and does not use that member. Moving
all remaining consumers to the actual storage interface is separate work.

B6F440 switches to D62C88 before callbacks. Its host projection now switches
virtual +34 to canonical B6E870 as well as the existing type, world-change,
attach and remove callbacks. This corrects a stale or unbound world-matrix
callback when a derived owner enters its plain-node destructor phase. Native
field order, current-child reloads, retained +130 cleanup, array/name/base
cleanup, and exception behavior otherwise remain in the existing shared body.

## Canonical reference companion

`NativePlainNodeReference` borrows the existing `NativeNodeBinding` and actual
node +04 atomic. Construction neither resets nor retains it. The supplied
storage, 0108FF58 pool, shared runtimes and current 22-word D62C88 table must
remain live. The scene companion must already be in the same scene runtime.
After successful registration in the existing `GeneratedModelLifetimeRuntime`,
the companion installs verified plain-node type/matrix/scene callbacks on that
same scene binding. It preserves its actual node identity and context.

Virtual +18 validates B6F310 and uses the existing shared logical-release
routine with live access to the actual +164/+168/+16C point-light array. It
does not copy that array or create new light ownership. Virtual +54 validates
B6EE10 and uses the existing recursive scene removal. Every dispatch validates
the current profile and relevant table entry; unsupported profiles have no
fallback implementation.

The zero-reference callback validates BD30E0/B6F8D0, performs scalar deletion
with flag 1, unregisters from the same lifetime runtime, marks its host state
retired, and invokes the required companion-retirement callback. That callback
removes the application's canonical raw-owner lookup and may destroy the
reference and node companions. Nothing accesses them afterward. This composes
with `release_native_render_actual_owner`, which consults the canonical lookup
only after the actual +04 decrement reaches zero.

## Verification and limits

The strict MSVC Win32 build and both existing CTests pass. One ignored fixture
verifies installed/live byte equality for B6F440 (297 bytes), B6F8D0 (32),
BD30E0 (14), BD30F0 (7), and the 88-byte D62C88 table. It executes all four
original function spans using explicit current-service rebindings listed in
`reports/native_plain_node_lifetime.json`.

For fresh unattached nodes, original and reconstructed destruction compare all
178 slot bytes, normalizing only the differing name pointer. Flags 2 and 1
exercise retained versus returned storage; names of lengths 7 and 149 exercise
small-ring versus ordinary-free cleanup. Pool free counts, balanced depth,
base profile, string-ring state and the bound destructor-phase matrix setter
are checked. Original virtual-zero dispatch also executes through the copied
scalar body, with only its table pointer rebound to callable fixture addresses.

The actual reference path checks creator count 1, retain to 2, canonical B6DFA0
and logical virtual +18 to 1, then actual zero-only owner lookup and terminal
return. The retirement callback reacquires the same slot and deletes the
reference, checking that it has already left the lifetime map. The bound
world-matrix setter writes the same physical node matrix. A reconstructed C++
missing-retained-owner exception verifies member cleanup but no scalar pool
return. The old shared runtime string allocator remains untouched throughout.

Original attachment/parent/root/retained-owner nonnull branches and original
exception dispatch were not executed by this fresh-node comparison. The copied
exception-handler immediate remains original. Existing scene/hierarchy and
logical-release bodies retain their own evidence and required bindings. Type
checking uses the existing LightTypeBootstrap predicate over explicit fixture
descriptor inputs; global type bootstrap is not established by this fixture.
There are no new permanent tests, native exception-ABI or gameplay claims.

## Follow-up packet

Compose the point-effect constructor's existing prefix, physical node allocation,
stable node/reference binding, registration, matrix and row stages with its
manager insertion and complete member/template unwind. Use the current packet
leases: another orchestrator is working on effect-manager dependencies. Keep
actual row virtual +18 factories and canonical raw-owner lookup/retirement
explicit until their production providers are connected.
