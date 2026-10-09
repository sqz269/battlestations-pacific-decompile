# CC12 tick-registration constructor Source candidate

Baseline: `6e98c035c1b345a04482e92deb4b1f6051ed6398`.
Packet: `cc12_tick_registration_constructor_source`; address `00875890`.
This packet adds an ordinary Source C++ candidate and its evidence. It is
unregistered, uncompiled and unlinked pending primary integration and review.
No additional Source/Original/Native ABI or gameplay credit is claimed.

## Inputs and actual bindings

`construct_native_tick_registration_00875890` takes the actual raw receiver,
opaque payload pointer, raw unsigned 32-bit group word, actual volatile registry
`00F878CC` and manager `01090AA0` publication references, a fixed actual raw
pending-tail pointer, and a `const volatile std::uint32_t&` current timer word.
It returns the captured receiver and has no `noexcept` specification.

The tail parameter binds actual Source storage corresponding to `00E0B704`.
The actual mutable pending-last slot is derived from that same storage's `+04`
field, preserving the Native identity `00E0B708 = 00E0B704 + 4`. A caller cannot
supply an unrelated pending-last cell through this interface. The tail address
is supplied by value and remains the fixed link identity throughout the call;
its field is volatile and is read twice at the required points.

The caller must provide valid, stable Source raw backing for the tail and its
writable four-byte pointer field at `+04`, every current neighbor dereferenced
through that field, the receiver's selected accesses through `+33`, and the
timer word. Appropriate Source object lifetime and alignment are required.
No Native absolute address is converted into fabricated Source storage. No
tail/sentinel default, self-link initialization, private cell, timer value,
bootstrap getter call or provider callback is introduced. The payload pointer
is stored as opaque data and its pointee is never accessed by this routine.

## Selected writes and their order

Before the actual admitted getter call, the candidate writes exactly these
selected raw fields, in the order shown:

| Receiver offset | Width | Value |
| --- | --- | --- |
| `+00` | DWORD | `00D0DEC8` opaque profile |
| `+14` | DWORD | All bits of the raw group word |
| `+18` | BYTE | Zero |
| `+19` | BYTE | Zero |
| `+20` | DWORD | Zero |
| `+1C` | DWORD | Zero |
| `+24` | DWORD | Zero |
| `+28` | Pointer/DWORD | Actual payload pointer |

The two BYTE stores use separate volatile byte accesses. All raw word and
pointer accesses use volatile views of their actual storage. No complete typed
node construction, memset, holes initialization or group conversion is used.
The minimum direct receiver extent is `34h`, not a recovered full class size.
There are no explicit writes to `+0C..+13` or `+1A..+1B` or beyond the selected
fields; indirect binding/neighbor aliases are not claimed to preserve them.

The actual `get_native_pending_registry_00875280` receives the two borrowed
publication references in registry/manager order. Its returned registry is
dereferenced at `+04` without a registry-null guard. This one raw section
pointer is captured. For a nonnull section, real `EnterCriticalSection` runs
before its volatile unsigned DWORD depth at `+18` is incremented modulo 2^32.
There is no locally protected cleanup scope.

The linking schedule then remains:

1. Read the current pending-last pointer from the supplied tail's `+04`.
2. Store that capture at receiver `+04`.
3. Store the fixed supplied tail pointer at receiver `+08`.
4. Reread the current tail `+04` after both receiver stores.
5. Store the receiver at that second current neighbor's `+08`.
6. Publish the receiver into the actual tail `+04` pending-last field.

The first and second last-pointer values can differ. Neither is replaced by a
cached constructor-entry value, by the receiver itself, or by an assumed
sentinel relationship. The second field read remains after the receiver stores,
including when those stores alias the tail field.

Only after all links are stored does the candidate write DWORD zero at receiver
`+2C`. It then reads the actual current volatile timer word and stores those
exact 32 bits at receiver `+30`. There is no floating arithmetic, conversion,
value normalization or early timer capture. The Native XORPS/MOVSS sequence
motivates these memory-word effects; the Source code does not reproduce or
promise residual XMM0 or arithmetic flags.

For the original captured nonnull section, the unsigned volatile depth is
decremented modulo 2^32 before real `LeaveCriticalSection`. A null section
skips both section operations/counter updates while retaining all link/timer
effects. The routine then returns its captured receiver. The condition uses
the captured pointer; there is no late registry or section-field reload.

## Failure boundaries and Source qualification

There are no catches, RAII guard, owner allocation/free, node-null check,
duplicate policy, group validation or rollback. A getter exception leaves the
preceding receiver writes; post-entry failure receives no cleanup from this
routine. The admitted getter has its own Source cleanup contract, which this
caller neither replaces nor extends. Source pointer helpers only form volatile
views; they do not substitute service providers or supply recovery.

The Native body receives its receiver in ECX, payload/group on the caller's
stack, and returns the receiver in EAX with RET8. This seven-argument ordinary
Source interface is different. Native incoming-register transport, caller-stack
aliases, mutable publication/backing aliases outside valid Source object rules,
residual SSE/flags, fault timing, original exception/CRT identity and nested
failure are qualified. Volatile access order is not portable C++ synchronization
or proof of concurrent list correctness. Numeric profiles are opaque data,
not callable Source vtables. Neither Source nor Native runtime execution is
proved by this unbuilt candidate.

## Accepted evidence and remaining production work

The accepted complete Native audit is
`reports/cc12_pending_registry_tick_registration_constructor_readiness.json`:
139 bytes through `0087591A`, 42 instructions, one direct call to `00875280`
and two real Win32 import calls. Its complete byte string, recorded hash and
contiguous instruction coverage were revalidated from the report. No Native
body, descendant, caller, handler, table, timer/global word, PE or Ghidra query
was reopened by this Source packet.

The getter's current primary report,
`reports/cc12_native_pending_registry_getter_primary_review.json`, admits its
250-byte/85-instruction Source body, 140-byte generated EH data, concrete
providers and actual Core resolution. That existing admission is reused;
it is not compilation evidence for this new caller. All 15 accepted constructor
readiness pins, all 30 selected getter-primary Source pins and both current
primary/readiness document pins still match the baseline checkout.

The canonical registry/manager cell and destruction binding are already
admitted. The concrete `UnitConstructionHost` override and actual Source
pending-tail/timer producers remain open. This candidate adds a direct Source
getter reference but no production caller or lifecycle activation for itself.
No game Host, Native caller, fixed-step bootstrap, alternative `F899E8`
pending-entity lock, CMake, ledger or Ghidra state is changed.

The JSON report records current source/evidence pins, the full accepted Native
body, the exact Source phases and qualifications. Static Source review, JSON,
evidence-pin and staged whitespace/four-file checks passed. The primary agent
must register and compile the candidate, review the entire emitted body and
actual provider bindings, verify Core resolution, and run the normal existing
checks before Source admission. This worker ran no build, test or probe.
