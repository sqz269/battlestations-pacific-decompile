# Native CRT memset chain (CK discovery)

The complete bounded chain is `BF79F0 _memset` -> `C0C965 __VEC_memzero` -> `C0C90E fastzero_I`, with a recursive call inside the dispatcher. All 352 physical bytes equal fresh saved-program bytes and the installed PE. There are 344 executable bytes/131 instructions, eight skipped padding bytes and no jump tables. Correct existing library names are preserved; descriptive future C++ names would be hypotheses.

| Entry | Physical span | Executable bytes/instructions | Other bytes |
| --- | --- | --- | --- |
| BF79F0 `_memset` | BF79F0..BF7A69, 122 B | 122 B / 47 | none |
| C0C965 `__VEC_memzero` | C0C965..C0C9F3, 143 B | 143 B / 60 | none |
| C0C90E `fastzero_I` | C0C90E..C0C964, 87 B | 79 B / 24 | C0C926..C0C92D: skipped self-LEA plus NOP, 8 B |

Base is `5c80806cc011d29830cde3a87ff46c7dd5bd5c26`. Supported BSP queries verify `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Installed PE SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. No source, build, test, native execution, Ghidra mutation, flow repair or settings change is performed.

## BF79F0 fill and tail contract

Original cdecl arguments are destination `[entry ESP+4]`, fill int `[+8]` and uint32 count `[+C]`; EAX returns the original destination and RET leaves argument cleanup to the caller. EDI is saved only for the scalar path. EBP/EBX/ESI remain untouched. EAX/ECX/EDX and arithmetic flags are volatile. DF is never changed; the ordinary contract requires DF=0 for any reached REP STOSD or vector-dispatch REP STOSB path.

The body reads count then destination. Zero count returns after rereading the destination argument, without reading the fill byte, canonical feature word or destination memory. Otherwise it zeroes EAX, reads only the low byte of the fill argument into AL and tests that byte. Vector dispatch requires low fill byte zero, unsigned count at least 100h, then current DWORD `0109EEA4 != 0`, in that order. High fill bytes do not affect this decision. The feature word is read only on that gated path. BF7A17 tail-jumps to C0C965 before any register push or stack change; the original three arguments and return address are intact.

Scalar count 1..3 enters an explicit ascending byte-store loop. For count >=4, alignment is wrapped `-destination & 3`. A nonzero prefix is subtracted from the remaining count and written byte by byte before pattern replication. The original shifts/adds replicate AL to all four EAX bytes. The body captures the DWORD count with logical SHR2 and the byte remainder with AND3, uses REP STOSD for positive DWORD count, then writes a positive byte remainder in an explicit ascending loop. Each explicit byte advances EDI with ADD1. The count/alignment guards prevent a zero-count entry to either explicit byte loop in the valid domain. No overflow, null, alignment or feature validation is added.

## C0C965 zero dispatcher

The actual ABI is three-argument cdecl `(destination, ignored_middle_word, uint32_count)`, despite the saved zero-parameter signature. It never reads `[EBP+C]`. It uses a 10h local frame, saves/restores EDI and EBP, leaves EBX/ESI untouched, returns the original destination and uses plain RET. It does not realign ESP, install a frame handler, read a feature word or issue a CPU query.

The destination remainder is the **signed** 32-bit remainder modulo 16, implemented by CDQ/XOR/SUB/AND/XOR/SUB. Aligned destinations split count into `bulk=count-(count&7Fh)` and tail. A positive bulk invokes actual `fastzero_I(destination, bulk)` at C0C996 with two pushed arguments; ADD ESP,8 restores the caller's frame. The destination and tail are reloaded afterward. A positive tail uses REP STOSB at `destination+count-tail`; zero tail performs no store.

A nonzero signed remainder chooses prefix `16-remainder`: 1..15 for positive pointer representations and 17..31 for negative ones. The dispatcher writes that entire prefix with REP STOSB **before** subtracting it from count. It then recursively calls itself at C0C9E2 with `(destination+prefix, 0, wrapped_count-prefix)` and ADD ESP,0Ch. It reloads the original destination after the recursive return.

An aligned direct zero-count call skips both bulk and tail. A direct misaligned short/zero call can write a prefix larger than the requested count and wrap the recursive remainder. This is not an independently safe short memset. BF79F0's >=256 gate makes the prefix safe for its valid writable, nonwrapping extents. Do not replace the signed remainder with unsigned low bits or insert a short-size repair.

## C0C90E actual bulk engine

The engine is two-argument cdecl `(destination, byte_count)`. It saves/restores EBP/EDI through a four-byte local frame and leaves EAX/EDX/EBX/ESI untouched. No meaningful return value is supplied. ECX and XMM0 are clobbered; XMM0 is zeroed by PXOR. DF is neither read nor changed. The actual CPU/OS must support SSE2 and the destination must be 16-byte aligned with sufficient mapped writable extent.

The engine shifts count right by seven, zeros XMM0, jumps over the eight padding bytes and immediately enters the store loop. Each iteration writes eight consecutive MOVDQA vectors at offsets 0,10h,20h,30h,40h,50h,60h,70h, advances EDI by 80h, decrements ECX and repeats on nonzero. There is no pre-loop count test. A direct count >=128 writes `floor(count/128)*128` bytes; the dispatcher supplies a positive multiple of 128. Zero/sub-128 direct calls still execute the first 128-byte iteration and underflow the loop count. A generic guarded zero routine would change this contract. No destination read, API, global, allocator, callback or exception frame occurs in this body.

## Feature ownership and next source packet

The existing BY feature discovery is reused by immutable report hash. Actual `0109EEA4` publication belongs to the recovered setter/detector and its separate startup/SEH4 owners. Current source CPU projections are not proof of that native ownership. These three bodies introduce no canonical state owner or IsProcessorFeaturePresent substitution.

The engine is the smallest complete independent source body. The engine plus dispatcher is a complete direct-provider pair using real recursive/two-argument calls. The whole three-body chain is source-ready with a borrowed reference to the actual canonical feature word and declared valid buffer/DF/SSE2 domain; native owning startup/frame/runtime placement remains separate.

A concrete future outer interface may add `const volatile uint32_t& actual_feature_word_0109eea4` as a fourth cdecl word. At the original BF7A0E gate, EAX is proven zero but remains live as the scalar fill pattern. A suitable replacement is `MOV EAX,[ESP+10h]; CMP DWORD PTR [EAX],0; MOV EAX,0; JE scalar`. The MOV-zero restores the live value without changing comparison flags. Do not reuse memmove's dead-EAX argument or use XOR to clear EAX between CMP and JE. The added pointer read remains behind the original count/fill gates; no new stack store is needed. The tail JMP still presents the original first three arguments, and the four-argument source caller removes all 16 bytes after return. This is a new source ABI with active-frame nonalias and changed instruction/fault locations explicitly qualified, not original binary/frame identity.

Current exact-address/name source queries found no implementation of these three entries at the packet base. The report retains precise ABI, calls, native bytes, code/padding classification, source query results and two full SHA256/SHA512 local inventories. Static discovery does not claim source reconstruction, ABI replacement, runtime or gameplay validation.
