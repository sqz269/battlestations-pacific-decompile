# Current session transport step

Packet `cc11_health_transport_step` closes three complete native bodies on the
actual host and client `+1C` profile entries: 60 bytes, 23 instructions. The new
Source interfaces are reconstructed and build/fixture tested; they are not
native class bindings or a game validation. Descriptive names are hypotheses.

| Entry | Complete native extent | Native contract | Source function |
| --- | --- | --- | --- |
| `00782870` | `[00782870,00782877)`, 7 bytes / 3 instructions | ECX transport, no stack arguments, current table `+1C` tail JMP | `step_current_native_session_transport_00782870` |
| `00A42280` | `[00A42280,00A422B4)`, 52 bytes / 19 instructions | ECX transport, no stack arguments, RET | `step_host_native_session_peers_00a42280` |
| `00A41130` | `[00A41130,00A41131)`, 1 byte / 1 instruction | ignored ECX, no stack arguments, literal `C3` RET | `step_client_native_session_transport_00a41130` |

The current dispatcher loads the actual table at transport `+00`, reads slot
`+1C`, then jumps through that value. The readonly image profiles at `00D243EC`
and `00D24448` contain `00A42280` and `00A41130` at cells `00D24408` and
`00D24464`. Live Ghidra bytes match the disk image for all three bodies, the two
reached 32-byte profile prefixes and the actual reset constant `00CE3910`.
`0077006F` in `0076FFC0` is the observed incoming CALL to the dispatcher.
The host retains its existing Ghidra name `STL_inst_00a42280`.

## List traversal and target state

The host captures the actual sentinel at transport `+0C`, reads its first node,
and compares that node with the captured sentinel. If equal it calls the fixed
invalid-parameter service at `00BF6713`, retaining the return continuation.
It then advances through the current node's `+00` pointer before comparing with
a freshly read sentinel. At the sentinel it returns; otherwise it reads the
current node's target at `+08`, loads actual float cell `00CE3910` with MOVSS and
stores its raw word at target `+D4C`. It freshly compares node and sentinel before
the next guard. Consequently the first **node** is skipped; a later node may
still refer to the first node's target. Transport `+10` count is never read.
One nonempty host node makes no target write. No queue, clock, flush, history or
lock operation occurs. The client body returns without reading the list.

Source borrows `NativeSessionPrimaryPeerFields` and the existing actual
`NativeUnitHealthRoutePeerNode`/`NativeSessionTargetStorage` layouts. The added
`NativeSessionTransportStepView` only holds references to the actual table
cell and existing peer-field view. Binding it reads no values and creates no
list or callback. The reset value is a borrowed volatile reference to the
actual readonly image cell, raw bits `3D23D70A`; it is freshly loaded for each
write. This is a raw MOVSS operation, including signaling-NaN target seed words,
and performs no floating-point arithmetic or conversion.

Source maps only the actual `00A42280`/`00A41130` slot words to these complete
implementations. Other words fail Source admission with `std::logic_error`.
Raw PE tables are not callable Source class profiles. All three Source
interfaces use new cdecl reference interfaces, not the native ECX class ABI.
The fixed Source service calls current SDK `_invalid_parameter_noinfo` and
may return. No test callback or substitute transport-phase implementation is
provided. Historical `00BF6713` encoded globals, Watson/private fault and EH
behavior remain unbound; the ordinary fixture does not execute an empty host
list or mutate lists from an invalid-parameter hook.

## Connected original fixture

The ignored fixture is in `local/cc11_transport_step_20261007_a`. It executes
the complete original 7-byte dispatcher through a copied actual profile prefix
into the complete original 52-byte host or actual original one-byte client.
The only original **code** edits are two four-byte host operands: rel32 at host
offset `0E` to a fixed real SDK service bridge, and the absolute constant address
at offset `20` to the actual readonly mapped `00CE3910` cell. All other 52 of the
60 code bytes are unchanged. Separately, each copied eight-DWORD profile prefix
changes only its `+1C` DWORD to the corresponding copied **original** body;
the other seven words remain identical. These two data relocations do not prove
an entire native transport profile/factory or make unrelated profile entries
callable. Code/profile copies become `PAGE_EXECUTE_READ` after setup, and the
frozen original image is mapped `PAGE_READONLY`/`FILE_MAP_READ`.

Each original/Source world has three genuinely Source-constructed `D74` targets,
their real owned OS locks, three separately allocated cursor objects per target,
and both genuine 50-/3-sample histories. Actual slots 0/1/2 produce mask 7 and
complete target destruction returns the mask to zero. Each cursor points into
one of the three **inline** `464`-byte payload slices in target `buffers_10`.
All slices are filled and cursor positions/bits made valid and nonempty.
Whole-target snapshots therefore include all inline payload bytes; separate
cursor snapshots cover the allocated cursor state. Whole locks, both history
headers and all 53 samples per target, transport, sentinel, nodes and slot mask
are also compared. Only expected visited target `D4C` words may change.

The six connected cases cover host one peer, host three peers with stored
count zero, host two peers with stored count 77, host later-node alias to the
first target, client coherent empty list with count 77, and populated client.
Each uses one fixed masked nondefault FP environment (CW `077F`, MXCSR `DFD5`,
two occupied x87 values and genuine sticky status) and seeded XMM registers.
Immediate FXSAVE compares full control/status/tag/TOP, all eight raw 80-bit x87
registers, full MXCSR and XMM0..7. Host writes leave exactly the MOVSS constant
in XMM0 low DWORD and zero its upper 96 bits; no-write/client paths preserve all
XMM seed values. Source and original agree. This is six ordinary list/profile
cases, not a broad arithmetic/invalid-handler matrix.

Final result: **511 checks, zero failures, six original dispatch cases**.
The only change between retained `inputs_01` and final `inputs_02` is a fixture
assertion label narrowed to separately allocated cursor state. Behavior and
coverage are unchanged; inline payload preservation was already checked by
the whole-target comparison.

## Build, inspection and limits

The strict MSVC Win32 `19.51.36244.0` build uses `/O2 /Gy /W4 /WX /fp:strict
/MD /EHsc`, embedded `asInvoker` manifest and `/OPT:REF`. Six repository
translation units are freshly compiled: step, target lifetime, target history,
transport buffers, singleton lifetime and renderer worker lifetime (the real
tracked OS-lock constructor). Together with 23 included headers and the probe,
30 immutable inputs are pinned. Three readonly main support libraries were
frozen before permission for the integrator's next rebuild; the originating
full-build codepoint is `ea9896f474b29e787cc7fb05c2d386a532e7c177`, observed main
and worker at freeze `fccf4c1f83b02709522a808e6a4ad57d655950eb`.
Afterward main core changed while the copies linked here stayed identical.
The report records both hashes rather than claiming current-main link inputs.

Full COFF review finds Source host 79 bytes/30 instructions, current dispatcher
148/50 and client 3/1. MSVC encodes the Source naked `ret` as `C2 00 00` (RET 0),
with the same return stack effect as original `C3`; it is not byte identical.
The current Source dispatcher inlines the complete host loop, retains every
fresh/captured load and may-return SDK continuation, and tail-JMPs to the
complete Source client. Thus there is no Source host-call relocation. Its
`logic_error` dependencies are confined to unsupported profile admission.
Both fixed SDK bridges are six-byte IAT tail JMPs. Original/Source FP drivers
are reviewed as well; the Source driver directly calls the new complete
dispatcher. Exact bytes, hashes, relocations and reviews are in the report.

The static report has two direct CALL rows (`0077006F -> 00782870` and
`00A4228D -> 00BF6713`) and two explicitly qualified indirect tail-JMP rows at
`00782875`, resolved by actual profile data to host/client. The verifier checks
all four rows with zero failures, while explicitly reporting the two indirect
targets as not independently resolved by its listing check. This does not
claim four direct calls. The full enclosing `0076FFC0` phase, transport factory,
all-profile native ABI, networking, failure paths and gameplay remain open.
The worker changes only its two Source files and this document/report; the
integrator owns shared build registration, annotations and main-build review.
