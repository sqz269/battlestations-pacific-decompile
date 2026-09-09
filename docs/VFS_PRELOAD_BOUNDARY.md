# Native FileStore preload and trace boundary

The nine live direct calls to `00be7ab0` select **five startup script files and four menu audio files**. These callers use literals and a fixed extension substitution, not an enumerated configuration table. They do not establish native preloading of the three font resources used by the diagnostic. Keep that font priming labeled diagnostic.

Read-only investigation on 2026-09-09 used `Client.verify()` before every live batch, confirming project `bsp`, program `/battlestationspacific.exe`, x86 and image base `00400000`. The assigned VFS code range was `00bdb2e0..00be882f`; the primary agent authorized the two complete caller bodies `0073d410` and `00686380` plus directly referenced data. Six primary routines were inspected, with a small completion adapter as supporting context. No Ghidra names, comments, functions or C++ were changed.

Machine-readable evidence, byte hashes, ABI notes and a proposed trace annotation are in [vfs_preload_boundary_audit.json](../reports/vfs_preload_boundary_audit.json). Raw exports and batch target checks are under ignored `exports/bsp/parallel_vfs_preload/`.

Integration follow-up: the primary agent independently rechecked the trace span,
named it `BSP_VFS_LogOpenedResource`, and corrected the caller's earlier
stream-tracking description. Prior comments were preserved, the project saved
and affected exports refreshed. This is analysis metadata, not a preload port.

## Direct population callers

Live function/xref queries return two callers and nine call sites. The disk callgraph independently contains those same two callers. No function-pointer data reference to `00be7ab0` appeared in the live xref result; this is not a proof that arbitrary indirect calls cannot exist.

`BSP_Application_Initialize` (`0073d410`) constructs these exact string literals and performs this sequence on the normal path through `0073e1e0..0073e45b`:

| Order | Cache call | Literal address | Requested resource | Flags |
|---:|---|---|---|---:|
| 1 | `0073e239` | `00cfeb80` | `scripts/datatables/inputs.lua` | `0x32` |
| 2 | `0073e2b7` | `00cfeb58` | `scripts/datatables/keyboardsetup.lua` | `0x32` |
| 3 | `0073e335` | `00cfeb2c` | `scripts/datatables/controllerinputnames.lua` | `0x32` |
| 4 | `0073e3b3` | `00cfeb04` | `scripts/datatables/controlpresets.lua` | `0x32` |
| 5 | `0073e431` | `00cff188` | `scripts/datatables/scoring.lua` | `0x32` |

Each call obtains the FileStore factory via `004fc150`, obtains its store through `00be80b0`, then puts that returned store in ECX for `00be7ab0`. The flags are actual `PUSH 0x32` instructions, not the decompiler's missing/incorrect argument reconstruction. There is no basename search at these call sites.

`FUN_00686380` is provisionally interpreted as main-menu setup: it logs `GVMainMenu`, initializes menu objects and performs the following audio cache sequence. Its only live reference is a function-pointer data entry at `00cf7750`, freshly verified to contain `00686380`; the outer virtual-call trigger was not recovered here.

| Order | Cache call | Requested resource | Flags |
|---:|---|---|---:|
| 1 | `00686476` | `sound/music/titlescreen.def` | `0x32` |
| 2 | `006864f5` | `sound/music/titlescreen.fsb` | `2` |
| 3 | `0068659e` | `sound/music/creditsfinal.def` | `0x32` |
| 4 | `0068661d` | `sound/music/creditsfinal.fsb` | `2` |

The `.fsb` literals are at `00cf77a4` and `00cf777c`. The `.def` requests are constructed by taking the original string minus its final four characters and appending the literal `.def` at `00cf779c` (`0068642f..00686459`, `00686556..00686581`). This interpretation uses the caller assembly and existing cached substring/concatenation evidence for `00469840`/`004261a0`. It is a fixed derivation from two fixed names, not a discovered asset manifest.

## Population primitive and smallest policy unit

`00be7ab0` remains **ECX store; name, flags on stack; RET 8**, ending at `00be7b17`. Its complete 106-byte body again matched the saved Ghidra image and installed PE, SHA-256 `7b91fe3afa47a5a50baa1944ac0ce230a16cc5baba7ed3d322efdf6a31384e9b`.

It opens the supplied name through global manager `0109ceec` virtual `+4`, obtains a memory view through `00bef750`, inserts through `00be7760`, then releases both temporary references. Opening precedes duplicate rejection. Insertion retains the supplied-name key, while manager opening normalizes and applies aliases to its own copy. Existing first-insertion-wins and lifetime boundaries remain those documented in [MOUNTED_RESOURCE_STREAMS.md](MOUNTED_RESOURCE_STREAMS.md).

The smallest native **startup policy** projection is an ordered five-request batch carrying the table's exact names and `0x32` flags to an explicit population operation. It should be invoked at the corresponding application initialization phase, after mounts/factory availability, without appending font assets or substituting search results. Menu audio population is a separate four-request operation at the menu setup trigger.

That policy can be reconstructed without inventing a preload list, but it is **not yet a faithful replacement for the current font-cache diagnostic**. The host `cache_resource_00be7ab0_fragment` and mounted-provider callbacks currently project flags `2` only (`include/bsp/mounted_streams.hpp`, `src/mounted_streams.cpp`). Before connecting native batches, preserve or establish the semantics of `0x32` through the complete chosen provider route. The cached physical-file body `00bf52a0` uses bit 0 and mask `0xE`, but intermediate wrapper `00bf5590` and other provider handling were not audited here; do not infer that the extra bits are globally ignorable.

## Open tracing is not cache selection or stream ownership

`00bde9c0` is a conditional opened-resource **trace** helper, not a FileStore population or stream-retention routine. Its original ABI is **ECX manager; name, stream, selected-mount payload byte as three stack arguments; RET 0xC** at `00bdeb36`. The complete 377-byte body matched disk, SHA-256 `dbab7cc9478aab0e0b2e85391cf1ec6946539c7aff40303cfe964f5c1d8085a8`. The stream argument is not consumed by the inspected body.

It emits a builder sequence equivalent to `<FILE><name>` only when global `0109cee8` is nonzero, manager byte `+79h` is set, the third argument's low byte is nonzero, and a filename-pattern suppression condition is false (`00bdeac4..00bdeb1f`). The suppressed pattern is an underscore followed by seven ASCII digits immediately before the last dot; both helper searches require positive indices. Constants `.`, `_`, `<FILE><` and `>` were freshly compared with disk. Existing cached `00467cf0` establishes backward substring search; general locale/string behavior was not newly validated.

At successful read-mode open, `00bdf310` calls the helper with the normalized/aliased name, returned stream and selected mount payload (`00bdf43a..00bdf44b`). Cached, previously byte-audited callback `00bda690` shows that this payload comes from matched mount node `+0Ch`; its full meaning remains separate from the observed trace gate. The caller increments manager `+28h` and adds the stream's virtual `+2Ch(0)` EAX result to manager `+2Ch` (`00bdf45a..00bdf46a`). Those are successful-open/size counters, not observed bytes read. Successful bit-0 opens instead increment `+24h`. Failed opens do not enter these success counters.

A separate bounded implementation can expose the exact trace predicate and successful-open accounting with an explicit real sink/observation boundary. Do not silently stub the builder sink, turn tracing into preload discovery, or introduce stream ownership from this helper's unused stream argument. Parent-owned annotations should correct the older “stream tracking” interpretation while preserving the previous comment history. The proposed descriptive name is `BSP_VFS_TraceOpenedResource`; it is a reconstruction name, not a recovered symbol.

## Separate pending-completion path

The only other live direct caller of `00be7760` is `00be78b0` at `00be7933`. This body has **ECX store; name, opaque callback argument, stream; RET 0xC**. Assembly resolves its misleading decompiler register/stack output:

1. Normalize a local copy of the name and find a pending entry in the secondary tree at store `+20h` (`00be78d2..00be78f2`). The native valid path requires an existing entry; invalid-parameter calls are not graceful “not found” handling.
2. Read its callback at node `+14h`, then remove the pending record (`00be7912..00be7922`).
3. Insert the supplied stream through `00be7760` under the normalized name (`00be7927..00be7933`).
4. Invoke the saved callback with the **original name argument** and opaque argument (`00be7938..00be7942`), then destroy the local normalized string.

The small adapter `00be7b20..00be7b3e` receives `(stream, name, opaque argument)`, obtains factory `004fc150`'s store at `+8`, and forwards `(name, opaque argument, stream)` to the completion routine. Live xrefs place that adapter's registration/reference at `00be7ee4` in **`FUN_00be7cd0`**. The latter is the next bounded submission/selection investigation; its body was not expanded in this lane. This pending path prevents claiming that the nine direct `00be7ab0` calls describe every possible FileStore insertion, or that native font caching is absent.

## Validation boundary

Eight selected code spans, twelve string/format spans and one menu-entry pointer matched the current installed PE and saved Ghidra bytes. This includes complete cache, menu, trace, open and completion bodies; application startup was byte-checked only around the decisive preload block. Fresh target checks, live xrefs and raw assembly support the claims above. Cached dependency evidence is identified where used. There was no original-game execution, build, fixture test, preload runtime trace or proof of complete provider/async behavior in this read-only lane.
