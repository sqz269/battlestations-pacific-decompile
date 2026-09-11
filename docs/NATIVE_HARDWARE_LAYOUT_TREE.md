# Native hardware-layout tree removal

This packet reconstructs seven operations over the actual tree header, nodes
and checked iterators used by renderer notification `00B2F4C0`. The public
entry borrows the address corresponding to global `0108D530` and erases the
first node whose value pointer matches. The original renderer `ECX` is unused.
The current renderer/current table `+44` dispatch remains with its caller.

The assembly evidence comes from discovery commit `425c9cd`, whose live Ghidra
captures verified project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and compared every captured span with the installed
executable SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The audit retains the seven whole owned spans (1,119 bytes), exception and free
dependencies, and their exact bytes. These are saved-analysis/file comparisons;
the bytes were not recaptured solely to duplicate the completed discovery.

| Entry and exclusive end | Native ABI | Reconstructed operation |
| --- | --- | --- |
| `00B20860..00B2087C` | ECX node, EAX maximum, RET | Follow nonsentinel right links. |
| `00B20880..00B2089B` | ECX node, EAX minimum, RET | Follow nonsentinel left links. |
| `00B20910..00B20962` | ECX tree, stack node, RET4 | Rotate right; update current root or parent link. |
| `00B22BD0..00B22C1E` | ECX tree, stack node, RET4 | Rotate left with the symmetric link updates. |
| `00B20DC0..00B20E23` | ECX iterator, RET or invalid-handler tail | Checked in-order successor. |
| `00B2EF00..00B2F1A7` | ECX tree; stack output/owner/node; EAX output, RET0Ch | Checked erase, transplant, color fixup, free, output publication. |
| `00B2F4C0..00B2F53E` | Stack value, ECX unused, RET4 | Checked scan and first matching-value erase. |

Names describe established behavior; original C++ symbols and template key
types were not recovered. The extrema return values and the erase return tail
come from assembly because their older pseudocode signatures/bodies are wrong.

The borrowed tree has opaque DWORD `+00`, head pointer `+04`, unsigned count
`+08`. A node's minimum accessed extent is `26h`: left/parent/right at
`+00/+04/+08`, opaque bytes `+0C..+1F`, value pointer `+20`, color byte `+24`
(black 1/red 0), sentinel byte `+25`. The head uses its links for minimum/root/
maximum. This packet does not claim a node allocation size or key layout.
The checked iterator is the actual two pointer words `{owner, node}`; the owner
is address identity. Existing alias-list node storage and erase behavior differ.

Removal preserves the assembly's material ordering:

- Notification captures its initial head once. Validation uses captured owner
  and node values across returning handlers; it reloads the iterator after
  increment. It compares the node with the owner's current head before reading
  the value and removes only the first matching node.
- Increment reloads the node after a returning null-owner handler. A sentinel
  node tail-calls the handler; when it returns, increment returns without
  advancing, even if that handler changes the iterator. Parent ascent publishes
  each intermediate parent into the actual iterator.
- Erase checks the input sentinel before increment and does not compare input
  owner with destination. Input is by value. Its two-child path at
  `B2F00F..B2F065` transplants the successor and swaps colors without copying
  opaque bytes or the value pointer. Single-child removal also updates extrema.
- Free at `B2F16C` always receives the original node. The complete continuation
  loads the destination's current count after free and decrements only when
  nonzero, then loads the advanced input words and publishes output owner before
  output node. Output may alias caller storage. Rotations use current head/root
  links and preserve sentinel guards on child-parent stores.

`SingletonLifetimeCallbacks::invalid_parameter` binds the actual potentially
returning/throwing `BF6713` service. No fallback no-op callback is supplied.
`singleton_lifetime_free` uses the established shared CRT free domain:
`BF6989 -> BF65AC -> BF9DC8`. Erase does not retain/release COM objects or destroy
the opaque key/value; the caller retains responsibility for its actual storage.

The sentinel erase constructs a `1Ch` temporary by initializing only capacity
15, length 0 and its first byte, then calls original reconstructed counted
assignment `408720` with 27 bytes from `CE44E0`. The literal is
`invalid map/set<T> iterator`: the 27-byte count includes its first NUL, and the
string helper adds another terminator at index 27. This unusual counted length
is intentional. `CompletedTemporary` is armed only after assignment succeeds,
matching state 0 at `B2EF52`, FuncInfo `DF6024`, and cleanup `CBD7A0 -> 4072D0`.

`NativeHardwareLayoutInvalidIterator` owns the actual `28h` legacy exception
storage. Construction delegates to `411700` and publishes native profile
`D6926C`; copy delegates to `441760`; destruction delegates to `4412B0`.
Those last two dependencies come from integrated commit `39d0586`. The host
class retains the original counted assignment, member ownership, copy and
cleanup; native ThrowInfo `D863A8` remains evidence data. Host exception RTTI,
dispatch and calling conventions are new and are not native ABI replacements.

One private composition fixture at `local/hardware_tree_fixture/fixture.cpp`
compiled the four production tree/string/exception source files unchanged with
MSVC 19.51 Win32, C++17, `/W4 /WX /fp:strict`. A tracked malloc/free service
boundary records releases and injects allocation failures; it does not replace
the tree, string assignment, exception copy or cleanup implementation. It passed
the scan's two-child transplant and duplicate-value first match; erase to an
empty tree; both rotations and extrema; a zero count changed at free; by-value
foreign ownership/output aliasing; the returning sentinel handler; owning
27-byte throw/copy/destruction; and failed allocations before and after the
completed-temporary arming point. This is host fixture evidence, not native tree
differential or game execution. The private fixture is ignored, as shared test
and build metadata belongs to the primary integrator.

`scripts/build.ps1` passed the repository MSVC Win32 build. Fresh
`ghidra_export.py verify-seeds` matched all eight existing math/PRNG reference
spans; the seeded repository rebuild then passed both existing CTest checks,
`reconstructed_math` and `native_math_differential`. Those checks exercise their
existing subjects. The primary integrator registered the tree source in CMake
and passed the combined Win32 build and both existing CTest checks. It also
verified all 17 retained live-PE spans and reran the private composition fixture.

The primary integrator restored the complete erase extent through exclusive
`B2F1A7`, preserved prior Ghidra comments, saved all seven names/comments and
complete-function ledger entries, and refreshed the affected exports. An
independent original-tree differential fixture remains in progress.
No tree constructor, insertion, general tree destruction, shader/input semantic
key, complete renderer binding, binary ABI compatibility or game validation is
claimed.
