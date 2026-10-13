# CC13 engine pass over segments 4, 6 and 9, continued (packet cc13-sys4)

Continues `docs/CC13_SYS_2.md` over `004486C0-00453C30`, `00465610-0047C680` and
`004BA290-004D0860`. Code: `src/engine_seg4_6_9_cc13.cpp`, `include/bsp/engine_seg4_6_9_cc13.hpp`
(namespace `bsp::cc13_sys`). Every name is a hypothesis. Nothing here is ABI compatible or
game-validated; the hosts take field values and callbacks where the image works on raw objects.

## Status

| Item | Exported | Reconstructed | Build-tested | Validated |
| --- | --- | --- | --- | --- |
| 13 lazy singleton getters, 2 teardowns | yes | table + `lazy_singleton_get`, `lazy_singleton_destroy` | yes | no |
| x87 geometry `0046F550..00470150` (7) | yes | yes | yes | no |
| `004656D0` particle-effect movie action run | yes | yes | yes | no |
| Segment-9 helpers (20 + 35 + 3) | yes | yes | yes | no |
| Segment-4/6 helpers, matrices, `004702E0` (23 + 12) | yes | yes | yes | no |

## Singleton getters

All thirteen share one body (listing `0047225A..004722BC` for `00472200`): test the global; on
null take the lifetime manager's lock (`00415350`, critical section at `+10h`, depth at `+18h`),
test again, `operator new` (`00BF681B`), construct (a `__thiscall` constructor that returns
`this`) or store 0 when the allocation failed, publish, and register (`00BD0C30`, ECX = manager)
the published pointer plus a sub-object offset. The register call is made even with a null
pointer: there is no test between the store and the call. The rows are in `kLazySingletonGetters`:

| Getter | Global | Size | Constructor | Registered |
| --- | --- | --- | --- | --- |
| `00472200` AnimClass registry | `00E18688` | 10h | `00472100` | +0 |
| `00473AF0` Bullets visual classes | `00E18690` | 10h | `004739F0` | +0 |
| `00475640` BuoyClass registry | `00E18698` | 10h | `00475540` | +0 |
| `004798A0` CloudClass registry | `00E186A0` | 10h | `004797A0` | +0 |
| `0047B2F0` VehicleClass registry | `00E186DC` | 10h | `0047B1F0` | +0 |
| `004C0960` world-view job | `00E18AD8` | 8 | inline: `[+4]=00CE7488`, `[+0]=00CE753C`, `[+4]=00CE7538` | +4 |
| `004C0A30` camera-update job | `00E18ADC` | 8 | inline: `[+4]=00CE748C`, `[+0]=00CE7544`, `[+4]=00CE7540` | +4 |
| `004C17D0` avoid-zone registry | `00E17620` | 14h | `00424730` | +0 |
| `004C1AC0` front-end frame screen | `00E18D80` | 164h | `00517D10` | +8 |
| `004C1B90` sound request queue | `00F89B34` | 28h | `00940DD0` | +0 |
| `004C1C50` game singleton | `00F8AAFC` | FD8h | `00A37130` | +0 |
| `004C1D10` game singleton | `00E19A64` | 50h | `004BF9A0` | +0 |
| `004C1DD0` profiler | `0109DB50` | 30h | `00BE3820` | +0 |

`004C0E40` (`00F87440`) and `004C0EF0` (`00F8753C`) are the matching teardown: under the same
lock, unregister (`00BCFCA0`), the deleting destructor (vtable slot 0 with 1) when the global is
still set, then clear it.

## x87 geometry

Traced from the disk bytes with capstone (a copy of the cc13 scene tracer with `FUCOMPI`/`FCOMPI`
pop handling added, `local/sys4_x87.py`), branch by branch, and checked against the decompiler.
Each named intermediate is stored to a float slot and reloaded; the sums between those stores stay
on the x87 stack and the hosts evaluate them in double, which matches 53-bit precision control (as
for `0046FDD0` in sys2). Under 24-bit precision control the sums would round differently.

- `0046F550` closest point on a line: `t = dot(dir, f(point - line_point))`, `out = f(line_point +
  f(dir*t))`, `*distance = 0042B2F0(f(point - out))` (inline copy).
- `0046F6D0`: the length of `f(dir*t)`, the projection length.
- `0046F7B0` (renamed `BSP_Geometry_ProjectPointOntoPlane`, was "ClosestPointOnSegment"): `out =
  f(point - f(normal*t))`, `*distance = |out - plane_point|`.
- `0046F940` (renamed `BSP_Geometry_ClosestPointsBetweenLines`, was "SegmentPlaneIntersect"): the
  second line's direction is passed by value at `[ESP+8..10h]`. When `f(dot(u,v)) == 1.0` exactly
  (`FUCOMIP`, `LAHF`, `TEST AH,44h`, `JNP`: a NaN dot takes the general path), `a = p1` and `b` is
  `0046F550(p1, p3, v)` with its distance. Otherwise `s = f((vw*uu - uw*uv) / (vv*uu - uv*uv))`
  with both differences on the stack, `t = f(-((uw - s*uv)/uu))`, and the distance is
  `0042B2F0(f(a - b))`. Anti-parallel unit directions (dot `-1`) divide by zero.
- `0046FE80`: `v` minus the `0046FDD0` projection.
- `0046FF70` (renamed `BSP_Geometry_PointSegmentDistance`): the `FLDZ` of the inverse-length test
  is left on the stack and reused as the zero for both end-cap compares. `d1 = dot(p - a, n) < 0`
  returns `|p - a|`, `d2 = dot(b - p, n) < 0` returns `|b - p|`, otherwise the perpendicular
  distance. NaN dots fall through to the next case.
- `00470150` (renamed `BSP_Geometry_PointProjectsOntoSegment`): the same two tests as a bool. The
  first failure clears AL only (`XOR AL,AL`), the second clears EAX. A NaN `d1` passes, and a NaN
  `d2` fails (`JB` after `FCOMIP`).

The Ghidra listing was not used for any of the arithmetic. `tools/x87_dc_form_scan.py` finds no
DC-form instruction in `0046F550..004702E0` and one in `004656D0`, `0046581F DC E9` (`FSUB
ST(1),ST(0)`), which the decompiler renders correctly.

## 004656D0, particle-effect movie action run

The run asks the entity for its parent (vtable `+18h`), refreshes the entity's world pose
(`00414DB0`) when the byte at `+C8h` is clear, acquires the effect handle by name (`00871BA0`, name
at `action+2Ch`), and creates the point effect with an identity matrix (`008687C0`, words 1, 0, 0).
It then drops the temporary result reference, clears the effect's byte `+9`, and steps the
effect's children (`00867790` with `[[00E188A8]+19FCh]`) up to StartTime (`action+20h`). The step
is 0.2 (double `00CE3D10`, float `00CE54A0`). The older ledger note says 0.1f, and that note is now
corrected. The run stores the effect at `+44h`.
The entity byte `+C8h` is read even for a null entity, so a null entity would fault the image.
RNG rule: the children update reaches effect code that can draw from stream 0 of `00BD2F10`,
so it is a host callback, and the header asks for a presentation stream or an OFF switch.

## Segment-9 helpers

`004BB600` (Live sign-in byte), `004BCB40` (counted-string find; start above the length is an
unsigned `JA`), `004BDF50` (apply pending size through vtable `+10h`), `004BEA10`/`004BEA40`/`004BEA70`
(owned-pointer delete: destructor, `_free`, clear), `004BEAD0` (unit cast on class flag `0Fh`),
`004BFD70` (session render resources: three counted references at `+D5Ch` step `14h` of the
`+5FCh` block), `004C2D40` and `004BEF00` (observer-slot constructors, vtables `00CE75CC` and
`00CE3C8C`), `004C2D60`/`004C2D90` (release if subject), `004C2FD0` (UTF-16 storage, `count*2+2`),
`004C5140` (input context level 5), `004C5E00`, `004C5E20` (wide-string assign), `004C5EC0`/`004C5EF0`
(profiler scope), `004C5F10`/`004C5F40` (session message E4).

## Segment-9 helpers, second batch (35)

- Pooled-string releases as one table (`kPooledStringReleases`): `004C67E0`, `004C6800`,
  `004C89E0`, `004C8260` (UTF-16, `len*2+2`), `004C8DB0`, `004CE690` (`__stdcall`), and the record
  destructors `004C5720` (three strings) and `004C6760` (two). `004C9420` and `004CC650` reuse it.
- Strings: `004C8DD0` (wide copy construct), `004C8900`/`004C8950`/`004C89A0` (the game's command
  line text at `+600h` and console string at `+2198h`), `004C9220`/`004CC390` (60h-byte entry
  assign/copy), `004C95D0` (named entry), `004C9680` (front-end frame select with an empty title).
- `004C7070` and `004C70C0` format the diagnostic text and then call only the sink getter
  `004C14C0`, with no arguments. The text never reaches the sink, so release-build diagnostics are
  dropped.
- `004CEB40` (platform pause; the tail-jump sibling `004CEC00` has no Ghidra function and no
  absolute reference), `004CA120` (high-latency warning via the client's `007349C0`), `004BC4E0`,
  `004BC520`, `004BC7F0` (XLive presence, friends enumeration, leaderboard cancel; the friends
  enumerator leaks its handle when `XEnumerate` fails), `004BB3E0` (lobby slots: state 3 to 1 in
  modes 0..3, 1 to 3 otherwise), `004C6B00` (next scene record, signed `IDIV`).
- Containers: `004CAF10`, `004CC8E0`, `004CDE20` (string deque pop), `004CE460`,
  `004CEC20`/`004CEC40` (resize to zero, then free), `004CF590` (forward to `004CC760`), `004C7F80`.

## Segments 4 and 6, third batch (23)

- `004702E0` closes the x87 cluster: `v = 0046FE80(v, axis)`, `0042B260` normalises the axis and then
  `v`, and `out = axis x v` with each component one product difference rounded once.
- Matrices (row-major, `00413920` is `out = left * right`): `0047C210` multiplies in place,
  `0047C240` builds an identity with a translation, and `0047C2A0` computes `out = (Rz * Rx) * Ry`.
  The order is now read from the listing, where the older note had left it unchecked.
- `00452B00` (renamed `BSP_LensFlare_PowResponse`) stores `|x|^k` with `k` at `+1Ch`. The body is
  `FYL2X`, `FRNDINT`, `F2XM1`, `FSCALE`, and it stores 0 for `x == 0`. The earlier "2^(k*|x|)"
  reading missed the `FYL2X`. `00452A50` sets the depth-of-field range. `00452B80` enables the
  lens effect; when it turns the effect off it calls `00B0D020(0, 0,0,0,0, 1.0f)` and stores
  `+4 = 1e-4f`. `004531F0` shows the last Win32 error in a MessageBox (`FormatMessageA` into
  `00E18000`, size 3FFh, `"%s\n%s"` through the static 400h formatter).
- Six more byte-identical `+0/+4` string releases join `kPooledStringReleases`. `00469B20` and
  `0046A0C0` are the `004C9420`/`004CC8E0` bodies, but they return the node to the pool at
  `00E18514` instead of `00E175E8`, because `0043B0A0` takes the pool in ECX.
- The key/value pair constructors `00469AB0`, `0044C5D0`, `004713B0`, `004747F0` and `00478A50`
  are identical apart from their call displacements. `0044C6E0` is the copy variant (RET 4).
  `0044CCE0` is the E1 message destructor (the constructor `0044C970` builds through stream
  objects and is still open).

## Segment 6, fourth batch (12)

`00470B80` (operator new then memset, with no null test), `00470BA0` (copies the static vector at
`00F87574`), and `00472540`. The last one calls vtable `+D8h` with the stack unchanged, so the
callee sees the caller's return address where an argument would be; the host calls the slot with
no arguments. Also in this batch: `0047BEC0` (observer link base), `00475E60` (recursive flag OR
over `+34h`/`+3Ch`), `00467B50` and `004683B0` (intrusive list erase and clear), `00467F60` (POD
push_back; Ghidra's decompile drops the `size++` at `00467F91`), the cloud particle array
`00476E40`/`00476EE0`/`00477020` (`00477020` frees the data, so it is a tidy, not a clear), and
`00477110` (cloud class destructor).

## Segment 9, fifth batch (3)

- `004BBDD0` clips the director-path segment to the world's xz box (caller `0071FDE0`, director
  path AddPoint). The x87 body was traced by hand and the DE forms checked with
  `tools/x87_dc_form_scan.py`. Two quirks: if the clipped end is less than 1.0 from the start in
  `|dx|` or `|dz|`, the end gets its x and z back, but the function still returns 1; and with no
  clip at all it returns 0 without writing `out`. Otherwise `out` is the end pushed `3000 / m`
  along the normalised xz direction, where `m` is the smaller of the two direction components,
  floored at 0.02.
- `004D0140` drains the award tracker's hint deque and frees the map, and `004C8310` reserves a
  vector of counted references (it adds a reference for each copy and releases the old elements).

## Packet cc13-sys4b (continuation)

- `004CD7F0` takes a third argument, `u8 append_mode` (RET 4). The existing host
  `derive_scene_short_name` (`src/mission_scene_load.cpp`) covers the body only up to
  `004CDB22`. The tail at `004CDB27..004CDBC1` appends a game-mode suffix from `00E08240`:
  IC1 IC2 IC3 IC4 Du Esc SiE Com SOLO SOLO. The index uses the rule shared with `004C0170`:
  mode `+614h` when `+61Ch` is set or `+1FE4h != 0`, otherwise `+614h` when it is 8 or 9, else
  8. The mission-load and scene-contents callers push 1. `derive_scene_short_name_004cd7f0` wraps
  the old host and adds the suffix; the old host is unchanged, and the lead has been told.
- `004C0170` publishes Live presence. Whenever 3 s pass, `XUserSetContext(8001h, 5)` runs outside
  a session or the menu. In a session there are four `XUserSetContextEx` calls (contexts 0, 1,
  0Fh, 8001h), made when the xuid, record id or mode index changes, the dirty byte `+22DCh` is
  set, or 3 s pass.
- `004BC580` and `004BC6D0` are the leaderboard reads. The friends read repeats
  `XUserReadStats` synchronously (NULL overlapped) after an `ERROR_INSUFFICIENT_BUFFER` probe.
  The rank/xuid read enumerates asynchronously into `+7054h` and never closes its enumerator.
- `00468050` (renamed `BSP_PodPointerArray_Resize`) ends with `size = n` at `00468098`. Ghidra's
  decompile drops that store, just as it drops the `size++` in `00467F60`. `004C9870` resizes four
  such arrays to 0 and frees their data.
- `004CEC00` now has a name and a record.
## Evidence method

Live `bsp.py ghidra decompile`, `disasm-raw` for every listed body, constants from the PE
(`local/sys4_consts.py`). Call-site rows: `reports/cc13_sys4_calls.json`
(`tools/verify_report_calls.py`: 100 rows, 0 failed).

## Handoff

Done: 116 reconstruction records (13 getters + 2 teardowns, 7 geometry, 1 particle run, 58
segment-9 helpers, 35 segment-4/6 helpers including `004702E0`), five Ghidra renames
(`0046F7B0`, `0046F940`, `0046FF70`, `00470150`, `00452B00`) and two corrected ledger notes
(`004656D0` step 0.2, `0047C2A0` order).

Still open in the three segments (not taken; owners in brackets where known):
- Larger bodies: `004C0170` (presence update, 181 decompile lines), `00452BD0` (lens flare from
  the camera, 169 lines), `004BC580`/`004BC6D0` (leaderboard reads), `0044C970` (E1 message
  constructor through stream objects), `004C9870` (calls the unnamed `00468050` and a `_free`
  thunk), `004CA2F0` (window close with the quit prompt), `004CAC30`, `004CB420`, `004CCBD0`,
  `004CCCC0` (LeaveSession), `004CD450` (unreferenced screenshot writer), `004C9EC0`, `004C3C90`,
  `004C6BE0`, the cloud entity bodies `00475990`/`00476550`/`00476BD0`/`00476D80`/`00477180`,
  `00471080`/`00479BD0` (SEntity init), `00466BB0`/`00466C00`/`00467C80`.
- `004CD7F0` already has a reconstruction (`derive_scene_short_name`, `src/mission_scene_load.cpp`)
  without a record; its owner should backfill after an ABI check (the third `char` argument was not
  checked here).
- Left to cc9: `004C74C0` (border zone, read by the cc9 retreat chain), `004BC410` (camera mover,
  cc9_mission_camera), `004CC3F0` (end-of-mission movie), the dialog/panel classes in segment 4,
  and the list in the brief.
- Left to the scene-database owner: the `SceneNameHash`/`SceneDatabase`/`SceneRecord`/`SceneClassMap`
  bodies in segment 6.
- Thunks and catch funclets (`004BD350`, `004C4590`, `004C8070`, `004C8B70`, `004C8BF0`,
  `004C9A60`, `00453570`, `00468790`, `Catch_All@...`) were not tagged.
- Precision: every x87 host assumes 53-bit precision control.