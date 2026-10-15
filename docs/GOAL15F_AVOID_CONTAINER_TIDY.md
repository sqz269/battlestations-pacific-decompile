# Complete avoidance container tidy kernels

Addresses: `0041DDF0`, `0041F230`, `0041E3A0`.

| Native extent, inclusive | Coverage | New raw-storage Source entry | ABI |
|---|---|---|---|
| `0041DDF0..0041DE19`, 42 bytes | complete | `tidy_native_avoid_body_vector_0041ddf0` | ECX header, no stacked arguments, plain RET |
| `0041F230..0041F24C`, 29 bytes | complete | `tidy_native_avoid_hull_list_0041f230` | ECX header, no stacked arguments, plain RET |
| `0041E3A0..0041E3BC`, 29 bytes | complete | `tidy_native_avoid_hull_list_0041e3a0` | ECX header, no stacked arguments, plain RET |

The existing `STL_Vector_Tidy_0041ddf0`, `STL_List_Tidy_0041f230` and
`STL_List_Tidy_0041e3a0` library names and inventory categories are retained.
These new public Win32 fastcall entries take actual raw header addresses and are
distinct from typed/semantic convenience interfaces. No new class layout, owner,
proxy cleanup, abstract callback or injected provider is introduced.

## Exact schedule

The vector header spans 10h bytes: untouched proxy+0, first+4, last+8, end+Ch.
The native body preserves ESI, captures the first pointer once, and calls the real
free service only if that captured value is nonzero. After normal return, it
stores zero to +4, +8, +Ch in that order, then restores ESI and returns. It does
not inspect or destroy elements, free the outer header, reread first after free,
or clean the proxy. A provider's intervening writes to those three header words
are overwritten by the subsequent native zero stores.

Each list body is an independent implementation, with identical owned bytes
after its two CALL operands are normalized. Its raw header spans 0Ch bytes:
untouched proxy+0, head+4, size+8. Preserve ESI and forward the actual ECX header
to complete hull-list clear. Only after clear returns, load the **current** head
from +4, push it, call real free unconditionally, ADD ESP,4, store zero to +4,
restore ESI and return. No pre-clear head snapshot, null-head recovery or later
header reload is added. If clear throws, the head-free/null tail is not executed.
The calls' real Source provider/frame behavior remains qualified.

All three declarations return void. No promise is made about the native incidental
EAX value, which depends on the reached provider/branch. Literal naked instructions
preserve the header accesses, flags, saved register and stack schedule; valid
reached backing, stack/control cells and matched providers remain necessary.

## Concrete dependencies and callers

The checked host transfers are:

```text
0041DDFB CALL 00BF65AC
0041F233 CALL 0041E200
0041F23C CALL 00BF65AC
0041E3A3 CALL 0041E200
0041E3AC CALL 00BF65AC
```

The returning cdecl free thunk jumps to the actual CRT free body, which returns
for null and normal release paths. Source calls existing `singleton_lifetime_free`,
backed by real current `std::free`, paired with the established current allocation
services. Original CRT heap/errno/SEH identities are not transplanted.

Both clear transfers compose `clear_native_avoid_hull_list_0041e200` from the
existing manager lifetime module. Its complete body selflinks the current sentinel,
zeros size, captures each node's next before actual hull destruction/free, reloads
the current sentinel for loop comparison, and retains the sentinel allocation.
The tidy kernels then load and release the current header head themselves. Clear
uses the genuine current hull/free providers; it supplies no extra Source context.

All eight referenced unwind shims were read in full. Their transfers are:

```text
00C5E2FE JMP 0041DDF0
00C5E4EE JMP 0041DDF0
00C6816F JMP 0041DDF0
00C6817D JMP 0041DDF0
00C96EFA JMP 0041DDF0
00C96F08 JMP 0041DDF0
00C5E309 JMP 0041F230
00C5E4F9 JMP 0041F230
```

Each shim loads its saved owner from [EBP-10h]. The first two vector shims add5Ch;
the next four add364h,374h,364h,374h respectively. The list shims add6Ch. The
manager constructor/destructor's established maps select these +5Ch/+6Ch actions,
matching the existing manager schema. No full layout for the other owners is
invented. The helpers themselves have no recovered local EH metadata to emulate.

Live references and a physical E8/E9/literal-address census agree on six vector
and two F230 tail references. E3A0 has no current reference in that census. It is
reconstructed as a complete standalone library duplicate, with no caller claim.
Source manager guard reuse is a separately owned integration change; the Native
normal destructor's original inline cleanup schedule is not described as calls
to these helpers.

## Verification and limits

Project/program were checked as `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`, base00400000. Complete owned listings, live prototypes,
all referenced shim bodies, full clear, actual free thunk/body, current service
and established manager layout were inspected. Strict MSVC Win32
`/MD /std:c++17 /EHsc /W4 /WX /permissive- /O2 /c /FAcs` passes.

`local/output/compare_avoid_container_tidy_bytes.py [COFF-object] [native-listing-directory]`
accepts the primary registered object. It verifies all 42+29+29 owned bytes, each
expected REL32 relocation, E8 opcode and fixed real provider symbol, masking only
the one/two/two CALL operands. Every remaining byte matches the live native byte
listing, including final RET. Listing files are
`local/output/avoid_container_tidy_<address>_bytes.txt`.

One ignored fixture, `local/output/avoid_container_tidy_probe.cpp`, uses real
current allocation/free, actual BF50 sentinels and both new list entries, each
with a genuine node/hull-data/vertex/adjacency chain. Its actual raw headers end
with proxy unchanged, head null and size zero; `_heapwalk` observes the five
tracked allocations released for each list. Vector storage is released, its
three words zeroed and its proxy retained; repeating the now-null vector tidy
also passes. Sentinel payload is never read. These are bounded current Source
observations, not a global no-leak, dynamic unwind or original-runtime proof.

Run `local/output/run_avoid_container_tidy_probe.cmd [archive-path] [archive-only]`.
Default mode overlays this one module on the primary existing Release archive;
`archive-only` reproduces with the registered archive. The probe embeds a manifest.
The worker overlay run and report/diff checks pass. Primary owns CMake registration,
full registered build, saved Ghidra annotation/export and manager guard integration.
No new tracked test target, Native differential, original heap/SEH compatibility
or gameplay validation is claimed.
