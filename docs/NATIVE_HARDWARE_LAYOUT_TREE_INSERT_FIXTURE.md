# Native hardware-layout tree insertion: independent fixture

The five insertion implementations at source commit `06da9a2` passed independent
full assembly review and one bounded original-instruction differential fixture.
No production defect was found in their documented valid-storage and supported
allocator profile. The fixture links a private copy of the actual primary
`bsp_core.lib`; no production translation unit was rebuilt or substituted.

The final run compared **8,582 behavior words**, **56 output stores on each
side**, and **five actual CRT throw/rethrow events on each side**. All sequences
matched. The target compiled as MSVC Win32 Release with `/O2 /Oy- /MD /EHsc
/fp:strict /W4 /WX`. This packet changes two evidence files only; it adds no
tracked test target, production source, shared metadata, or Ghidra mutation.

## Native coverage and review

| Original function | Complete bytes | Native entry and return |
| --- | ---: | --- |
| `00B20D30` iterator decrement | 137 | ECX iterator; RET or invalid-parameter tail |
| `00B28370` node initialization | 94 | ECX node; five stack arguments; RET14h |
| `00B29CD0` node allocation | 55 | Five stack arguments; RET14h; incoming ECX unused |
| `00B2F1B0` link and rebalance | 492 | ECX tree; output/left-byte/parent/pair; RET10h |
| `00B2F540` pair insertion | 235 | ECX tree; output/pair; RET8 |

All **1,013 owned bytes** were reviewed against assembly. Initialization writes
left, right, and parent before copying the pair. The forward copy rereads source
count after each key store and reads source value after destination count is
written. The link function checks the unsigned `0AAAAAA9h` limit before
allocation and uses the current count after the allocator returns. The inlined
left rotation at `B2F326` rereads the replacement's left child after its first
store, as the source implementation does.

The original link result publishes NODE before OWNER. Every original insertion
exit publishes NODE, INSERTED BYTE, OWNER. Page protection and single-step
observations recorded the real native and compiled-host store instruction
addresses and intermediate contents. All 18 insertion results had offsets
`4,8,0`; the direct-link result had offsets `4,0`. The report includes the
individual original PCs and decoded host stores.

## Checked behavior

- An overlapping initializer source at `node+8` observes the preceding
  right-link store, forward-copy overlap, and the late source-value reread.
  Color, sentinel, unused key words, and final padding match the original.
- A direct allocator call uses a deliberately irrelevant incoming ECX and
  the actual five-argument callee-cleanup convention. The real 40-byte node is
  initialized, observed, and freed. This also retains the allocator's distinct
  compiled-library symbol in the linker map.
- Fifteen ordered insertions, followed by an aliased-result new insertion and
  a direct right link, retain 17 nodes. Full tree snapshots agree after each
  operation. Separate invariants check key ordering, parent links, black root,
  red-child rules, equal black heights, and exact reachable node count.
- Duplicate key 30 rejects two different incoming values without allocation.
  One duplicate and one successful insertion alias the result with the input
  pair. The stored node keeps the appropriate original value and key.
- Iterator decrement walks from end through maximum to minimum. A returning
  null-owner handler repairs the iterator, and both implementations continue
  using its current node.
- The exact length limit leaves tree and output unchanged and constructs the
  owning `D69260` length-error payload. Its message has 19 counted bytes,
  including the literal's NUL, capacity 31, and the additional terminator.
  An explicit owning copy uses original `411940` or the host owning class
  copy; it owns separate storage and preserves the opaque prefix. Destruction
  resets the member and publishes the base vtable.
- Exception-member allocation failure runs the normal 32-byte request and
  20-byte fallback through the actual CRT new handler. The completed message
  is released and the exception propagates with identical throw/rethrow
  events. A separate failed 40-byte node request leaves count/output unchanged.
  Every completed owned string allocation is freed in these paths.

There are 18 successful 40-byte allocations: 17 retained tree nodes and the
direct helper node that is freed. Deliberate failures return null from the
observed actual malloc boundary and invoke the installed CRT new handler;
they do not invent successful null-node insertion behavior.

## Original instructions and shared services

The fixture freshly captured 46 code/data spans, totaling 2,251 bytes, through
`bsp.py ghidra bytes`. Each invocation verified
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, language, and image base.
Every span matched the installed PE. Complete runtime postimages match only
the listed 25 address relocations, 12 service-entry bridges, and three
executable FH3 registration adapters. Relative control transfers preserve
their original distances. All five insertion symbols and 11 dependencies
resolve to the frozen library in the linker map.

Native execution includes the complete five owned bodies, original comparator
`B20BF0`, rotations `B20910/B22BD0`, owning exception operations
`411700/4118D0/411940/411780`, and reached original base operations. Original
FuncInfo, unwind maps, ThrowInfo, and catchable-type storage remain active.
The executable registration adapters supply the relocated native FuncInfo in
EAX to the actual host `__CxxFrameHandler3`.

The native `D83F98` ThrowInfo reaches the actual host CRT throw function.
Page-guard observations record cleanup entry at **CBD7C0** for the successful
length-error throw, and **C5E010 followed by CBD7C0** for failed exception-member
construction. This directly confirms both base cleanup and the completed
temporary-message cleanup. The observer changes page protection, not code
bytes. The original `DF6050` FuncInfo and `DF6048` unwind map drive insertion
cleanup; the cleanup becomes armed only after assignment returns at `B2F201`.

`BF681B` bridges to the frozen library's existing
`singleton_lifetime_allocate`, which uses actual host malloc and `_callnewh`.
`408720/408120/4072D0` use exact ABI adapters to existing counted assignment,
substring assignment, and string destruction from that same library. These
allocator and string internals are shared production dependencies, so this
fixture does **not** independently validate their original instruction bodies.
Actual x86 CRT module paths and hashes are recorded. Some loaded closure
branches and original registration-handler spans are not executed.

## Reproduction and pins

The ignored fixture lives under
`J:/PROG/battlestations-pacific-decompile-native-hardware-layout-tree-insert-fixture-20260910/local/tree_insert_fixture`;
its executable, map, and raw traces are under that worktree's
`build/tree_insert_fixture`. `capture.py`, `fixture.cpp`, `CMakeLists.txt`,
`reference.hpp`, and `finalize.py` are pinned in the report. The finalizer
verifies every complete postimage, source archive, library provider, trace,
unwind observation, and artifact hash before writing the proof.

| Artifact | SHA-256 |
| --- | --- |
| Installed executable | `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6` |
| Frozen actual primary library | `0c2a637b2735b6df010fd7011937faaebea5a5e6500af1b9828701185fab2e97` |
| Final proof JSON | `e26d0607ada52ac6ae54f3f57e0c09c01ba96b973349883c687dceb29f712f92` |

The [machine-readable report](../reports/native_hardware_layout_tree_insert_fixture.json)
contains source snapshots, all original/postimage bytes, relocations, bridges,
map providers, CRT modules, observed PCs, allocator/free events, and artifact
pins. The [production reconstruction](NATIVE_HARDWARE_LAYOUT_TREE_INSERT.md)
retains the ABI and valid-storage contracts.

This establishes bounded fixture agreement for new C++ interfaces. Host RTTI,
catch types, and the surrounding runtime differ from the original executable.
It is not a drop-in native ABI claim, full-process execution, or game validation;
malformed trees, unsupported pointers, concurrency, and untested failure modes
remain outside this result.
