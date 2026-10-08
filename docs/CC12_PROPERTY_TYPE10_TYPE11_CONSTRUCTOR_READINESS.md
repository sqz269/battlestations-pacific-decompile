# Type10/11 raw constructor readiness

Two whole Native bodies are contract-ready for bounded successful current-domain reconstruction: Type10 `008EF3E0` and Type11 `008EF460`. **Source0:** this packet implements, compiles and executes neither interface, adds no names or ledger credit, and makes no native class, private CRT, exceptional-cleanup or gameplay admission. Each body was inspected independently; the model's Type9 similarity comments are not evidence for either contract.

| Property | Type10 | Type11 |
| --- | --- | --- |
| Whole body, exclusive end | `[008EF3E0,008EF454)` | `[008EF460,008EF4D7)` |
| Bytes / instructions | 116 / 36 | 119 / 37 |
| Stored tag at `+04` | 10 | 11 |
| Byte count | `4 * count`, modulo `2^32` | `12 * count`, modulo `2^32` |
| Proposed positive, nonoverflowing count domain | `1..3FFFFFFFh` | `1..15555555h` |
| Child allocation CALL | `008EF412 -> 00BF55BE` | `008EF495 -> 00BF55BE` |
| Copy CALL | `008EF421 -> 00BF7680` | `008EF4A4 -> 00BF7680` |
| Retaining branch | `008EF43B` | `008EF4BE` |

The current Main `src/` and `include/` entry-address search returns only two semantic switch comments in `scene_property_bag_merge.cpp`. It finds no callable raw Type10/11 constructor implementation. Existing `IntArray` and `Vector3Array` names describe model intent; these bodies copy raw bytes without element-wise integer, float or vector operations.

## Original constructor ABI and flags

ECX supplies actual fresh, writable, unowned **56-byte** root storage. Incoming EDX is unused. At constructor entry `T = ESP`, the three stack DWORDs are `[T+4] = count`, `[T+8] = actual input pointer`, `[T+C] = copy_flag_bits`. The comparison reads **only the low byte** of the flag. A flag such as `100h` selects retention; a nonzero low byte selects copying. Both exits return the root in EAX and execute `RET 0Ch`, leaving the caller's ESP at `T+16`.

ESI and EDI are saved/restored explicitly. EBX and EBP are not directly modified; the copying path also relies on genuine providers' normal calling convention. On retention, ECX remains the root and EDX becomes the exact input pointer. On copying, ECX/EDX are unconstrained provider residuals. The copy return value is not used as the constructor's result: EAX is overwritten with the root.

The retaining path's last flag writer is `CMP byte(flag),0`, taken with zero: `CF=OF=AF=SF=0`, `ZF=PF=1` (arithmetic mask `08D5h`, value `0044h`). The copying path's last flag writer is `ADD ESP,10h` at `008EF426` or `008EF4A9`: immediately before it ESP is `T-24`, and the result is `T-8`. All six arithmetic flags follow that exact 32-bit addition. Later MOV/POP/RET instructions do not change them. A C++ interface declaration alone cannot establish these register/flag results.

Neither constructor directly uses x87, SSE, MXCSR, segment state or FS. Retention preserves DF. Copying requires DF clear for the real current providers; this audit gives no blanket DF/ES/FPU/MXCSR preservation or exceptional-unwind promise through those calls.

## Arithmetic, ordered stores and preserved bytes

Type10 loads count after pushing ESI/EDI, doubles it, stores phase/tag, then doubles it again. Type11 loads count into EAX **before** pushing those registers, computes `3*count` with LEA, doubles to six, stores phase/tag, then doubles to twelve. Every arithmetic step uses 32-bit integer bits. Neither body checks positivity, overflow, input length, pointer validity or allocation success. The later flag-byte CMP replaces the arithmetic flags before branch selection.

After the final scale, both bodies read the low flag byte, zero root `+18` and `+1C`, store byte count at `+24`, then branch. Retention loads the original `[T+8]` into EDX and stores that pointer at `+20`. Copying pushes the byte count and calls allocation, then loads the original input pointer **after** the allocator returns. It pushes count/input/destination for memcpy and stores the actual returned allocation at root `+20` **before** the copy call. A single `ADD ESP,10h` removes the outstanding allocation-size argument and all three copy arguments.

Both tails restore EDI, zero ordinal `+34`, store byte `1` at `+2C`, return the root in EAX, restore ESI and execute `RET 0Ch`. The root's final partial-storage mask is the same for both bodies and branches:

| Root bytes, half-open | Result |
| --- | --- |
| `[00,04)` | Literal phase `00CE89D4h`, data only |
| `[04,08)` | Tag 10 or 11 |
| `[18,20)` | Zero |
| `[20,24)` | Actual retained input pointer or actual copied allocation |
| `[24,28)` | Scaled byte count |
| `[2C,2D)` | Byte 1 |
| `[34,38)` | Zero ordinal |
| `[08,18)`, `[28,2C)`, `[2D,34)` | Preserved unchanged |

There are **29 written and 27 preserved bytes**. In particular, owner DWORD `+30` is preserved allocation content: it is neither initialized nor published. Byte `+2C` is one in both branches and does not distinguish an owned copy from caller-managed retained input. Neither that byte nor the phase literal supplies a native object/destructor contract.

For a future bounded Source interface, choose positive counts within the table's bounds, require an actual stable readable span of exactly `stride*count` bytes, and require nonwrapping addresses and real successful allocations. Both maximum arithmetic sizes are `FFFFFFFCh`; that is an arithmetic bound, not a promise that such an allocation is possible. Zero, overflow, invalid/overlapping spans, failed allocation, reentry and faults remain outside this proposed domain. Native code does not enforce these restrictions.

## Actual helpers and current-domain design

`00BF55BE` is a whole **five-byte JMP** to `00BF681B`; it is not an independent allocator implementation. The inspected native allocator's normal path calls malloc `00BF9F1A` and retries through new-handler `00C055B1`. Its native globals, failure construction and throw path remain separate dependencies. Copy target `00BF7680` is the named cdecl `_memcpy(destination, source, size)` entry. Its inspected entry checks ranges and feature cell `0109EEA4`; its metadata names `__VEC_memcpy`. This packet does not qualify the complete private CRT, vector, overlap or exception behavior.

The existing Source `singleton_lifetime_allocate/free` uses current malloc/new-handler/free. A future reconstruction can bind that actual current allocator and genuine current memcpy on a successful, disjoint-span domain. It must not call numeric native addresses, synthesize native provider globals or use stand-in callbacks. Preserve the raw root's untouched bytes. Keep copied children and caller roots in the matching actual allocation/free domain; observe each copied child while live, then free it once before root disposal. Retained input remains caller-managed and must not be freed as a newly allocated child.

Two distinct raw-storage interfaces are sufficient. If the established MSVC Win32 fastcall transport is chosen, an explicit unused second register formal can reserve EDX and keep count/input/flags on the stack. This is a non-executable design suggestion, not ABI proof. Exact registers, flags, complete helper closures and production linkage require later primary implementation and qualification. Do not attach `noexcept`: no constructor-local cleanup frame protects provider failure. No Source code, header, helper, test or build registration is added here.

## Selected clone arms and direct producers

Fresh dispatch-table entries bind tag10 to `[008F51FA,008F5242)` and tag11 to `[008F5242,008F528A)`, **72 bytes each**. Both allocate a 38h root directly through `00BF681B`, save it in EDI and the exception local, and set state 5 or 6. They push literal DWORD `1`, then source `+20`, call `008EF7F0` with source ECX, push its returned count, set ECX to the fresh root, and call the matching constructor at `008F5226` or `008F526E`. Thus these clone arms always choose copying.

For tag10 the count helper returns unsigned source byte count `>> 2`. For tag11 it multiplies the unsigned byte count by `AAAAAAABh`, takes the high DWORD and shifts it right three. This equals **floor(byte_count / 12)**: `12*AAAAAAABh = 2^35+4`; the multiplication's extra error is less than `1/24` for any 32-bit input, while the largest fractional part of `x/12` is `11/12`. The helper does not validate zero or divisibility. A bounded connected clone therefore needs a stable genuine source with `byte_count = stride*n`, positive and within the chosen bound; malformed nonmultiples must not be admitted silently.

After constructor return, each arm rereads source `+34` and stores it to the returned root at `008F522F` or `008F5277`. Owner `+30` remains preserved and unpublished. Both allocation-null branches enter `008F528A`, clear EAX, then write `[EAX+34h]` at `008F5290`; that path does **not** supply a safe null-return contract. Only the selected normal arms and necessary common/count slices are claimed as evidence, without whole-clone Source, ABI, private CRT or states 5/6 cleanup admission.

Each constructor has exactly two current call references. Besides the clone arm, Type10 is called at `008F35D4` within `[008F3590,008F3604)` and Type11 at `008F3654` within `[008F3610,008F3684)`. Both direct producer callers are 116 bytes/36 instructions. Their incoming contract is destination bag ECX and, at producer-entry ESP `P`, **`+4 = key`, `+8 = input`, `+C = count`, `+10 = flags`**. They rearrange those arguments into constructor count/input/flag order, then pass the resulting record and original key to insertion `008F33F0` at `008F35ED` or `008F366D`. They use `RET 10h`; successful insertion returns the destination bag. Their allocation-null path still passes a null record to insertion, without establishing safe OOM behavior.

Insertion publishes the destination bag pointer into record `+30` at `008F345F`. For zero record ordinal it copies old bag counter `+10C` and increments that counter; for a nonzero ordinal it leaves both unchanged. Bag `+110` is a separate backlink. The direct producers begin with the constructor's zero ordinal; clones first replace it with the source ordinal. Native handlers `00CA496B`/`00CA498B`, genuine bag/key/map/node ownership, complete insertion/failure behavior and cleanup remain named incomplete dependencies. No caller reconstruction credit is awarded.

## Evidence closure and next boundary

Only addresses `008EF3E0` and `008EF460` and these two metadata files were leased. Other addresses were inspected read-only. Every live query used the configured existing `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, `x86:LE:32:default`, base `00400000`, with `bsp.py`'s project/program/language/base verification and autostart disabled.

The fresh ignored family is `local/t1011/` in worktree `J:/PROG/battlestations-pacific-decompile-cc12_property_type10_type11_constructor_readiness`. Its all-file manifest covers every utility, query/result, frozen input, failed attempt, closing check and exact snapshot of both tracked metadata files. The sole manifest excludes itself to avoid recursive hashing; its independent SHA-256 is supplied in the commit handoff. No old helper or Root active/accepted fixture was read or executed, including `Main/local/t8p3`.

Closing verification passed **35 Native bookend pairs**: 14 byte ranges totaling **986 bytes**, 13 listings, five prototype captures, two caller-reference sets and one exact return-instruction response. All five strict current Source pins, ten frozen inputs and the Source entry search matched. One local closing check initially compared Main's CRLF target JSON to the new checkout's LF bytes. The failed utility/error/diagnosis are retained; parsed target settings match and both query-tool files match exactly. Source hashes remain strict, and all 72 read-only queries succeeded. No compiler, runtime provider-process query, Source/Native API execution, Ghidra write or ledger/name mutation occurred.

Ready next work is primary implementation review of these two bounded raw-storage interfaces, followed by fresh complete-helper/production qualification. Full insertion, genuine admitted Source record producers, raw array-count binding, native exceptional paths and record/array/recursive release (`008F0640`, `008F0DE0`, `004E6730` and connected bag/key/node ownership) stay separate. Source0 readiness does not promote either constructor, producer or clone to class/lifetime/game admission.
