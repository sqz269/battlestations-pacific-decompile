# Native string/float tree used by SoldierClass LoopLengths

This packet supplies eighteen complete source bodies for the actual tree at
SoldierClass+28h. The same specialization also serves the previously named
weather maps; their existing correct library names are retained. Descriptive
names are hypotheses, not recovered symbols.

## Storage and source reuse

The header contains an opaque word at0, sentinel at4 and count at8. Nodes are
1Ch bytes: left/parent/right at0/4/8, the raw eight-byte string at0Ch, mapped
float at14h, color at18h and nil at19h. Bytes1A/1B remain untouched. A temporary
pair is0Ch bytes, with the float at8. The comparator is the genuine443D00
case-insensitive raw-string routine. No host map or replacement string owner
is introduced.

The report records nineteen native instruction comparisons against the now
complete Soldier registry specialization: seventeen match after replacing
corresponding helper addresses, internal branch targets and EH-handler push
addresses. Two have expected mapped-value differences. This comparison also
covers443FB0/4B0090 pair cleanup and444252/4B0312 detached allocation catches.
Masking the EH-handler address alone is not exception proof; the metadata below
was separately checked against the installed image and live Ghidra bytes.

Eleven source entry points reuse the verified registry lookup, iterator,
rotation, min/max and lifetime routines. Those bodies neither inspect nor own
the mapped value, and their field/load/call schedules match. Insertion uses the
same concrete balancing helpers with the genuine float-node allocator. There
is no forwarding to missing source.

## Float and cleanup differences

- `00444150` deep-copies the string, then transfers pair+8 to node+14 through
  x87 `FLD32/FSTP32`. Inline assembly preserves the float conversion and x87
  exception behavior, including signaling NaNs. The native color argument load
  occurs between those instructions; the new C++ interface does not claim that
  stack-load interleaving or hardware-fault behavior.
- `00444BE0` initializes the mapped temporary to positive zero after the key
  copy. The source preserves those bits; it does not require native
  `XORPS/MOVSS` opcodes. After insertion it captures the returned owner and node
  before normal temporary cleanup.
- Normal subscript cleanup returns the captured data pointer with the current
  temporary length. Its true EH unwind instead destroys the current header
  through443FB0. A second C++ exception during true unwind terminates.
- `004441E0` frees only the captured raw node if initialization fails. The
  detached catch444252 explicitly calls free and rethrows; it is not a second
  exception terminating unwind action. The placement cleanup401130 is a no-op.
- BF7680 copies use `memmove`, retaining its admitted overlap behavior and the
  native post-resize pointer/length reload order.

## Exception and repaired-listing evidence

| Body | Handler / FuncInfo / map | Established action |
| --- | --- | --- |
| 4441E0 | C5FBF1 / D8687C / D86864 | Three states; placement cleanup C5FBE0, explicit catch444252 frees captured node and rethrows. |
| 4442A0 | C5FC08 / D868A8 / D868A0 | State0 owns the completed length-message temporary; C5FC00 tails to4072D0. |
| 444490 | C5FC28 / D868D4 / D868CC | State0 owns the completed invalid-iterator temporary; C5FC20 tails to4072D0. |
| 444BE0 | C5FC48 / D86900 / D868F8 | State0 owns the current pair header; C5FC40 tails to443FB0. |

The restored444490 post-free tail and full444252 catch are documented in
`NATIVE_SOLDIER_REGISTRY_FLOW_ORCH4_G7.md`. No copied stale truncated listing is
used. Length/invalid-iterator exceptions compose the existing owning native
layout transports; host C++ exception identity remains an adaptation.

## Validation and limits

All eighteen full body spans and the detached catch match live/disk SHA256.
Exact direct call rows and prior Ghidra names/comments are preserved in
`reports/native_soldier_loop_lengths_orch4.json`.

The strict MSVC Win32 build and all three existing CTests passed. A focused
ignored probe linked the actual `bsp_core.lib` and compared copied original
444150 against source on twenty float/alias inputs: full1Ch node bytes, return
pointers and x87 exception flags agreed for signed zero, ordinary values,
denormals, infinities, quiet and signaling NaNs. Keys were empty; original
resize calls used the genuine source bridge. This does not establish nonempty
raw-pool allocation, native FH3/SEH, original binary ABI or game behavior.

The native full reader4B0DE0 still requires genuine48F670 and effect-component
closure. These tree providers do not make the reader or factory source-complete.
