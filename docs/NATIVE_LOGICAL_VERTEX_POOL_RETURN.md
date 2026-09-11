# Native logical vertex pool return

`B49570..B495D9` is the complete 105-byte raw return operation for the logical
vertex pool, whose canonical storage is `0108FE18`. This leaf is independent
of the unresolved non-null `B62010 +4C` destruction path. The existing terminal
discovery and a fresh saved-Ghidra/installed-PE byte comparison establish its
instructions. The strict build and focused original-code replay now pass.

The original ABI is ECX pool, one stack slot pointer, RET4, with no semantic
result or local exception cleanup. The new free fastcall interface places the
slot pointer in EDX. It borrows initialized raw pool/slab/slot storage and calls
real Windows critical-section functions. No singleton, allocator, destructor,
validation or recovery path is synthesized.

After entering the captured pool+0C critical section, it increments the current
tracked depth at pool+24. Slot+74 selects the current pool+28 slab table. The
wrapped 32-bit slot-minus-slab difference is interpreted as signed and divided
by 120, truncating toward zero; only its low WORD is stored. The original signed
multiply/high-half/shift sequence establishes this operation even for negative
differences, rather than unsigned division or pointer subtraction in C++.

The current WORD at slab+F40 indexes the free stack at slab+F00. The slot index
store occurs before a fresh count load and increment. This matters if those
accesses alias. The pool+34 lowest slab index uses an unsigned comparison and
is reread at that comparison. Finally the current tracked depth is decremented
and the saved critical-section address is passed to LeaveCriticalSection.

Raw DWORD and WORD accesses preserve x86 unaligned access and observation order.
The interface requires writable/readable storage at every reached native access.
It is not an original-caller ABI replacement, a general owner implementation,
or a game-validation result.

## Integration and focused verification

The source is registered in CMake; the strict MSVC Win32 build and both existing
CTests pass. Eight native seeds were verified. The fixture links frozen main
library `d9092fcaeff91428b94835cc843c8c8bd2b318f7f056d22175ac31574c014986`;
its exact object archive member and entire linked COFF entry were checked.
Three original/library pairs match 53,521 literal DWORDs across 13 snapshots
per implementation. The same actual Windows critical section is reused for
paired observations, including all its bytes, so no pointer normalization is
needed. All 12 actual OS calls have original/source callsite and loaded x86
NTDLL-file evidence. Seven complete native-code and seven entire fixture-text
postimages match their expected bytes. Only the two absolute import operands
are relocated in the 105-byte original; the other 97 bytes are unchanged.

The three cases cover an ordinary return, a deliberate negative-offset and
free-stack/count alias stress case with WORD/DWORD wrap and unsigned comparison,
and an after-real-Enter observer that changes current slot/table/depth fields.
Stress-case addresses are valid but its metadata is deliberately noncanonical.
This targets concrete arithmetic/access-order risks; it adds no permanent test.
The descriptive name and appended evidence are saved in Ghidra, the export is
forcibly refreshed, and the complete reconstruction record is registered.
