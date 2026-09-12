# Avoid-zone closest projection and corner offset

Addresses: 00416F30

`avoid_zone_closest_offset_point_00416f30` completely reconstructs the native
closest-edge query, including all three output writes. It consumes the existing
producer-backed `ShipAiPathLateralRecordList` and `ShipAiPathLateralRecord` from
`bsp/ship_ai_lateral_record.hpp`; it requires no host or library bindings.
The descriptive name is a hypothesis. Ghidra remained read only.

| Routine | Native body and ABI | Coverage | C++ entry |
| --- | --- | --- | --- |
| `00416F30` | `00416F30-004171B4`, 645 bytes; ECX zone; stack point, point output, four-float edge output, endpoint-byte output, float push; no return value; `RET 14h` at `004171B2` | complete | `avoid_zone_closest_offset_point_00416f30` |

The final instruction is three bytes. No missing function definition or unread
body span remains. The new semantic C++ signature is not a native ABI replacement.

## Selection and outputs

The loop starts with `(last, first)` and then visits each pointer in array order.
It initializes the best squared distance to `FLT_MAX`, not infinity: live bytes
at `00D7A248` are `FF FF 7F 7F`; `00D7A24C` is float one. The count is the exact
native signed count. Zero returns without dereferencing a record or writing any
output; one and two are not rejected. Negative counts and unreadable pointers
are outside the valid-input contract. The native implementation calculates a
last-pointer address before its empty check but does not dereference it there;
the C++ avoids forming an invalid before-array pointer for an empty list.

For each edge `a -> b`, the parameter is
`dot(point-a, a.outgoing_unit) / a.outgoing_length`. This uses the producer's
stored unit and length, not a recomputed generic segment parameter. A nonpositive
parameter selects `a`, sets parameter to positive zero and endpoint byte to 1.
A parameter at least one selects `b`, sets parameter to one and byte to 1.
Otherwise it interpolates the coordinates and sets the byte to 0. Exact endpoint
parameters therefore count as endpoints. The native byte does not identify which
endpoint was used.

Assembly corrects the pseudocode for unordered parameters: `FCOMIP(0,t); JC` at
`00416FD5/FD7` and `COMISS(t,1); JC` at `00417004/007` both take the carry branch
for NaN. NaN therefore interpolates, produces an unordered squared distance and
cannot replace the best. Zero stored length is not guarded: positive/zero gives
positive infinity and selects the end, negative/zero can select the start, and
zero/zero reaches unordered interpolation. There is no minimum-length cutoff.

`004170B5/B9` accepts a candidate only if its unshifted squared distance is
strictly smaller than the current best. Equal distances, NaNs and distances
equal to `FLT_MAX` do not replace. Infinity cannot win. No winning candidate
leaves all existing output values untouched, just like empty input.

Only after selection does a nonzero push add
`push * ((1-t)*a.offset_dir + t*b.offset_dir)` to the candidate. The routine
does not renormalize this interpolation or use the record's clearance cache.
Both signed zero pushes skip it; NaN push takes it (`UCOMISS`, `LAHF`,
`TEST AH,44h`, `JNP` at `004170BF-004170C6`). Thus a NaN push can produce NaN
output coordinates while still writing a selected edge and endpoint flag.
The ranking distance remains the unshifted value even after an offset.

On each replacement, the native write order is point x/z, endpoint byte, then
edge `(a.x,a.z,b.x,b.z)` at `0041715B-00417183`. The C++ keeps that order.
Edge copies retain the native `FLD/FSTP` conversion, including signaling-NaN
quieting. Inputs and outputs must not alias, and the borrowed list/records must
remain stable. Native reloads the array/count at each loop tail; the C++ requires
their stability rather than inventing concurrent mutation or aliasing behavior.

## Arithmetic and producer evidence

The implementation uses small private x87 arithmetic helpers. They do not stand
for native calls: `00416F30` contains no calls. Existing generic vector/segment
helpers have different spill/parameter rules and are not substituted here.

| Stage | Native stores / comparison | Retained rule |
| --- | --- | --- |
| Query and delta | `00416F8C/98/9F/AC` | binary32 query loads and separate binary32 x/z subtraction |
| Dot and parameter | `00416FB0-00416FCB` | multiply both terms in x87, add without intervening product stores, spill dot, divide by stored length, spill parameter |
| Interior projection | `00417034-00417069` | separately spill edge subtraction, multiplication by t, and addition to a |
| Distance | `00417083-004170A9` | spill both candidate-query differences; square/add in x87 without product stores; spill squared distance |
| Selection predicates | `00416FD5`, `00417004`, `004170B5` | use the same x87/SSE comparison families and unordered decisions; SSE DAZ must not change the x87 lower clamp or ranking |
| Offset | `004170CA-00417125` | spill both t*b components, spill 1-t, spill both weighted a components, their sums, then products by push |
| Final point | `00417129-00417139` | add each pushed component to its unshifted candidate and spill binary32 |

The record is reused, not redefined. `docs/SHIP_AI_LATERAL_RECORD.md` and the
existing declaration establish its 24h size and nine fields. The consumed fields
are backed by `0041CCD0` x/z stores at `0041CD92/9C` and `0041CF03/0D`, and
`0041A200` stores: outgoing length `0041A33E`, outgoing unit `0041A36A/371`, and
normalized right normal of incoming plus outgoing units at `0041A418/423` and
`0041A454/45A`. The producer's derived-field assembly was checked. This function
does not establish global outwardness for a degenerate or self-intersecting ring.

Whole-listing register filtering confirms EBP is the previous pointer iterator,
ESI the current iterator, ECX the point output after `00416F6C`, EDI the edge
output after `00416F7F`, and BL the endpoint byte. The zone pointer is saved at
`00416F4B` and reloaded at `0041718E`. No register input is inferred from a
decompiler parameter name.

## All callers

Every live xref and its argument setup was read. Each supplies five stack words,
consistent with the callee's `RET 14h`; the report carries the call-site address,
native callee and actual containing function for mechanical verification.

| Containing routine / call | Push | Output usage |
| --- | --- | --- |
| `004174A0` / `00417556` | zero from `00417541/44` | caller record: point +0h, edge +8h, endpoint +1Ah; containment +18h is separate |
| `00417580` / `004175C5` | caller float argument at `004175AB/B0` | local point copied to caller output; edge/endpoint temporaries |
| `00417B10` / `00417BAA` | caller float argument at `00417B91/95` | local point replaces the successive group query; edge/endpoint temporaries |
| `0041AEA0` / `0041AF87` | one from `0041AF6F/72` | local point becomes the inside branch's direction basis; edge/endpoint temporaries |
| `0041B290` / `0041B353` | zero from `0041B339/33C` | local point used in subsequent distance/direction calculation; edge/endpoint temporaries |

## Verification and limits

Every live query used `tools/bsp.py ghidra`, whose client verifies
`C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe` before the batch.
No names, comments, function definitions, flow flags or saved analysis were
changed. Proposed name/old name/evidence are recorded in the repository ledger.

One ignored native differential probe is `local/avoid_zone_offset_probe.cpp`,
run by `local/run_avoid_zone_offset_probe.ps1`. `verify-seeds` passed before it.
The probe's 645 bytes match the installed executable, SHA256
`1d9d8f7671b11b4e716bd8133784b3a8bcdf7377d2770685fdeb8709cdf8eadc`.
It executes the native body in an isolated allocation with only the two absolute
constant operands relocated to identical float values. There are no patched
branches, replacement calls or dependencies. The executable embeds a manifest.

The probe passed 96,000 point/edge/endpoint bit comparisons: x87 precision
24/53/64, all four rounding modes, and SSE DAZ/FTZ off/on. The same focused
input loop covers counts 0-4, finite/arbitrary float bit patterns, NaN push,
zero/NaN edge length, exact endpoints, and a square center whose equal-distance
closing edge must remain selected. The byte outputs' prior sentinel values are
compared too. These are isolated native comparisons, not game validation or a
proof for all float inputs. Exception status flags, traps and aliased objects
were not compared.

The standard Win32 build and existing CTest result, plus the mechanical report
call check, are recorded in `reports/avoid_zone_offset.json`. No shared tests
were added. In this worktree `src/game_hosts_ship_ai.cpp:1684-1692` still has
the zero group/input-point boundary bindings; this new module itself is not
connected to the runnable game. No frame-causation claim is made. The primary
integrator can now bind `AvoidZoneBoundaryHost::call_00416f30` directly.
