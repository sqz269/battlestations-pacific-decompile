# Actual resource registry lookup

This packet reconstructs the complete 108-byte `00B19E90` body for the four
established actual factory profiles. It calls the completed raw `B19D60`
lookup and the concrete `BBC6F0` / `BBC810` creators. It does not construct or
populate a registry. `B1B730`, `B1B810`, registry destruction and the cache
consumer `B1A4F0` remain separate work.

The implementation is
`create_native_registered_resource_00b19e90` in
`src/native_resource_registry_lookup.cpp`, with a new explicit context in
`include/bsp/native_resource_registry_lookup.hpp`. Primary reviewed both
complete files before the focused comparison. The companion audit freezes
the original bytes, current source inputs, actual library and comparison.

## Native body and caller storage

Fresh guarded queries verify `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. All 21 captured spans, totaling 959 bytes, match
the original installed PE, SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The saved body is exactly `[00B19E90,00B19EFC)`; both native exits end in
`RET 4`. No extent or flow repair is needed for this entry.

Original ABI: ECX is the actual registry; the single stack argument is the
raw requested-name header. EAX is the created resource pointer or zero on
miss. The body has no local native EH registration, ownership state, name
copy, cleanup, retain, or registry-vtable access. The C++ API is a new source
interface, not a drop-in binary or native SEH replacement.

Caller-owned native registry storage is 10h bytes: identity at +0, a retained
word at +4, head at +8 and count at +C. This entry only forms `registry+4` as
the tree and uses its current head at tree+4. Nodes are 1Ch bytes with links
at +0/+4/+8, key length/data at +C/+10, factory pointer at +14, color byte at
+18 and nil byte at +19. The existing raw providers read this actual storage
directly. No projected or reconstructed tree, copied header, allocator or
population callback is supplied.

The recovered schedule is:

1. Capture the requested key and form the actual tree. Call complete
   `B19D60` with a local two-word iterator output.
2. Capture the output owner, then capture the **current tree head before**
   any returning invalid-parameter call. Null or unequal owner calls
   `BF6713`; execution continues if it returns.
3. Read the output node **after** that handler. A node equal to the captured
   head returns zero immediately.
4. Validate the captured owner for null. Then compare the captured node with
   a **fresh head read from that captured owner**; equality calls the same
   returning CRT boundary. No handler return is converted into an early exit.
5. Read current node+14, current factory+0, and current selected profile+4.
   Invoke that concrete creator and forward its EAX result unchanged. There
   is no later native state read, cleanup or ownership adjustment.

The source retains this order using raw volatile word reads. It requires
usable storage only where the native body reaches it; it adds no tree,
registry-profile, factory-deleter, reference-count or unused-view validation.

## Qualified actual factory dispatch

The context borrows actual read-only eight-byte profile views. Factory
objects retain their native identity words. Each reached call reads the
current object identity and then the current +4 selector from the selected
view. It does not copy a vtable or use a runtime provider callback. Only the
reached view must be usable.

| Actual identity | Native +0 (not read here) | Current +4 | Concrete source |
|---|---|---|---|
| D64470 | BBC650 | BBC6F0 | caustics creator |
| D644AC | BBC7A0 | BBC810 | shore-wave creator |
| D644E8 | BBC890 | BBC6F0 | caustics creator |
| D644F0 | BBC8E0 | BBC810 | shore-wave creator |

All four are original `.rdata` storage with PE characteristics `40000040`.
The existing discovery records actual registration and final-profile writers
in `BBC900`, `BBC5F0` and `BBC740`; it does not infer arbitrary registry
population. Factory and registry lifetime are caller preconditions. Lookup
retains neither. The final factory deleters do not unregister registry nodes.

The base `D64468` pure-virtual profile, unknown identities, and unknown
current selectors are outside this source contract. `std::invalid_argument`
at these source boundaries is **not recovered native exception behavior**.
The original indirect call is broader; this packet claims a complete body
only for the established four-profile domain. No unknown native route is
silently treated as one of the supported creators.

## Reached providers and exception limits

`B19D60` and its full lower-bound `B19B90` come from
`native_resource_cache_leaves.cpp`; the actual-header `443D00` comparator
comes from `native_vfs_date_leaf_providers.cpp`. Their current-header,
returning-validation, and `_stricmp` behavior is retained. The iterator
provider always publishes owner equal to the actual tree. The two creators
use the complete `B19980` / `C30470` raw constructors and existing
`singleton_lifetime_allocate` boundary for exactly 34h bytes. They preserve
the native uninitialized bytes and return actual raw resources with count
+4 equal to one and final identity `D64478` / `D644B4`.

The current allocation boundary implements the original malloc/new-handler
retry policy through the host CRT. `_stricmp`, `_invalid_parameter_noinfo`,
malloc/free, `_callnewh`, bad-allocation support and creator FH3 runtime remain
explicit library/runtime boundaries, even when native helpers have familiar
names. This packet does not reconstruct the CRT internals. `B19E90` adds no
catch: provider exceptions leave without new resource or name cleanup.

The original creator FH3 maps and captured-allocation free actions
`CC4B70` / `CC4BB0` are freshly pinned. Their prior source qualification is
unchanged: valid raw-storage constructor operations cannot throw a C++
exception, so optimized source may omit unreachable constructor cleanup;
this is not native SEH equivalence. Neither creator unwind nor allocation
failure is newly runtime-tested by this packet. Later resource destruction
through `BBC6D0` / `BBC7F0` and `C304A0` is unfinished and is not reached here.

## Focused evidence and limits

The strict Win32 build uses an ignored hook to add this source and the
already completed leaf provider source, which this base checkout has not
yet registered in shared CMake. Shared build files are unchanged. The
existing two CTests and eight native seed comparisons are the only standard
tests; no permanent test was added.

One ignored fixture runs seven comparisons of the **full original B19E90**
against the actual compiled library entry. It preserves the original literal
factory table addresses and all eight bytes of each table in read-only
process memory. Native provider entries bridge to the same complete current
library B19D60 and two creators used by the source entry. This proves this
entry's composition with those providers; it does not execute the original
provider bodies again. Every original span is checked against the installed
PE before any fixture-only bridge patch.

The comparison covers each of the four actual identities, both creator
selectors, an ordinary miss, and two reachable returning-CRT mutations.
One `_stricmp` invalid handler changes current node+14 to the other actual
factory; the lookup must create from that current factory. Another changes
the current tree head to the candidate during the reverse comparison; the
lookup must capture that post-provider head and return zero. The completed
find provider is never replaced with fabricated iterator output. Unused
context profile views are null, including all views on the mutation miss.

All seven comparisons pass across 327 trace words. The probe owns a private
`.orig` PE section `[0040C000,00F0C000)`, characteristics `C0000040`, at a
fixed image base of `00400000`. Static PE inspection and runtime array bounds
prove the literal regions `[00BB0000,00BC0000)` and
`[00D60000,00D70000)` lie wholly within that dedicated section and overlap no
other fixture section. It exists solely to prevent loader/CRT mappings from
occupying the original table and creator addresses. Earlier fixed-address
allocation attempts failed during setup, before any comparison ran.

The full original B19E90 is copied to a separate executable allocation; only
its four direct-call displacements are redirected after checking their
preimages. Its relative branches, field reads, validations, indirect call
and both exits remain original. The two creator entry bridges are separately
patched in the fixture's reserved region. All 32 bytes of the four original
tables are checked again after the comparisons and remain read-only.

For valid nonnull single-threaded storage, full B19D60 always writes
owner=tree. Therefore B19E90's null/mismatched-owner sites are not reachable
in this composition. With that valid owner there is no callback between its
head capture and later head read, so the second head-equality validation is
also not artificially forced. These native branches and their continuation
order are checked statically against the full assembly. No concurrency,
invalid-memory fault or native SEH behavior is claimed.

The fixture observes five real allocations, exact resource bytes (including
untouched bytes initialized by a fixture allocation observer), unchanged
count one, miss behavior and native `RET 4`. It separately frees returned
raw blocks after observation through the existing free boundary. That test
cleanup is outside the native lookup algorithm and does not establish a
production resource destructor. The original game is never run or changed.

Four exact library member objects contain 110 nonempty COFF code sections,
6,985 bytes and 244 relocations. The linked subset contains 15 sections and
1,132 bytes; it includes boundary exception support that is not executed by
these admitted-profile cases. The source lookup COMDAT contains 248 bytes
of instructions and 149 bytes of embedded switch tables, explicitly
separated in the frozen disassembly. Full lower-bound and raw constructor
algorithms are inlined into the linked find and creator helper. They are not
claimed as independently executed original bodies or standalone source
exports by this fixture.

The audit distinguishes full object/static EH evidence from linked and
executed providers, and pins the immutable archive, COFF sections and
relocations, source inputs, probe source, executable, link map, native spans,
trace and build logs for primary relinking. It makes no ABI-compatible,
whole-cache, whole-registry or game-validation claim.
