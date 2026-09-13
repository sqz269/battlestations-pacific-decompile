# Ship AI neighbour candidate walk

This packet reconstructs two fragments of `BSP_ShipAi_BrainPrePass` at
`009F1420..009F1BBA`: the second collection timer's state operation and the
gated walk of world list 6. It does not reconstruct the complete pre-pass or
bind GameUnitsHost. Candidate ownership and the complete `009F0D20` admission
body belong to the independent admission packet.

| C++ entry | Native coverage | Limits |
| --- | --- | --- |
| `ship_ai_neighbour_timer_due_009f15cc` | Floating-point state/due-byte operations at 009F15CC..009F15F5 and 009F15F8..009F15FD | Partial relative to 009F1420; excludes the intervening TEST BL of the independent first timer and all other parent work |
| `ship_ai_walk_neighbour_candidates_009f1856` | Complete gated fragment 009F1856..009F1A43, 494 bytes | Partial relative to 009F1420; explicit pose/settings/admission calls and pure borrowed field/list lookups |

The parent takes a float step in a thiscall with ECX=brain and `RET 4` at
`009F1BB8`. These fragments have new C++ interfaces; neither is an original
native function entry or a binary-compatible replacement.

## Entry gate, timer and parent lifetime

The walk's only entry condition is the byte at parent stack +17h, tested at
`009F1856`; zero branches to `009F1B3F`. That byte comes from the second timer
at brain+B50h. Ordered `step >= old_countdown` sets due and stores
`old_countdown + (period_B4C - step)`. Otherwise, including the unordered JC
arm, it clears due and stores `old_countdown - step`. It adds one period only,
without catch-up looping or clamping. The helper retains the original x87
operation order and explicit binary32 spill of the prior countdown.

The earlier first timer at B44h/B48h independently gates the preceding threat
scan. When either timer is due, `009F160A..009F1628` captures optional brain+4,
enters that CRITICAL_SECTION and increments its depth at +18h. The lock stays
held through both scans and the later `00F89AD8` chain; `009F1B3F..009F1B56`
decrements depth and leaves it, with the parent's unwind machinery outside
these fragments. The new walk requires the caller's existing native lock and
lifetime schedule; it neither substitutes nor silently acquires a new lock.

The timer constructor is `009F1160`. It initially writes B4C=1.0f at
`009F1346`, then stores `-RandomThreads(stream 1, 0, 1)` into B50 at
`009F1351..009F1360`. Later two settings getter calls feed another stream-1
draw over `ShipAvoidance.CollectTimer[1]` (+190h) and `[2]` (+194h), with the
result stored into B4C at `009F13F0`. `00BD1860` returns the lock stored at
brain+4 at `009F13FF`. The helper borrows period/countdown and does not create
timer defaults, add draws, reorder the constructor's RNG stream or construct
that lock. Those remain concrete owner prerequisites for runtime integration.

## Captured and live reads

`ShipAiNeighbourCandidateHost` resolves opaque canonical identities into
borrowed fields. Lookup methods must be pure: no allocation, state mutation,
implicit pose refresh, fallback identity or whole-list snapshot. Unit views
borrow the actual C8 validity byte, world X/Y/Z at FC/100/104, and float +9C8.
Class lookups identify the existing +538 object and its float +500. No new
candidate, pose, descriptor or list owner is introduced.

The exact walk is:

1. Capture self from brain+AA8 and refresh that captured unit only if C8 is
   zero. Capture list-6 head, that original self's X, signed count, then its Z.
   Head/count come from the same world header at `[[E188A8]+19CC] +60/+64`.
   The origin X/Z stay fixed for the entire walk. A nonpositive count skips
   iteration after these initial reads.
2. Read the current node's payload +8. Skip only if it equals the current
   brain+AA8 pointer. No null, active, faction, class, party or distance proxy
   filter occurs before this comparison.
3. Refresh a dirty candidate, reload self, and refresh that captured self if
   dirty. Use these live candidate/self Y values for the altitude comparison.
4. If altitude passes, recheck candidate C8 and refresh again when needed.
   Spill candidate X through x87, reload current self, capture its class,
   spill candidate Z and capture the candidate class. Use the fixed origin
   for planar deltas; use the newly selected self's +9C8 for radius.
5. Compute the hull contribution, then call the settings getter. Read both
   previously captured class objects' +500 fields after that callback and
   compute the closing contribution. Call the getter again and read +198.
   If the strict squared-distance comparison passes, call admission with the
   unchanged candidate identity and the host's existing nav receiver.
6. Decrement the captured positive count and reload **the current node's
   live next pointer +4 after all callbacks**, even on the final iteration.
   Continue while the captured remaining count is nonzero. Changes to the
   header count/head do not change this bound; changes to node-next do change
   traversal. The current node and required successor chain must remain alive.

The source uses a field view, not snapshots of the unit state. Pure lookups
may adapt actual process owners while preserving these identities and read
contracts. Concurrent mutation and invalid lifetimes are outside the native
normal domain; no defensive null terminator or replacement collection is added.

## Producer and arithmetic evidence

`006FE620` passes ESI, the actual unit, into `00484540` at site `006FE653`
for list +60h. The push-back body stores that argument directly
at node+8, links previous tail -> next, updates tail and increments count.
Its first insertion also sets head; every new next is zero. This establishes
registration order and direct unit payloads for this particular list. The
broad older `local_player_unit_lists.hpp` comment about wrapper-valued other
world lists does not apply to this producer/consumer pair. Root separately
audits which concrete leaf registrars populate class list 6 at runtime.

Altitude uses a binary32 spill of candidate Y minus captured self Y. Positive
delta is retained; the other arm uses SSE `-0.0f - delta`. It then requires
**magnitude < 15.0**, with 15.0 from `00CF3F20`. Equality and unordered results
are rejected. This is not replaced by a generic absolute-value operation.

Radius is the binary32 hull contribution `(self_9C8 + candidate_9C8) * 0.5`
plus the selected reach. The closing contribution is computed on x87 as
`(candidate_class_500 + self_class_500) * settings_19C` and then spilled.
After the second getter, the spilled minimum at +198 is selected only when
it compares strictly greater than closing. Otherwise, including unordered,
closing is selected. Radius is spilled, squared and spilled again. Both X/Z
deltas are spilled before their extended products and sum are finally spilled.
`009F1A1F JBE` rejects equality: admission requires **distance_squared <
radius_squared**.

The +9C8 value is the full hull length, not the half-length suggested by the
old helper's parameter names. The producer at `0081106E` stores descriptor+A0
on its fallback arm; current `GameUnitsHost::unit_hull_length_09c8` documents
the same contract. The existing class-field loader maps +500 to `MaxSpeed`
(read/store `00831903`/`0083191A`). The new view keeps offset-based identifiers
and does not create substitute dimensions or apply a second half factor.

The existing `ship_ai_neighbour_admission_radius_009f1987` helper is not reused
because its C++ float intermediate/NaN selection differs from this instruction
schedule. Its adjacent header also says `<=`, contrary to the strict branch.
This packet leaves `sector_scan` unchanged, reuses its verified altitude and
negative-zero constants, and reuses `GameplayTuningSettings` for +198/+19C.

| Call site | Native target | Receiver and arguments |
| --- | --- | --- |
| 009F1872 | 00414DB0 | Captured initial self; ECX only, plain RET |
| 009F18D2 | 00414DB0 | Candidate if dirty |
| 009F18E8 | 00414DB0 | Reloaded self if dirty |
| 009F1944 | 00414DB0 | Candidate rechecked after the other refresh |
| 009F199D | 00424C40 | No arguments; +19C read after class capture |
| 009F19B8 | 00424C40 | No arguments; +198 read from this return |
| 009F1A25 | 009F0D20 | ECX=brain+8/nav; one pushed candidate pointer; RET 4; result ignored |

The real pose-refresh body and existing implementation were read; it refreshes
parent/local/world matrices and their validity flags. The admission dependency
was read for its ownership/argument contract; its filters, allocation, retained
node construction and duplicate lifetime updates are the separate admission
worker's implementation, not simulated by this walk.

## Verification and remaining scope

The ignored decoder script verifies 544 original bytes against live Ghidra and
the installed executable, then records every relocation. A fixture bridge
supplies native EDI, private stack cells, and the timer's incoming x87 step.
Seven native CALL displacements and four absolute operands are relocated; two
external walk exits go to one appended fixture RET. The bridge does not execute
the parent SEH, lock or other scans. Pose/settings/admission calls are explicit
fixture observation providers, not claims of native callee execution.

At x87 precision 24/53/64, 27 paired walk cases and 15 paired timer cases agree:
153 callback events per side, all compared unit/brain/list-node/world bytes,
full x87 status, and walk MXCSR. No pointer normalization or byte masks are
used. Cases cover gate false, zero/negative count, self skip, strict altitude
and radius equality, selected NaNs, repeated dirty refreshes, changed self and
origin fields, changed head/count, and post-admission next-link rewiring.
Classes/settings/global publication buffers are fixture inputs rather than
fully compared native owners. Arbitrary float environments, traps/unwinds,
concurrency and complete game execution are not established by this fixture.

The existing goal fragment before 009F158A, first timer/threat scan through
009F1855, parent locking, and 009F1A44..009F1BBA (special entity chain, unlock
and final housekeeping) remain outside this implementation. Build and call
gate results are recorded in `reports/ship_ai_neighbour_candidates.json`;
the ignored `local/neighbour_candidate_artifact_manifest.json` hashes code,
original-byte metadata, probe, build and result evidence. No tracked tests,
runtime host edits or Ghidra mutations are added.
