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
| Segment-9 helpers (20) | yes | yes | yes | no |

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

## Evidence method

Live `bsp.py ghidra decompile`, `disasm-raw` for every listed body, constants from the PE
(`local/sys4_consts.py`). Call-site rows: `reports/cc13_sys4_calls.json`
(`tools/verify_report_calls.py`: 33 rows, 0 failed).
