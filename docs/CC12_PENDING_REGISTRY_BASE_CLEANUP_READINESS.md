# CC12 pending registry base cleanup readiness

Baseline: `29aeaf6f7aa4cbf45e6a3b32ff3c65d80b347399`. This read-only packet owns
`008748F0` and the present document/report. It adds no Source, CMake, ledger or
Ghidra change. Evidence:
[cc12_pending_registry_base_cleanup_readiness.json](../reports/cc12_pending_registry_base_cleanup_readiness.json).

## Complete ordinary result

`008748F0..00874900` is **17 bytes, three instructions, zero calls or tail
transfers**. It passes the 300-byte gate. The full body is:

| Address | Instruction | Effect |
| --- | --- | --- |
| `8748F0` | `MOV DWORD[F878CC],0` | Unconditionally clear the actual publication cell. |
| `8748FA` | `MOV DWORD[ECX],CE3818` | Then stamp the supplied receiver's base profile. |
| `874900` | `RET` | Return without consuming caller arguments. |

The cleanup effects left unresolved by the constructor EH audit are now
established for ordinary completion: **publication clear before base-profile
store, with section `+04` untouched**. No missing child service remains in this
body. The ordinary Source store schedule is recoverable; no Source entry or
production publication/lifetime binding is implemented or admitted here.

## Physical ABI and exact storage effects

ECX supplies the actual receiver. This body writes only its first DWORD,
`+00..+03`, and performs no receiver read. The prior constructor establishes
that the surrounding owner allocation is raw8; this cleanup does not access
the section field at `+04..+07`. EDX is neither read nor changed. No caller
stack argument is read, and plain `RET` consumes only the return address.

There is **no semantic return value**. EAX is unchanged, rather than assigned
the receiver. On ordinary completion, the instructions preserve general
registers and arithmetic flags except for ESP consuming the return address.
The saved empty prototype was not used to infer input absence; the complete
assembly establishes ECX and the decompiler also projects a void receiver call.

The first store does not read or compare the current publication. It clears
`F878CC` even if its current value names another owner. The second store follows
without a branch, lock, identity check or receiver validation. Preserve these
two stores and their order, including the actual publication cell identity.
No substitute private cell or same-layout owner follows from this evidence.

There is no section release/unlock, allocation/free, manager lookup,
registration/removal, publication restoration or local exception frame in this
function. The complete body has no descendants. This is a statement about these
three instructions, not a guarantee about the outer unwind chain or retirement.
`CE3818` is a native profile identity; no profile contents or target functions
were expanded and no callable Source table is established.

## Concrete Source reuse

Current `destroy_native_generic_singleton_base_00412430`, declared in
`native_render_service_base.hpp` and defined in `native_render_service_base.cpp`,
already performs the second store:

```cpp
*static_cast<volatile Word*>(actual_receiver) = 0x00ce3818u;
```

`Word` is `std::uint32_t`. Both files are pinned. This is a concrete Source
equivalent for the profile-store effect only. **Native `8748F0` does not call
`412430`**; using that Source service later would not recover native call
topology, register preservation or hardware-fault equivalence.

Targeted `.hpp/.cpp/.inc` searches found no actual `8748f0` or `f878cc` binding
under `include/` or `src/`. The remaining ordinary integration need is the actual
receiver plus a stable binding to the application's mutable `F878CC` cell. No
manager, lock provider, allocator, callback default or owner alias is needed for
these two stores. Implementation remains outside this read-only packet.

## Qualified constructor state-zero implication

The pinned constructor EH record has state-zero pair `{-1,C963E0}`. Its action
loads current `[EBP-10h]` into ECX and tail-jumps to this target. If the shared
helper supplies EBP equal to the ordinary constructor's entry ESP, that slot is
the saved receiver. The shared helper/frame protocol remains unexpanded and is
a required qualification, not an independently proved runtime ABI.

The constructor arms state zero before writing `D0DEA0` and calling the section
creation service. Its normal section `+04` store occurs only after that service
returns. If the action reaches `8748F0` with valid storage and both stores
complete, `F878CC` is cleared before that receiver's profile becomes `CE3818`,
without this function touching section `+04`. The action's current spill is not
replaced by a newly fetched publication or an assumed immutable receiver.

Successful runtime dispatch or cleanup is not implied. A completed publication
store can precede a fault from an invalid receiver on the second store. This
body has no local handler; subsequent nested-failure, original FH3 and hardware-
fault behavior remains unproved. A Source `noexcept` declaration does not close
that gap. No guessed catch or RAII behavior is added.

Getter `C96433`, outer raw-allocation free, captured-manager unlock, complete
owner retirement, callable table delivery, canonical publication lifetime and
the prior pending/group composition remain separate contracts. Local absence
of a section access does not determine what an outer cleanup may do.

## Verification

Fresh queries verified existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 LE32 image base `00400000`; live and saved total
function counts both remained 64,729. **All 17 fresh file-backed bytes** match
the installed PE. Independent Capstone decoding covers all three instructions.
Body SHA-256:
`d6b00cb7e86ca84a971ae28580fec99ba8b7de3ccaf5750da1df4bf81b0d20cb`.
Complete PE SHA-256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Eight current Source/prior pins, fifteen inherited file pins, five inherited
native windows and the existing SDK layout-reference pin pass. The prior
constructor/EH windows were compared with the PE without live re-queries.
JSON parsing and owned-file diff checks complete this audit. No native
descendants, getter EH, other callers, profiles or bootstrap were expanded.
No build, test, probe, runtime run or new Original credit is claimed.
