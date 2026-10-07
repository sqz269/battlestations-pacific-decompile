# Session target base lifetime

This packet reconstructs the complete ordinary bodies at `007839D0`, `00783B90`
and `00784320`. Names describe inferred behavior, not recovered symbols. The
new Source interfaces compose the completed transport-buffer and history
prerequisites with existing canonical allocation and tracked-lock services.

| Body | Complete end-exclusive interval | Original ordinary ABI |
| --- | --- | --- |
| Constructor | `[007839D0,00783B85)` | ECX target; metadata pointer and length on stack; EAX target; RET8 |
| Destructor | `[00783B90,00783C75)` | ECX target; RET |
| Scalar destructor | `[00784320,0078433E)` | ECX target; flags on stack; EAX captured target; RET4 |

`NativeSessionTargetStorage` is the `D74h` base span actually written through
byte `D73`. It contains the real `D3Ch` buffer member at `+10`, three separately
allocated cursors at `D40/D44/D48`, the captured tracked lock at `+4`, optional
metadata at `+8`, and two separately allocated `18h` histories at `D54/D58`.
The constructor does not initialize the address at `D74`; callers bind derived
storage independently. This is not a claim to reconstruct the derived `DC0h`
allocation, registration or publication path.

The target profile is the raw numeric stamp `D0426C`, deliberately uncallable
in Source. Its native table also contains `0076B630`, outside this packet. The
embedded buffer profile remains its prerequisite's raw `D04264` stamp. The
history profile is different: native `D04268` contains exactly `00783970`, so
this composition installs a genuine one-slot Source table in each live
history's four-byte profile field. Its fastcall bridge ignores EDX and invokes
the complete history scalar destructor. This extends the standalone history
storage contract only while constructed here. The scalar destructor's numeric
post-destruction history stamp remains raw storage; it is not callable.

Construction preserves the native order:

1. Stamp the target and complete embedded-buffer construction. Only after that
   returns does constructor cleanup become active. Clear `D72`, then `D70`,
   create the actual canonical tracked lock and publish it at `+4`.
2. Null metadata publishes null. Non-null metadata requests `98h`, publishes
   the allocation, then copies the unchecked original length. The existing
   native `BF7680` helper handles overlap; Source uses the host `memmove`
   boundary, without adding a clamp or a zero-length bypass.
3. Clear `D50`; allocate the first `18h` history. On non-null, execute
   `FLDZ/FSTP` for its actual argument word before installing the callable
   profile and invoking complete `00782F40` with count 50. Publish `D54` only
   after successful initialization.
4. Allocate the second `18h` history. On non-null, store count 3, profile and
   initial zero, then allocate 12 bytes. Preserve `FLD` borrowed native 3.0 and
   `FMUL` borrowed native +0.0 across array publication and all three sample
   writes: the first uses the returned array directly, the next two reload the
   array cell. Store the x87 total, then zero the index.
5. Store timeout `D60`, load the native negative-one operand, publish `D58`,
   then store `D5C`, flags 7, `D64` and `D68`. Capture the actual `F871B0` word
   once for scanning. A free bit is ORed into the freshly accessed current
   word with the native unprefixed memory instruction. A full mask produces
   index `FFFFFFFF` without modifying the mask. Publish `D6C`, then clear
   `D4C` last.

The bindings borrow the application's actual mask and the four native constant
operands. No default globals, allocation domain, opaque constructor phases,
profile-token dispatch or callback-based replacement allocator are supplied.
The original `00773120` path clears the selected mask bit separately through
`007827D0`; these base functions do not assert universal lifetime ownership of
that bit or make concurrent mask operations atomic.

Destruction stamps the raw target profile and conditionally clears a current
mask bit for indices below 16. It frees non-null metadata and then clears `+8`.
It independently reloads each history pointer, profile and scalar entry at
`D54`, then `D58`, invoking actual slot zero with flags 1. It captures `+4`,
drains positive signed depth with real `LeaveCriticalSection`, deletes the
critical section and canonically frees the captured lock. Slot `+4` remains
unchanged. Complete buffer destruction follows, preserving released cursor
words and backing bytes. Scalar destruction optionally frees the target after
completed destruction when flags bit zero is set and returns its captured
address even then. Consumers must be quiescent before destruction.

The recovered native constructor unwind map at `DBA964` has state zero
`{-1,C8A490}` for buffer destruction and states one/two `{0,C8A49B}` and
`{0,C8A4A6}` for freeing only the currently constructing `18h` history. The
destructor map at `DBA9A0` has `{-1,C8A4C0}` for buffer destruction. Source
ordinary C++ catches reproduce that limited cleanup and rethrow. They do not
reclaim already published lock, metadata or prior history allocations. Buffer
construction is outside the constructor catch, so its partial failures gain no
new cleanup. Original CRT handler identities, private EH/SEH and hardware-fault
unwinding remain unbound; failure paths were inspected, not fault-injected.

Validation used a fresh strict MSVC Win32 build of 23 actual translation units,
67 immutable source/header inputs and the separately hashed probe. Three main
support libraries were copied after the completed build at `2f11cd1d7` and
remained unchanged before and after linking and execution. The executable has
an embedded `asInvoker` manifest. The connected probe passed 232 checks:

- Actual constructor-owned lock, cursors and histories drove the existing D2
  codec, threshold serializer and owned enqueue. Queued payload survived
  overwriting all three backing buffers and complete target destruction.
- Metadata copying, full mask, derived early bit release, recursive lock
  draining, scalar flags zero/one and unchanged `D74` tail were exercised.
- The complete Source constructor's second-history result and x87 state
  matched the original 47-byte inline kernel in 24 PC/RC/empty-or-three-occupied
  cases. Only its two constant addresses were relocated; original `ADD ESP,4`
  was retained with an explicit matching caller stack. CW, SW, FTW, all eight
  80-bit register images and MXCSR matched; instruction/data pointers differ.
- The retained transport fixture passed its 72 original timestamp-kernel
  comparisons. No packet was sent and no game process was exercised.

Generated assembly was inspected for cleanup activation, publication order,
the two x87 sequences, single word mask operations, fresh virtual dispatch,
captured lock drain, unchanged slot `+4`, and the actual one-slot bridge's RET4.
The stored Ghidra destructor still ends at `00783C57`; complete disk/live bytes
establish its tail through `00783C74`. The scalar's stored range reaches
`0078433D`, but the original ADD ESP,4 listing gap remains qualified. No worker
Ghidra mutation, full-class ABI proof, native CRT equivalence, runtime
publication or gameplay validation is claimed. Primary registration, full
main build and integration are separate from this worker evidence.

Machine-readable evidence: `reports/cc11_session_target_lifetime.json`.
Ignored reproducible artifacts: `local/cc11_target_lifetime_20261007_a/`.
