# Native allocator failure-object constructor Source candidate

`bsp::construct_native_allocator_failure_object_00bf6802` implements the
complete effects of the accepted 25-byte `00BF6802..00BF681A` wrapper through
the now-admitted 24-byte `00BF638E` base-message leaf. The Original library name
`bad_alloc` remains unchanged. The descriptive Source name is provisional.

This worker packet is a static Source candidate. The primary integrator owns
CMake registration, the normal MSVC Win32 build, complete emitted call/body
and Core review, and final ledger/Ghidra admission. No worker compilation,
fixture execution, ABI admission or game validation is claimed. Applied
Original/Source admission credit is zero.

## Qualified Source interface

The new ordinary C++ API is:

```cpp
void* construct_native_allocator_failure_object_00bf6802(
    void* actual_raw12_receiver,
    const void* actual_pointer_slot_address);
```

It requires MSVC Win32. The receiver is actual live backing for 12 bytes. The
second argument is a borrowed actual address of a readable four-byte slot,
which substitutes the Native wrapper's fixed immediate `00E154B4`. The Source
API does not provide that Native address, its actual cell producer, a shared
static failure-object owner or the slot's pointee. No string contents, profile,
RTTI, ThrowInfo or other static data are read for this packet.

The Native wrapper instead receives its receiver in ECX, accepts no explicit
stack arguments and uses plain `RET`. The new two-argument ordinary C++ API
is a qualified Source interface; it is not a drop-in Native entry or a promise
of exact wrapper register/flag behavior. The child retains its admitted naked
fastcall binding and literal memory-access sequence.

## Composition and memory schedule

The implementation captures the passed receiver in `captured_receiver`, calls
the real `construct_native_allocator_base_message_00bf638e` with that receiver,
unused EDX word zero, the passed actual slot address and ignored numeric word
one, then performs a volatile DWORD store of `00D6923C` at the captured
receiver+0. It returns the captured receiver and ignores the child's result.
The final store is unconditional after a normal child return.

| Order | Provider | Operation |
| --- | --- | --- |
| 1 | Child | Capture the actual slot address |
| 2 | Child | Store raw profile `00D69370` at receiver+0 |
| 3 | Child | Read one DWORD from the captured slot address |
| 4 | Child | `AND DWORD [receiver+8],0`, an actual read-modify-write |
| 5 | Child | Store the captured slot value at receiver+4 |
| 6 | Wrapper | Store raw profile `00D6923C` at captured receiver+0 |
| 7 | Wrapper | Return the originally captured receiver |

The child writes every byte of the 12-byte receiver footprint. Its last DWORD
must be readable as well as writable. The wrapper adds one four-byte write
within that footprint and does not enlarge it. The child's complete audit and
primary admission resolve the earlier wrapper audit's transitive-footprint
uncertainty; that older report remains historical rather than being rewritten.

Aliasing retains the child's ordering. A slot at receiver+0 supplies the
child's first profile value before the wrapper installs the final profile.
A slot at receiver+8 is read before the child's RMW clears that DWORD. A zero
copied word is allowed, and its pointee is never accessed. There are no receiver
or slot null checks. A fault or non-returning child prevents the final wrapper
store; earlier effects may remain. No catch, `noexcept`, free, destructor,
allocation policy or fallback path is added.

Both profile constants are raw numerical data. The implementation provides no
callable Source profile, exception class, RTTI, ownership transfer or
`std::bad_alloc` identity. It performs no cast to the separate 0x28-byte owner.
Actual backing, slot/pointee lifetime and broader exception/runtime identity
remain separate caller contracts.

## Accepted complete bodies

The wrapper's nine instructions save ESI, push numeric one and fixed address
`00E154B4`, capture ECX in ESI, call `00BF638E`, store `00D6923C` at captured
receiver+0, copy that receiver to EAX, restore ESI and use plain `RET`:

```text
566a0168b454e1008bf1e87dfbffffc7063c92d6008bc65ec3
```

The admitted child's seven instructions capture ECX, capture the slot address,
publish the first profile, read the slot, perform the DWORD RMW, store the slot
value and use `RET 8`:

```text
8bc18b4c2404c7007093d6008b0983600800894804c20800
```

These are complete Original bodies, not predicted encodings of the ordinary
C++ wrapper. The child primary review establishes a complete emitted-byte
match for that child and its normal build/Core admission. This packet inherits
that report and verifies its current input pins; it does not rerun the earlier
build or claim access to unselected emitted artifacts.

## Validation and remaining admission work

The report pins the accepted wrapper audit, complete child readiness, admitted
child primary review, child Source files and relevant build inputs at published
base `a6b846ecce1f780962154003f1d30c7f4933870f`. It replays the wrapper and child
readiness pins and excerpts plus all child-primary Source/document pins. The
report distinguishes physical working-tree hashes from canonical-LF hashes.

Static checks independently decode and compare all 25 wrapper bytes / nine
instructions and all 24 child bytes / seven instructions with the configured
PE, check the complete candidate Source call/store/return sequence, and verify
the four-file staged scope. No new Native body, static data, caller or Ghidra
query is used. The historical library names and saved prototypes are preserved.

The wrapper is unregistered at the worker base, so no worker build, ad hoc
compiler probe or new test is run. The primary must register this file, run the
normal build, verify the real child call and complete emitted wrapper, inspect
Core selection and finalize admission. The conditional proposal is one newly
reconstructed Original function / 25 bytes. The already-admitted 24-byte child
is not counted again. Broader allocator/CRT/exception, startup, static throw
object, Original ABI and gameplay behavior remain unclaimed.
