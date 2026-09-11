# Actual 88h material-pass lifetime

Addresses: 00B44B10, 00B454E0, 00B42250, 00B46910, 00B172B0, 00B172C0.

`NativeMaterialPassStorage` reconstructs the complete 88h payload in the
actual 0108FBF8 pool's 8Ch slot. It composes the established 5Ch pass base,
actual state owners, shared retained-owner domain and actual string storage.
The complete constructor/destructor now accompany the older fallback-only
fragments. Names remain descriptive hypotheses; these are new MSVC Win32 C++
interfaces, not drop-in original ABI replacements.

## Storage and ABI

| Actual field | Offset / size |
| --- | --- |
| Established pass base, including actual atomic +04 | 00 / 5Ch |
| Four owned binding pointers | 5C / 10h |
| Unsigned live binding count | 6C / 4 |
| Retained vertex / pixel shader | 70 / 74 |
| Two signed indices, meaning not resolved here | 78 / 7C |
| Byte / preserved padding | 80 / 81..83 |
| Owned fallback texture | 84 / 4 |
| Slot's slab ID, outside this payload | 88 / 4 |

The actual binding object is 10h: uninterpreted word00, retained owner04 and
the actual eight-byte string header08. Binding destruction does not free the
object itself. The full pass's numeric table D61BE8 begins with BD30E0 and
scalar deleting destructor B46910; the remaining entries are B454D0, B5F6A0
and B44BF0. Their other pass methods remain outside this packet.

| Complete span, end exclusive | Original ABI | Reconstructed function |
| --- | --- | --- |
| B44B10..B44BE6 | ECX fresh pass; EAX same pass; RET | Constructor |
| B454E0..B455BF | ECX actual pass; RET; no stable result promised | Destructor |
| B42250..B422D0 | ECX actual binding; RET | Binding destruction |
| B46910..B46930 | ECX actual pass, stack flags; EAX original pass; RET 4 | Scalar deleting wrapper |
| B172B0..B172BA | ECX pass root, stack effect identity; EAX that identity; RET 4 | Borrowed effect setter |
| B172C0..B172C4 | ECX pass root; EAX borrowed effect; RET | Borrowed effect getter |

## Construction and cleanup order

B44B10 invokes the complete B5F720 base constructor, publishes D61BE8,
clears binding count, shader pointers, byte80 and fallback84, and writes -1
to both indices. **The four binding pointers and padding81..83 are unwritten.**
It initializes an actual temporary string, resizes to nine characters and
copies `white.tga` including its terminator. Only then does it read the current
F8D394 renderer, invoke its callable virtual+64 with the temporary header and
flags0, and directly publish the returned owned identity at84. There is no
additional retain or release of that returned identity. The temporary is
released after the store. A null returned owner is preserved.

The host constructor explicitly registers the base's three already-created
state owners before the first throwing string/renderer operation. This callback
creates canonical companion metadata in the caller's existing owner domain;
it must not retain, replace or copy the native owners, and must not throw.
It supplies the same terminal bindings needed by normal cleanup and by
constructor unwind. There is no private child registry or second count in
the pass implementation. `resolve_actual` remains a lookup without ownership
side effects.

Constructor FuncInfo DF7D24 has two unwind states: CBF3C0 calls B5F510 on
the actual base; CBF3C8 calls 0041DD20 on the stack temporary. The temporary's
state is armed after resize/copy and before renderer acquisition. Failure
therefore cleans the armed temporary followed by the complete base. The
constructor itself does **not** return its raw pool slot; its allocating
caller remains responsible for that cleanup.

B454E0 publishes D61BE8 and walks bindings while the unsigned cursor is less
than the **current** count6C, reloading the count after each entry. A nonnull
entry is destroyed through B42250, freed through the shared heap and then
cleared in the pass. Count6C remains stale. A callback can publish another
binding and raise the count before the next comparison. Valid readable
extents0..4 are required; corrupt extents are rejected by the host.

B42250 captures retained04, decrements its actual +04, dispatches the current
canonical terminal path only on zero, then clears the binding field after
the callback. It releases the string afterward, leaving its header as the
actual string helper leaves it. Its FuncInfo DF7A8C has one member-unwind
state: CBF110 invokes 0041DD20 at binding+08. The parent's heap free happens
only after binding destruction returns.

After bindings, pass destruction reads and releases fallback84, vertex70 and
pixel74 in that order, clearing each field after its callback. Later fields
are read at their call sites, so earlier callbacks can publish later owners.
Then B5F510 destroys the actual base: render18, sampler20, third1C, retained54,
retained58, four embedded headers and reference base. Destructor FuncInfo
DF7E54 has one unwind state, CBF480 to B5F510. The member cleanup order is
preserved; native exception-handler encoding is not emitted by the C++ port.

B46910 destroys the payload first, then returns its original address through
the actual 0108FBF8 pool's B40A40 iff flags bit0 is set. It does not heap-free
the slot. `NativeMaterialPassReference` borrows the same actual +04 counter,
validates the current D61BE8/BD30E0/B46910 profile, and retires its companion
only after payload destruction and pool return. The +14 effect accessors
perform no reference-count or ownership operation.

## Evidence and validation

Fifteen current live/installed-PE spans cover the six complete functions, pass
and context tables, literal and all three unwind maps/thunk groups. All six
stored Ghidra bodies already cover their full returns and have no internal
gaps. Prior names/comments are preserved, selected annotations read back,
the project saved and all six exports refreshed.

One ignored fixture executes all six complete original functions. It reuses
the prior frozen original base/state code and the original B40A40 pool-return
body. The paired comparison covers 1,508 constructor bytes including actual
base state headers/default rows, plus all 136 destructor bytes, normalizing
pointer identities. It checks untouched binding/padding preimages, one owned
fallback reference, native flags1 pool return, and immediate reuse of the same
raw slot. The Windows lock and actual slab metadata are exercised.

The fixture's first binding callback raises count1 to2 and publishes the next
binding; the fallback callback publishes a later pixel owner. The matched
owner/string event sequence is binding0, string0, binding1, string1, fallback,
vertex, pixel, base render, base sampler, base third. Borrowed effect count
remains unchanged. A composed canonical final-zero case returns the slot
before companion retirement; a count2 release first preserves the owner.

A separate rebuilt-code throwing acquisition case verifies temporary cleanup,
all three state-owner terminal paths and base cleanup, with the raw slot still
allocated for its caller. Across the paired/composed/failure cases there are
12 actual state retirements, 15 actual context retirements and one pass
companion retirement. No original exception delivery was executed; the native
unwind maps and targets were independently checked against the PE.

The strict Win32 build and both existing CTests pass; no permanent tests were
added. Original imports use current heap, Windows APIs and actual string-pool
adapters. Original child virtual0 tables are relocated to adapters that restore
the numeric profile and dispatch the canonical companion over the already-zero
physical counter. Fallback/shaders in the fixture are actual generic context
owners: real texture/shader loading, original exception ABI, drawing and
gameplay remain unvalidated. `reports/native_material_pass_owner.json` pins
the tested source, dependency evidence, fixture and preserved build products.

## Follow-up packets

Full pass copy B455C0 and secondary-pass construction B45E00 can now compose
the actual base, payload lifetime and pool. The copy's writes to fresh owner
reference counts require instruction-level review. Effect-loader B45EE0,
B46950 policy and undefined B469A0 remain broader follow-up work. Recheck
leases before claiming any of these functions; their pass/shader population
contracts must be recovered before a runnable renderer claim.

## Correction from docs/NATIVE_MATERIAL_PASS_COPY.md

The complete B455C0 copy, its four actual row helpers, binding constructor
B44690 and B45E00 secondary builder are now reconstructed and native-fixture
tested. Copy preserves the source's current state-owner counts in independent
allocations and appends bindings. Secondary creation publishes before copy;
constructor and copy failures have different cleanup scopes. See
docs/NATIVE_MATERIAL_PASS_COPY.md for exact ordering and validation boundaries.
