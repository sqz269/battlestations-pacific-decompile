# Native SEH4 handler transfer (EF)

This packet reconstructs the complete `_EH4_TransferToHandler` at
**00C0DCCD..00C0DCE5**, 25 bytes and 11 instructions. Its original library
comment identifies `@_EH4_TransferToHandler@8`: the native inputs are ECX, the
actual selected handler entry, and EDX, its actual frame. The source uses one
naked MSVC Win32 `__fastcall` entry with those two register words and no added
binding, owner argument, global shadow or callback interface.

The actual dispatcher at C07C90 reaches this helper with `JMP` at C07E00.
Fresh PE/live bytes establish opcode E9, preceded by `MOV ECX,[EBX+8]` and
`MOV EDX,EDI`. Ghidra's xref categorizes this edge as UNCONDITIONAL_CALL; that
metadata label does not override the physical jump. It pushes no return word
or arguments, and the dispatcher's existing stack remains in place.

## Native instruction and stack contract

Let H be incoming ECX, F incoming EDX, and S incoming ESP. The helper changes
EBP to F, retains H in ESI and EAX, pushes the original code word1, and calls
the accepted DJ `notify_native_crt_nlg_00c16879`. NLG enters with ESP=S-8,
EAX=H, EBP=F, and code1 at its ESP+4. The real 31-byte NLG body preserves
registers and flags and returns with RET4, restoring ESP to S.

The helper then zeroes EAX, EBX, ECX, EDX and EDI, in that order, and ends in
the actual two-byte `JMP ESI`. At that transfer, ESI=H, EBP=F and ESP=S.
No register is saved or restored. Final arithmetic flags come from
`XOR EDI,EDI`: CF/OF/SF=0, ZF/PF=1, and AF undefined. DF and the remaining
unaffected flag bits retain their incoming values. There is no RET, frame
cleanup, cookie check, null check, catch, scope unwind or stack repair.

The native handler must interpret its inherited frame and dispatcher stack
and determine its own continuation. A normal C++ callback call would add a
return address and assume a different preservation contract. The declaration
documents register assignment; ordinary C++ invocation does not establish
the actual native entry domain. No `noreturn` attribute prunes an unproven
eventual handler continuation or rewrites the native transfer.

## Real providers and evidence boundary

DJ supplies the actual NLG instruction entry and writes the canonical
E16830 descriptor, in code/destination/frame order, leaving its signature
unchanged. Its already accepted DD owner must have mapped and initialized the
actual canonical pages and retained the RO/RW ownership for process lifetime.
EF performs no initialization, validation, mapping, lifetime change or
notification replacement. NLG faults can expose partial stores; EF adds no
rollback. The existing owner and NLG source/header bytes remain dependencies.

The source is registered once in `bsp_core`. Static proof compares the full
25-byte native body against current MSVC output and its unique archive member,
allowing only the one real REL32 operand at offset9 for the NLG call. A
never-executed forced-link image retains the actual EF/DJ/canonical-owner
providers for complete linked code and target checks. Ordinary game placement
is reported separately; archive availability alone is not native caller adoption.

The report records fresh original PE/live/listing bytes, caller edge, original
register/ESP/flags schedule, source and compiler inputs, all matching command/
read/write/Fo records, current objects/archive/image bindings, and both hash
inventories. The prescribed strict Win32 build, eight seed comparisons and two
existing CTests are retained; no new repository test is introduced.

The strict build passed on the first attempt, including `reconstructed_math`
and `native_math_differential`; all eight seed byte comparisons passed. All
2,700 protected tracked source/header/CMake/build-script inputs were unchanged
across the build. Six current C++ units have unique command/read/write groups,
exact Fo bindings, and 234 distinct captured read inputs. Their current archive
members are unique. The selected-provider image audit compares 274 mapped rows,
28,841 code bytes and 1,311 resolved relocations with zero unresolved operands.
The complete transfer25 and NLG31 bodies match native bytes except the transfer's
one actual NLG call relocation. Ordinary game MAP omits both entries; the forced
image retains them and actual canonical owner providers. It was never executed.

The correct `_EH4_TransferToHandler` library name and old comment are preserved.
Annotations are applied through the supported write lock, saved and read back.
This branch predates the root exporter-name correction, so final exports must
follow a forced current snapshot or explicitly retain current live-name evidence.
No caller prototype, no-return property, flow override, data-owner metadata or
adjacent function is changed by EF.

No owned transfer, NLG, native handler, exception, unwind, original game,
forced image or gameplay path is executed. Source/emitted ABI evidence is
separate from native dispatcher/frame/handler adoption and runtime validation.
The later C16DD0 image-helper dependency investigation is read-only readiness
work; it does not expand this source packet or claim the C07C90 dispatcher.
