# Ordinary retirement unwind map: Native readiness

The exact sixteen candidate bytes at `00E01CE8..00E01CF7` are `FF FF FF FF 50 72 CC 00 00 00 00 00 58 72 CC 00`. Four raw little-endian DWORDs at offsets 0, 4, 8 and 12 are `FFFFFFFF`, `00CC7250`, `00000000` and `00CC7258`.

Only under the already qualified classic MSVC x86 nonrelative absolute `UnwindMapEntry` comparison, these form two eight-byte entries:

| Candidate state index | Entry address | Candidate signed toState | Candidate action-address value |
| --- | --- | --- | --- |
| 0 | `00E01CE8` | -1 | `00CC7250` |
| 1 | `00E01CF0` | 0 | `00CC7258` |

This is a conditional declared state path `1 -> 0 -> -1`. It does not show actual state writes, traversal, calls, cleanup, or retirement. The comparison comes only from the accepted [descriptor Root contract](CC12_APPLICATION_DESCRIPTOR_PRIMARY.md) and its report. Older compiler-definition captures remain opaque; no installed header or web source was read. Original compiler/runtime identity, FH3 authenticity, frame ABI and invocation remain unresolved.

Root reviewed the initial typed metadata for every byte before admitting the exact sixteen-byte reads. All 48 responses from three batches agree: a defined DataDB listing unit starts at `00E01CE8`, reports length 16, and contains all sixteen sites; only its first address is a listing-unit start. Instruction, function, flow and target metadata are absent throughout. Listing coverage is the basis of the bounded reads; explicit memory-block, backing and type proof remains absent.

Both exact data requests returned identical bytes. Their full retained outputs are BSP CLI formatted text, not raw HTTP captures. Typed responses retain full raw/base64 forms and hashes. Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, epoch 98 and count 64,730 close unchanged across three metadata batches and six count calls. No instruction text, candidate action, other pointer or neighbor was queried.

The report pins a narrow evidence ZIP, all five helper bodies, current query-tool sources, immediate Root contracts and admission receipt, exact captures, and frozen Git object evidence. Replay passes on the original ZIP and a distinct physical copy under a read-only guard that denies post-preload imports. Actual frozen HEAD, `src`/`include` subtree identities, twelve prerequisite Git blobs, and exactly two staged/final output objects are checked by retained receipts. There is no Source content census, recursive old archive replay, or transfer of previous artifact/build/runtime credit.

Both candidate action addresses remain unread. Further action-target metadata or data needs a separate bounded Root admission. No remaining descriptor bytes, Ghidra mutation, C++ change, test, build, probe, process, game, OS or audio action is part of this packet.

Evidence: [machine-readable report](../reports/cc12_application_ordinary_retirement_unwind_map_readiness.json). This is Native listing/data readiness and a conditional table comparison only.
