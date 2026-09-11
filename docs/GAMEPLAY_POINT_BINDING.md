# Gameplay definition to point-effect construction

Packet `orch3_gameplay_point_binding_ab` supplies the concrete C++ composition
from an actual gameplay definition through `008689C0`, admission `0086A650`,
the complete `008680B0` point constructor, current component dispatch, and
the recovered shake side effect. It reuses the existing node, string, insertion,
frame-pool and teardown implementations. The application must still supply its
actual associations and remaining component implementations; this is not a game
runtime or a native vtable replacement.

Source is `include/bsp/gameplay_point_binding.hpp` and
`src/gameplay_point_binding.cpp`. Evidence and annotation history are recorded in
`reports/gameplay_point_binding.json`. The verified project is
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, against executable
SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Native boundary | Original ABI retained as evidence |
| --- | --- |
| `008689C0` | ECX fresh output, EDX parent; stack consumed definition, XYZ pointer, transform byte, option byte, tail word; EAX output; RET14. |
| `008680B0` | ECX raw114h; seven stack DWORD slots: consumed definition, parent, third word, matrix pointer, transform byte, option byte, tail word; EAX raw owner; RET1C. |
| `00B6F5A0` | ECX raw node; stack actual name header; EAX same node; RET4. |
| `00870256..00870279` | Inline fragment, EAX raw24h and ESI0 on entry; ESI same raw owner on exit; no native RET. |
| `00BD30E0` / `00871440` | Zero callback: ECX owner, no stack args, RET; current scalar: ECX owner, stack flags, EAX original owner, RET4. |
| Embedded `00BE0A30` copy | Subexpression of ECX FileBlock, stack name/flag, RET8; the copy helper itself is a new C++ interface. |

## One actual definition count and cache

`GameplayEffectDefinition` remains exactly 24h bytes. Its constructor fragment
`00870256..00870279` now starts the C++ atomic object at the original refs=1
write to +04. It likewise starts the component pointer/count/capacity fields
at the original zero writes. The native representation, initialization order,
untouched +14/+18 words and raw Interlocked acquisition/array paths are retained.
There is no count initialization when a companion is bound.

`GameplayDefinitionReferences::bind` creates only a stable host companion,
borrowing that actual atomic word. Repeated binding returns the same companion
without a retain or write. `definition_for` is a pure identity lookup of an
already-bound reference. The host registry is not another native definition
cache and owns no native references. The existing manager's ID map continues to
hold weak raw pointers.

At zero, the companion validates the current D0DA58 profile: virtual0 is
`00BD30E0`, which invokes current virtual4 with flags1; virtual4 is the existing
`CG_scalar_deleting_dtor_00871440`. It calls the actual reconstructed scalar and
`00870D00` destruction sequence, then removes only the host companion. The name
and raw component array therefore release through their established paths, and
the weak ID entry is erased by the existing destructor. No raw owner access
follows its free. Correct compiler/library names are retained.

Bound definitions must have been constructed by the actual constructor, and all
their terminal releases must use this companion domain. Metadata allocation
failure during first binding leaves the caller's raw reference unchanged. The
domain must outlive all bound owners. Its terminal path follows the existing
nonthrowing intrusive-reference contract and valid-cache/component preconditions;
packet AB did not extend the definition destructor's normal-path coverage.
The subsequent AC packet adds its recovered three-state C++ cleanup; original
native exception dispatch and exception-object ABI remain unvalidated. See
`GAMEPLAY_DEFINITION_UNWIND.md`.

## Borrowed name and current component fields

The constructor chain now accepts a borrowed actual eight-byte name header.
`GameplayPointConstruction` derives its address from the original incoming
definition+1C without reading or copying its values. The complete point
constructor and its node-allocation stage forward that address to `00B6F5A0`.
Existing typed `NativeString` overloads delegate to the same implementation.

`copy_native_string_header_00be0a30_fragment` preserves the established copy
subexpression: compare the two header addresses, resize from source length,
then reload source length/data and destination length/data after allocation.
It does not construct, reset or snapshot the source string. Destination string
initialization remains in its owning constructor. As elsewhere in the existing
string APIs, a zero-byte memcpy is omitted.

`GameplayPointRows` borrows the definition's actual pointer field+08 and count+0C,
and each row's gate+10, threshold+18 and admission byte+1C. It captures no copied
container or row list. Every virtual call rereads the row's current table identity
and the corresponding current word of its supplied actual table binding.
Verified function `0086B7D0` routes to the base predicate; `0086A820` routes to
the real shake factory. Other functions go to required implementations with
the current function identity and original arguments. Unknown tables are rejected;
there is no default success or fabricated null factory.

`GameplayPointConstruction` requires the same row, definition and string
bindings as the constructor. It invokes the real `00866440` insertion lock,
`0086A650` admission, actual114h allocation/free and complete `008680B0` body.
The outer `008689C0` continues to consume the original input reference and its
constructor's extra argument in the recovered order, with the actual F87610
matrix and shared singleton domain. Required node companion associations remain
the existing canonical `NativePlainNodeReference` bindings.

## Validation

The strict MSVC Win32 build and both existing CTests pass. One ignored composition
fixture at `local/gameplay_point_binding_ab.cpp` checks:

- The complete 36-byte original definition-construction fragment against the
  C++ constructor's full24h preimage, with a harness RET and balanced stack.
  Original fragment/table/copy evidence matches live Ghidra and the installed EXE.
- Repeated binding at actual count7 leaves every native byte unchanged and returns
  the same companion. Definition row views address the original +08/+0C fields.
- A null-template `8689C0` call still creates the actual lock and updates XYZ.
  A real definition and shake component then run the complete creation chain,
  store a null event result, apply the expected target side effect, and enter
  the actual live manager with the correct point/template/node references.
- Replacing the source name header inside node-name allocation changes the
  resulting node name. The callback observes actual definition count3 and outer
  insertion-lock depth1; no copied header can satisfy this check.
- Releasing the point through the existing manager performs actual point/node
  teardown, returns the physical node slot, erases the weak definition entry,
  frees its name, and retires the canonical definition companion.
- Throwing from that name allocation leaves the caller's output untouched,
  returns the raw node slot, retains the native constructor counter increments,
  and consumes the actual definition after the insertion lock is released.

The existing Z fixture was recompiled and passed against the changed string/node
chain: its full807-byte original point constructor comparison covers both matrix
selectors, stack balance, owner images, real lifetime, and C++ allocation/factory
failure cleanup. No permanent tests or test framework were added.

The new full-chain fixture runs reconstructed C++ with actual fixture storage;
it does not execute the original whole game entry wrapper. The component has a
separate fixture-held reference, so definition teardown verifies its real array
reference decrement without claiming component terminal destruction coverage.
The fixture also explicitly cleans retained failure counters and the displaced
source name allocation after assertions; neither cleanup was added to production.

Remaining work includes application binding of the real game fields/associations,
remaining current component predicates and factories, actual component-result
terminal ownership, native exception-dispatch ABI, concurrency, and gameplay validation.
