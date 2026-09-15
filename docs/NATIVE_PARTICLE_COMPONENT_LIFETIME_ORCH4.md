# Raw Particle component lifetime

Packet `orch4_particle_lifetime_h8` reconstructs six complete bodies against actual
Win32 storage. Descriptive names are hypotheses. The constructor `0086BC80`
already establishes the 34h owner, Particle profile `00D0D5B4`, names at +8/+20h,
and the 0Ch vector at +28h. This packet adds no alternate owning object.

## Complete bodies and ABI

| Entry | Inclusive end | Bytes | Original interface | Behavior |
|---|---|---:|---|---|
| 0086A4D0 | 0086A52E | 95 | ECX header; signed capacity on stack; RET4 | Clamp to one, signed capacity comparison, allocate DWORD-wrapped capacity*4, copy current pointer slots, free old data, publish data and capacity. |
| 0086ABB0 | 0086ABFF | 80 | ECX header; signed count on stack; RET4 | Reserve if needed; zero newly added slots; decrement count when shrinking; store requested count. |
| 0086B6C0 | 0086B6D6 | 23 | ECX header; RET | Resize0 then free current data; leave dangling pointer/capacity. |
| 0086B7E0 | 0086B846 | 103 | ECX component; RET | Write D0D570; release actual name+8 through raw pool; BD30F0 writes CEB130. |
| 0086BB80 | 0086BC4D | 206 | ECX component; RET | Write D0D5B4; reverse resource releases; vector cleanup; name+20 cleanup; base cleanup. |
| 0086BC60 | 0086BC7D | 30 | ECX component; flags on stack; EAX original component; RET4 | Complete destruction, then free iff bit0. |

The resource loop captures the current final vector element, atomically decrements
its DWORD refcount at +4 through imported `InterlockedDecrement`, and calls the
resource's current vtable slot0 only when the result is zero. There is no null
guard or second decrement. After virtual dispatch it rereads the component count
and decrements that current value if nonzero. A virtual destructor may change the
count or storage; the next iteration reads the current fields. Shrinking the
pointer vector itself does not destroy pointed-to resources. The copied allocation
does not acquire references. Untouched owner bytes and all stale name/vector
header fields survive.

The Particle table's +4 slot is `0086BC60`, and its +14h slot is reader `00871D00`.
Direct resource dispatch requires actual executable Win32 thiscall table entries.
Writing original numeric component profiles does not install executable source
vtables or relocate original game code.

## Raw pool and exception composition

The new `NativeStringRawPoolContext` overload of `0086B7E0` composes the concrete
raw singleton/pool providers through `destroy_native_string_header_0041dd20`.
Every nonnull return resolves `00419CC0` and then calls `00BD1510`, including
large blocks and disabled small returns. The old `NativeStringStorage` overload
remains its separate host projection. No host owner is reinterpreted as raw pool
storage. Allocation/free use the existing concrete `singleton_lifetime_allocate`
and `singleton_lifetime_free` providers; BD30F0 uses its existing raw source body.

FH3 handler `00C955BE` selects info `00DC75B4`, whose three-entry unwind map starts
at `00DC759C`:

| Armed state | Next state | Unwind action | Operation |
|---:|---:|---|---|
| 2 | 1 | C955B3 -> 86B6C0 | Destroy vector at component+28h. |
| 1 | 0 | C955A8 -> 41DD20 | Destroy current name at component+20h. |
| 0 | -1 | C955A0 -> 86B7E0 | Destroy base component. |

State2 is armed before the first resource release. State1 is armed before vector
resize/free; state0 before returning the Particle name; state-1 before entering
the base destructor. If a resource callback throws, the remaining pointed-to
resources are not visited: the vector storage, name and base are unwound.
Base handler C95538 selects DC7510 with the single DC7508 map row
`0 -> -1, C95530 -> BD30F0`; the base profile cleanup therefore also runs if
the raw pool getter throws. Source RAII guards implement those C++ cleanup
states. An exception from a cleanup during active unwinding terminates; it does
not replace the original exception or continue through lower cleanup stages.

## Evidence and listing repair

The report records complete disk/live bytes and SHA256 for all six bodies,
the two EH handler/action spans, both FH3 maps/info records and the Particle
vtable. The existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, was checked through the repository Ghidra client
before each live query. The executable SHA256 and previous names/comments are
retained in the report.

Parent-serialized repairs restored reserve gap A521..A529 (9B), destructor tail
BC09..BC4D (69B), scalar gap BC75..BC77 (3B), and vector destructor tail
B6D2..B6D6 (5B). They followed false no-return metadata on the CRT free paths.
The parent saved the project and refreshed exports. This worker performed no
Ghidra writes. Call rows include the previously unowned destructor tail
BC22/BC29/BC38 and retain correct CRT library names. Prior base comments are
preserved; the compiler scalar name is preserved.

## Validation and remaining boundaries

Validation receipts are in `reports/native_particle_component_lifetime_orch4.json`.
The local focused probe uses the original six byte bodies with only external
call/IAT operands rebound to existing heap, raw pool and Interlocked providers.
It compares pointer-normalized complete 34h owner images, reverse virtual release
order, current-count and vector-pointer mutation, vector reserve/grow/shrink behavior, and scalar
flag bit0 behavior. Its exception case exercises the source C++ unwind schedule;
the original FH3 handlers are not relocated or executed on exceptional paths.
No permanent test suite is added.

The C++ APIs explicitly carry the raw pool context; they are not drop-in native
ABI entries. Original FH3/SEH identity, original CRT/OOM and memory-fault behavior,
arbitrary resource vtable relocation, and gameplay remain unvalidated. Build,
byte and focused fixture evidence does not establish a runnable reconstructed
game.
