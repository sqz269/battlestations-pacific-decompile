# Actual scalar-tree iterator equality, 00553070

Whole Native `00553070..00553098` is 41 defined bytes and 18 instructions.
This packet reconstructs that ordinary body only. The established Ghidra name
`STL_inst_00553070` and stored `undefined STL_inst_00553070(void)` prototype
remain unchanged; the descriptive Source name is a hypothesis.

The explicit MSVC Win32 API is:

```cpp
bool __fastcall equal_native_input_scalar_tree_iterators_00553070(
    void* actual_left, void* unused_edx, const void* actual_right);
```

Both pointers borrow actual 8-byte iterator storage: owner at 0 and node at 4.
Native ECX supplies the left pointer, entry EDX is unconsumed, one stacked
DWORD supplies the right pointer, and the physical return is `RET4`.
There is no new carrier, context, callback, container, payload or allocator.

The implementation is a direct naked instruction schedule. It first captures
CURRENT left.owner, then captures the actual right pointer from the stack.
A zero captured left.owner skips the right.owner read and calls the actual
diagnostic. Otherwise it compares captured left.owner with CURRENT right.owner
and diagnoses a mismatch. A returning diagnostic does not revalidate owners or
repair either iterator. The continuation reads CURRENT left.node before CURRENT
right.node and returns their equality in AL. Only the reached provider can
modify storage; this body performs no publication stores.

Upper EAX retains the high 24 bits of CURRENT left.node, ECX becomes 0 or 1,
and ESI/EDI are restored. POP EDI, SETZ CL, MOV AL,CL, POP ESI and RET4 leave
the final node CMP arithmetic flags intact. EBX/EBP are not written by this
body; the provider's nonvolatile preservation remains required.

| Actual Native instruction | Ownership and actual provider |
| --- | --- |
| `00553082 CALL 00BF6713` | Owned whole equality; published qualified SDK diagnostic entry |

The sole Source relocation targets
`invoke_native_invalid_parameter_00bf6713` from the published
`native_invalid_parameter_noinfo_call.hpp` and Source implementation. That entry
uses the currently selected SDK `_invalid_parameter_noinfo` policy. Native
BF66EF encoded handler selection, debugger/Watson delivery and its CRT bootstrap
are not closed by that SDK policy. The provider may return, throw or terminate;
no noexcept, noreturn, handler installation or exception conversion is added.

Whole equality has no local EH registration, cleanup action, handler, guard or
rollback. Returning diagnostics and transitive provider effects remain inherited.
Actual iterator storage and the caller stack must support the reached accesses.
Caller DF and SDK ABI contracts remain required. Matching the instruction
schedule does not establish Original code placement, private caller frames,
fault PCs, FH3/SEH behavior, heap/CRT identity, binary replacement compatibility
or runtime/game behavior.

Frozen readiness `15f076ed9cea656a46dab4e5d5bacd5e71e7dbf4c0f5c0fd201c418c31ba57cb`
inspected the complete 444B/183-instruction hint and this 41B/18-instruction
equality within its 500-byte cap. Hint insertion, map indexing, predecessor,
insertion, unique insertion, allocation and rotation bodies are excluded from
this Source packet. Root readiness8 and Source-only8 reviews accepted the
measured equality and exact current header/Source before compilation.

Fresh Source evidence rechecks all 41 PE/live bytes, all 18 current instruction
starts and all three physical flow sites against the frozen whole body. The
Original stored prototype and comments are captured without mutation or export.
Published provider fingerprints are tied to exact Main
`1c82033a07d8e9f99746cecea29a190fde5a16ed`.

All transitively cited current object paths were copied to immutable SHA256
archives before the normal build. The distinct prebuild mapping preserves the
map object, current unique object and historical f963/4338 archives without
rewriting older receipts. Source51's nine pinned blobs, 24 artifacts,
precommit/frozen/Root seals, annotation and publication receipts remain
preserved, with shared CMake/ledger unions distinguished from pinned blobs.

The normal `./scripts/build.ps1` exits 0 and all three existing suites pass
in 7.51 seconds. No new tests, probes or owned Native/runtime execution are
claimed. The existing unrelated LNK4006 spawn-request duplicate warning is
retained in the build log.

The complete actual CMake object is 869 file bytes across four sections.
Its single CODE section is exactly 41 bytes/18 instructions. All 37 bytes
outside the one 4-byte CALL placement operand match Native exactly, including
every branch, register access and physical RET4. The sole relocation, at
offset 13h, is REL32 to the published diagnostic entry's actual C++ symbol.
There are no extra helpers, code padding, EH/SAFESEH sections, BSS, literals or
typeinfo. Noncode sections are `.drectve` 47B, `.debug$S` 216B and `.chks64` 32B.
All section bytes, symbols/auxiliary records, string table and file regions
were reviewed; there are no unclaimed file-padding bytes.

All 56 cited current object paths and 370 preexisting cited JSON receipts
remain byte-identical after this build. The distinct postbuild mapping also
verifies every immutable archive copy. This caller CODE comparison preserves
the inherited Current SDK boundary; it does not establish the Original
diagnostic implementation or runtime behavior.
