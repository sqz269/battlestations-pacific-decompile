# Native allocator base-copy constructor Source candidate

`bsp::construct_native_allocator_base_copy_00bf63a6` implements the complete
owned schedule of `00BF63A6..00BF63FD`: 88 Original bytes / 38 instructions.
It retains the four paths, three CRT calls, field widths, real null-message
DWORD read-modify-write, late source-pointer reload and `RET 4`. The saved
library name `exception` remains unchanged; the Source name is provisional.

The existing private `copy_base` helper now calls this adapter using addresses
of its actual `NativeLegacyExceptionStorage` owners and explicit unused EDX
word zero. This removes the helper's previously unexpressed RMW contract and
corrects its old endpoint comment from `00BF63FE` to `00BF63FD` / 88 bytes.
No other legacy-owner implementation is changed.

This is an unregistered Source candidate. The primary integrator owns CMake
registration, the normal MSVC Win32 build, complete emitted/CRT-call/Core and
consumer review, and final ledger/Ghidra admission. No worker compilation,
emission, fixture execution, Original ABI or gameplay claim is made. Applied
admission credit is zero.

## Raw interface

```cpp
void* __fastcall construct_native_allocator_base_copy_00bf63a6(
    void* actual_receiver,
    std::uint32_t unused_edx,
    const void* actual_raw_source);
```

The header is guarded for MSVC Win32. The implementation is naked inline
assembly, with pointer, DWORD and `size_t` widths asserted to be four bytes.
The explicit second argument occupies EDX so the third argument is the actual
source address in the first stack word. The owned code does not read incoming
EDX; its current CRT children may clobber volatile registers. The public
declaration makes no additional EDX preservation promise.

ECX supplies destination `D`; `[entry ESP+4]` supplies source `S`. After
`PUSH EBX`, `[ESP+8]` captures the same source word in EBX. ESI captures `D`.
The body saves/restores EBX, ESI and EDI on normal return, puts the captured
destination in EAX and consumes the source stack word with `RET 4`.

Callers provide actual raw 12-byte backing. Destination DWORDs 0, 4 and 8 must
be writable, source DWORDs 4 and 8 must be readable, and destination+4 must
also be readable on the null-message RMW path. The source profile at +0 is
not read. No larger type is inferred or manufactured for either raw argument.
Pointee validity and allocation lifetime remain separate caller/provider
contracts.

## Complete schedule

The common prefix publishes raw `00D69370` at `D+0`, reads the full control
DWORD at `S+8`, writes that unchanged DWORD at `D+8`, tests it, and reads the
message pointer from `S+4`. The message load and `PUSH EDI` preserve the
control-test flags used by the first conditional jump.

| Condition | Retained behavior |
| --- | --- |
| Control is zero | Store the initially captured message at `D+4` |
| Control is nonzero and message is null | One actual `AND DWORD [D+4],0` RMW |
| Control and message are nonzero, allocation is null | Call length/allocation providers, publish null at `D+4`, then return with the control word unchanged |
| Control and message are nonzero, allocation succeeds | Publish allocation at `D+4`, reload current `S+4`, call the copy provider, ignore its result and return captured `D` |

The length result is copied to EDI and incremented as a 32-bit value. The
delivered allocation size is `(length + 1) mod 2^32`. The length-call argument
remains on the caller's stack while the allocation size is pushed. After
`malloc`, two `POP ECX` operations remove both argument words without altering
the flags from its result test; the following pointer store also preserves
those flags for the null-allocation branch.

On success, the exact reload is `PUSH DWORD [EBX+4]`, after destination pointer
publication. The next two pushes deliver size and allocation, forming
`strcpy_s(allocation, size, reloaded_source)`. Caller cleanup removes 12 bytes.
The shared epilogue returns the captured destination regardless of the copy
provider's return value.

The null-message operation remains the single DWORD RMW at Original site
`00BF63ED`. No C++ null store is substituted. There is no destination/source
null check, self-copy check, prior free, extra EH frame, catch, `noexcept`,
throwing singleton allocator, ownership reset or additional helper call.
The actual inline assembly retains every owned instruction and branch.

The provider identifiers are current Source CRT `strlen`, `malloc` and
`strcpy_s`. Their actual emitted bindings require primary compilation/Core
review. Native child implementations, addresses, invalid-parameter behavior,
allocator identity and failure handling are not established by those names.
No new Native child body or static data is read for this packet.

## Aliasing, ownership and profile qualifications

The profile store occurs before source-field reads. The destination control
store occurs before the first message read, and the allocation is published
before the later source reload. Partial effects may remain on faults or
non-returning calls. With exact destination/source aliasing and a successful
allocation, the reload observes the newly published allocation; no provider
outcome for that delivered argument pair is inferred.

The control DWORD is copied in full and tested against zero, without bit
masking or boolean normalization. It is not cleared after a null allocation.
The leaf's own field accesses remain within the 12-byte base; accesses through
pointees and CRT effects are separate contracts.

`00D69370` remains opaque numerical data. No callable vtable, RTTI, exception
type, static failure owner, Native slot, string contents or throw metadata is
created. The Source fastcall interface, current CRT calls and future emitted
code do not establish Original placement/callers, whole-call flags/fault
equivalence, Native exception/runtime identity or drop-in binary ABI.

## Existing typed consumer

The private helper remains `void copy_base(NativeLegacyExceptionStorage&,
const NativeLegacyExceptionStorage&)`. Its two actual 0x28-byte owners already
provide valid leading 12-byte storage. It now performs only this delegation:

```cpp
(void)construct_native_allocator_base_copy_00bf63a6(&owner, 0u, &source);
```

These are direct addresses of actual typed owners. No raw 12-byte allocation
is cast to the larger type. The helper discards the returned pointer just as
its existing `void` contract requires. Member-string initialization, cleanup
guards, destruction, scalar retirement and all other owner code remain
byte-for-byte unchanged in Source outside the one include and helper edit.

## Static evidence and primary gate

The accepted copy-readiness report supplies the complete 88-byte Original
span, 38 instruction boundaries and metadata-only children. This packet
independently compares that full span with the configured PE and checks the
candidate's 38 assembly statements, three label targets, call-identifier
substitutions, real RMW and complete return sequence. Original addresses in
Source comments identify the evidence; they are not Source placement claims.

The report pins current Source/header/document outputs, accepted inputs and
the original legacy-owner blob at published base
`6e98c035c1b345a04482e92deb4b1f6051ed6398`. All 48 inherited pin references are
replayed against that base. Their one intentionally changed path is the owned
legacy-owner source; the report records both its accepted base and candidate
hashes instead of treating the edit as unchanged historical evidence.

The new file is not yet selected by CMake, so the worker performs no build,
ad hoc compiler probe or new test. After registration, the primary must run
the normal build and inspect the whole emitted adapter, real CRT bindings,
Core definition and actual legacy-owner call. Admission remains pending;
the earlier 24-byte and 25-byte constructors are not counted again.
