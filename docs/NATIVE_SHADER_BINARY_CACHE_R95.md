# Native shader binary cache lifecycle

Addresses: 00B34C70, 00B34C80, 00B352B0, 00B35340, 00B38A70,
00B3A600, 00B3B140, 00BE45F0.

## Result

Eight normal-path bodies (975 original bytes) now construct, populate and retire
the actual 10h shader cache and its 10h records. They borrow the existing raw
string pool, numeric VFS dispatch, allocation services and compiler mode cells.
The application-object probe loaded the installed `shaderfx/shaders.bin` twice
through the current application's VFS and completed ordinary singleton drain.
The full 0073BF80 material-preload loop is still not connected to normal startup.

Sources: `include/bsp/native_shader_binary_cache.hpp`,
`src/native_shader_binary_cache.cpp`. Machine-readable evidence:
`reports/native_shader_binary_cache_r95.json`.

## Recovered contracts

| Address | Behavior | Original ABI |
| --- | --- | --- |
| B34C70 | Clear row00/04/08; preserve byte-count0C | ECX row; EAX same row; RET |
| B34C80 | Free captured blob; clear current08; release current pooled name | ECX row; RET |
| B352B0 | Gate on source mode; optional seek/count write; reverse cookie destruction; stream decrement/terminal/clear | ECX cache; RET |
| B35340 | Gate before owner read; cursor0; count; saturated allocation plus cookie; live name/size/blob loop | ECX cache; RET |
| B38A70 | Preserve cursor; clear04/08/0C; open flags2 or5; read records or write initial count0 | ECX allocation; EAX same; RET |
| B3A600 | Allocate10h, construct, then publish108D6EC; null allocation publishes null | No consumed input; RET |
| B3B140 | Capture publication, destroy/free captured owner, clear current publication | No consumed input; RET |
| BE45F0 | Call current stream48 with output/count; return captured output independently of callee EAX | ECX stream; output/count stack; RET8 |

The loader re-reads current row storage, stream and count at the original call
boundaries. Names use the same actual pool, not a projected string collection.
The array cookie retains the captured allocation count; destruction walks it in
reverse. Record name headers and byte counts remain stale after destruction.
Source mode skips I/O and all cache member cleanup. Variants mode is sampled
again after temporary-name release. Original cursor/byte-count preimages are
preserved where no native store exists.

## Ghidra correction

Three incorrect CALL_RETURN overrides hid returning `free` continuations:

- B34C8B: B34C90..B34C99 clears the blob then continues to pooled-name cleanup.
- B35302: B35307..B35310 clears the row pointer then continues to stream release.
- B3B153: B3B158..B3B165 clears the global publication and restores ESI.

The owning worktree repaired these under the shared write lock, saved the
project and refreshed exports. The report preserves the repair receipt and
prior names/comments. Descriptive names are hypotheses, not recovered symbols.
No global no-return attribute was changed.

## Validation

- Strict MSVC Win32 build and all three existing CTests passed; no new repository tests.
- Eight complete function spans plus filename/table data: 1,099 live Ghidra bytes
  equal the installed PE. Every direct call row is mechanically checked;
  indirect calls remain separate evidence.
- One forwarding probe links 51 current production application objects and
  three current libraries. Its explicit mode cells are cached mode0/variants0;
  the cache publication is probe-owned pending application preload integration.
- Installed file: 2,250,700 bytes, 1,628 records, 2,205,720 bytecode bytes.
  Ordered name/bytecode FNV64 `2733cb68f3747826` matches an independent disk
  parser; sampled first/last rows match separately. Entire file consumed.
- Two full loads and releases exercise both nonzero and zero stream reference
  branches. The first takes and separately retires one diagnostic stream
  reference to observe the native decrement without reading freed memory.
- Inaccessible-owner source-mode gates and untouched constructor/record words
  pass focused checks. Probe and unmodified application each exit0 after two
  ticks/one Present; worker joined; final device/API COM references0/0.
- The first probe used position slot2C instead of20 and failed after loading all
  records. Its executable, logs and inputs are preserved in an immutable local
  archive. Only the probe check changed; production source was already passing.

## Remaining work

This is source/runtime evidence, not an original whole-cache differential or
drop-in binary replacement. Allocation failure, partial reads, callback mutation,
variants-mode writes, original CRT/FH3/SEH and arbitrary stream entries are not
runtime-validated. Failed source frames retain their acquisitions; they do not
imitate original exception cleanup. The existing physical writer and VFS stream
providers remain explicit dependencies.

Follow-up: compose the same cache publication and mode cells into 0073BF80's
real descriptor/material compiler loop, recover startup control settings, and
complete B107F0's post-effect graph. The sampler release stack-residue boundary
and other compiler source-path prerequisites remain open. No gameplay or pixel
parity claim follows from this packet.
