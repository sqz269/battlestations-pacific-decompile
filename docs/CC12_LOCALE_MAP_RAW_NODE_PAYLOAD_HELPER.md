# Dormant raw locale node payload cleanup

`destroy_native_locale_map_node_payload_00737ce0(void*, NativeStringRawPoolContext&)`
now composes the existing raw wide and narrow header destructors. It is compiled
into Core and has no application caller. The only source changes are the new
header/implementation and one `startup.cmake` Core source-list entry.

The helper retains the original node and supplied context in a local armed
`NodeNarrowUnwind`, calls raw wide cleanup on `node+8`, clears the guard's armed
flag, then calls raw narrow cleanup on the original node. The guard's
`noexcept(false)` destructor calls raw narrow cleanup only while armed. It follows
the accepted local `NameUnwind` pattern without adding a public guard mechanism,
another publication, pool owner, lifetime domain or callback family.

## Contract and evidence scope

Root accepted the bounded Source readiness in `cf3f1dc18`; its whole committed
report and the 24,769-byte Root working report (SHA-256
`4c63ccebc92fc02659f6aa5686eee2106f6ada07b0d7f2e9cd08f4572469b348`)
are retained. The original complete `00737CE0` body is 119 bytes, with `ECX=node`
and plain `RET`. Its selected `C86260 -> 0041DD20` cleanup correspondence still
depends on the previously stated external `EBP=entry ESP` premise. This normalized
two-argument C++ API does not reproduce that calling convention or exception frame.
No fresh Native query or Ghidra mutation was performed.

The caller keeps both headers, the original node, supplied context object, and
its publication-cell references valid through normal return or cleanup. Provider
mutations are observed in their accepted order; uncontrolled concurrent mutation
and invalid raw reads are outside the contract. The context continues borrowing
the canonical `01090AA8` pool, `01090AA4` live return gate and `01090AA0` raw-manager
cells. No storage bridge or cached pool is introduced.

Wide cleanup captures its own pointer and wrapping `2*length+2` size before its
getter. The subsequent normal or unwind narrow call reads the original node's
current narrow header and captures wrapping `length+1` before a separate getter.
The node helper adds no header reads, snapshots, writes, zeroing, alias
deduplication, overflow checks, wide cleanup repetition, map clearing, receiver
freeing or node-slot recycling.

If the wide getter throws a C++ exception, the armed guard performs current
narrow cleanup. If that cleanup returns, the original exception propagates.
A normal narrow getter failure occurs after disarming and does not repeat cleanup.
A narrow getter throwing during C++ unwind terminates; native double-exception
parity is unproved. The existing `00BD1510` Source return provider remains
`noexcept`, so exceptions attempting to cross it are outside the covered
propagating-getter domain. No catch-handler replacement, swallowing or SEH
translation is added.

Native pointer loads occur on specific sides of the native state stores: wide
pointer before state 0, current narrow pointer before state -1. Existing Source
header helpers own those reads, so the ordinary C++ scope does not reproduce
their fault boundaries. This was an explicit readiness constraint, retained in
the public header and [readiness report](CC12_LOCALE_MAP_RAW_NODE_HELPER_READINESS.md).

## Complete emitted C++ paths

The normal function is 103 bytes / 37 x86 instructions. Its complete body and
relocations show:

| Normal section offset | Emitted operation |
| --- | --- |
| `+27h..30h` | Retain original node in `EDI`/`[EBP-18h]` and the same context in `ESI`/`[EBP-14h]`. |
| `+33h` | Set the guard byte `[EBP-10h]` to 1. |
| `+37h` | Form `node+8` without reading a header. |
| `+3Ah` | Set the compiler's EH state to 0. |
| `+43h` | Call raw wide cleanup; REL32 operand at `+44h`. |
| `+4Ah` | Clear the guard byte to 0. |
| `+4Eh` | Call raw narrow cleanup with the original node and same context; REL32 operand at `+4Fh`. |
| `+66h` | Return after restoring the emitted Source frame. |

The compiler **does not write EH state -1 before the normal narrow call**. Its
metadata can still select the unwind funclet, but the cleared guard byte prevents
a second raw narrow call. Disarming is therefore the emitted guard flag, not a
claim that the Source EH state schedule is identical to the original.

The complete selected exception path is retained and resolved:

- The 8-byte / 2-instruction unwind funclet forms the guard address
  `[EBP-18h]` and tail-jumps to its destructor.
- The complete 20-byte / 7-instruction destructor tests guard `+8`. Zero branches
  directly to `RET`; nonzero passes captured context `+4` and original node `+0`
  to raw narrow cleanup. Its REL32 operand is at `+0Ch`.
- The complete 42-byte exception code section contains that funclet, five `CC`
  padding bytes, and a 29-byte / 9-instruction handler. All handler relocations,
  including security-cookie checking, FuncInfo and `__CxxFrameHandler3`, are kept.
- The complete 44-byte `.xdata$x` section consists of one 8-byte unwind row
  (previous state -1, relocated action = selected funclet) and 36-byte FuncInfo
  (magic `19930522`, one state, relocated unwind table, remaining literal fields).
  The 4-byte `.sxdata` entry resolves to this Source handler's exact COFF symbol.

This includes complete sections, symbols and relocation targets, not only entry
prefixes or symbol-name sightings. All 13 zero-raw-pointer sections in the retained
selected cohorts have empty file payloads with their declared storage sizes;
object-header bytes are never substituted for BSS.

## Build and cohort qualification

The normal worker `scripts/build.ps1` passed the three existing checks:
`reconstructed_math`, `native_math_differential`, and `tool_tests`. No test or probe
was added, and the new helper was not executed in a fixture or game. The build
and static COFF evidence do not establish native ABI or gameplay equivalence.

The worker merged current main before capture and built from
`f339c5560d73bfca1ef8b0432fb02f253fe7c50c` plus the three owned source changes.
The frozen Root baseline remains `Source3352-native-wide-raw-context`: all 3,352
selected inputs and four artifacts matched live Root at the pre-edit capture.
Worker-before inputs were all exact to their actual Git blobs; 3,345 matched Root
bytes and all 3,352 were LF-equivalent. The guard-pattern Source is separately
retained outside that selected manifest.

The worker's new selected Source3354 epoch adds the new header and implementation;
among the prior 3,352 inputs, only `cmake/startup.cmake` changes. All 3,351 other
worker-before inputs remain exact. Against the prior Root source cohort, 3,345
old inputs remain byte-exact and 3,351 LF-equivalent. The complete before/after
input cohorts are retained; these selected manifests do not claim to enumerate
every source in the generated project.

Core has 1,989 object members after the build, with exactly one added object.
All 1,988 old object payloads are byte-identical to worker-before. In particular,
the entire raw wide, raw narrow and existing loading-queue guard-pattern objects
are unchanged. The full raw wide (57-byte) and raw narrow (51-byte) function
sections match bytes and relocations across frozen Root, worker-before and
worker-after. Compiler dependency logs identify only the new implementation as a
consumer of the new header.

All 80 actual worker `bsp_game.dir/Release` objects remain byte-identical and
contain no new helper symbol. This is a worker directory census, not a borrowed
Root App census or a claim that every directory entry belongs to the current
generated project. All four retained worker artifacts changed, including Core,
the game executable/map and test log; no unchanged-artifact claim is made.

## Portable proof and remaining work

`local/cc12_locale_map_raw_node_payload_helper/evidence.zip` contains 3,613 pinned
files, 83,810,188 bytes, SHA-256
`7c03c8b06889b72a6f1a1e045030d9cb008c64df8bfbe161efde999282202b73`.
It retains whole Core archives and indices, complete selected COFF objects and
all their sections, Source/Git cohorts, actual App objects, compiler logs,
accepted whole reports and finite reused Native dependencies. The ZIP-only review
passed under a read-only audit guard and again from a physical copy with its
extracted scripts; external file/process/network/dynamic-loading/directory access
was denied. The review performs no compiler or target execution.

Root review, its independent build and integration remain pending. No map/base/
locale lifetime composition, node-slot recycling, application caller switch,
Native/Ghidra or ledger mutation is included. Machine-readable details:
[report](../reports/cc12_locale_map_raw_node_payload_helper.json).
