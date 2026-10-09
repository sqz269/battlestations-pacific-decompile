# Model descriptor CRT placement readiness

The original model initializer has a verified static-table position: cell
`00CE3568`, zero-based slot **909**, points to `00CD7E60`. Current original-PE
and live-Ghidra data establish the represented type order
**camera `00CD7D80` -> model `00CD7E60` -> animation `00CD82F0`**.
This closes the earlier placement gap for a qualified Source subset insertion.
It does **not** admit faithful CRT execution or complete shared-counter/ID parity.

The immediate next entry, `00CD7EB0`, is an already reconstructed model-base
descriptor initializer which consumes the same type counter. Production does
not currently call it. Adding the model entry alone therefore leaves a concrete
counter consumer omitted; the other uninspected intervening entries are not
assumed harmless. No composition or startup change is made by this packet.

Machine-readable evidence: [report](../reports/cc12_model_descriptor_crt_placement_readiness.json).
Source baseline: `5615ccb5ac429256e512b0a59babae976e27656c`.

## Current data evidence

The configured original executable is
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The live program is `/battlestationspacific.exe`, x86 little-endian 32-bit,
image base `00400000`. The Java process command line identifies
`C:/Users/sqz269/bsp.gpr`; the approved `bsp.py ghidra` queries verify the
configured project/program before each batch. No Native function body was opened.

The initial xref query returned `00CE3568 [DATA]` for `00CD7E60`.
After separately approved metadata reads, camera and animation xrefs returned
`00CE3558 [DATA]` and `00CE35B4 [DATA]`, respectively. Three separately bounded
byte reads cover exactly **28 bytes / seven DWORD cells**. Their raw bytes match
both the installed PE and live saved analysis:

| Address interval, end exclusive | PE file offset | Bytes |
| --- | --- | --- |
| `00CE3558..00CE355C` | `008E3558` | `80 7D CD 00` |
| `00CE3560..00CE3574` | `008E3560` | `F0 7D CD 00 40 7E CD 00 60 7E CD 00 B0 7E CD 00 00 7F CD 00` |
| `00CE35B4..00CE35B8` | `008E35B4` | `F0 82 CD 00` |

All selected bytes are in `.rdata`. Their decoded pointers are:

| Cell | Zero-based slot | Pointer | Existing meaning |
| --- | ---: | --- | --- |
| `00CE3558` | 905 | `00CD7D80` | Camera descriptor; production type subset. |
| `00CE3560` | 907 | `00CD7DF0` | Unopened target; no exact indexed function entry. |
| `00CE3564` | 908 | `00CD7E40` | Existing mesh pool startup. |
| `00CE3568` | 909 | `00CD7E60` | Model descriptor; absent production call. |
| `00CE356C` | 910 | `00CD7EB0` | Model-base descriptor; absent production call. |
| `00CE3570` | 911 | `00CD7F00` | Existing reconstructed model pool entry. |
| `00CE35B4` | 928 | `00CD82F0` | Animation descriptor; production type subset. |

No bytes from `00CE355C..00CE3560` or `00CE3574..00CE35B4` were interpreted.
No target body, table sweep, new Ghidra annotation, profile or numeric type ID
was recovered or invented. Function names identify existing Source hypotheses.

## Table membership and conditional order

The pinned [earlier CRT route receipt](CC12_PENDING_GROUP_CRT_INITIALIZER_ROUTE_READINESS.md)
is reused as **historical instruction evidence**, not replayed here. Its report
SHA-256 is `230a27ad3c9b78b37fc072483fbe9c7dccee7354a613cbf413949913eba48bef`
and its whole-image identity matches the executable hashed above. It establishes
`__cinit`'s void-table bounds `[00CE2734,00CE36E4)`, 1004 DWORD slots, and the
ascending four-byte traversal. Every iteration reloads its current pointer,
skips zero, otherwise calls it without a pushed argument; the loop does not
use a child EAX result as an early-exit test.

That historical contract supplies membership and traversal semantics; the new
28-byte reads supply current cell contents. `(00CE3568-00CE2734)/4 = 909` is
aligned and within the accepted bounds. Thus the model lies immediately after
the mesh-pool cell and immediately before the model-base cell, with camera
before it and animation after it. This is pointer-cell order, not a conclusion
from the numerical order of function addresses.

Invocation still depends on the prior integer-initializer gate returning zero,
earlier children returning with usable stack and preserved loop registers,
and each freshly read cell retaining the captured target. The previous receipt
owns that conditional pre-WinMain route. No Original or Source startup execution
is asserted by this read-only packet, and no boundary or CRT body was reread.

## Production subset and missing counter consumers

`GameNativeTypeStorage::initialize_resource_types`, lines 39-54, currently calls
three selector entries, then camera, then animation/bone, then the mesh family.
The header already limits this to represented families and disclaims original
absolute IDs. `GameNativeVfsApplication::Impl` owns the actual type storage and
common bootstrap; it passes the same `owner_services.types()` counter at core
initialization. The [prior storage/lifetime readiness](CC12_PRODUCTION_MODEL_DESCRIPTOR_BOOTSTRAP_READINESS.md)
remains the composition contract for borrowed cells, sticky guards and retention.

The now-supported **subset** position is a model static call after the current
camera call and before the current animation call, using retained model cells,
the existing counter and the existing common bootstrap. This is not permission
to execute it in a constructor, create another counter, choose fixed IDs, reset
guards, use an alternate lazy target or move unrelated process pools.
The current game main initializes pools separately from the VFS type subset;
this candidate position does not recreate the full original CRT interleaving.

The decisive omitted consumer is model-base `00CD7EB0` at the very next table
cell. Its existing Source, `src/native_model_base_bootstrap.cpp:17-30`, sets its
own guard/name, initializes the shared node as required, captures parents,
gets the shared counter, increments it and publishes the old counter as its own
ID. Its header requires the same counter lifetime as the shared bootstrap.
The bounded repository search finds only declaration and definition, with no
production call. On its cold guard path it consumes an ID; on a nonzero guard
it returns. Neither guard state nor execution is asserted here.

Consequently model-only insertion is insufficient for full counter parity.
It is unnecessary to open further children to establish that limitation. The
unread cell at slot 906 and slots 912-927 can contain additional dependencies;
their behavior, guard state and counter effects remain unknown. Complete parity
would need those and the broader schedule, including earlier lazy initialization,
under a separately bounded task. This packet deliberately stops at the approved
anchors. Actual model construction/publication, atlas binding and numbering
admission remain separate open work.

## Validation and limits

The exact seven pointers, live/PE byte equality, full original-image SHA,
slot arithmetic, accepted receipt identity, Source callsites and all report
input pins were checked. Both output files were fully reviewed and checked with
`git diff --check`. No C++/CMake, ledger or Ghidra mutation, build, test, probe,
Native execution, startup or gameplay validation is performed. The earlier
Source121/507 build receipts are historical; Root's newer build/smoke status is
not independently replayed or credited here. This result admits static relative
placement and a qualified Source subset proposal only.
