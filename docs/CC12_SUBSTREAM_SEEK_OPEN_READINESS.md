# Substream open and seek raw-entry readiness

Addresses: 00BF1090, 00BF10E0, 00BF5020, 00BF4F20.

The selected physical file-stream route is **not ready** for raw Source
composition of BF1090 or BF10E0. Both concrete physical providers have complete
ordinary C++ implementations, but neither has a current raw original-register
entry. The new BF1080/BF10A0/BF10B0 raw queries remain separate memory-domain
leaves and do not establish a callable source object for these dispatch paths.
This is a read-only readiness audit; no production code, executable fixture, build registration,
ledger or Ghidra annotation was changed, and no old fixture was replayed.

## Installed bindings and complete Original bodies

The current installed PE and live verified Ghidra `bsp` program agree on all
four complete function spans below, the physical table, the substream slots,
and the seek IAT word. The configuration points to
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Body boundaries and
assembly were inspected directly; the saved prototypes alone omit the register
inputs. Names are reconstruction hypotheses.

| Entry | Complete span, inclusive | Bytes | Original ABI and behavior |
| --- | --- | --- | --- |
| BF1090 | BF1090..BF1099 | 10 | ECX substream; replace ECX with `[owner+8]`, load source table into EAX, load its +18h method into EDX, `JMP EDX`. No own return or stack cleanup. |
| BF10E0 | BF10E0..BF1123 | 68 | ECX substream; stack low/high/origin; EAX from source seek; `RET 0Ch`. Preserves ESI around the indirect call. |
| BF5020 | BF5020..BF5029 | 10 | ECX physical stream; no stack arguments; full EAX is exactly 0 or 1; plain RET. |
| BF4F20 | BF4F20..BF4F3F | 32 | ECX physical stream; stack distance-low/high/origin; full SetFilePointerEx BOOL in EAX; `RET 0Ch`. |

The physical table is D691B0: its installed words at D691C8 (+18h) and
D691CC (+1Ch) are BF5020 and BF4F20. Their live incoming xrefs are those
data slots. The substream table's D68DC8/D68DCC words are BF1090/BF10E0.
BF1090 and BF10E0 themselves accept the current source table; neither tests a
physical profile. The physical choice in this audit comes from the installed
concrete table, not an inferred restriction inside the receiver bodies.

BF1090 tail-dispatches without normalizing the provider result or returning
through a facade. The common interface consumes AL; for the selected physical
provider the complete BF5020 body is `XOR EAX,EAX; CMP [ECX+8],-1; SETNZ AL;
RET`, so the full EAX is 0/1. It tests only the stored HANDLE word against
FFFFFFFF. Zero and every other word compare as true; no OS validity query is
performed. Incoming EDX is not a semantic argument to this provider.

BF10E0 selects start for origin 0, current for 1, and end for every other
DWORD. It adds the input distance with wrapping ADD/ADC and publishes both
new current words at owner+20/+24 **before** loading the adopted source,
its current table, or its +1Ch method. The original PUSH ESI between ADD and
ADC preserves carry; the shifted stack offset still addresses distance-high.
At BF111E it calls the current source method with ECX=source and three stack
DWORDs: absolute-low, absolute-high, origin 0. Incoming EDX at that call is
absolute-high; EAX is the loaded method word. It forwards the provider's EAX,
restores ESI and executes RET0Ch. There is no rollback after a failed seek.

BF4F20 assembles the five stdcall argument DWORDs for SetFilePointerEx in
original order: HANDLE from source+8, signed-64 distance bits from the two
stack words, output pointer directly to source+10h, and the original full
origin DWORD. BF4F37 calls `[00CE22D8]`; the installed import directory names
that exact slot as `KERNEL32.dll!SetFilePointerEx`, matching the live Ghidra
callee. The API owns its five-DWORD cleanup; BF4F3D then pops the caller's
three DWORDs with RET0Ch. No output temporary, result narrowing, handle guard,
origin normalization or later GetLastError call occurs in this Original body.
No additional internal provider is hidden behind the imported OS boundary.

## Current Source readiness and producer boundary

`open_native_adopted_substream_00bf1090` and
`seek_native_adopted_substream_00bf10e0` remain ordinary interfaces taking a
`NativeAdoptedSubstreamDispatch&`. The callable adapter requires real host
functions with its documented original ABI; that requirement does not supply
such a function. Its numeric runtime implementation switches BF5020 and
BF4F20 to the ordinary physical helpers.

`valid_native_physical_stream_00bf5020(const void*)` reads +8 and compares
against FFFFFFFF. `seek_native_physical_stream_00bf4f20(void*, low, high,
origin)` calls the real SetFilePointerEx and uses the actual +10h output
storage. Their existing complete semantic reconstructions and historical
fixtures remain valid within their recorded contracts. Neither declaration or
definition is a naked/explicit original-register-ABI entry, and neither has a
current whole raw-body ABI qualification. The older `PhysicalFile` methods
also use a separate owning C++ object; its seek adds an error output and a
temporary position, so it cannot substitute for this raw provider.

The existing ordinary Source constructor `construct_native_physical_stream_00bf50d0`
writes numeric D691B0, reference count 1, INVALID_HANDLE_VALUE at +8, zero
position and size, and leaves +Ch untouched. This establishes its documented
raw20h storage layout for ordinary Source services. It does not change the
installed slot words into addresses of the current linked raw Source methods
or prove Original class/virtual-call admission. No constructor, handle producer,
physical provider open path, allocation or runtime binding was executed here.

Current numeric dispatch reads the table word and calls a selected C++ helper;
BF1090's literal JMP and BF10E0's literal CALL execute the table-selected code
address directly. Those are distinct contracts. A manufactured vtable,
numeric-profile backing, host callback, or test-only dispatcher would not close
the genuine-source admission gap. This audit does not qualify a producer path
for a raw callable physical source. Other source families are outside this
bounded physical-table audit.

## Smallest next packet

The first whole missing provider for open is **BF5020, 10 bytes**. A bounded
next implementation can preserve the existing helper and add a distinctly
named raw Win32 entry with ECX actual backing, unused EDX, full EAX 0/1 and
plain RET. Its body has no calls, globals or relocations, so all ten bytes can
be checked literally. That would qualify only the live +8h word memory domain;
it would not itself qualify BF1090's source owner or table.

Seek independently stops at **BF4F20, 32 bytes**. It needs a whole raw provider
entry and a separately evidenced genuine SetFilePointerEx IAT binding while
retaining direct +10h output storage, full BOOL and RET0Ch. The existing
ordinary helper is not that raw entry. No receiver wrapper should be treated
as composed until both its required provider and a genuine callable source
producer/table path are admitted. This audit stops at these first missing
whole providers and does not expand through pool, VFS or archive construction.

Evidence and input hashes are in
`reports/cc12_substream_seek_open_readiness.json`; static capture artifacts are
under `local/cc12_substream_seek_open_readiness_20261008a/`. The report-call
checker validates containing functions and the two real provider starts; the
installed table words independently resolve its indirect edges. The OS import
is resolved from the installed PE directory. No compile, runtime fixture,
ABI execution or game validation is claimed by this documentation packet.
