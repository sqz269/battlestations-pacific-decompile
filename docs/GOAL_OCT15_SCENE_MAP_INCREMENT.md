# Scene-record map iterator increment

Addresses: `00467980`, `004681F0`, `004688D0`.

The complete raw iterator increment and both wrappers operate on actual eight-byte
`{owner,node}` storage. The implementation uses the existing concrete current CRT
invalid-parameter service. It adds no host callback, alternate tree owner, layout
overlay, null fallback, or success-only admission. Existing library-shaped names
are retained as descriptive hypotheses.

| Routine | Inclusive body | Bytes / instructions | Native ABI |
| --- | --- | --- | --- |
| `00467980` | `00467980..004679E2` | 99 / 38 | ECX actual iterator; no stack arguments; plain RET; no unified semantic EAX result |
| `004681F0` | `004681F0..004681FB` | 12 / 6 | ECX iterator; plain RET; EAX captured iterator |
| `004688D0` | `004688D0..004688DB` | 12 / 6 | ECX iterator; plain RET; EAX captured iterator |

This covers all 123 bytes and 50 instructions. Incoming EDX is not consumed.
ESI preserves the original iterator address. Source retains the observed ECX/RET
shape through MSVC Win32 `__fastcall`; it does not promise native flags, ambient
register values, return/fault addresses, private frames, or byte identity.

## Current storage and access schedule

Iterator owner is DWORD `+0`, node is DWORD `+4`. A node has left `+0`, parent
`+4`, right `+8`, and a BYTE nil flag at `+19h`. Raw DWORD address arithmetic
wraps at 32 bits. Every access reached requires valid storage. Node links need
not be distinct; Source preserves alias-sensitive accesses with volatile loads
and stores. There is no new nonnull-owner or nonnil-node precondition.

1. Read owner once. If zero, report at `00467988`; if the service returns,
   load the CURRENT iterator node at `0046798D`. Do not recheck owner.
2. If that captured node is nil, the POP-ESI/JMP at `00467997` reports and
   returns. A returning handler causes no later iterator store or link access.
3. Otherwise read captured node.right, then its nil byte. If nonnil, descend
   left: read candidate.left, check the child's nil byte, advance candidate and
   repeat. Publish only the final candidate at `004679BA`.
4. If the right child is nil, read captured node.parent. For each nonnil parent,
   reread CURRENT iterator.node at `004679C8`, then read parent.right and compare.
   On equality publish parent at `004679D0` BEFORE reading that parent's parent
   at `004679D5`. Repeat the nil check. On inequality or nil, publish the selected
   parent unconditionally at `004679DE`.

The final ascent store remains present even when it repeats an intermediate
store. Ordinary C++ node snapshots could erase the current-cell distinction when
the iterator and node storage overlap; this implementation retains those reads
and writes. Concurrent unsynchronized mutation and invalid raw storage are not
admitted by the C++ interface.

The core has no consistent native EAX result: right-subtree return leaves the
nil left child in EAX; ascent leaves the selected parent; the report tail leaves
the current service's ambient result. Source therefore exposes `void`. Each
wrapper captures the original iterator address in ESI, invokes the core at
`004681F3` or `004688D3`, then explicitly returns that captured address, including
when a returning handler has changed iterator contents.

## Concrete invalid-parameter policy and callers

Both reports bind existing `invoke_native_invalid_parameter_00bf6713()`. Its
complete Source entry uses the actual current SDK `_invalid_parameter_noinfo`
declaration, without `noexcept` or `noreturn`. The native 16-byte service has
XOR EAX, five zero pushes, a child transfer at `00BF671A`, ADD ESP,14h, and RET.
The Source's five zero words are unused padding for the real zero-argument SDK
API. No five-argument provider or callback is invented. Current handler return,
throw, termination, and side effects remain current CRT policy. A thrown C++
exception propagates; these routines add no cleanup or exception conversion.
Original encoded handler selection, debugger hook, Watson delivery, original
CRT globals and SEH transport are outside this qualified existing boundary.

Live references to the core are exactly six: both wrappers, and parent routines
`0046A470`, `0046B570`, `0046B680`, `0046BD10`. The four parent transfers are
at `0046A4EE`, `0046B609`, `0046B6F1`, `0046BD6A`. Their ECX inputs are stack
iterator addresses; all discard the ambient core return. The latter two reload
owner/node from the actual iterator after return. The wrappers have no live
references. Caller bodies and the existing provider remain outside this packet.

## Validation

Source draft `59c0ac1ab718631fd29beb8e1679e679ecff8870` passed the worker's
required `scripts/build.ps1` Release MSVC Win32 build with `/W4 /WX /fp:strict`.
Only `/MP4` was added to this process's CL options for parallel compilation.
All three existing CTests passed in 8.51 seconds: reconstructed math 0.24s,
native math differential 0.04s, and tool tests 8.16s. The build retained an
unrelated existing `LNK4006` duplicate `spawn_request_id_matches` warning.
No new tests were added. Existing math differential coverage does not exercise
this packet; packet-specific native differential and game validation did not run.

All three complete live byte spans match the installed PE (123 bytes/50
instructions); the receipt is `local/goal_oct15_scene_map_increment_bytes.json`.
Eight typed native transfer rows pass `verify_report_calls.py`; the Markdown
check reports 26 cited addresses with zero failures, unchecked claims or warnings.
Prior comments and full documentation were archived. The annotation tool retained
all three existing names and comments under its write lock, saved the project,
and recorded prior values in `local/ghidra-annotations-20261015T203624Z.json`.
All three exports were refreshed with `--force` and comments read back.

Focused review of the actual compiled object found a 108-byte core plus two
12-byte wrappers. The core calls the concrete report at offset `+09`, reloads
the current node at `+0E`, and emits POP-ESI/JMP at `+18/+19` for nil-node
failure. Its ascent reloads iterator.node at `+50`, reads parent.right at
`+53`, publishes the parent at `+5A`, then loads that parent's parent at
`+5D`; the final unconditional publication is `+67`. Right-leftmost traversal
also retains all volatile link/nil reads and its final selected-node publication.
Both wrappers contain PUSH ESI, MOV ESI,ECX, CALL core, MOV EAX,ESI, POP ESI, RET;
their call relocation is at `+04`. The core has exactly two code relocations,
both to the existing concrete report service. This verifies the intended access
and transfer schedule without claiming native byte or COFF identity.

The object is 2093 bytes with SHA-256
`2d17720da35712f2c65d60a3b6839e21dff0b391623efdf5e0607dae2077abd6`.
The disassembly/relocation receipt is
`local/goal_oct15_scene_map_increment_object.txt`. Diff whitespace checks pass.
