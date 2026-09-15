# Native SEH4 dispatcher and scope closure FT

The original image guard, destructor pointer and scope records admit an authentic
linker relocation model. They do not require fixed original code addresses or
invented state. **The complete SEH4 source cycle is still blocked by the original
termination/PTD providers.** This packet proves the metadata design and identifies
the exact remaining entry/state boundary; it adds no partial source, object,
registration or callable fallback.

The clean worker baseline is `c8010d8fa964667a58d9cec6144d3fc9b97e7a49`.
FQ commit `e5aa22135c8c3ec94f0384a5cfae43bd6d9c7f15` supplies the earlier
prolog/dispatcher evidence. Its complete 105-file local seal was verified against
`3bca81d8068b0b1087387808b77c3c17bfe4ae8297a4feb5cf87c3252ea34e2f`.

## Authentic metadata ownership

| Original operand/storage | Established contract | Faithful source placement |
|---|---|---|
| `C16E0A`, `C16E1E`, `C16E24` use `00400000` | The two calls and RVA subtraction use the image containing the statically linked guard. Installed Microsoft `pesect.c` explicitly derives it from `&__ImageBase`. | Relocate all three operands to the real linker `__ImageBase` in that same PE. Preserve the original PE checks, section walk and access-violation-only filter. |
| `D6A618 = C06AC8`, `.rdata`, characteristics `40000040` | A constant cdecl destructor pointer, tested for null, whose **storage address** is guarded before invocation. Installed `chandler4.c` and `ehhelpers.cpp` corroborate the linker-selected `_pDestructExceptionObject` owner. | One immutable, image-resident pointer definition to the complete reconstructed destructor. Every consumer must reference the same symbol. Preserve the pointer-word guard and the two original outgoing arguments. |
| `E03738` and `E033C0` | Compiler-owned EH4 scope records; header `{-2,0,-28h,0}`, entry `{-2,filter,handler}`. The actual code operands and cookie frame offsets match their owning prologs. | Real constant scope records with relocations to their actual emitted funclets. Preserve the original header, enclosing level, encoded scope pointer and code targets. |
| `E15590` | The actual canonical cookie used by the prolog, scope checks and local unwind. | Keep the existing canonical data word/checker. A host compiler cookie is a different provider. |

The original PE has `IMAGE_FILE_RELOCS_STRIPPED` (characteristics `123h`), a
zero RVA/size base-relocation directory and no relocation records. Therefore
this is a library-source-corroborated ownership interpretation matched to the
native bytes; it is **not** recovered original COFF symbol/relocation evidence.
The installed current CRT is corroboration for these metadata contracts only:
its destructor exception filter differs from this VS2005 body and cannot be
substituted as the implementation.

The design preserves the guard's policy at the rebuilt image base. Merely
mapping `D6A618` read-only outside that image, checking the destructor's target
instead of its pointer word, or replacing the guard with a section observation
would change that policy. Compiler scope metadata is code-owned storage; it
is not a private copy of mutable game state.

## Complete physical boundary

| Entry or scope target | Inclusive native bytes | Coverage |
|---|---|---|
| `C07C00 __SEH_prolog4` | `C07C00..C07C44`, 69 bytes | Complete evidence; no source |
| `C07C90 __except_handler4` | `C07C90..C07E25`, 406 bytes | Complete evidence; no source |
| `C16DD0 __IsNonwritableInCurrentImage` | `C16DD0..C16E8A`, 187 physical bytes | Complete evidence including unlisted targets; no source |
| `C06AC8 ___DestructExceptionObject` | Declared `C06AC8..C06B0A`, 67 bytes; physical tail through `C06B1B` | Complete 84-byte physical evidence; no source |
| Guard filter and handler entry | `C16E59..C16E6C`; `C16E6D..C16E6F` | `no_ghidra_function`; disk decode matched to Ghidra memory |
| Destructor filter, handler entry, termination tail | `C06B0B..C06B13`; `C06B14..C06B16`; `C06B17..C06B1B` | `no_ghidra_function`; last five bytes JMP `C07A75` |

The FQ frame proof remains authoritative: the prolog consumes the compiler's
`return,scope,local-byte-count` stack layout, saves the nonvolatile registers,
encodes the scope with actual `E15590`, pushes `cookie XOR frame`, stores saved
ESP, publishes `FS:[0]`, and returns through the copied continuation. Its final
flags come from that XOR. Ordinary cdecl wrappers or extra context arguments
do not establish this frame. The dispatcher must preserve its complete native
stack/register schedule around filter execution, unwind and handler transfer.

Existing original-entry dependencies were inspected: `BFE120` checker,
`C07C45` epilog, `C0DCB6` filter, `C0DCCD` transfer, `C0DCE6` global unwind,
`C0DD00` local unwind, `C16D50/C16D80` image queries and `BF6AA6` member transfer.
The report records their original calls. Key cleanup facts are guard `ADD ESP,4`,
destructor-pointer invocation `ADD ESP,8`, image-query cleanups `4` and `8`,
and local unwind `RET 8` after the two stack words plus ECX/EDX. `BF6AA6`
uses POP/POP/XCHG/JMP to forward the two words as the actual member call;
it is not an ordinary cdecl function returning to an added cleanup wrapper.

Scope storage and SafeSEH admission are separate obligations. A future build
must emit this actual dispatcher in `.sxdata`, supply that metadata object as
a real linker input, root the exact dispatcher symbol for archive extraction,
and inspect the linked SafeSEH table. Existing C0DC54 registration does not
admit C07C90. No SafeSEH source or startup registration was changed here.

## Exact remaining provider and smallest next packet

The destructor's omitted handler restores ESP and jumps to **original
`C07A75 terminate`**. That 51-byte routine creates another SEH4 frame, calls
`C0522E __getptd`, reads the returned original PTD's `+78h` callback, invokes
it if nonnull, catches any callback exception through scope `E03558`, then
calls `C04DE2 abort`. Scope targets `C07A95..C07A98` and `C07A99..C07A9B`
also have no Ghidra function. They cannot be replaced with host `terminate`.

The 24-byte `C0522E` delegates to `C051B7` and calls `BFBA09(16)` for null.
`C051B7` uses the real getter cache and PTD index, allocates `(1,214h)`,
publishes the actual record through the decoded FLS/TLS setter, initializes
it through `C050F8`, and preserves LastError at the normal joins. Its omitted
`C0521F..C05221` continuation removes the free argument and zeros ESI on
setter failure; the pseudocode's unidentified return is not the contract.

FH commit `1e1ddcdf1285e25b5538638c2b6de617ed72807a` already owns the
bounded thread-initialization design. FT revalidates 15 of its 16 direct edges
from fresh matched bytes; the ninth-byte `C051AE` cleanup edge is inherited
explicitly. The actual writer is `C053DC`: it selects all four FLS exports
or switches the whole set to TLS, publishes a raw getter cache before pointer
encoding, creates `E15AFC`, and publishes the PTD before field initialization.
`E15AFC/E15B00` initially contain `FFFFFFFF`; `109DE34..43` are PE loader
zero-fill, not initialized callable providers. Current canonical admission
does not commit their `109D000` page or implement their original OS/heap owner.

Two FH findings are superseded at this pin: complete canonical one-word
`C0190B` locale-reference and `C11B31` unlock entries now exist. They retain
real storage, lifetime and lock preconditions. The original pointer codec/gate
and lock-acquisition/initialization owners remain absent. The existing decoder
has an extra context word and cannot be composed into the native PTD callers.

The smallest next boundary is **`C07A75` + its two true funclets/`E03558`
and `C0522E`**, coordinated with FH's actual PTD initialization/lifetime work.
It remains blocked on original `C051B7`, its canonical state providers,
`BFBA09`, and the complete `C04DE2` abort policy. The latter reads actual
`E15AF8`, signal-handler/raise state, captures the native context, and exits
through `BFBDCC(3)`; a host abort or synthetic failure is not equivalent.
Once those providers exist, this boundary and the FT dispatcher/prolog cycle
must be integrated together because `terminate` itself calls `C07C00`.
There is no ready independent source entry inside FT's assigned addresses
that closes these cycles without an unresolved provider or a surrogate.

## Verification

Read-only CLI queries verified project `bsp`, program
`/battlestationspacific.exe`, language and original image base, with autostart
disabled. Configured project path is `C:/Users/sqz269/bsp.gpr`; the path is
configuration evidence rather than an independently queried live filesystem
path. No listing/function creation, annotation, save, import or restart occurred.

All 1,803 code bytes across 11 spans and 100 initialized data bytes match the
original PE and fresh Ghidra memory. Another 16 bytes match explicitly proved
PE virtual zero initialization. Original SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The live direct-call checker passed 51 rows with zero failures; 25 indirect
rows are recorded separately. The unlisted C06B17 tail is byte-checked outside
that checker without false function attribution.

Local methods, failed attempts, original bytes, source provenance, FH/FQ input
hashes and the exact inventory remain in `local/seh4_dispatch_scope_closure_ft`.
Only its top-level `seal.json` is excluded from that inventory. No C++/ASM
source or build registration changed, so no compiler guard, seed run, build,
CTest, object/archive admission or native execution is claimed. This delivery
is bounded design evidence, not reconstructed or game-validated behavior.
