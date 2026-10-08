# CC12: complete physical-stream table extent and next raw provider

## Result

The installed physical-stream table is **26 DWORD entries / 104 bytes** at
**D691B0..D69217**, ending immediately before D69218. The former thirteen-word
window covered only offsets +00 through +30. Offsets +34 through +64 are a
shared thirteen-method stream tail, not a separate table starting at D691E4.

The current raw Source entries qualify two of these slots individually:
BF5020 at +18 and BF4F20 at +1C. The separate raw BF50D0 constructor is not a
table slot and still writes Original numeric profiles. No genuine Source
physical table or raw substream composition is admitted by this audit.

The smallest next missing OS-backed stream provider is **BF4FA0 at +2C**:
its complete 27-byte / 9-instruction body directly calls the real
`KERNEL32.dll!GetFileSize` import. It alone was expanded here. The existing
Source implementation is ordinary cdecl, not the Original receiver ABI.

## Boundary evidence

All captured bytes below were freshly compared between the installed PE and
the verified `C:/Users/sqz269/bsp.gpr` program
`/battlestationspacific.exe`:

1. Native constructor references publish **D691B0**, fixing the table start.
   The complete Original BF50D0 body and current production object identity
   were also rechecked against the prior constructor report, without rerunning
   that fixture or reclaiming the integrator's constructor lease.
2. The 28 bytes immediately before the table, D69194..D691AF, are the complete
   terminated string `cPhysicalDirectoryX86::exit`. A live data reference at
   BF4DA2 identifies its use by the physical-provider constructor. The final
   DWORD is ASCII `xit\0`, not a separately established RTTI locator. A raw
   backwards scan for executable-looking DWORDs would be misleading here.
3. All 26 words from D691B0 through D69214 point into the PE's executable
   section. Their second half, +34..+64, is byte-identical to the same offsets
   in memory profile D642C0 and adopted-stream profile D68DB0.
4. Original shader-cache code reloads a stream from `[EBX+8]`, loads its
   current table, reads **table+64 at B3BBF4**, and calls that entry at B3BBF7.
   The corresponding physical Source route checks D691B0/+64/BE4460 before
   invoking its ordinary counted-string writer. This confirms actual use of
   the high slot rather than relying only on adjacent pointer-shaped data.
5. **D69218 begins the complete terminated `cFileX86` name**. Original
   CD9045 stores that exact address in descriptor field 0109DC3C. Saved
   function metadata names this initializer `BSP_PhysicalFileType_InitializeStatic`;
   current `NativeStreamTypeIds::initialize_physical_00cd9030` likewise stores
   it as the name of the four-word physical descriptor at 0109DC30. The next
   aligned string starts at D69224 (`Error code=...`). Treating offset +68
   as another slot would consume the name's first word, 6C694663, which is
   outside the image's executable mapping.

Together, the constructor origin, complete adjacent strings, separately used
custom type descriptor, high-slot caller and shared tail establish the
104-byte table. Saved method names and prototypes are supporting labels,
not recovered source-class definitions. No standard MSVC RTTI Complete Object
Locator or class-hierarchy structure is established at vtable[-1]; the
observed name belongs to the game's custom descriptor system.

The CLI's function-comment and instruction-context queries cannot supply
typed data definitions for the table itself. This conclusion does not depend
on pretending those failed data queries produced a declared array extent.

## Complete slot inventory

“Ordinary” below means a current C++ service, context-bearing interface or
typed projection. It does not qualify an Original register-ABI table entry.
“No entry found” means no address-named Source declaration was identified;
no missing provider body was invented or expanded to fill the inventory.

| Offset | Original target | Current Source status |
| --- | --- | --- |
| +00 | BF55A0 | Ordinary recycle service with context. |
| +04 | BF5090 | Ordinary deleting-destructor service. |
| +08 | BF4EE0 | No Source entry found; no standalone live function definition. |
| +0C | BF4FF0 | Ordinary query-type service with actual ID storage. |
| +10 | 6F9D20 | Saved shared return-zero/RET8 leaf; no Source entry found. |
| +14 | BF4EF0 | No Source entry found; no standalone live function definition. |
| +18 | BF5020 | Qualified raw validity entry, separately from ordinary helpers. |
| +1C | BF4F20 | Qualified raw seek entry, separately from ordinary helpers. |
| +20 | BF4F40 | Ordinary physical-position interface. |
| +24 | BF5030 | Ordinary physical-read service with context. |
| +28 | BF4F50 | Ordinary physical-write interface. |
| +2C | BF4FA0 | Ordinary fresh-size query; selected next raw candidate. |
| +30 | BF4F90 | Ordinary cached-size interface. |
| +34 | BE42E0 | Ordinary/context-bearing DWORD readers; see ABI caution below. |
| +38 | BE4300 | Ordinary/context-bearing or typed DWORD readers. |
| +3C | BE4340 | Typed `MemoryStream` word reader. |
| +40 | BE4320 | Typed `MemoryStream` word reader. |
| +44 | BE4360 | Ordinary/context-bearing or typed float readers. |
| +48 | BE4620 | Ordinary counted-string readers with services/context. |
| +4C | BE4770 | Live unnamed function; no Source entry found. |
| +50 | BE40B0 | No Source entry found; no standalone live function definition. |
| +54 | BE40D0 | Ordinary physical DWORD writer with profile validation. |
| +58 | BE40F0 | No Source entry found; no standalone live function definition. |
| +5C | BE4430 | Ordinary diagnostic-string writer. |
| +60 | BE45F0 | Ordinary cache-string forwarding service. |
| +64 | BE4460 | Ordinary counted-string writer with physical profile/services. |

In particular, `read_native_raw_dword_00be42e0` and
`read_native_raw_float_00be4360` use naked assembly but expose explicitly
**context-bearing cdecl** interfaces. Their header states that these are not
native ABI thunks. Neither the word “raw” nor a naked implementation makes
them callable through the Original slots. Ghidra's fastcall prototypes
describe Original, not those Source service signatures.

The current normal dispatcher continues to read Original numeric words and
select ordinary helpers. Raw validity, seek and constructor symbols have
only their declaration/definition references in Source. `PhysicalFile` and
the typed `MemoryStream` functions are separate projections; this table
inventory does not promote their object layouts or virtual lifetimes.

## One expanded candidate: BF4FA0

Among the missing direct OS-backed I/O leaves considered, BF4FA0 is 27 bytes,
BF4F50 is 55 bytes and BF5030 is 84 bytes. Only their saved body bounds were
queried for selection; only BF4FA0's complete body was expanded. Destructor,
allocator and exception graphs were not expanded.

```text
00BF4FA0  PUSH ECX
00BF4FA1  MOV ECX,[ECX+8]
00BF4FA4  LEA EAX,[ESP]
00BF4FA7  PUSH EAX
00BF4FA8  PUSH ECX
00BF4FA9  MOV DWORD PTR [ESP+8],0
00BF4FB1  CALL DWORD PTR [00CE2114]
00BF4FB7  POP ECX
00BF4FB8  RET 4
```

Original takes ECX=actual receiver and one unused stack DWORD. It reads the
actual HANDLE at +8. PUSH ECX reserves the four-byte local whose address is
passed to `GetFileSize`; that local is initialized to zero before the call.
After the stdcall API removes its two arguments, POP ECX consumes the local
high-word result. EAX remains the API's low DWORD. RET4 discards the unused
caller argument. Incoming EDX is not a semantic input, although the API may
clobber volatile registers. No receiver field or cached size is written.

The installed import directory identifies CE2114 exactly as
`KERNEL32.dll!GetFileSize`; live callees agree. The sole absolute call operand
occupies bytes 19..22 of the complete 27-byte body. A future raw Source entry
must preserve all other bytes and bind that operand to its real linked
`__imp__GetFileSize@8` import, rather than substituting a callback or newer API.

Current `query_native_physical_file_size_00bf4fa0(void*, uint32_t)` is a
29-byte / 11-instruction ordinary cdecl function in the current production
COFF. Its receiver comes from `[EBP+8]`; it returns with plain RET. Its sole
COFF relocation is DIR32 at offset 23 to `__imp__GetFileSize@8`. It therefore
establishes a genuine ordinary OS-backed implementation, but cannot be cast
into the Original ECX/RET4 slot. Exact membership of its entire production
object in the current archive was verified.

Microsoft's [GetFileSize contract](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfilesize)
returns the low DWORD and optionally writes the high DWORD. FFFFFFFF can be a
valid low result or failure, distinguished using last-error information.
Original forwards that low result without checking the error or converting
it to a 64-bit return. Future native verification needs actual file handles
and the real API's behavior; this audit executed neither Original nor Source.

## Evidence and remaining limits

Fresh evidence is under
`local/cc12_physical_table_extent_readiness_20261008a/`. It includes all live
byte comparisons, the full table and shared tails, bounded native caller/type
publication windows, slot prototypes, current Source references, complete
BF4FA0 instructions, its actual import, current COFF and archive membership.
Forty-four relevant Source files, the installed image, current production
artifacts, prior reports and audit tools have stable pre/post hashes.

The tracked report is `reports/cc12_physical_table_extent_readiness.json`.
No Source/config/CMake/Ghidra changes, build, fixture replay, native call or
game execution occurred. The next bounded Source packet can add the distinct
raw BF4FA0 entry and its focused real-API verification. That would qualify one
more leaf, while complete Source table publication, remaining providers and
class/reference/handle/pool lifetime still require their own evidence.
