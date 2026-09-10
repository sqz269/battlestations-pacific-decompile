# Particle clock lifetime

`ConcreteParticleClockLifetimeAccess` supplies the concrete allocation, shared
manager mapping, captured Win32 critical section, registration and destruction
used by `particle_clock_singleton_004de4b0`. It uses the same `ParticleClock` and
`00F8D420` slot as the frame-time updater and system constant prefix.

The root integration also preserves the updater's raw `shader_time` store and
the separate per-sink x87 argument load/spill. One original-byte comparison with
signaling NaN `7F800123` retains that word in the clock, forwards quieted
`7FC00123`, and matches x87 invalid status. This is separate from the lifecycle
fixture described below. The installed-system probe now exercises actual lazy
allocation, one manager-driven particle destruction and both cleared slots.

The adapter requires the actual `SingletonLifetimeDomain` and string
`SizedStoragePool`. It creates neither another manager nor another allocator or
lock. Its callback target and string pool must remain alive through domain
shutdown. `SingletonLifetimeCallbacks::destroy_registered` can dispatch an
actual registered particle owner to `deleting_destructor_004de340(owner, flags)`.

## Evidence and ABI

Live queries used `tools/bsp.py`, whose client verifies project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base before each query.
The configured project is `C:/Users/sqz269/bsp.gpr`. Current installed PE SHA256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The installation was read only. These descriptive names are hypotheses, not
recovered symbols. Ghidra mutations and export refresh remain integration work;
the audit records preimages and proposed annotations.

| Address | Established behavior / native ABI |
| --- | --- |
| `004DE340` | `CE7D38[0]`; ECX complete clock, stack flags, RET 4. Calls `00B1B680`, frees iff bit 0, returns original address even after free. |
| `004DE360` | `CE7D24[0]`; `SUB ECX,4; JMP 004DE340`. No Ghidra function exists at this entry in the inspected state. |
| `00B1B680` | ECX clock, RET. Writes derived identities, destroys the embedded container at clock+4, unconditionally clears `00F8D420`, writes base `CE3818`. |
| `004DE290` | ECX embedded container, RET. Sets `CE7D08`, drains records, resizes array to zero again, frees its allocation. |
| `004DDA40` | ECX container, RET. Releases the last sink, reloads base/count, destroys the current last record if count remains nonzero, decrements count. Ends with resize(0). |
| `004DDB40` | ECX container unused, stack sink, RET 4. `InterlockedDecrement(sink+4)`; zero calls sink virtual+0 with ECX sink and no stack flag. |
| `004D45A0` | ECX record, RET. Clears its linked string list, frees/zeros sentinel, releases its own string buffer with length+1. |
| `004D05E0` | ECX embedded list at record+8, RET. Resets sentinel links/count before walking all former nodes, returning strings and freeing nodes. |
| `004DC410` | ECX array at clock+8, stack signed new count, RET 4. Only the zero-size path reached after drain is reconstructed here. |

Assembly was necessary: incorrect no-return annotations at free calls truncate
several decompilations. Raw `004DE355` proves the scalar destructor returns its
original ESI. `004D0625..004D062F` contains the linked-list loop continuation.
`004D45D9..004D4617` zeroes the freed sentinel and releases the record's own
string, which pseudocode omits. Live bytes matched those disk continuations.
The original wrappers use x86 exception registration. The host destructor path
is `noexcept`; it does not recreate native SEH tables or foreign virtual throws.

## Retained allocations and records

The allocator request records native size `0x1C` and allocates enough raw bytes
for the C++ projection. Before placement construction, the adapter copies native
raw bytes `+18h` to an integer observation. It restores that representation in
the canonical `shader_time` with `memcpy`, never loading a float. Constructor
stores in `004DE4B0` therefore preserve the first shader read, including signaling
NaN payloads. Host aggregate defaults are not native constructor evidence.

`ParticleClockOwnedRecord` is exactly `0x2C` on Win32: its string is at `+0/+4`,
list sentinel/count at `+C/+10`, and actual sink pointer at `+28`. Nodes are
16 bytes: next/previous pointers followed by string length/buffer. Both the
record string and each node string return through the supplied pool with
`length+1`; nodes, sentinel and record allocation use the common CRT free.
The destructor leaves the freed array pointer/capacity and record string fields
unchanged, matching the native writes. It does not unregister from the manager.
Normal manager shutdown already pops the entry before calling its destructor;
standalone destruction retains its registration unless the caller separately
removes it.

The updater now reads `owned_records` when present. It advances its cursor by
`0x2C` and reloads the end from the current base/count after every sink call,
matching `00B19A10`; it does not silently switch to index iteration if a callback
reallocates the array. The older `sinks` projection remains a borrowed
compatibility path. Actual lifecycle owners use the owned array and its same
sink objects for both updating and reference release.

`ParticleClockOwnedSink` retains the actual time sink and reference counter.
Its `destroy_zero_reference_00` is an explicit owner-specific boundary: the
particle-system virtual body is not reconstructed by this packet. The lazy
clock starts with no records, so its creation and destruction are concrete
without inventing that owner callback. None of these C++ interfaces are binary
replacement vtables or drop-in native ABI implementations.

## Validation

MSVC Win32 `scripts/build.ps1` passed and the existing CTest case passed 1/1.
An isolated local fixture separately compiled the new lifetime source, shared
singleton implementation and current system-time implementation with
`/W4 /WX /fp:strict`, linked the real core library, and passed:

- raw allocation and getter preimages survived; one recycled allocation held
  signaling NaN representation `7F800123` and remained bit-exact;
- the secondary destructor with flags 2 cleared the slot, returned the owner,
  retained its allocation, and left manager registration untouched;
- two owned records updated the same shared sink twice, then references 2 to 0
  invoked the fixture's zero-reference virtual once;
- four linked nodes plus two record strings returned six buffers to the real
  sized pool; the actual manager dispatched one live particle registration.

The fixture's sink body is test evidence for virtual dispatch, not a recovered
particle-system destructor. No game execution, image parity or installed draw
validation is claimed by this packet; the primary integration performs its own
full prefix and installed-mesh probe checks.
