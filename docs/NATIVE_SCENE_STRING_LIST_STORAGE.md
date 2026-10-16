# Native scene string-list storage

This reconstructs the constructor and growth helpers for the actual caller-provided
12-byte string-list header. It supplies no list-owning class or application caller.
The new translation unit is unregistered; Root review, registration and Main admission
are separate. Descriptive Source names are hypotheses; Ghidra was not changed.

| Original whole body | Source entry | Physical Original contract |
| --- | --- | --- |
| 0094EFB0..0094EFE0, 49B/18 instructions | construct_native_scene_string_list_storage_0094efb0 | ECX raw12; EAX same header; no stacked inputs; plain RET |
| 0094F010..0094F060, 81B/34 instructions | grow_native_scene_string_list_storage_0094f010 | ECX raw12; no stacked inputs; plain RET; no semantic EAX result |

The header is aligned writable raw storage: slots pointer at +0, count DWORD at +4,
capacity DWORD at +8. It has no vtable or allocator field. Each slot is an opaque
four-byte cell; these helpers do not inspect or retire any element.

## Preserved schedule

Constructor publishes capacity 64 before requesting exactly 256 raw bytes. After
allocation it publishes the returned slots pointer, then count zero, and returns the
actual header. It does not initialize any slot.

Growth publishes capacity +64 before allocation and retains the native unsigned
MUL4/SETO/NEG/OR allocation-size saturation instructions. After allocation returns it
captures the current slots pointer and current count, forms count*4, and calls the
current CDECL memcpy even when that byte count is zero. It then reloads the current
slots pointer, conditionally frees it and clears +0, and finally publishes the captured
new allocation. Count is untouched. No bounds guard, rollback or element cleanup was
introduced.

The independent naked fastcall entries preserve all 114 non-CALL-operand bytes of
the 130-byte, 52-instruction Original bodies. Only four four-byte CALL operands are
rebound: one constructor allocation, and growth allocation/copy/free. No code was
inserted at either native entry.

## Current Source services and admitted domain

The private noinline CDECL size bridge calls the established
`singleton_lifetime_allocate({object, n, n})`. Release calls
`singleton_lifetime_free`; byte copy calls real current `memcpy`, with
`#pragma function(memcpy)`. No callback, Seg61Host, provider stub or replacement
container participates.

The admitted domain is ordinary successful positive allocatable, nonwrapping sizes:
genuine live constructor/earlier accepted-growth storage from the matching current
heap, coherent 0 <= count <= oldcapacity, oldcapacity <= 3FFFFFBFh, and nonwrapping
capacity+64 and four times that capacity. All reached opaque bytes must be initialized
and valid; header, old/new buffers and private frame are disjoint. Flat ES and DF clear
are required. Operations are serialized with no fault, throw, reentry, alias, trim or
teardown. Empty growth still uses real nonnull old/new allocations and calls memcpy.

A positive count is a storage-boundary precondition, not proof of an admitted
population caller. Retaining overflow instructions does not admit overflow or failure.
Actual append 0094F270, clear 0094F3E0, retirement 0094F540 and application 008F3270
create/pass/retire ownership remain incomplete and separate.

## Validation and qualifications

Fresh live typed metadata verified C:/Users/sqz269/bsp.gpr and
/battlestationspacific.exe. Exact whole live bytes, disk bytes, instruction starts and
boundaries agree for both functions. No listing or analysis mutation was performed.

There were two actual strict Win32 compiler attempts, one successful. The first
rejected naked attributes on header declarations (C2488) and produced no object.
Only those declaration attributes were removed. A second fresh preflight preceded
the corrected /W4 /WX /O2 /MD /EHsc /fp:strict compile; all 81 actual declared inputs
matched its prospective pins, including current tool and compiler UI observations.
Both literal invocations/results and the rejected input snapshot are retained.

The complete new COFF audit accounts for every file byte, section, symbol and
auxiliary slot, relocation, directive, CodeView record, checksum word and string-table
byte. Three executable sections contain 188 bytes/72 instructions: the two native
entries and a 58-byte/20-instruction allocation bridge. Seven code relocations and
zero metadata relocations are present. The bridge has its own GS cookie/check and
constructs object-kind3 with equal native/host size fields; it receives zero Native
credit. Current CRT headers also emit an unused four-byte weak COMDAT
`__Avx2WmemEnabledWeakValue`; no relocation reaches it. There is no static initializer.

One required normal `scripts/build.ps1` run passed its two existing CTests. That build
excludes this unregistered TU. No new test, probe, replay or execution of these new
bodies was performed. No CMake, packet registry, ledger or Ghidra write was made.

Evidence is in `local/WholeSceneStringListStorageSource01`; the tracked report is
`reports/native_scene_string_list_storage.json`. Current CRT/GS/compiler bindings and
new Source interfaces do not prove Original private CRT/EH/fault identity, complete
binary ABI, populated caller/class ownership, application startup, runtime or gameplay.
