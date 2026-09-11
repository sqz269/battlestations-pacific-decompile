# Resource registry scalar deletion

Two distinct native entries, `B1B660[30]` and `B1B710[30]`, now have complete
raw Win32 source. Their 11-instruction bodies are identical except for native
relative call operands. The profile words at `D5E594` and `D5E59C` select these
different entries; both profiles also contain `B1B3A0`. This identifies the
two deletion routes without establishing their constructors or factory policy.

## Order and interfaces

Native receives the registry in ECX and a DWORD flags argument at entry
ESP+4. It captures ECX in saved ESI, calls the full registry destructor, and
only after normal return tests bit 0 of the current **byte** in that argument
slot. When set, it passes the captured registry allocation to free. Finally
it returns the captured pointer bits in EAX, restores ESI and executes RET4.
Other flag bits are ignored. A freed return pointer is not dereferenced.

The three-byte `ADD ESP,4` after each free call is part of the complete body.
Saved Ghidra listings initially omitted those instructions after `B1B670` and
`B1B720`; the original executable and complete independent decoding establish
the missing cleanup. Neither path follows the possibly changed publication
cell to select the allocation to free.

The new fastcall interface retains native ECX and the actual flags stack slot,
and adds EDX pointing to a 12-byte `NativeResourceRegistryDeleteBindings`.
Its references identify the actual `F8D41C` publication, actual string-pool
storage provider and established invalid-parameter service. These bindings
must remain valid and unchanged during the call; the underlying native state
remains current and mutable. No publication value or flag snapshot is made.

A fixed 18-byte fastcall bridge pushes those three reference addresses and
the captured registry into the existing complete destructor, then removes its
16 argument bytes and returns. A fixed five-byte cdecl tail bridge reaches
`singleton_lifetime_free`. Both are concrete service adapters. Neither entry
accepts a replaceable destruction/free callback or invents a partial
destructor. The existing destructor performs full range erasure and current
head free, clears head/count/publication, and installs the base profile.

## Verification and limits

Eight freshly guarded Ghidra/PE spans total 213 bytes: both complete entries,
full destructor/reset identities, original free thunk, both profile tables
and adjacent padding. Each query verified the existing `bsp` project and
`/battlestationspacific.exe`. An independent worker reviewed the full source,
header, native bytes and actual stack/provider contracts without changing
them.

The strict main Win32 build, both existing CTests and eight seed checks pass.
`local/registry_scalar_delete_build_frozen/` retains the actual library,
11 exact archive members and 34 unchanged source/header/build inputs. The
proof verifies all 60 owned bytes and 22 instructions after only four
declared direct-call relocations. It also verifies every byte and relocation
of both fixed bridges and records all 451 COFF sections across those objects.
The ten provider objects are byte-identical to the previous main library
whose complete destructor/reset composition was executed successfully.

No new runtime test was needed for these completely checked raw wrappers.
The prior provider execution is not a claim that either new scalar entry was
executed. The proof does not establish linked addresses, original CRT bytes,
native SEH/unwind or arbitrary original-caller ABI compatibility. Nonvolatile
preservation across the concrete C++ providers relies on their established
Win32 ABI; provider volatile-register results remain outside the contract.
No new null guard, exception cleanup or allocation policy was added. The
inherited pool/free exception restrictions remain. Registry construction,
singleton getters, population and gameplay validation are separate work.


## Primary integration

Both complete entries are registered in the address ledgers. The original returning-free flow and both missing three-byte cleanup spans are repaired in saved Ghidra; prior comments and old values are retained. Both reviewed names and evidence comments are saved and affected exports forcibly refreshed. Immutable primary evidence is in `local/registry_scalar_delete/`. The strict main build, two existing CTests and eight fresh seed spans passed; wrapper runtime and game behavior remain untested.
