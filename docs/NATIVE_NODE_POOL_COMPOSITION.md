# Native node and string pool composition

Addresses: `00B6F5A0`, `008680B0`, `00868193`, `00CC1A31`, `00B6ED70`, `00B6E670`.

Packet `orch3_node_pool_composition_q`, 2026-09-11. This joins the existing
physical node constructor to `NativeStringStorage` and implements the bounded
point-effect node-allocation stage. It reuses `NativeNodeStorage`, the actual
0108FF58 node pool, and the existing actual native string-pool bridge. No new
pool, shadow node, retained count or successful terminal callback is supplied.

## Constructor and string storage

`B6F5A0..B6F8CD` is a complete 814-byte constructor: ECX actual prefix, stack
pointer to the native eight-byte name header, EAX same prefix, RET4. The original
RET starts at B6F8CB; the end-exclusive address is B6F8CE. Its complete field
write map remains in `docs/NATIVE_NODE_CONSTRUCTION.md`.

The shared implementation now accepts `NativeStringStorage&`. Passing
`ActualNativeStringPoolStorage` executes the existing 419CC0 singleton getter
and BD1120/BD1510 actual owner/ring operations. Existing `SizedStoragePool`
callers construct their `PooledStringStorage` adapter and call the same body.
The constructor does not choose a second owner or silently fall back to CRT
storage. Name cleanup calls canonical `destroy_native_string_header_0041dd20`
with the same storage interface, preserving its nonnull capture and lack of
header clearing.

Assembly at B6F63C compares the actual destination +54 header with the source.
Otherwise B6F656 resizes from the source length, and B6F65B reloads source length
before deciding whether to copy. The copy reads the current source pointer,
destination pointer and destination length. The existing shared name-copy helper
has that same order and remains the implementation. In-place name-header alias
is valid: constructor initialization has already zeroed those two words.

The constructor only writes a 174h prefix. For the plain-node 0108FF58 pool,
the physical slot is 178h and the authoritative slab ID is at +174. The earlier
generic statement that the node pool is always 1F0h referred to a different
directional-light pool. Derived model/light/camera allocations retain their
own larger extents; this change creates no alternate prefix type.

CC1A31..CC1A3A was not a saved function. Verified bytes define a ten-byte compiler
handler selecting DFA93C and jumping to BF6B43. Its DFA924 map is state 0:
CC1A10/AA6E10 base cleanup; state 1: CC1A18/41DD20 name cleanup; state 2:
CC1A23/B6F3E0 point-light array cleanup. It is now defined and saved under the
shared Ghidra write lock. The existing reconstructed C++ member cleanup remains
in place; execution of the original exception dispatcher is not claimed.

## Point-effect allocation stage

`construct_point_effect_node_00868193` covers only `868193..8681BD`, plus the
stage's raw-slot cleanup on a C++ construction exception. This is a fragment
of the seven-stack-DWORD/RET1C constructor at 8680B0, not another complete native
function or an independent native calling convention.

The native stage supplies ECX=174 to B6ED70, which overwrites that size with the
actual global pool address. It captures the returned raw slot and advances
unwind state from 5 to 6. A nonnull result is passed to B6F5A0 with the name at
the original captured template's +1C; the caller supplies a borrowed reference
to that exact header, so no name value is copied before allocation. A null
result skips construction and yields null. The following retain stage still
requires nonnull storage; no recovery behavior is invented for it.

On a construction exception, the node constructor cleans its members, then
the stage returns the captured raw slot through B6E670 and rethrows. Native
DC6EB8 state 6 maps to C95012..C95019, which loads the saved slot and jumps to
B6E670, then advances to state 5. The outer effect's states 5..0, consumed
template argument, counters and other members remain the eventual whole
constructor's responsibility.

The result is physical node storage. A stable `NativeNodeBinding` with the
actual current scene/type/terminal operations must be supplied before the
existing 8681BE registration stage. This fragment does not publish +110,
increment references, attach hierarchy, or destroy a successfully built node.

## Verification

The strict Win32 build and both existing CTests pass. One ignored fixture
uses the complete 814-byte original constructor and 43-byte original effect
stage, each verified live against the installed PE. A small assembly caller
supplies the fragment's original EBX=0, captured ESI template and stack scratch;
the fall-through reaches one appended RET at the fragment boundary. Calls and
constant operands are explicitly rebound, as enumerated in the report.

Original and reconstructed execution use separate actual node and string
pools with the same input storage. Name lengths 0, 7, 148, 149 and 299 exercise
empty, embedded-arena and ordinary-allocation paths. The comparison covers
all 178 slot bytes after normalizing only the differing allocated name pointer,
including untouched prefix bytes and the +174 slab ID, plus name bytes and
terminator. It checks the actual string-pool bump/ring counters and the fact
that an empty name alone does not initialize the string singleton. An input
header alias check also passes.

A C++ storage allocation exception verifies member cleanup reaches base
CEB130 and the point-effect stage returns exactly the captured slot for reuse
with balanced pool depth. This is reconstructed C++ exception-domain evidence;
the copied original handler immediate is unchanged and never exercised. The
fixture explicitly cleans only fresh unattached test nodes. It supplies no
production terminal or scene callback, and no scene/terminal comparison is
claimed. No permanent tests were added.

## Follow-up packet

Close the actual plain-node retained terminal and stable scene/type binding,
then compose the already recovered point-effect prefix, allocation, registration,
matrix and row stages with the manager insertion and whole-constructor unwind.
This packet does not establish original exception ABI or gameplay compatibility.
