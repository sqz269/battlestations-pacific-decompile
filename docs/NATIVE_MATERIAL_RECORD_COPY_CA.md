# Native 300-byte material/renderer record copy construction (CA)

This packet reconstructs complete native bodies `00B13070..00B1317E` and
`00B0D230..00B0D24E` over actual records and vector storage. Descriptive
source names are hypotheses. The pinned installed PE is
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Live Ghidra project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, had 64,018 functions at evidence collection.

| Native body | Bytes | Installed PE SHA-256 | Original ABI |
| --- | ---: | --- | --- |
| `00B13070..00B1317E` | 271 | `b8043f36c93ebdb808cbe559333da42fca277d4f3c29ac3864d1b70a1e2952f9` | ECX destination, stacked source, EAX destination, RET4; preserves EBX/EBP/ESI/EDI |
| `00B0D230..00B0D24E` | 31 | `6b567ca16f817bfc0b82393d812857a5ca79ec837b2f57dbcd6ab7dfb2bfc79c` | ECX actual vector, EAX signed count, plain RET |

`00B13070` reads and writes DWORD0, then runs **five distinct forward**
`REP MOVSD` blocks of 14 DWORDs each at record offsets `+04h`, `+3Ch`,
`+74h`, `+ACh`, and `+E4h`. This copies the opaque prefix through `+11Bh`.
The blocks reload their base-relative addresses separately. The source keeps
those actual copy instructions under the native clear-DF convention; a single
bulk copy would change some overlapping-record results.

The constructor next forms destination/source header addresses at `+11Ch`,
compares the **addresses before either store**, and zeroes the destination's
length/data DWORDs. If addresses differ, it passes the current source length
with preserve=1 to complete raw `0041DD40`. After that callback it tests the
current source length; only when nonzero does it reread destination length,
source data, and destination data, in that order, for `00BF7680` `_memcpy`.
The latter has a backward overlap path, represented by `std::memmove` in the
source. The same address-guard-before-zero and fresh-field schedule applies
to second header `+124h`. Calling assignment body `00B13180` would miss the
constructor's zeroing on a self-header and is not used.

The first header is not armed for unwind while its own resize runs. Native
state 0 is stored at `00B13137`, after the first header finishes and before
zeroing the second. Handler thunk `00CBC1FE` loads FuncInfo `00DF42B0`
(magic `19930522h`, maxState 1), then jumps to `___CxxFrameHandler3` at
`00BF6B43`. Its unwind-map entry at `00DF42A8` is `{toState=-1,
action=00CBC1F0}`. Funclet `00CBC1F0` loads saved destination `+11Ch` and
tail-jumps to complete `0041DD20`. Accordingly, if second construction
throws, the source destroys the **current** first header through the concrete
`NativeStringRawPoolContext` provider. A first-header resize failure does not
invoke this cleanup. The raw provider resolves current `00419CC0` and
`00BD1510` storage when the header owns data. A C++ exception from cleanup
terminates as in the existing adjacent renderer-record source; SEH faults
remain outside the source compatibility claim.

`00B0D230` returns zero when vector `begin` at `+04h` is null. Otherwise it
reads `end` at `+08h`, subtracts `begin` with 32-bit wrap, and uses the
original signed magic multiply (`1B4E81B5h`) and sign correction for division
by the 300-byte record stride. There is no extent/order validation. Its only
live caller is `00B145E0`; the copy constructor has callers including
`00B132C0`, `00B13350`, and `00B13B00`.

The constructor's new EDX raw-pool context is not in the original ABI. The
source preserves ECX destination, stacked source, EAX destination, and RET4,
while MSVC emits its own C++ cleanup frame. It does not claim byte identity
for the native private FH3 frame, hardware-fault behavior, vector insertion,
or whole-game execution. Provider/source hashes and object/build validation
are recorded in `reports/native_material_record_copy_ca.json`.

MSVC Win32 Release and the existing CTest `reconstructed_math` passed. The
object's vector-size `.text$mn` COMDAT is exactly 31 bytes with zero
relocations and matches the installed PE byte-for-byte. The prefix helper is
a separate 90-byte COMDAT with zero relocations; its disassembly has DWORD0
and the five ordered `REP MOVSD` blocks. The generated header helper compares
addresses **before** zeroing, calls the raw resize provider, then rereads the
source length and the three memcpy fields in native order. Its constructor
arms the C++ unwind state after the first helper call and before the second.
The build, disassembly, header, and relocation logs remain in ignored
`local/output/` for review. These checks do not establish in-game behavior.
