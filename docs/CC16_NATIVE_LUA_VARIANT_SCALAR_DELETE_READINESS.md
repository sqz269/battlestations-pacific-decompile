# Raw Lua variant scalar-delete readiness

Frozen published Main `82e4d87735e48b2796c9a316780ad1c146d7a5a4` contains the
admitted qualified retirement candidate `a4774a47499cf3ad0f38af058a53fef1e71cea61`.
The two retirement Source blobs are exact. The actual qualified884AF0 child
and canonical `singleton_lifetime_free` are available, but no complete Source
binding for scalar00884FA0 was found. Existing generic scalar operations do not
directly provide its late-home/status contract. One bounded, complete additive
ordinary C++ entry is ready to implement; this packet implements nothing.

This corrects the initial brief's28-byte extent. The complete body is
**00884FA0..00884FBD,30 bytes/11 instructions**. RET4 starts884FBB and occupies
three bytes;28 bytes ends at its first byte. The cached listing and fresh
installed-PE decode agree. Existing CG/library naming and classification are
preserved; this readiness supplies no new Original/vendor/duplicate-body credit.

## Exact Native body and current routing

Image SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The complete scalar bytes are
`568bf1e848fbfffff644240801740956e8f715370083c4048bc65ec20400`,
SHA256 `1e12378e15ffd6846e22c5321444c71e85ca37f9f502a575667414cc12a7f02b`.

| Native epoch | Complete effect |
| --- | --- |
| 884FA0..884FA1 | Preserve ESI and capture entry ECX receiver in ESI. |
| 884FA3 | Direct call884AF0 on that receiver. No scalar-local EH/cleanup frame is introduced. |
| 884FA8 | Only after child return, TEST the CURRENT LOW BYTE at the actual incoming flags home, ESP+8 after saved ESI, with mask01. |
| 884FAD..884FB5 | If clear, skip free. If set, push captured ESI, callBF65AC `_free`, then remove its argument. |
| 884FB8..884FBD | Return captured receiver bits in EAX, restore ESI and RET4, including when that allocation was freed. |

Fresh PE checks also match the entire101-byte884AF0 child and four-byte
D0E6F4 profile word. D0E6F4 points at884FA0. The current indexed graph has no
direct caller and one data-pointer site atD0E6F4; callees are884AF0 at884FA3 and
BF65AC at884FB0. This is a bounded indexed graph observation, not proof of no
indirect/runtime callers or complete CF/D0 adoption. Lookup has the existing
`cg_scalar_deleting_dtor` tag and no reconstruction record for this address.

The current raw008849B0 initializer writes numericalD0E6F4 into20-byte backing.
The admitted884AF0 child also writes that numerical profile. Neither installs
or authenticates a callable Source table. Broad Left55/Argon/Hooks routes and
CF/D0 object/current-slot/producer transport remain separate. No caller, table,
global cell or dispatch is changed here.

## Existing operations do not close this contract

| Operation inspected | Why it cannot directly bind this complete scalar |
| --- | --- |
| `engine_services_cc13::scalar_delete_via` | Flags are passed by value. Its fixed enum invokes unrelated void Host methods, with no Lua case or returned non-completion. |
| `cc13_scene2::model_scalar_delete` | Flags are passed by value; void `std::function` destructor/pool callbacks cannot forward the qualified child's explicit Result. A callback shim, swallowed status or fabricated exception would change the contract. |
| `scalar_delete_storing_vtable`, singleton/string-record helpers | Different specific teardown effects, value flags and no qualified-child status propagation. A table store or one string release is not884AF0. |
| Concrete enum scalar body | Real canonical free is available, but its fixed enum destructor/domain and value flags do not supply this body. |
| Resource-registry scalar entriesB1B660/B1B710 | Their naked bodies correctly read a late LOW BYTE, but bind fixed unrelated void registry bridges. They expose no returned Source non-completion or direct qualified Lua binding; their ABI/provider exception transport cannot be borrowed. |
| GUI/address dispatch helpers | Unrelated destructor/pool tables, value flags, void Host calls and optional default returns; no direct actual Lua composition. |

The scoped include/src address, scalar-template and flags-home searches are
retained with their full results and a corpus digest. Their semantic limit is
explicit: name search alone is not a proof about every possible operation.
The relevant generic and concrete alternatives above were inspected. Reusing
them would require new adapters/policies and still lose required epochs or
status. The smallest closed packet is the actual30-byte body's complete Source
composition in the existing retirement header/CPP, with no new provider seam.
The three scalar-template-neighborhood hits are GUI pool allocate/free
declarations or a field-access template beside an unrelated singleton helper;
none is a generic status-aware scalar deletion operation.

## Proposed closed ordinary C++ interface

The following is a proposal only, not an installed declaration or implementation:

```cpp
struct NativeLuaVariantScalarDeleteResult {
    NativeLuaVariantRetirementResult completion;
    std::uintptr_t captured_receiver_bits;
};

NativeLuaVariantScalarDeleteResult
scalar_delete_native_lua_variant_00884fa0_cpp_cleanup(
    void* actual_variant,
    const volatile std::uint32_t& actual_flags_home,
    NativeStringRawPoolContext& strings,
    NativeLuaVariantRetirementOperation& operation,
    const NativeLuaVariantCppCleanupBacking& cleanup);
```

There are no default fields, implicit success/Boolean conversion or pointer-
success convention. The two32-bit fields remain separate: `completion` is this
Source entry's returned completion; `captured_receiver_bits` is the captured
identity on any ordinary return. Only completed status corresponds to the Native
normal EAX return. A non-completion's identity field is diagnostic custody, not
success. Even on completed status, those bits can identify a freed allocation
and must not be dereferenced as a live object.

The complete body must:

1. Capture entry V and its pointer bits without reading flags.
2. Call actual `destruct_native_lua_variant_00884af0_cpp_cleanup` with the SAME
   actual strings, operation and sidecar backing. Use its returned Result;
   `operation.result` is a latest nested diagnostic, not the completion test.
3. If the returned Result is non-completed, return it with captured identity
   before any flags read or receiver free. Retain the child's stopped metadata.
4. Only after child completion, read exactly ONE CURRENT unsigned byte at
   offset0 of `actual_flags_home`, through a volatile unsigned-char alias.
   Test mask01. The DWORD reference borrows a real live home; evaluating the
   DWORD itself would widen the Native byte read and is not this proposal.
5. If set, call actual `singleton_lifetime_free` on captured V. Return completed
   status and captured bits after normal free/skip. Do not reload a receiver
   from a profile, owner, publication, output slot or changed object field.

Passing a flags value, copying it into another home at entry, reading before
the child, or caching a bit cannot substitute for the actual late home. A legal
alias that changes the home during the child must be observed at the late read.
The explicit Source home does not reproduce/authenticate the Original private
stack slot or arbitrary Native private-stack aliases/ABI.

No scalar catch, guard, owning destructor, extra metadata frame or new cleanup
action is needed. If a genuine C++ exception escapes the qualified child, normal
C++ control flow must skip flags, free and result return. Preserve its retained
operation/sidecars and existing primary/secondary/provider limits. Do not turn
that escape or a returned non-completion into success, another exception,
fallback cleanup, retry, forced demotion or forced termination.

## Actual domain and implementation gate

V must identify genuine live20-byte variant backing satisfying all qualified
884AF0 nested payload/name/map and provider preconditions. If the LATE low-byte
bit0 is set, V must also be the exact root allocation base compatible with the
existing canonical malloc/free domain. Entry flags cannot establish that domain:
the actual home can change before the test. A retained embedded receiver may be
usable when the actual late free bit is clear; it cannot be freed as a root.
Numerical profile words, sizes and tags authenticate none of these obligations.

`singleton_lifetime_allocate` uses actual malloc/new-handler behavior;
`singleton_lifetime_free` directly invokes `std::free` and remains noexcept.
Use those current providers, not an injected free callback, class operator
delete, pool return, Native address cast or fallback allocator. The required
actual AA8/AA0/AA4, manager/deletion/section/ring/storage and matching heap
custody remain those of the admitted qualified child.

Keep the flags home live/readable through its real read, and context plus typed,
disjoint caller-owned operation/sidecars live through last use and actual failure
disposition. Backing covers the actual child/nested-map chain. There is no
count/fixed/history limit, heap logging, journal replay or added null/default/
profile/flags-policy guard. Volatile supplies the read epoch, not cross-thread
synchronization or fault/atomic proof.

Main's frozen qualified-cleanup admission provides prior common build/link/PE
receipts for the child and actual free. This readiness verifies current immutable
Source pins and Native evidence; it performs no fresh build/link or execution.
Direct provider symbols or ordinary parent catches do not prove every actual
naked/register/provider-boundary exception reaches C++ handlers. Existing
noexcept/CRT/OS/nonlocal exits, Original ABI/FH3/SEH/fault/private-home state,
secondary-terminal/exception-object custody and CF/D0 caller graph remain open.

After Root authorizes the separate complete Source packet, lease its actual
address/files, preserve old APIs/layouts/legacy Hosts, freeze the implementation
and independently review exact late-byte load, all status/exception exits,
captured free/return identities and real provider imports. Root common build/link
and the later explicit adoption/runtime/gameplay gates remain required. This
packet changes only its two readiness files; it adds no Source, scalar activation,
table/global/caller/CMake/ledger/Ghidra change, test, build, link or target run.
