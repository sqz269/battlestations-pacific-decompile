# Native CRT region comparison

`compare_native_regions_00c1ef55` reconstructs the whole ordinary input behavior
of the correctly named Original `_memcmp`. The Source preserves its mixed-width
payload read schedule and returns exactly `-1`, `0`, or `+1` from the first
differing unsigned byte. Embedded NUL bytes are ordinary data.

The Original owned body is `[00C1EF55,00C20553)`: 5,630 bytes and 1,934
instructions. A separate fixed 128-byte table at `[00C20555,00C205D5)` supplies
32 tail targets. The intervening two bytes belong to neither claim. The whole
body, every table case, and disk/live equality were reviewed in the accepted
author and independent readiness families. No owned body remainder is omitted.

## Interface and admitted storage

```cpp
std::int32_t __cdecl bsp::compare_native_regions_00c1ef55(
    const void* first, const void* second, std::uint32_t count) noexcept;
```

This is a new ordinary MSVC Win32 C++ interface. Original arguments are cdecl
stack slots `[EBP+08]`, `[EBP+0C]`, and unsigned32 `[EBP+10]`, with signed32 EAX
result and caller cleanup. The implementation is not a drop-in physical ABI
replacement, and its C++ `noexcept` declaration provides no Original EH credit.

- Count zero admits any pointer values, including null, without payload reads
  or endpoint arithmetic.
- For positive count, both inputs designate genuine live allocations containing
  every requested readable, determinate, stable byte. Each requested endpoint
  is **within or one past the same enclosing allocation**, admitting prefixes
  and interior subranges. It need not equal the allocation end. This implements
  the required C1 wording correction; historical sealed readiness is unchanged.
- Requested address additions and advances must not wrap in the 32-bit domain.
  No signed count cap, pointer subtraction, initial alignment, nonalias condition,
  or pointer-equality shortcut is added. Identical and overlapping regions work
  within the same stable readable-domain preconditions.
- Ordinary flat Win32 memory is required. Mutation, lifetime changes,
  indeterminate bytes, concurrency, volatile/MMIO synchronization protocols,
  guard-page/fault observation, and nonflat segment environments are outside
  this ordinary contract. Payload scheduling gives no instruction, bus,
  speculation, fault, register, stack, flag, segment, runtime, or game identity.

## Exact payload schedule

Initial counts one through four use ascending BYTE pairs, first then second
(LR), stopping immediately on a difference. This route is separate from a
small remainder reached after an initially larger count.

For an initial count of at least five, unsigned remaining counts of at least
32 use eight DWORD pairs at offsets `0,4,...,28`. Each pair reads the first DWORD
then the second. Equality skips that block's byte fallback. An unequal pair
compares its four byte lanes in ascending order until the first difference.
Fresh byte pairs are LR except batch offset16 lane0, which is second then first
(RL). The fallback rereads the payload bytes; it does not replace these reads
with shifts of the earlier word value.

After equal batches, endpoint-relative tail DWORD displacements start at
`-remaining` and increase by four. Arithmetic routing represents all 32 fixed
Original table cases without a runtime dependency on the Original table.

| Tail DWORD group | Unequal-word byte fallback |
|---|---|
| 23 ordinary groups | Lane0 reuses the actual first DWORD's low byte and reads only the second BYTE; lanes1..3 read LR. |
| Displacements `-12,-9,-31,-15` | Lane0 rereads RL; lanes1..3 read LR. |
| Displacement `-10` | All four fresh byte pairs read RL. |

Together with batch offset16 lane0, these are exactly nine reversed fresh
DWORD-fallback pairs. The terminal suffix has a separate schedule:

| Remainder modulo4 | Reached suffix reads |
|---|---|
| 0 | None. |
| 1 | Final BYTE pair RL. |
| 2 | WORD16 pair LR at endpoint-2; equality returns0. On inequality, BYTE endpoint-2 then endpoint-1, each RL. |
| 3 | BYTE endpoint-3 LR, then endpoint-2 and endpoint-1 RL; bypass the WORD16 comparison. |

Every difference is normalized to exact magnitude one. Every reached payload
read stays inside the requested region under the stated bounds. A wider read
can include later in-range bytes before returning an earlier byte mismatch.

Private addressed inline-assembly MOV32 and zero-extending MOV16 helpers read
genuine possibly unaligned byte storage. They do not invent live uint32/uint16
objects or typed aliasing/alignment requirements. Separate volatile unsigned
BYTE reads preserve their specified sequence. The Source adds no validation,
SDK memcmp/memcpy wrapper, region copy, provider, global, TLS, errno policy,
caller binding, or runtime fallback.

## Verification and remaining gates

The single actual strict MSVC Win32 compilation passed with compiler
19.51.36244.0, `/std:c++17 /O2 /W4 /WX /permissive- /EHsc /MD /Gy /GL-`.
No explicit `/Oy` override was used. Both `/showIncludes` and
`/sourceDependencies` identify nine includes plus the Source file; all ten
match genuine before-compile pins. Source/header bytes were unchanged.

The 3,753-byte object has eight code sections: 1,029 bytes / 426 decoded rows,
including eight alignment NOPs occupying 38 bytes. All 418 nonalignment rows
were reviewed. The public body is 757 bytes / 296 rows with 29 payload-read
sites. All seven separately emitted private copies were also reviewed; the
public body inlines their work and contains no calls or code relocations.
The complete object review maps all 36 Original DWORD blocks and 32 tail cases
to the Source and actual public control/read sites.

All 19 sections, 29 primary symbols plus 19 auxiliary slots, and the complete
461-byte string table are accounted. Eight `0007` relocations are DIR32NB FPO
debug associations, not SECREL. CodeView, FPO, directives, padding, and opaque
checksum metadata are retained. There are no unresolved symbols, GS/EH tables,
static initializers, vector/string instructions, or runtime data providers.

An initial CMD transport failed before activation or compiler entry because
its executable path used forward slashes. A paired echo-only diagnosis proved
the path issue. Its receipt, two failed correction-preparation calls, and the
corrected launcher's separate before-compile pins are preserved. This produced
one compiler invocation, no recompilation, and no Source correction.

The packet's initial file/range lease fields required bookkeeping corrections
after the Source write/compile. The evidence records the actual extension
times without claiming retroactive ownership. The explicitly authorized four
files are the only tracked outputs.

Report/doc call checks are call-free: zero checked rows provide no behavioral
proof. No worker baseline build, test, probe, SDK comparison, target-body
execution, or Ghidra mutation was performed. Root admission, normal registered
Main build, and separate Main object/archive inspection remain pending.

Machine-readable evidence: `reports/native_crt_memcmp.json` and immutable
local family `local/WholeNativeCrtMemcmpSource01`. Original and independent
readiness are external evidence; current compilation and receipts are this
Source phase's own evidence.
