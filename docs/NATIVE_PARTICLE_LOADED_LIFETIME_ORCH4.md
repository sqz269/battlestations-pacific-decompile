# Loaded particle resource lifetime composition

## Result and application contract

The existing cache release path now carries the actual loaded-resource lifetime
domain into `AF4280` / `AF46E0`. Nested emitter rows recognize Cone, Sphere and
SmartArea native identities, and Object model cleanup uses the same concrete
resource-container reference chain used when loading those models.

Two appended optional borrowed pointers preserve existing aggregate initializers:

```cpp
NativeParticleTypeLifetimeContext lifetime{strings, actual_f8d344_pool,
    &actual_resource_container_references};
NativeParticleResourceCacheContext cache{manager_publication,
    particle_cache_publication, &lifetime};
```

Both contexts must outlive every resource using them. `strings` must be the same
raw string publication/gate/manager domain supplied to cache operations;
`actual_f8d344_pool` and `actual_resource_container_references` must be the same
objects used by parameter and Object-model loading. These pointers add no owner,
allocator, registration list, host vtable, lifetime callback, or rollback.

The default-null cache pointer preserves the older string-only path and its
callable-vtable precondition for emitter children. Applications releasing loaded
numeric emitter profiles must supply `loaded_resource_lifetime`. A default-null
Object reference pointer accepts an empty model vector; reaching any nonzero
DWORD model count throws an explicit source-domain exception. The native
`count != 0` test is retained, including negative bit patterns. Parameter cleanup
precedes this boundary; the existing model-vector/base unwind still runs.

## Address and dispatch evidence

| Native address | Composition |
| --- | --- |
| `871420` | Decrement actual resource `+4` once. At zero, capture current profile; proved `BD30E0` dispatch rereads the profile before scalar(flags=1). |
| `871310`, `871480`, `871730` | Existing context-aware clear and inner destruction carry the same cache context without another decrement. |
| `871AE0`, `871B30` | Existing outer destruction carries that context and preserves base/publication cleanup. |
| `871FA0`, `871CA0` | Derived cached resource removes its alias first, then invokes the context-aware base destructor; `C96110` unwind invokes that same base domain. |
| `AFA100` | Both retained-child row loops recognize `D5DEBC -> B03B40`, `D5DE88 -> B02FB0`, and `D5DE48 -> B01EA0`; every table's native slot zero is `BD30E0`. |
| `AF8A40`, `AF8BB0` | Reached Object model cleanup composes existing `AF8940(owner, references)`, then existing vector and base destruction. |

Native `871420` ignores incoming ECX and takes one stacked resource (`RET4`).
Destructors use ECX owner and `RET`; scalar destructors use ECX owner, stacked
flags, EAX original owner and `RET4`. The added contexts are new source C++
interfaces, not recovered native parameters or binary-compatible replacements.

The report preserves fresh pre-change Ghidra comments and points to earlier
name/reconstruction evidence. No Ghidra names, comments or ledger records were
mutated by this worker. All 11 routed function bodies, three derived emitter
tables, resource tables and relevant unwind support were compared in full with
the installed PE. Every body ends at its native `RET`/`RET4`. The release/body
report contains 49 CALL sites, of which 42 direct rows pass the live call checker.

## Validation and limits

The focused ignored fixture `local/loaded_lifetime_probe.cpp` builds a cache
resource containing a Cone -> Sphere -> SmartArea -> Object graph. Four parameter
slots come from one actual F8D344-compatible pool companion. Object's model
uses actual `NativeResourceContainerReferences`, a native `D63228` table copied
from the installed PE, and real container destruction. A retained primary item
proves that container cleanup decrements exactly once. The fixture also exercises
direct `D5D958` resource release, absent Object domains at zero/nonzero valid
counts, and the resource nonterminal reference gate. It installs no callable
host replacement for a numeric table.

The negative-count predicate is checked against assembly/source. An exploratory
`FFFFFFFF` count with null data faulted in the existing `AF89C0` unwind resize,
whose initialization writes at `data-4`; that malformed vector is outside the
valid-storage fixture. No clamp, signed-count shortcut, or rollback was added.

This is source composition validation over an explicitly constructed graph.
It is not parser/load integration, original-body differential execution, native
FH3/SEH or hardware-fault equivalence, original CRT allocation-domain proof, or
gameplay validation. Unknown virtual profiles retain their existing callable
entry-point requirement. See the report for build and fixture receipts.
