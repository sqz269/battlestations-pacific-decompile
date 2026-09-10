# Light type bootstrap and shared type-counter lifetime

This packet reconstructs the root, node, light and directional-light descriptor
chain and its three type predicates. The counter is the actual eight-byte owner
published at `0109DB7C`, registered with the same supplied
`SingletonLifetimeDomain` used by the other process singletons. Neither service
owns a private counter, initializes a guard on construction, assigns replacement
type IDs, nor implicitly runs startup from a predicate.

Evidence is in [the audit](../reports/light_type_bootstrap_audit.json). All nine
owned native spans (688 bytes), the five-byte CRT free observation boundary and
13 data spans (111 bytes) match the installed executable and the live saved
Ghidra program. The executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Entry | End, exclusive | Original ABI and behavior |
| --- | --- | --- |
| `006FAC20` | `006FACD5` | No arguments; EAX shared counter; RET |
| `006FAD40` | `006FAD69` | ECX counter, stack flags; EAX original address; RET 4 |
| `00BEA780` | `00BEA7AC` | ECX root descriptor; RET |
| `00B6F110` | `00B6F14E` | ECX node descriptor; RET |
| `00CD80A0` | `00CD80EF` | No arguments; initialize actual light descriptor; RET |
| `00CD80F0` | `00CD8191` | No arguments; initialize actual directional descriptor; RET |
| `00B6F570` | `00B6F598` | Stack token; AL boolean, other EAX bits unspecified; RET 4 |
| `00B7C580` | `00B7C5A8` | Stack token; AL boolean, other EAX bits unspecified; RET 4 |
| `00B7C6D0` | `00B7C6F8` | Stack token; AL boolean, other EAX bits unspecified; RET 4 |

The three predicate entries and `006FAD40` were not defined as Ghidra functions
when this packet was inspected. Their disk instructions, matching live bytes and
complete terminating RETs establish the bounds above; the integrator owns their
Ghidra definitions, names, saved annotations and ledger records.

## Actual storage and initialization order

| Descriptor | Guard | Descriptor address | Token words, in native order | Name word |
| --- | --- | --- | --- | --- |
| Root | `0109DB80` | `0109DB84` | own | `00D68BBC`, `cRoot` |
| Node | `0108FF54` | `0108FF90` | own, root | `00D62C7C`, `c3dNode` |
| Light | `0109010D` | `0109018C` | own, node, root | `00D62F14`, `cLight` |
| Directional light | `0109010E` | `0109019C` | own, light, node, root | `00D62F1C`, `cDirectionalLight` |

`LightTypeBootstrapStorage` retains references to these four guards and four
descriptors. Its plain storage structs have no member initializers. Name words
retain native address identities; they are not host string pointers. Win32
descriptor sizes are respectively 8, 12, 16 and 20 bytes.

Every initializer treats any nonzero guard as complete. It publishes guard 1
before making the calls that may allocate or throw; it does not roll the guard
back. Root consumes the shared counter, increments it, writes the own token,
then writes the name. Node first writes its name, initializes the actual root
descriptor, copies its token, and consumes the same counter for its own token.
The ECX targets of the root and node entries need not be the global descriptor;
their guards remain process-wide even for another target.

Light writes its name, initializes the actual node descriptor, captures both
node/root tokens before storing their copies, then consumes its own token.
Directional initialization tests the light guard at `00CD80FD` before writing
its own guard/name. The implementation preserves that captured decision. It
initializes light only when that earlier test requested it, captures all three
parent tokens before storing them, and finally consumes the directional token.
Unsigned counter addition preserves native modulo-2^32 arithmetic; token
uniqueness relies on normal process lifetime and no wraparound.

The predicate leaves compare live DWORDs in the table's order and return at the
first match. They read no guard and initialize nothing. In zeroed startup
storage, token zero therefore matches even before valid type initialization.
Inherited words are stored copies, so a later change to the node descriptor is
visible to the node predicate while the light/directional copies stay unchanged.

## Shared counter ownership

`TypeIdCounterLifetime` takes the actual `TypeIdCounterStorage* volatile&`
published slot and the existing process `SingletonLifetimeDomain&`. The caller
keeps both alive through lifetime-manager shutdown and dispatches registered
owner vtable `00CFB6C4` to `deleting_destructor_006fad40` with the manager's flags.
The companion destructor does not initiate shutdown or reset static state.

The getter's fast path returns the first global load. On a null slot it obtains
the manager, captures its current section, enters that section, increments the
explicit recursion field and rechecks the slot. It allocates eight raw bytes,
sets vtable `00CFB6C4` and counter zero when nonnull, then publishes the pointer.
It calls the manager getter again and reloads the slot for registration, even
on the native returning-null branch. It unlocks the originally captured section
before its final global load. C++ exception unwinding releases that captured
section. The established allocator's new-handler retry/throw behavior is reused.

The deleting destructor unconditionally clears the actual published slot,
writes base vtable `00CE3818`, and frees the supplied owner only when flags bit
zero is set. It returns the original address. It does not unregister the owner,
clear its count word, or reset descriptor guards. Calling it on an arbitrary
owner still clears the global; there is no invented identity check. Process
startup sequencing and the existing manager's dispatch routing belong to the
integrator; this packet supplies the complete bounded type-chain behavior.

## Validation boundaries

A single focused Win32 native/host scenario passed: six matching 20-word state
snapshots and 108 predicate calls per path. Two other type IDs first consume the
same provided counter; directional initialization then produces root/node/light/
directional IDs 2/3/4/5. Repeated initializers preserve all guards and alternate
root/node targets, live node-token changes affect only its predicate, and another
consumer advances the shared counter without rerunning initialization. Teardown
observes global null and base vtable before the real free on both paths.

The fixture runs the nine verified native bodies in sparse reserved image space,
rebasing 58 recorded absolute operands/vtable words. Only explicit CRT free
boundaries are redirected to observing real frees; unprovided code is inaccessible
or INT3-filled. It supplies an already existing counter to the native getter, so
native counter creation, native registration and native manager shutdown were
not executed. The host path separately creates the real manager/counter,
verifies one registered owner and zero retained lock recursion, and dispatches
the registered owner once through real manager shutdown. Both free observations
retain the count word value 7 and the descriptor guards remain set.

The new source compiles under MSVC x86 `/W4 /WX`; the fixture passes. The standard
repository build and existing CTest are also run, while the integrator adds the
new source to the shared CMake source list. This is reconstructed and narrowly
fixture-tested behavior, not a native ABI replacement or game validation. Native
slow-path exception handling, returning-null allocation, allocator failure,
concurrent initialization, wraparound, alternate-target first initialization,
and full process startup/exit ordering were not executed by this fixture.
