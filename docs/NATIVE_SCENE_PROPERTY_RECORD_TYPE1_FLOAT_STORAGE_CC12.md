# Native property-record type-1 raw float storage, CC12

The reconstructed whole leaf at `008EF170` copies a raw 32-bit payload into
fresh caller-owned 56-byte storage. The new Win32 Source body is byte-identical
to the original 53 bytes: 15 instructions, no CALLs and no relocations. The
descriptive function name is a hypothesis; this does not reconstruct an owning
property class or its phase-table dispatch.

Worker validation is complete as supporting evidence. Root admission still
requires its independent complementary family, including the complete normal
probe security-cookie helper in its runtime code gates, followed by the main
Win32 build and repository checks. The worker's terminal family contains eight
declared runtime code spans. Its main function's reference to
`@__security_check_cookie@4` was relocation-resolved and the executable/library
were file-pinned, but that helper was not a separate pre-entry/post-entry code
span. The successful family remains unchanged; this gap is not silently counted
as a completed provider gate.

## Evidence and physical interface

- Whole native range: `[008EF170,008EF1A5)`, 53 bytes / 15 instructions.
- Native, fresh Source COFF and unique linked body SHA-256:
  `684d66087700bdea183c40bc4344d965ad5ccf6f8aab80f0f7b9dbff9fe3c51f`.
- Current declaration:
  `void* __fastcall construct_native_scene_property_record_type1_float_storage_008ef170(void* actual_record_ecx, void* unused_edx, std::uint32_t payload_bits) noexcept`.
- At entry, ECX is the actual storage root, EDX is unused, and `[ESP+4]` is
  the complete four-byte payload. `RET 4` removes that argument. EAX returns
  the actual root; ECX becomes zero and EDX becomes one.
- EBX, ESI, EDI, EBP and DF are preserved. Final defined XOR flags are
  CF=OF=SF=0 and ZF=PF=1; AF is undefined and excluded from checks.

Supply writable fresh/unowned 56-byte storage disjoint from the active call
frame. Ownership remains with the caller. Reinitializing an active owning
property object is outside this contract.

| Offset | Effect |
| --- | --- |
| `+00` | Store opaque native phase identity `00CE89D4` as a DWORD |
| `+04` | Store tag 1 as a DWORD |
| `+0C` | Store the exact raw argument DWORD through legacy MOVSS |
| `+18,+1C,+20,+24,+30,+34` | Store zero DWORDs |
| `+2C` | Store one byte with value 1 |
| `[08,0C),[10,18),[28,2C),[2D,30)` | Preserve all 19 existing bytes |

The original order and widths are retained: the MOVSS argument load occurs
first, before any destination store, and the `+2C` byte store is last. These
stores write exactly 37 bytes. The literal phase identity is never dereferenced
or mapped to a Source vtable by the leaf.

## Floating-point boundary

The C++ interface takes `std::uint32_t`, not a numeric floating-point argument.
Legacy `MOVSS xmm0,[ESP+4]` preserves every low payload bit and clears XMM0's
upper 96 bits. The following MOVSS store copies that low DWORD without
conversion or NaN quieting. The leaf contains no x87 instruction, arithmetic,
MXCSR update or other XMM-register write. No YMM/ZMM guarantee is made.

The separately audited native clone arm calls an allocator, then uses
`FLD m32` / `FSTP m32` before entering this leaf. That x87 conversion, ambient
exception behavior, whole clone, null path and EH are not part of this Source
operation or its executed fixture. The readiness audit documents their limits
in [CC12_PROPERTY_RECORD_TYPE1_FLOAT_READINESS.md](CC12_PROPERTY_RECORD_TYPE1_FLOAT_READINESS.md).

## Fresh worker validation

The terminal supporting family is ignored `local/f53c/`, built from exactly
three fresh translation units: this constructor, unchanged current
`singleton_lifetime.cpp`, and its new probe. It uses no BSP archive or prior
object and adds no tracked test. All consumed 184 standard/project headers,
including extensionless headers, seven libraries, tools, input Source, native
PE and recipe were pinned. Short hash-based frozen filenames were checked for
consistent duplicate mappings before compilation. Strict MSVC Win32 compilation
used `/O2 /MD /W4 /WX /fp:strict /EHsc /Gy /GL-`; the executable contains an
`asInvoker` manifest.

Complete fresh COFF sections were checked against linked bytes and every
relocation: constructor 53 bytes, canonical allocator 90, canonical free 6,
ordinary Source caller 18, ordinary indirect caller 66 including five trailing
INT3 alignment bytes, raw capture caller 174, bad_alloc constructor 24, and
probe main 2906. The two ordinary UInt32 callers establish all four payload
bytes and the register/stack contract statically; they were not executed. The
raw caller has one indirect CALL and captures the returned machine state.

Before creating either actual root, the probe checked CPU/OS capture features,
the eight declared linked code spans and actual I386 CRT providers for malloc,
free and `_callnewh`. Each actual IAT target was tied to its mapped module,
export RVA, held physical-file identity/NT path, full-file SHA-256 and normalized
32-byte export prefix. These checks were repeated after both matching frees.
The mapped provider was `Windows/SysWOW64/ucrtbase.dll`, SHA-256
`60c5a497b52de80a3a0677564270dbea7e486086637debd567b4dffb28584c1b`.
The separate security-cookie limitation above remains explicit.

The sole target process entered Source once and an unchanged 53-byte RX copy
of Original once, with two distinct actual canonical 56-byte malloc roots:

| Entry | Raw word | DF | Root poison | Captured EFLAGS |
| --- | --- | --- | --- | --- |
| Source | `7F812345` | 0 | `A6` | `00000246` |
| Original | `80000001` | 1 | `59` | `00000646` |

Both complete 56-byte results, all 19 untouched bytes, the other live root,
external capture guards, stack canaries, actual argument word, register results,
nonvolatile registers, DF and RET4 cleanup passed. XMM0 began with nonzero upper
words and ended with the exact payload and zero upper 96 bits. XMM1..7 matched.

Aligned FXSAVE snapshots checked CW, SW, abridged tags, defined FOP bits, MXCSR,
MXCSR_MASK and each nonempty logical 80-bit x87 payload using the physical FTW
and TOP mapping. The observed ambient state was empty: FCW `027F`, FSW `0000`,
FTW `00`, MXCSR `00001F80` for both entries. Thus this run provides no active-x87
payload case. Reserved FXSAVE padding, empty-register payloads, FIP/FDP and
YMM/ZMM were deliberately excluded. No FNSTENV or numeric/trap experiment ran.

Both roots were freed exactly once through the current canonical provider; the
RX copy was released. Original PE/Ghidra bytes and all declared linked/RX code
bookends were unchanged. All 1,091 prior pins were preserved, including the
earlier audit, accepted Root byte/append families, historical frozen Source
associations and both unexecuted precursor families.

`f53a` passed compilation but stopped at read-only symbol-selection ambiguity;
`f53b` passed compilation but its static gate used the wrong x86 import-name
decoration. Both were preserved with zero target processes. `f53c` rebuilt all
three TUs and passed its sole process. No successful phase or old family was
replayed, and no further worker process was launched after the cookie-gap
review.

## Receipts and limits

The machine-readable [worker report](../reports/native_scene_property_record_type1_float_storage_cc12.json)
records exact file paths, hashes, preparation failures and the pending Root
qualification. The terminal family has 327 manifest entries plus `seal.json`:

- `static_gate.json`: `88a2a6b6052e97d0c9f67df360e4c145fc1a6430ac3484d637efd26d7bbe5a77`
- `runtime.json`: `434fe3ed44c8165abcdbf1d7abba205c21ad9fa9d5b981835625dfba891a96b5`
- `post.json`: `7b31dfd69ba42a7edb9c263068c91533dc91ee1bceda7c53152068f6d35169fa`
- `seal.json`: `86596afb8c15dd10e588299d3fbf8b21c9d64e9f8f116011c89a8d3e204a0fe1`

The worker made no shared metadata, CMake, ledger or Ghidra changes. Root owns
independent complementary validation, the omitted cookie-helper runtime gate,
full main build/checks and eventual annotation/integration. Native allocator
internals, phase dispatch, class ownership, clone/EH paths and startup/gameplay
remain outside the reconstructed leaf's evidence.
