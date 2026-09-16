# Native render-entry cache adoption (R49)

R49 adopts the reviewed render-entry cache component from commit
`9ed3a6de77995315a41980b6941cdcee4fb4e22b`. The header and implementation are
byte-identical to that commit. Current main supplies every lower provider used
by the component; this packet adds only the `bsp_core` source registration.

The component borrows the process cells `01090AA0` (singleton manager),
`0108FE88` (current cache), and `00D7A24C` (the DWORD value one) through
`NativeRenderEntryCacheContext`. It does not allocate a renderer or camera,
publish an application owner, or call the window fragment from production.

## Native evidence

Fresh live-Ghidra bytes from `C:/Users/sqz269/bsp.gpr` program
`/battlestationspacific.exe` equal the installed PE bytes for all 795 claimed
bytes:

| Native range | Bytes | SHA-256 | Source boundary |
| --- | ---: | --- | --- |
| `00BEBF00` | 39 | `4824475a24b6d8b067f066d519320f11ce64ca22567cd307430518200122c58c` | initialize one `28h` record |
| `00BEC240` | 10 | `e74511a9ae745a98c66be3eda107a3f202c02e3527ae663903e9bd0cb60e6def` | free array data only |
| `00BEC3E0` | 154 | `888ec947b5c396015b66687b838f4dd50b5c2262e51c9e86cf67b235e4ca5cbc` | resize record array |
| `00BEC590` | 145 | `46bb43b73455a8b62cfafafe6b7af6c56f9cc088adbac62e0f02ac62ee40ef3c` | publish/register base |
| `00BEC630` | 153 | `0aa6d03a277e63853289c6654c4a81861c2e1c93bd289f13a09d186411945fd9` | unregister/clear base |
| `00BEC6F0` | 30 | `9457543553a38722ca67cdc17fe2db921681c5195381ffe9a2ebb91c74881317` | scalar base delete |
| `00BEC870` | 110 | `3b9fbbaa28ff660c0678b5cd5bac7cf8fba931a91016cc551aa1bedbd8cbbe34` | construct derived cache |
| `00BEC8E0` | 37 | `25a3b7c4228adb3219e27d08767abe7ced61aaf323ae0164aac0d664efe436fd` | destroy derived cache |
| `00BEC910` | 58 | `da0de365558953df99d7496403df7175a6492619ca1ff1dede9599bb088f2db4` | scalar derived delete |
| `00BED1E8..00BED222` | 59 | `9f477e2b48557a26c845296d7f1db86330f520215fad06c8bf36ffe657d05780` | bounded `00BECEE0` window fragment |

The installed executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The report records every direct and indirect call decoded from these spans.

## Preserved contracts

- `00BEC3E0` compares the request with the used count at header `+4`, not the
  capacity at `+8`. A zero request after a mismatch frees the allocation and
  retains the stale pointer while clearing count and capacity.
- `00BEC870` captures the old count before clearing the data pointer. When the
  old count is zero, the old capacity survives. When it is nonzero, the native
  order clears count/capacity, frees the current data pointer, and clears
  capacity again.
- Base publication captures the first manager's section, then reloads both the
  manager and `0108FE88` for registration. Removal follows the corresponding
  current-manager/current-publication schedule.
- The bounded window helper allocates `14h`, constructs only when nonnull,
  reloads `0108FE88`, disarms captured-owner cleanup before resizing to 10,000,
  and therefore retains a registered owner if resize throws.
- Source cleanup objects preserve the reviewed native arming order. They model
  ordinary C++ exception cleanup; they are not original FH3/SEH metadata.

## Validation

MSVC Win32 compiled the unit with `/MD /W4 /WX /fp:strict`. The complete build
passed the three existing CTests. An ignored focused probe linked the rebuilt
`bsp_core.lib` and exercised a genuine raw singleton manager, registration,
10,000 initialized `28h` records, the used-count comparison, stale zero-request
pointer, old-capacity preservation, and explicit scalar destruction. It also
executed a relocated copy of the original 39-byte `00BEBF00` leaf and compared
the complete 40-byte record against the source result.

The earlier historical fixture is retained as provenance and is not presented
as the current focused fixture. That historical run composed a full renderer
and device. R49 does not repeat or inherit its renderer, window, or gameplay
claims.

## Boundary

This is a source component with reviewed behavior and focused runtime evidence.
Its C++ interfaces are not original register/stack ABI, and the fixture does
not execute the original constructors, resize body, FH3/SEH paths, all of
`00BECEE0`, an active renderer frame, application startup/shutdown, or gameplay.
No production path activates the cache in this packet.

Machine-readable receipts and artifact hashes are in
`reports/native_render_entry_cache_main_r49.json`.
