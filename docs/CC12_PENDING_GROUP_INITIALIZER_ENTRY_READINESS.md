# Pending group initializer entry: read-only candidate

The original image contains a complete linear **153-byte / 22-instruction**
candidate at `00CD27C0..00CD2858`. It connects five sentinel pairs at group
stride `68h`, with each tail at head+34h. A separately checked read-only word
at `00CE3020` contains its entry address. Ghidra currently recognizes neither
the function nor instructions at its entry/return; its saved analysis was
preserved. This is recovered-byte evidence with zero Source or runtime credit.

The former nearest-index lookup at `00CD27D6` pointed to `00CCFFE0`, whose
actual saved body ends at `00CD002C`. That predecessor does not enclose this
store. The new audit does not infer a function boundary from that lookup.

## Complete candidate and exact stores

The first instruction is `XOR EAX,EAX`. For each of the five pairs below,
in ascending order, four distinct DWORD stores occur in this exact sequence:
head+4=0; head+8=tail; tail+8=0; tail+4=head. A plain `RET` follows at
`00CD2858`. There are no calls, branches, pushes, local frame or x87 operations.

| Index | Head | Tail |
| --- | --- | --- |
| 0 | `00F876C0` | `00F876F4` |
| 1 | `00F87728` | `00F8775C` |
| 2 | `00F87790` | `00F877C4` |
| 3 | `00F877F8` | `00F8782C` |
| 4 | `00F87860` | `00F87894` |

On ordinary return EAX remains zero; ECX, EDX and nonvolatile registers are
untouched, and ESP advances by the return-address word. The first XOR supplies
the final arithmetic flags: CF/OF/SF=0, ZF/PF=1, AF undefined. MOV/RET preserve
them. These machine residuals do not establish a semantic Source return type.

The body writes twenty DWORDs. It does not write either sentinel profile,
counts, intervening fields or all other group storage, and performs no general
zero-fill. Selected group addresses lie in the writable `.data` virtual tail
beyond its file backing. That section fact alone is not live initialization
or startup-order proof. The maximum direct write ends at `00F8789F`; five full
68h object extents or a complete class layout are not established by these stores.

## Entry evidence and diagnostic scope

The first diagnostic lease covered `00CD27C0..00CD27FF`. With no return there,
Root Astra explicitly expanded the read-only window to `00CD27B0..00CD28FF`.
All 336 selected live bytes match the original PE. Only the aligned candidate
`00CD27C0..00CD2858` is classified here. Ten preceding and seven following CC
bytes delimit that local candidate; the first diagnostic byte can be inside
an earlier instruction. Adjacent instructions/functions are not reconstructed
or used to establish this candidate's callers or lifecycle.

The four selected bytes at `00CE3020` match both live Ghidra and the PE and
decode to `00CD27C0`. They are read-only `.rdata`. No adjacent table words were
read. A byte-pattern search found the pointer, while Ghidra reported no entry
xref. Its role as an actual CRT initializer-table entry remains provisional:
table bounds, walker, startup invocation and execution order are unopened.

Project-aware live queries verified existing project `bsp`, program
`/battlestationspacific.exe`, x86/00400000 context and unchanged count 64,729.
Configured analysis remains `C:/Users/sqz269/bsp.gpr`; no reimport, analysis,
listing, AddressSet, prototype, annotation, no-return or save operation occurred.
The JSON report retains all diagnostic bytes, exact candidate instructions,
PE section metadata, hashes, input pins and captured CLI results.

## Production closure

This establishes the missing sentinel-link stores behind the earlier ten-byte
`00CD27D6` fragment. It does not bind Source mutable storage, raw group indexes,
the separate global pending head/tail pair, or an actual production caller.
The admitted Source registration constructor still takes concrete borrowed
storage/publication inputs; there is no new default owner or production lifecycle.
Do not clamp its raw index to five or substitute these tails for `00E0B704`.

Next work must recover the bounded initializer-table/walker/startup route and
then establish actual Source backing and caller identity. Source/CMake/ledger,
builds, tests, Original execution and gameplay are unchanged by this packet.
