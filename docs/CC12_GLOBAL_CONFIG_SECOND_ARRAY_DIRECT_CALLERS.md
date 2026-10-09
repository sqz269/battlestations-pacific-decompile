# GlobalConfig second-array direct-caller audit

The **nonnull producer of `GlobalConfig+2CC/+2D0/+2D4` remains unattributed**.
This packet classifies all **75 recognized direct callsites** of getter
`00432650`: 66 sites belong to 41 recognized caller functions, and nine sites
have no containing function in the current Ghidra listing. It also reviews all
seven recognized code references to singleton cell `00F878E4`.

The inspected return-use slices show scalar/vector/string reads, colour copies
and borrows, a timestamp clear, and the previously established loader/objective
paths. A separate effect-acquisition path publishes into **Game+718C**.
Six getter uses reach five unresolved virtual colour-setting dispatches. Those
are explicit remaining indirect edges; no readonly or non-retention behavior
is assumed for their current targets.

This is bounded return-use coverage, **not complete control-flow coverage of
every caller or a whole-program absence proof**. Parent commit `0B2EEDE10` was
merged before the audit. The only owned outputs are this document and
`reports/cc12_global_config_second_array_direct_callers.json`. No Source file,
Ghidra annotation/listing, configuration, ledger, build or probe was changed.

## Coverage and receiver provenance

The report retains a classification and exact instruction context for every
callsite. The table below groups the 66 function-contained sites by their
recognized caller. Names in the analysis database remain descriptive hypotheses.

| Caller entry | Getter sites | Observed use of the returned owner |
| --- | ---: | --- |
| `00432750` | 2 | Read floats `+78/+7C`. |
| `004329D0` | 1 | No explicit use before the next call; `0042F0C0` immediately overwrites incoming EAX from `FS:[0]`. |
| `00442AE0` | 1 | Borrow vector header `+4C` to `0043F880`, then read the returned heap element. |
| `004DC6A0` | 3 | Read effect-name vector `+0C`, acquire into a local reference and append to Game+718C; clear GlobalConfig+2D8 timestamp. |
| `004DC940` | 1 | Read `+F4+index*4` for FOV. |
| `004DCDF0` | 1 | Read `+F4+index*4` for FOV. |
| `004DDB90` | 1 | Pass the whole owner to loader `0087D7B0`. |
| `0051F050` | 2 | Read `+04` for binocular turn scaling. |
| `00526A40` | 2 | Read scalar `+5C` and an element through vector header `+3C`. |
| `00565FB0` | 6 | Select string headers `+A8/+B8/+B0/+C0/+D0/+D8`, then read their data pointers. |
| `00566050` | 1 | Read data pointer of string header `+C8`. |
| `005AB190` | 1 | Read `+2BC` for map-screen scaling. |
| `005ADDA0` | 5 | Colour borrows through current virtual+50; direct colour-copy helper; copy `+1A8..+1B4` to a UI buffer. |
| `005B25D0` | 1 | Copy `+1A8..+1B4` to a UI buffer, then restore the saved owner register. |
| `005BC640` | 1 | Read `+80` into voice-manager timing arithmetic. |
| `005C0F20` | 2 | Read `+70/+6C` minimap values. |
| `005D07E0` | 2 | Borrow colours `+218` or `+208/+218` through current virtual+50. |
| `005D09A0` | 1 | Borrow `owner+208+index*10h` through current virtual+50; no index domain is assumed. |
| `00642C20` | 2 | Copy four DWORDs from colour `+108` or `+128` into stack storage. |
| `0072F6E0` | 1 | Preserve owner in EDI across random-number call and read `+8C` at `0072F772`. |
| `0077F7B0` | 2 | Read `+104/+100`. |
| `00826F10` | 1 | Read an element through vector header `+1C`. |
| `00864680` | 3 | Read `+94/+9C`; borrow `+94` to scalar-only helper `00415550`. |
| `00864880` | 1 | Read `+88`. |
| `00864BD0` | 1 | Read `+88`, then store caller+6C and overwrite EAX. |
| `00864D90` | 4 | Read `+90/+98/+A0/+A4`; borrow `+90` to `00415550`. |
| `00864FE0` | 1 | Read `+88` for the gunnery update comparison. |
| `0087D730` | 1 | Read an element through vector header `+1C`. |
| `008D1340` | 2 | Read `+E0` as the blackout scalar default. |
| `008D16B0` | 1 | Read `+E0` as a Lua scalar default. |
| `008E1B90` | 1 | Pass owner+2C0 to `008DD460`, index zero. |
| `008E1D30` | 1 | Pass owner+2C0 to `008DD460`, index one/two. |
| `009103F0` | 1 | Read an element through score vector header `+2C`. |
| `00910480` | 1 | Read an element through score vector header `+2C`. |
| `00910500` | 1 | Read an element through score vector header `+2C`. |
| `00910570` | 1 | Read an element through score vector header `+2C`. |
| `00911C00` | 1 | Read an element through score vector header `+2C`. |
| `00911CE0` | 1 | Read an element through score vector header `+2C`. |
| `0094A140` | 2 | Read `+2E0/+2E4` for spawn placement. |
| `0094C490` | 1 | Read `+2DC` for the spawn-request decision. |
| `0095DA00` | 1 | Read an element through vector header `+4C`. |

The nine unassigned callsites were also inspected without repairing their
listing ownership. `0053B37B` derives `owner+108` and copies colour DWORDs from
branch-selected fields into stack/UI storage; the owner-derived EAX is replaced
by a colour value at `0053B483`. `005B4989`, `005B4EC4` and `005B4F5B` read
`owner+2BC`. `005B4F93`, `005B58E5`, `005B5909`, `005B593C` and `005B595E` read
`owner+2B8`. These are instruction-window findings, not invented function bodies.

The 75 classifications comprise 36 scalar reads, seven string-data reads,
12 vector reads, two scalar borrows, five colour value copies, one direct
colour borrow, six opaque virtual colour borrows, one Game-owned effect
publication, one timestamp clear, one discarded getter result, one loader
forward and two objective forwards. No bounds on authored Lua values or runtime
indices are used to turn these findings into a sound-payload contract.

## Effect acquisition and timestamp writes

At `004DC6BC`, EBP captures the original Game receiver from ECX. The getter at
`004DC78C` returns GlobalConfig into EAX, then ESI. Instructions
`004DC793..004DC7AE` read the name vector beginning at owner+10 and form the
selected name address in EDX. `004DC7B3` forms a **stack** output address in ECX
for `00871BA0`; that wrapper explicitly returns its supplied output address in
EAX at `00871BC3`.

`004DC7BC` pushes that returned local-reference address, `004DC7BD` sets
`ECX=EBP+718C`, and `004DC7C8` calls `004D9C00`. The append helper's receiver is
therefore the original **Game+718C** vector. Its copied reference is not stored
at GlobalConfig+2CC/+2D0/+2D4. This is the concrete destination of the general
effect-acquisition edge that previously lacked caller attribution here.

The later getter at `004DC802` is followed by `ADD EAX,2C0` at `004DC80A` and
`MOVSS [EAX+18],XMM0` at `004DC811`, after `XORPS XMM0,XMM0`. That write clears
the **timestamp at owner+2D8**. Treating the enclosing block offset alone as a
second-array pointer write would misidentify the destination and value.

The accepted previous packet, commit `9473BC6B9`, already follows
`008E1B90/008E1D30 -> 008DD460 -> 00A7E490`: the sound output is a stack
temporary, with manager+8C publication and local reference cleanup. It also
qualifies the loader's unchecked `008DBE90` sample indexing. Both paths remain
excluded from second-array sound attribution; their analysis is retained by
reference and hash in the new report.

## Borrowed colour pointers and the exact indirect frontier

Two direct helper contracts have concrete read direction. `00415550` reads
floats through ECX and EDX and returns its x87 result without writing either
pointee. `0059A9B0` reads four DWORDs from its supplied colour pointer and copies
them into one or two UI outputs obtained through `00B179F0`; it neither writes
nor retains the supplied pointer. The `005AE5B0` return is held in EDI, offset
by `108h`, and used by the retained colour-selection branch through `005AE9BF`.
These observations describe native pointer/data flow, without reconstructing
new floating-point or UI behavior.

The following **six getter sites reach five indirect dispatch instructions**.
Their current receiver profiles and exact virtual+50 targets have not been
attributed in this packet. A borrowed pointer is an explicit escape here; it is
not assumed to be discarded or readonly merely because neighbouring calls
copy colour data.

| Getter site | Borrowed address | Dispatch instruction and receiver |
| --- | --- | --- |
| `005ADFD8` | owner+108 | `005ADFF7`, current virtual+50 of `[ESI+18]` |
| `005ADFE4` | owner+128 | `005ADFF7`, same branch-merge dispatch |
| `005AED36` | owner+158 | `005AED48`, current virtual+50 of `[EDI]` |
| `005D08F1` | owner+218 | `005D0904`, current virtual+50 of `[ESI+5C]` |
| `005D092B` | owner+208 or +218, selected by caller+8C byte | `005D094E`, current virtual+50 of `[ESI+60]` |
| `005D0AD7` | owner+208+stack_index*10h | `005D0AF2`, current virtual+50 of ESI |

These are colour-pointer edges, not positively attributed sound producers.
They bound what this direct-caller pass can establish. It does not expand all
possible Sound kind-one factories, because none of these paths supplies a
second-slot publication requiring that expansion.

## Singleton-cell references and construction

The seven live `00F878E4` xrefs consist of three writes and four reads:

- `0042AC00` stores literal zero; the rest of that small constructor only sets
  its separate receiver's vtable.
- `00432665`, `004326A1`, `004326DE` and `004326FB` read/check the cell inside the
  getter. At `004326E7`, the owner is passed to manager registration `00BD0C30`.
- `004326D4` installs the owner returned by `004324E0`, or the null allocation
  path, into the singleton cell.
- `00432631` clears the cell after aggregate destruction.

The owner's construction route is `004326C6 -> 004324E0 -> 008DBCD0` with
`owner+2C0`. The embedded slot constructors `004BA0D0` and `0054D440` each write
literal zero; the aggregate's explicit second-array stores at `008DBD79` and
`008DBD81` also write zero. This establishes the inspected initialization
operations only. It is not proof of later emptiness and does not authorize
no-op stop/final-release callbacks.

The registration call exposes the whole owner to an existing retained manager.
This packet does not expand every later indirect lifetime callback. Cached
owner aliases, unrecognized code and indirect calls to the getter remain
outside the recognized direct-xref graph.

## Evidence and remaining admission

Every live query used `bsp.py ghidra`, verifying project `bsp`, program
`/battlestationspacific.exe`, language and image base before reading. The
configured project file is `C:/Users/sqz269/bsp.gpr`. The report embeds 82 initial instruction contexts,
six extended continuation contexts, the selected HUD branch listing, every
callsite classification, and **70 merged native address windows / 8,310 bytes**.
All recorded address windows match the preserved installed PE. They are bounded
windows and helper bodies, and may include intervening padding or neighbouring
instructions; they are not presented as complete exports of all 41 callers.

No nonnull raw second-array profile has been established. Source admission
still requires an actual publication into owner+2CC/+2D0/+2D4 (or block+0C/+10/+14),
its input domain and current raw pointee profile, followed by compatible Source
storage and stop/non-decrementing final-release implementations. Existing
canonical Sound string/lifetime composition is not changed by this audit.

No build, probe, fixture, native code execution, GPR mutation, game startup or
gameplay validation was performed. The original PE was read only.
