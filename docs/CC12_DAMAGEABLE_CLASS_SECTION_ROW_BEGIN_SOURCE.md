# DamageableClass section row-begin Source

Owned native region: `0087CE00..0087CE87`, exactly 136 bytes / 32 instructions
inside `0087CA80`. Excluded successor: `0087CE88` current-row selection.

This supplies an ordinary, caller-retained Source owner for the complete
temporary initialization, genuine section-vector append and matching temporary
cleanup. It borrows the actual descriptor, temporary, enclosing iterator owner
and bindings. It does not supply an application binding or the rest of the
reader. The candidate compiles under strict MSVC Win32 settings; Root CMake
registration and normal build review remain pending.

## Retained native evidence

The complete parent remains 3,238 bytes / 858 instructions with SHA-256
`00d679920bd1b8f8653e9a0ab1de627f954cfadaf48b8bf0642676fde5162808`.
The fresh local replay matched installed PE bytes, every saved and retained-live
instruction start, and the complete selected region. Its SHA-256 is
`16c1a68b21959bb3c291bdfc3215e32f9b548345f1ff64e7b5bb25df42b917bd`.
There were no live Ghidra queries, Native child/data/handler openings or new
constant-cell reads. State 14 evidence comes from the retained 37-state audit.

CE12 is **MOVSS XMM0,[CE38B8]**; CE1D and CE26 are its two MOVSS stores.
The candidate preserves one actual-cell load followed by two raw stores.
There is no introduced x87 initialization. Existing section-copy provider x87
operations retain their separate contract. The retained actual cell contains
little-endian `00 00 20 41`, float32 10.0; the Source receives its stable address
and does not substitute a literal or fabricated cell.

With S=ESP after E4h locals and four saved registers, the actual temporary is
S+A8h..S+D7h: 30h, or 48 bytes. Memset zeros all 48 bytes, then the two stores
write +28h/+2Ch. Temporary+24h is the intrusive owner slot. Actual descriptor+18h
is a 10h vector header; header+0 is preserved and +4/+8/+Ch are begin/end/capacity.
The original append uses ECX=header, stacked source and RET4, with no semantic
return value. These fragments expose new ordinary interfaces, not that ABI.

## Borrowed owner and bindings

`NativeDamageableClassSectionRowBeginFragment` is fixed: copying and moving are
deleted. Construct it before its one `run()` while the successful enclosing
`NativeDamageableClassSectionIteratorSetupFragment` still owns the actual key
S+58h/value S+2Ch under state 13. The supplied iterator scratch is the same
unchanged scratch used by that owner. Damage, Sections and Unique remain in
their existing outer owners. The new owner does not construct or copy any Lua
object and does not destroy those enclosing objects.

The actual descriptor and 48-byte temporary must remain stable, aligned and
disjoint from each other, live Lua slots, owners, binding records and provider
frames/arguments. Writable extents must not wrap; DF=0 is required for the
genuine memset provider. Raw temporary loads/stores use inline assembly, so
the interface does not invent a typed float or section object in byte storage.
Actual intrusive owners must supply their genuine aligned LONG count at +4
and current machine vtable/vslot0. No callback reentry, replay, or scratch/access
binding mutation is permitted. The observed callback replacement of temporary
contents is still handled by the specified final clear.

`NativeDamageableClassSectionRowBeginFragmentAccess` requires all of:

- Actual `NativeDamageableSectionVectorAccess`, with stable D0DF04-equivalent
  table identity and the real returning invalid-parameter service/context.
- A reference to the actual canonical 0109EEA4 DWORD for the genuine memset
  Source interface. Count 48 bypasses reading the cell, but the interface still
  requires the genuine binding. Its fourth argument is not the original ABI.
- The actual stable CE38B8 float-cell address with the retained 10.0 bits.

These are caller preconditions, with no fabricated fallback table, callback,
zero cell, container or semantic row. This packet does not establish the game's
application receiver or service wiring.

## Normal and unwind lifetime

The owner starts at ordinary state 13. It calls genuine BF79F0 Source, performs
the three MOVSS operations, derives descriptor+18h and arms state 14 immediately
before genuine 87C870 append. The append provider retains its existing actual
record construction, header reloads, allocation/catch and partial-write rules.
No transactional rollback is introduced.

The successful tail performs these ordered operations:

1. Capture temporary+24h once and save the captured nullness.
2. Lower 14->13 before publishing the actual table to temporary+0.
3. Branch using the saved nullness; null skips decrement, dispatch and slot clear.
4. Decrement the captured owner's actual +4 count with Interlocked semantics.
5. Only on zero, read the captured owner's **current** vptr/vslot0 and dispatch
   with ECX=owner, thiscall, no stacked flags.
6. After a returning callback or nonterminal decrement, clear original temp+24h.

A callback replacement in that original slot is cleared without being retained
or released. A throwing normal callback sees state 13 already stored; destructor
unwinding must not retry the temporary. Pair and outer owners retain their duties.

If append throws while state 14 is active, this guard lowers to 13 and calls
genuine `destroy_native_damageable_section_00878ef0` on the actual temporary.
That provider writes the table before reading/releasing its slot, matching the
retained state-14 action C96994 on EBP-58h=S+A8h, parent state 13. It does not free
the stack record or roll back the descriptor's vector. It is deliberately not
used as the normal-tail implementation. Slot-based 41DE40 likewise cannot replace
the normal parent's earlier capture.

The destructor is noexcept. Secondary failure terminates under ordinary C++
semantics. The owning caller must already exist before `run()` and unwind its
inner guard before the pair. No native FH3/SEH/longjmp, fault-PC or double-exception
identity is claimed. No actual native parent's unwind-state slot is overwritten.

## Complete candidate and provider COFF review

Focused compilation used `/O2 /Ob2 /Oy- /MD /W4 /WX /fp:strict /EHsc /std:c++17`,
with Win32 Release defines. It completed with exit 0. No test or executable was
added. The entire candidate has ten physical sections and four code sections:

| Physical section | Definition | Bytes | Instructions |
| --- | --- | ---: | ---: |
| 4 | constructor | 33 | 12 |
| 5 | noexcept destructor | 88 | 32 |
| 6 | destructor FH3 handler | 29 | 9 |
| 7 | `run` | 187 | 66 |
| Total | complete object code | 337 | 119 |

Run offsets +35/+39/+3E are the MOVSS load/stores; +46 arms 14 and +56 calls
actual append. +61 captures, +67/+69 records nullness, +6D lowers to 13, +84
writes the table and +86 checks saved nullness. +97 is LOCK XADD; its result
flags select the zero transition. +A0/+A2 load the current table/slot, +AA calls
with ECX=the captured owner, and +B0 clears the original slot. Destructor +2B
lowers to 13 before +3C calls genuine 878EF0, only when state 14 is active.

The complete indexed receipt includes all raw sections, instructions, physical
symbols and auxiliary records, relocations, symbol/string tables and EH data.
Physical section 9 is the destructor's 36-byte FuncInfo: magic 19930522,
max-state 0, empty maps and flags 5. SafeEH section 8 selects physical symbol 25,
whose handler is code section 6. `run` relies on the already-created caller-held
owner and has no private FH3 scope. Compiler weak AVX2-header storage is recorded
as a compiler artifact, not an invented native global.

Four genuine provider objects were refreshed with the same flags. Exact candidate
decorated targets each resolve uniquely: BF79F0 to memset section 3 (127 bytes),
87C870 to vector section 22 (324 bytes), and 878EF0 to section-provider section 9 (47 bytes).
Full objects and physical relocation/symbol/EH receipts are retained for memset
(357 code bytes), section (255), vector (3,078) and fill (230).

The report lists exact candidate constructor/destructor/run decorated prefixes
and **all 12 unique external project targets** across those five objects. Five
have unique definitions among the refreshed providers. The remaining seven
resolve at the Source level to existing allocator/free, legacy-string and
length-error-owner implementations; their current definitions and hashes are
identified. They are not replaced by stubs. Their separate objects and the normal
archive/link review remain Root's pending build boundary. CRT/security/EH symbols
are inventoried separately. Indirect owner and invalid-parameter dispatch remain
actual borrowed interfaces rather than fabricated callbacks.

All 29 retained tracked provider/context artifact hashes remain unchanged. Root's
readiness document gained its independent acceptance paragraph after the worker
report was frozen; the new current document and original report are pinned
separately. Prior Source 603 and historical vector validation are context only.

This candidate is Source-present and focused-compiled, unregistered and not
fixture- or game-executed. CE88 row selection, CEBB fields, next iteration, whole
reader, actual application binding, native ABI/FH3 and runtime remain held.
