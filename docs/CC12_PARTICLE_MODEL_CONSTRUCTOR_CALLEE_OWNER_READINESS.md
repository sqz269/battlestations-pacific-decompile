# CC12 particle model constructor callee owner readiness

The smallest ready family is `NativeParticleModelConstructionCallees::call_00bd2f10`.
Its complete x87 provider already exists. The existing application has the canonical
`RandomThreads` owner and mapped read-only scalar cells needed to lend a three-pointer
`NativeParticleUnitRandomAccess` view. A future concrete application callee method can
call that provider directly. This packet adds only this readiness document and report;
it does not create a concrete callee owner, implement C++, or activate model creation.

## Concrete provider and borrow contract

| Required input | Existing production Source | Smallest future binding |
| --- | --- | --- |
| Random state domain | `GameStartupHost::random_threads_`, created by `random_threads_initialize`, selected by `RandomThreads::state_00bd2ed0` | Use the supplied `RandomThreads&`; compose the constructor access with that same application-owned instance |
| Unsigned correction cell | `GameNativeReadOnlyData::data_at(0x00ce3978, 4)` through the existing canonical data owner | Borrow `const volatile float*` |
| Unit scale cell | `GameNativeReadOnlyData::data_at(0x00d63b80, 8)` through the same service | Borrow `const volatile double*` |
| Range arithmetic | `native_particle_random_range_00bd2f10` and `native_particle_random_state_range_00bd2e60` in `src/native_particle_object_state.cpp` | Call the existing function with a local borrowed `NativeParticleUnitRandomAccess` and unchanged stream/bounds |

`NativeParticleUnitRandomAccess` already consists of those three pointers; Source
asserts Win32 offsets 0, 4 and 8. The view carries no ownership. The actual
`GameStartupHost` already has both `random_threads_` and `native_data_` privately;
the future application composition can lend them internally. No extra public accessor,
random domain, seed, ownership wrapper, mapping, copied scalar literal or context owner
is required for this family.

This is a contract for the future method body, not an implemented partial class:

```cpp
const NativeParticleUnitRandomAccess view{
    &supplied_random,
    static_cast<const volatile float*>(native_data.data_at(0x00ce3978, 4)),
    static_cast<const volatile double*>(native_data.data_at(0x00d63b80, 8))
};
return native_particle_random_range_00bd2f10(stream, &view, minimum, maximum);
```

Here `native_data` denotes the existing borrowed canonical read-only service;
`supplied_random`, `stream`, `minimum` and `maximum` denote the actual method arguments.
The existing AF74A0 caller supplies `access.random`, primary stream, zero minimum,
and the current CE3D64 maximum interpreted as float. Preserve those values and the
existing result conversion and manager registration order.

## Exact arithmetic and Source ABI boundary

The complete compiled Source BD2F10 wrapper is 39 bytes / 13 instructions. It forwards
the actual stream to `RandomThreads::state_00bd2ed0` through the existing nine-byte
bridge, then calls the complete BD2E60 provider. Its relocation fields at +16h and
+1Fh identify the state bridge and range body. BD2E60 is 112 bytes / 38 instructions:

1. Retain state+9C5/+9C4 guard checks and the existing deliberate null-write failures.
2. Generate a word from the selected actual state through `random_next_u32_00ba2c20`.
3. Signed `FILD` at +3Eh, conditional current float correction at +46h, and current
   double scale at +4Bh produce the unit value, spilled to binary32 at +4Dh.
4. Evaluate the affine range in x87, then spill and reload the binary32 result at
   +63h/+67h before `RET 8` at +6Dh.

The Source signature has the added EDX access pointer: ECX is stream/state, bounds
are two stack binary32 values, and the result is ST0. This is not a claim of an
Original drop-in ABI. This packet did not re-export or execute Original BD2F10/BD2E60.
The retained complete 1868-byte AF74A0 section calls the callee virtual entry+1Ch
at section+5AEh and spills the result to binary32. That selected call site does not
establish a total class or vtable size.

The double-based gun-dispersion convenience function explicitly admits a one-ULP
rounding difference; the BD2F40 helper returns only a unit sample; the script-orders
method uses its own LCG state. These are not substitutes for the existing x87 range
provider. No new random arithmetic is needed.

## Actual owner lifetime and remaining boundary

The application creates its `RandomThreads` in `GameStartupHost::random_threads_initialize`.
The object owns twenty states, its actual fallback state, thread IDs and lock.
`state_00bd2ed0` selects `states[2 * index + stream]` for the current registered
thread or the actual fallback. Registration and removal use the existing lock;
lookup itself is unlocked. No concurrent-use or fallback-sharing safety follows
from this audit, and the future binding must not add registration, reseeding or
substitute state.

`run_win_main` initializes random before application construction. Its normal path
drains application and singleton lifetime owners before unregistering and shutting
random down. `GameStartupHost` destructor fallback likewise drains singletons before
deleting `random_threads_`; process-retention `_Exit` paths remain separate. The
already-running early return intentionally omits the explicit `run_win_main` teardown.
Every future constructor/callback borrow must end before actual random destruction.

The already-selected CE and D6 mapping bands contain both scalar cells. `data_at`
validates section bounds and mapped bands, then returns the actual address; it can
throw for absent/invalid storage. `GameNativeCanonicalDataOwner::initialize` publishes
only after the checked handoff ACK and deliberately retains its process owner. It
owns the read-only service passed to `GameStartupHost`, with lifetime through CRT and
thread teardown. This establishes a Source owner path, not live mapping validation.

The full `NativeParticleModelConstructionCallees` remains abstract. Its other methods
are explicitly unresolved here: `bind_particle_profile`,
`retire_failed_particle_profile`, `bind_mesh`, `bind_material`, `bind_section`, and
`renderer_virtual5c`. No forwarding to another unresolved interface, stub, partial
concrete class or new Native body was introduced. The concrete application owner
and `NativeParticleModelConstructionAccess` composition remain future C++ work.

The separate F8D2D0 pool contract is preserved: actual slot allocation, exactly-once
raw return after failed construction, and the same pool borrow for successful derived
retirement. This RNG view neither retains nor returns model slots. The whole pool
dependency document/report from `cbac550781e09b6d96a68c46895a24e429a8ad79` is retained;
its acceptance was pending when this packet was assigned. No old proof tree is followed.

## Retained evidence and replay

Captured Source commit: `e491e369708422f35fec6f582555387a2e4b7550`. The accepted C++ baseline is
`99b170231e937da304703fe01221bcc8b8b8aef7`. Separate actual Git/current-Source verification also
compares the five C++ root entries with current main `20a408581fe2b4da2deffaa11238bad059fd649a`;
they are exactly equal. This Source-only packet ran no build or tests.

The full frozen `Source3352-particle-model-typed-views` qualification retains 3352
project inputs plus four build artifacts (3356 epoch pins). Separately, this packet's
selected project readset is also 3356 paths: the frozen project inputs plus the real
tlog dependency closure of twelve selected translation units (436 tracked paths) and
build inputs. The equal number is coincidental. The complete query corpus is 4185
tracked C/C++ files under src/include/tests; its union with the selected readset is
4214 paths. Whole committed, worker and Root bytes are retained. Root bytes match the
frozen epoch exactly; seven known worker files differ only by CRLF/LF. External SDK
and compiler dependency paths are enumerated without claiming their byte preimages.

All 56 compiler logs are retained. The selected twelve complete COFF objects comprise
ten Core translation units and two App translation units. The full Core archive walk
has 1991 entries / 1988 COFF members. All 80 current App objects are independently
enumerated from the vcxproj and compiler item logs. The older Root metadata contains
a reviewed subset of 70; its ten omitted objects are enumerated without claiming old
before-snapshots. No archive or App coverage is inferred from a selected interface.

Full object indices preserve all sections, payloads, labels, primary/aux symbols and
relocations. Twelve complete selected function sections total 4010 bytes /
1228 instructions.
Twenty selected sections declare storage with raw pointer zero; their payload is
correctly empty. The complete Core+App targeted census finds one BD2F10 external
definition / fourteen code relocations and one BD2E60 definition / one code relocation.
It finds no named concrete `call_00bd2f10` or `NativeParticleModelConstructionCallees`
external definition. The complete Source queries separately show only the abstract
interface, borrowed access and call. These are bounded Source conclusions, not proof
against every possible alias, indirect call, or runtime implementation.

Ten full-corpus queries and 27 Source excerpts are replayed. The report contains
their exact expressions/results, the selected complete decodes, and external pins
for every retained full index and local helper. The portable replay validates Git
commit/tree/blob preimages and whole pool dependency bytes from the ZIP. Actual Git
object/current-working-file checks remain a separate live verification.

| Artifact under `local/cc12_particle_model_constructor_callee_owner_readiness` | Bytes | SHA-256 |
| --- | ---: | --- |
| `evidence.zip` | 41761238 | `cde8df79a740a235efae23456605460f419587ac920fbcf6d59e4c5402adab2d` |
| `portable_replay.py` | 21570 | `74483208ba41c33fc28a6459016a7d5cca8a5546b5d588aca47dd76271ddd313` |

The ZIP has 4414 members and the helper validates 4409
unique retained pins. In-place replay and replay from a physically copied ZIP/helper
both pass with identical results. After Python, Capstone, codec and XML imports, the
audit guard permits file opens only for the supplied ZIP and blocks writes, process,
network, library-loading and directory-enumeration operations. No compiler, Native
function, game, or reconstructed executable runs inside either replay. Copy hashes
are independently rechecked before publication.

```powershell
python local/cc12_particle_model_constructor_callee_owner_readiness/portable_replay.py local/cc12_particle_model_constructor_callee_owner_readiness/evidence.zip
python local/cc12_particle_model_constructor_callee_owner_readiness/verify_git.py
```

Result: the selected random family has an existing provider and concrete borrowed
owner path ready for future composition. Original ABI equivalence, live RNG behavior,
model activation, terminal lifetime safety and a completed application callee owner
remain unestablished.
