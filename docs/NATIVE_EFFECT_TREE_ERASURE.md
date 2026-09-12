# Raw effect-tree erasure and destruction

Five complete source entries remove the remaining tree-erasure and owner
destruction dependencies for the raw gameplay-effect manager. They use the
raw18h node/header layout and actual providers established by
[NATIVE_EFFECT_TREE_CONSTRUCTION.md](NATIVE_EFFECT_TREE_CONSTRUCTION.md).

| Address | Complete bytes | Recovered operation and original ABI |
| --- | ---: | --- |
| `0086E8A0` | 679 | Iterator erase: ECX tree; stack output, input owner/node; EAX output; RET0Ch. |
| `0086EE50` | 201 | Range erase: ECX tree; stack output, first owner/node, last owner/node; EAX output; RET14h. |
| `0086FDE0` | 52 | Tree destruction: ECX tree header; RET. |
| `0086FE20` | 159 | Effect-manager destruction: ECX raw10h owner; RET. |
| `008703E0` | 30 | Scalar deletion: ECX owner, stack flags; EAX captured owner; RET4. |

## Complete erasure behavior

Iterator erasure retains the native argument slots and full successor,
transplant, symmetric red-black fixup and rotation paths. It frees the original
node, conditionally decrements the current unsigned count, captures both output
words and writes owner before node. It does not compare the input owner with
the target tree or release the opaque payload. The input node is captured
before checked increment mutates the by-value iterator argument.

The new naked source entry preserves native normal-frame offsets using54h of
scratch storage. Its nil-input branch restores entry ESI/EBP/ESP before tailing
an actual owning C++ exception helper. That helper assigns27 literal bytes
(`invalid map/set<T> iterator`, excluding its terminator) before arming temporary
string cleanup. It constructs the existing40-byte
`NativeHardwareLayoutInvalidIterator` transport through411700/D6926C and uses
the completed441760 copy/4412B0 destruction providers. Its class name does not
select hardware-layout tree operations. Original FH3 linkage and cleanup are
replaced by this explicit source C++ exception boundary.

Range erasure implements both whole-tree reset and partial iteration. It
advances the actual first-iterator argument, then independently erases the
captured old iterator through the full iterator routine. Returning invalid
handlers preserve the native checks and continuations. The temporary result
from each erase is discarded; the final returned owner/node pair follows the
native capture/store order. No empty-range-only shortcut is used.

Tree destruction captures current begin/end, invokes full range erasure, frees
the current head and zeros current head/count. It does not own node payloads.

## Owner cleanup and source bindings

The effect-manager destructor writes D0DA64, captures the initial root and
activates tree/base cleanup before subtree erasure. It resets the current
sentinel/count, ends tree cleanup, performs full range erasure, frees the current
head, clears current head/count, then clears actual F87664 and writes CE3818.
Its new fastcall interface supplies a stable EDX reference to that publication.

Native FuncInfo `DC7D9C`, map `DC7D8C`, has state1 to0 action C95E18, which reloads
owner EBP-18 and calls86FDE0 on owner+4. State0 to -1 action C95E10 reloads the same
owner and tails869C50. Source nested catches express that tree-then-base cleanup.
The compiler can disarm tree cleanup before subsequent nonthrowing memory
stores; native hardware-fault/SEH cleanup and cleanup-provider exception identity
are excluded from this C++ interface.

The scalar wrapper preserves all30 native bytes except its two named calls.
It carries the added publication reference to the destructor, then reads bit0
of the current flags low byte. If set, it frees the captured owner and returns
its original pointer bits. Publication clearing is unconditional, without an
owner equality check or unregister operation.

Iterator FuncInfo `DC7AE4`, map `DC7ADC`, activates state0 after assignment;
C95BB0 destroys the completed temporary string. The source helper has its own
complete EH/unwind table and owning throw metadata. Native exception RTTI,
callable ABI, private EH-spill aliases, provider register identity and arbitrary
hardware-fault behavior are not claimed.

## Validation

Twelve fresh saved-Ghidra/disk comparisons cover1121 complete function bytes,
143 EH bytes and47 literal/profile/throw-info bytes. All837 retained normal
bytes across iterator erase, range erase, tree destruction and scalar deletion
match emitted bodies except their20 named calls. The iterator additionally has
an explicit27-byte source frame/throw prefix. The complete compiled destructor,
owning throw helper, cleanup blocks, source EH tables and provider relocations
were reviewed separately.

The strict Win32 build, both existing CTests and eight native seeds passed.
One frozen main archive contains all nine captured object members; seven
existing provider objects are byte-identical to the preceding accepted build.
Fifteen source-provider bindings resolve in that archive. Thirty-six unchanged
prebuild inputs, nine actual compiler commands and204 read dependencies are
pinned in `reports/native_effect_tree_erasure_audit.json`. An initial capture
detected a concurrent startup.cmake change; a fresh snapshot and complete
rebuild supplied the accepted evidence.

One local source fixture, linked against that exact archive with an embedded
manifest, passed. It constructs a raw owner, erases a node with two children,
erases a partial range, catches and inspects the owning invalid-iterator
exception, and scalar-deletes the remaining manager. It verifies the final
publication is null and the returned owner bits are preserved. Four new entry
bodies execute in this fixture; the separate86FDE0 unwind entry and throwing
destructor-cleanup paths are statically reviewed, not exercised. No persistent
test suite was added. This source fixture does not execute original native
bodies or validate gameplay.

## Saved-analysis limitation and next integration

The complete erasure body ends86EB46 and the owner destructor ends86FEBE.
Ghidra's function objects still end86EB10 and86FE97. Their free-call overrides
were already NONE; both complete tails were disassembled and saved. In-place
body extension was unavailable because the configured Ghidra server disables
`run_script_inline`. The existing function objects, names and comments were
preserved. Full native captures in the audit supply the missing extent evidence;
short exported pseudocode must not be treated as the complete functions.

These raw terminals make the gameplay-effect owner available for the lifetime
manager's eventual fixed source dispatch. D0DA64 remains an identity DWORD,
not a callable rebuilt C++ vtable. Completing raw00BD0400 still requires the
actual bindings for every owner admitted to that manager. The executable's
typed lifetime domain has not been migrated by this packet, and full gameplay
and original caller compatibility remain unproved. Names are hypotheses.
