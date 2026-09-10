# Native render alias checked operations

The four complete checked-iterator/list-erase bodies now operate on the actual
eight-byte iterator, twelve-byte list owner and sixteen-byte alias nodes.
Returning validation handlers retain the native capture, reload and publication
behavior. A focused original-byte comparison passed with an identical 225-word
trace, and the strict MSVC Win32 build passed both existing CTest checks.

Sources: `include/bsp/native_render_alias_checked_ops.hpp` and
`src/native_render_alias_checked_ops.cpp`. Evidence and artifact hashes:
`reports/native_render_alias_checked_ops_audit.json`. The prior dependency
discovery is `docs/NATIVE_RENDER_ALIAS_CHECKED_OPS_NEXT.md`. Names are descriptive
hypotheses rather than recovered symbols.

| Address and complete span | Original ABI | Reconstructed operation |
| --- | --- | --- |
| `004BE820..004BE849` exclusive, 41 bytes | ECX left iterator; stack right address; RET4; AL boolean only | Compare current node pointers after owner validation |
| `004BECC0..004BECE6` exclusive, 38 bytes | ECX iterator; RET0; EAX original iterator | Move to previous, then validate the published node |
| `004B9FF0..004BA018` exclusive, 40 bytes | ECX iterator; RET0; EAX original iterator | Validate current node, then reload and move to next |
| `004D0990..004D0A0B` exclusive, 123 bytes | ECX destination list; stack output address, input owner, input node; RET0Ch; EAX output | Erase captured node and publish next/input-owner iterator |

## Actual storage and validation

`NativeRenderAliasIterator` contains the owner identity/address at +0 and node
pointer at +4. An owner is the real embedded list at
`NativeRenderResourceRecord+8`: unknown DWORD +0, sentinel pointer +4, count +8.
The existing alias node stores next +0, previous +4, string length +8 and string
data +0Ch. Win32 sizes and offsets are asserted. List fields are accessed at
their actual addresses; the implementation creates no replacement list header.
Volatile field accesses preserve the assembly's load/store sequence when native
storage overlaps or a callback mutates it.

The caller supplies the existing `SingletonLifetimeCallbacks` domain with a
callable `invalid_parameter` function. Native `00BF6713` can return through the
registered invalid-parameter handler. These operations therefore have no
unconditional abort, validation exception policy, no-op callback or synthesized
safe result. The callback's registration/default CRT termination mechanism is
an explicit external binding. If it returns without a necessary repair, the
original unsafe continuation remains unsafe; in particular erase cannot repair
its captured null input owner by changing the caller's iterator.

Comparison reads the initial left owner once and calls the handler when it is
null or differs from the right owner. After return it loads both current node
words, without another owner check.

Previous first checks for a null owner, then loads the current node's previous
pointer and the current owner. It publishes previous before reading that owner's
sentinel and calling the second handler on equality. A repair made by that second
handler survives the return.

Next checks for a null owner, then reloads owner and node for the sentinel check.
After that check and any handler return it reloads node again, loads its next and
publishes it. This is different from previous's publication order.

## Erase ordering

Erase captures input owner/node by value and destination owner before validation.
It checks input owner non-null, then captured node against the input owner's
current sentinel. It does **not** require input owner to equal destination owner.
It next compares captured node against the destination's current sentinel, and
captures node.next regardless of that comparison. Equality skips unlink, string
release, node free and count decrement.

For a removed node, it writes previous.next from captured next, reloads both
node links, and writes the reloaded next.previous. It captures string data and,
when non-null, length+1 for the existing actual string-pool release. It then
frees the captured node through the existing native lifetime malloc/free domain.
Only after free does it reload and decrement the destination count, with native
DWORD wrapping. The `004D09F1..004D09F8` cleanup/decrement tail is present even
though false `_free` no-return analysis omitted it from the pseudocode.

Finally, erase writes captured next to output+4, then captured **input owner** to
output+0, and returns the output address. This preserves output/input aliasing
and output that overlaps actual list storage. It also preserves the captured
next even if string-release reentry changes the removed node's next field.

## Verification and limits

Each of the four native code spans was refreshed using `bsp.py ghidra bytes`,
which verified project `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe` before reading. Every byte matched the installed PE
with SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
All eight existing seed byte checks also passed before native execution.

`scripts/build.ps1` passed MSVC Win32 `/W4 /WX /fp:strict` and CTest 2/2. An ignored
local CMake top-level include registered the new source without changing the
integrator's CMake files.

One ignored fixture, `local/native_alias_checked_ops_check.cpp`, executes all four
complete installed bodies with their ten dependency calls relocated to observed
validation and the same actual pool/free boundaries. The compiled implementation
uses its production library; its existing `singleton_lifetime_free` entry is
temporarily instrumented in this isolated fixture to observe and perform the
real `std::free`. The entry bytes are restored afterward. No game process or
installed file is modified.

The matching 225-word sequence verifies ordinary iterator behavior, comparison
after node repair without owner revalidation, previous publication before its
second handler, both returning next handlers, captured erase inputs despite
caller-iterator mutation, foreign input owner, unlink-before-release, real string
release followed by node free, captured next despite reentry, a count changed
from 3 to 41 during string release and to 43 during free then decremented to 42,
destination-sentinel skip, result aliasing list sentinel/count storage, null-string
node disposal and count underflow to `FFFFFFFF`. Unknown list DWORDs remain intact.

The fixture observes the registered-handler boundary; it does not execute the
CRT's pointer decoding/default termination chain. Native exception injection,
unrepaired invalid-pointer faults, concurrent access and every malformed alias
graph are not tested. These are reconstructed and fixture-tested C++ interfaces,
not register/stack-ABI replacements or game-validated rendering. Range insertion,
list growth/length-error construction and record assignment remain separate
reconstruction work. Ghidra annotation, ledgers, source registration and packet
integration remain with the primary integrator.
