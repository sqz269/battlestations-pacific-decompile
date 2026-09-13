# Ship AI brain timer and lock producers

Addresses: 009F1160, 009F126B, 009F39C0, 009E4330, 00BD2F10, 00BD2E60,
00BD2ED0, 00BD1860, 00424C40, 0041CC80, 009F1420, 009F0D20.

`ship_ai_brain_produce_tail_009f126b` reconstructs the normal producer tail of
the existing `BSP_ShipAi_BrainRecordConstruct` name. It initializes goal fields,
performs seven stream-1 random draws, fetches settings four times and allocates
the actual tracked critical section last. It neither constructs a complete brain
nor schedules its pre-pass. The names of these C++ interfaces are hypotheses.

| Routine | Coverage | Original ABI |
| --- | --- | --- |
| 009F1160..009F1413 | Partial: 009F126B..009F1401 normal producer operations; private SEH bookkeeping excluded | ECX brain, one owner pointer, EAX same brain, RET4 at009F1411 |
| 00BD2F10..00BD2F30 | Existing complete range helper reused | ECX stream, stack min/max, RET8, binary32 result in ST0; source adds EDX access |
| 00BD1860..00BD1886 | Existing complete raw lock constructor reused | cdecl, no arguments, EAX lock, plain RET |

The full 157-instruction containing listing was read. The sole caller is
009F39E5 in009F39C0: its ESI is the incoming ECX and it passes the incoming
owner pointer unchanged. Inside009F1160, ESI is captured ECX, initial EDI is the
owner stack argument, EBX becomes zero at009F11B5, and EAX becomes3 at009F1237.
The first draw obtains ECX=1 via `LEA ECX,[EAX-2]`; each remaining draw explicitly
sets ECX=1. No constructor-owned seed or random bank exists.

The excluded prefix009F1160..009F126A installs table00D21990 and calls
009E4330 on brain+8 with the owner at009F118D. It copies owner, class+538,
owner+73C and owner+738 into AA8/AAC/AB0/AB8. The actual owner vtable+5C query
at009F11BF receives8; its AL result selects owner-or-null atAB4. If nonnull,
operator_new(4) at009F11DF may produce an owner-pointer holder atAC4. Native
null results publish zero there. The remainder initializes AC8, AD0/4/8,
AE0/4/8, AF0/4 and the inline target header AF8..B0C, including its actual
table00D21898. Those helper, observer, vtable, allocation and navigation owners
are prerequisites, not synthesized by this packet. Unassigned holes such as
AC0/ACC/ADC/AEC and target offset cells are not inferred to be zero.

The existing navigation projection returns `ShipAiNavBlockFields` through
`ShipAiNavBlockCtorHost`; it does not establish every original raw subobject.
Its earlier range draw at009E465F and sector construction at009E46A9 must have
occurred in their original order before this tail. Calling the new tail after
an older runtime substitute for that draw does not reproduce the native RNG
history. No full navigation/brain ownership claim follows from this API.

`ShipAiBrainProducerView` borrows the existing `ShipAiGoalVectorState`, six
timer cells and the real four-byte lock owner slot. It contains no defaults and
owns nothing. A runtime must bind canonical cells, including the same goal state
used by the goal consumer. Distinct cells, context literals and settings storage
must not overlap. This is a typed projection, not a native-layout overlay.

`ShipAiBrainProducerContext` borrows the existing `NativeParticleUnitRandomAccess`
(the application's `RandomThreads` and current correction/scale scalar cells),
the actual one/two/vector cells and a fresh settings-return function. It supplies
no global, seed, stream bank, current-thread registration or dummy lock.

The native tail's ordered operations are:

| Site | Operation |
| --- | --- |
| 126B..12C7 | Load CE3958 for first upper bound; zero B20/B24, set B28, copy F87574/78/7C to B2C/30/34, clear B38. These reuse existing goal fields. |
| 12CD | Stream1 range(1, CE3958), binary32 spill/reload, store B3C. |
| 12F1 | Stream1 range(0, prior draw), FCHS, store B40. B3C/B40 semantics beyond their actual producer remain unnamed. |
| 1316,1321,1330 | Initially B44=D7A24C (1); stream1 range(0,1), FCHS, store B48. |
| 1346,1351,1360 | Initially B4C=D7A24C (1); stream1 range(0,1), FCHS, store B50. |
| 1358..138C | Load CE3958 separately for B54 and the fifth upper bound; B54=2; stream1 range(0,2), FCHS, store B58. |
| 1392,1399 | Two fresh settings returns: retain first pointer in EDI; second returns EAX. |
| 139E,13B0,13B9,13BE | After both returns, x87-load/spill first+1F0 as maximum, then second+1EC as minimum; stream1 range(min,max) overwrites B44 only. |
| 13C4,13CB | Two more fresh settings returns, with the same pointer retention. |
| 13D0,13E2,13EB,13F0 | After both returns, x87-load/spill first+194 as maximum, then second+190 as minimum; stream1 range(min,max) overwrites B4C only. |
| 13F6,13FF | Construct actual raw1Ch tracked section, then publish returned pointer to brain+4. |

These are seven draws regardless of the later period values. B48 and B50 remain
the negated initial range(0,1) results; they are not resampled, scaled by the new
period or set to a zero default. CE3958, D7A24C and the vector are borrowed live
scalar inputs. FCHS and x87 binary32 load/store steps are explicit; no bounds
sorting, finite checks, clamping or replacement NaN behavior is added.

The existing settings layout is producer-backed: 0083B7AD/0083B810 populate
ShipAvoidance.CollectTimer[1]/[2] at190/194 and0083BF3F/0083BFA8 populate
TorpedoAvoidance.CollectTimer[1]/[2] at1EC/1F0. Thus B44/B48 is the torpedo
collection timer and B4C/B50 is the ship collection timer. The same settings194
upper bound is also reused by neighbour admission009F0D20 in its lifetime
calculation; calling194 merely a dedicated memory-duration field is inaccurate.

The pre-pass confirms the roles. 009F158A..009F15C6 updates B48 against B44,
putting its due flag in BL. 009F15CC..009F15FD updates B50 against B4C, putting
its due flag atESP+17. Ordered step>=old countdown makes the timer due and adds
one period minus step; unordered follows the subtraction-only arm. There is no
catch-up loop. Either due flag admits the optional brain+4 lock. BL then gates
009F163F..009F1855, which reads world list fields21C/220; the second flag gates
the list6 walk009F1856..009F1A43. The later special-entity scan, unlock and
remaining pre-pass logic are outside this constructor packet. An interleaved
block009F15B1..15BD belongs to the earlier goal timer, not to B48.

00BD2F10 forwards both floats to00BD2ED0/00BD2E60. The state lookup obtains the
current Win32 thread ID, finds the registered slot and selects
states[2*slot+stream]; an unregistered thread selects the shared fallback.
Stream1 means `RandomStream::secondary`, not an independently created generator.
The existing range routine preserves native guard faults, signed-to-unsigned
correction, scale, interpolation and binary32 spills. Both bodies and the lookup
were read; this packet reuses them without implementing a second distribution.

The lock call uses `create_native_tracked_critical_section_00bd1860` from the
existing native worker-lifetime module. It allocates exactly1Ch raw bytes via
the shared BF681B host allocation boundary, calls real InitializeCriticalSection
and sets depth+18 to zero. The older new/delete convenience constructor is not
used. The shared allocator calls current CRT malloc/new-handler/retry/throw;
it ordinarily returns storage or throws. The native null-result branch remains
in the reused helper, and the tail publishes the helper's result directly.
No previous lock is released. Allocation/OS failure before publication leaves
the preceding stores and random consumption complete and the owner slot
unchanged; original parent SEH rollback and hardware-fault behavior are omitted.
Release the produced raw lock with the existing
`release_native_tracked_critical_section_0041cc80` on the owning/quiescent thread.

The ignored original-byte sequence probe is recorded in
`local/brain_producer_artifact_manifest.json` and the report. It executes the
407-byte tail through a synthetic entry with ESI/EBX/EAX and private stack
storage, retaining all12 call sites and relocating eight literal operands.
The native random and lock call targets use the same existing reconstructed
helpers as source; this checks the caller sequence, not independent fidelity of
those helpers or the CRT. The settings provider has four distinct records and
can mutate the first retained record on the second call. Its snapshots compare
the whole B60-byte brain projection and random index. The final full random
state, output/untouched brain bytes and x87 status are compared.

Only the produced lock pointer is normalized after real acquire/leave/release;
OS-private section bytes are not compared. Projection copies only the nine
documented goal cells; all other native buffer bytes remain compared. Explicit
fixture seeding, one prior range draw and raw A5 backing are inputs, not actual
navigation/brain construction. The probe does not execute original parent SEH,
the settings singleton's lazy construction, original allocation failure or game
runtime. No new tracked tests or GameHosts bindings are introduced.

Validation passed: nine paired cases at x87 precision24/53/64,36 settings
snapshots per side, seven tail draws per case,18 constructed/released sections,
zero mismatches and matching full x87 status. `scripts/build.ps1` completed the
Win32 Release build; `reconstructed_math` and `native_math_differential` passed.
The report call gate checked15 rows with zero failures.
