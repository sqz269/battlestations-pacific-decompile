# Resource support on the application's raw lifetime

`GameSingletonHost` now owns the actual `0108FEDC` publication cell and a stable
`NativeResourceSupportRawContext` borrowing its existing `01090AA0` manager
publication. The host installs the `D62B64` deletion binding before any support
owner can register. Metadata construction does not allocate a native owner.

The new raw overload of `resource_support_singleton_00b3e730` uses the complete
existing `415350`, `BD0C30`, `B61D50`, and `411EE0` source bodies against raw
storage. It preserves the fast captured publication, captures the first manager's
actual section at `+10`, enters/increments that section, rechecks publication,
allocates eight bytes, and stamps only the original profile. It publishes before
registration, looks up the manager again, and reloads the publication **after**
that second lookup. Registration failure keeps the published allocation and
unwinds only the captured guard. The slow return reloads after leaving the lock.

`NativeSurfaceOwnerContext::actual_lifetime_01090aa0` accepts a borrowed
`NativeResourceSupportLifetime`. Existing semantic callers still select their
previous overload; application callers can select the actual raw context without
constructing a second manager. The raw adapter rejects a different publication
cell before native effects. It owns no native allocation or lifetime state.

The raw manager's finite current-profile dispatcher now admits `D62B64` and calls
the existing `B61D60` deleter. That body unconditionally clears the bound current
publication, stamps `CE3818`, and frees for flags bit zero. It does not require
the popped owner to equal the current publication. The new optional binding is
appended at offset 104; prior offsets are preserved and the source-only binding
structure is now 108 bytes.

## Evidence and validation

The companion report records 324 matching original PE/live bytes: the full
189-byte getter, 9-byte constructor, 41-byte deleter, 29-byte immediate EH region,
52-byte unwind map/FuncInfo, and four-byte `D62B64` slot. All six direct call sites
pass the existing checker. The original two-state map distinguishes guard cleanup
from raw-allocation cleanup; the recovered constructor itself cannot throw an
ordinary C++ exception. Native hardware-fault cleanup remains unproven.

The strict Win32 build and all three existing CTests passed. One ignored probe
links the actual `GameSingletonHost` and built game/core objects. It confirms
shared publication identity and preinstalled deletion binding, one real manager
registration, zero tracked lock recursion after return, stable fast paths,
rejection of a mismatched adapter cell, and real shutdown clearing both support
and manager publications through the recovered dispatcher/deleter.

The probe does not inject allocation or registration failure, execute original
machine code, establish original ABI/FH3/SEH behavior, create surfaces/textures or
a renderer, or validate gameplay. The surface/texture lifetime input is now
composable with the application's actual manager; the remaining renderer graph
still needs concrete application ownership.
