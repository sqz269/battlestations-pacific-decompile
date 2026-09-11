# Retained Icon scale dispatch

Address newly reconstructed: 00AB2820. Existing contracts consumed:
00AA7950, 00AA7220, 00AB2600, 00AB1230, 00AB24B0, 00AB17B0,
00AB10D0 and 00AB3CB0. Names are hypotheses, not recovered symbols.

`GuiIconRuntime::set_scale48_00ab2820(GuiWidgetOwner&, const GuiWidgetSize&)`
closes Icon's virtual+48 on its existing retained runtime. The float carrier
uses width as x scale and height as y scale. The owner must reference the
runtime's same `GuiLayoutWidget`; a mismatched owner throws before any write.
No new callback interface, replacement Icon state or geometry owner is added.

| Routine | Coverage | ABI | Body evidence |
| --- | --- | --- | --- |
| 00AB2820 | Complete for the supported concrete Icon virtuals | ECX Icon; stack scale-pair pointer; RET4 | Start 00AB2820, final 00AB2849 RET4 length 3, inclusive end 00AB284B; 44 bytes, 15 instructions |

The full native sequence is base scale/recomposition, filter-choice query,
comparison with the live cached byte at +134h, and current+8Ch rebuild only
when they differ. There is no equality shortcut before applying the scale,
no base bounds refresh, no -1 current-state exception, and no direct write
to +134h in this routine. The existing rebuild owns any cache update.

| Containing routine | Call site | Native callee | Binding |
| --- | --- | --- | --- |
| 00AB2820 (raw at worker review) | 00AB2828 | 00AA7950 | x87 stores into the same scale_x/scale_y fields, then retained owner recomposition |
| 00AB2820 (raw at worker review) | 00AB282F | 00AB2600 | Live Icon filter eligibility and actual texture dimensions when required |
| 00AB2820 (raw at worker review) | 00AB2846 | Current +8Ch, Icon 00AB10D0 | Existing runtime rebuild using current state, through concrete 00AB3CB0 |
| 00AA7950 | 00AA795F | 00AA7220 | Actual retained native-model local matrix publication |
| 00AB2600 | 00AB2661 | 00AB1230 | Existing signed-index state lookup; failure is explicit |
| 00AB2600 | 00AB2668 | 00AB24B0 | Existing native-size comparison arithmetic |
| 00AB24B0 | 00AB24C0 | 00AB17B0 | Same captured texture: width before height; then resolved UV/size comparison |

00AA7950 was read in full. 00AA7954/56 and 00AA7959/5C use FLD/FSTP
to write +28h/+2Ch; 00AA795F calls 00AA7220; 00AA7964 is RET4.
The C++ also writes through pointers to those two actual projected scalar
fields, avoiding a cast of separate scalar fields to another object type.
Canonical definitions and producers remain `GuiWidgetTransform::scale_x/y`,
the Scale descriptor 00AAAED0, and defaults in 00AA9390.

## Filter-query evidence

00AB2600 was read as both pseudocode and its complete listing before naming
the value. The existing name `BSP_GuiIcon_PrefersBilinearFilter` agrees with
the material selection producer: 00AB3DEF calls it, 00AB3DFF compares the
result with +134h, and 00AB3E22 updates that same cache during a material
replacement. Existing `GuiIconWidget::cached_prefers_bilinear` is reused.

The query returns false without any platform or texture query for an
untextured Icon or a nonempty shader name. Otherwise it returns true when
the actual platform point-filter byte is clear, rotation is unequal to zero,
or either scale is unequal to one. Native UCOMISS/LAHF parity branches send
unordered values to the true result as well. Only the remaining case looks
up the current signed16 state and evaluates its native-size predicate.

That final lookup uses 00AB1230, whose body checks nonnull vector storage
and unsigned index against a 40h-record count. C++ reuses the existing
`gui_icon_state_at` and throws on a missing state; it does not fabricate a
record or skip a -1 index. 00AB24B0 captures that state's texture before
00AB17B0 calls texture+48h (width) at 00AB17C0 then +4Ch (height) at
00AB17E5. The implementation calls those existing actual services in that
order and reuses `gui_icon_state_differs_from_native_size_00ab24b0`.

The caller's scale pointer is loaded before PUSH ESI at 00AB2820. ESI
captures ECX at 00AB2826, survives both direct calls, supplies the cache
comparison and optional virtual receiver, and is restored at 00AB2848.
The original one-DWORD stack argument is confirmed by RET4 at 00AB2849;
no vararg list is inferred from pushes.

## Raw routine and virtual provenance

During worker review Ghidra has no function at 00AB2820 or 00AB282F.
The disk listing's 44 bytes were compared to the verified live byte dump:

```
8b44240456508bf1e82351ffff8bcee8ccfdffff3a8634010000740c8b168b828c0000008bceffd05ec20400
```

Live xrefs are the three vtable cells 00D5BD28, 00D5C508 and 00D5CC60.
Icon's table is 00D5C4C0: its +48h cell 00D5C508 is `20 28 AB 00`,
and its +8Ch cell 00D5C54C is `D0 10 AB 00`. Other derived types sharing
the raw scale body may have different current+8Ch behavior; this concrete
method supports the retained Icon implementation, not those unbound profiles.
Ghidra remains read-only for this worker. The parent owns the locked raw
function definition required before the normal call-site verifier can
attribute the three raw-body call rows; no checker exception is fabricated.
The verifier was run: seven rows checked, three failed solely for those
missing containing-function records, and the other four passed. Integration
must rerun it after the reviewed raw function is defined.

## Validation and prerequisites

The changed runtime and map adapter strictly compile and link with MSVC
Win32 `/W4 /WX /O2 /fp:strict`. The existing single actual-owner probe was
rerun and passed; it checks retained matrix publication and borrowed map
state. It does **not** execute the new scale/filter/rebuild path. No new test
target or fake resize/rebuild service was added. The layout worker also
strictly compiled its direct call against this header, removing its proposed
abstract scale callback. Standard combined build belongs to integration.

The runtime still requires its existing actual platform/texture, material,
geometry, D3D and lifetime services. Supplied owner, layout and state-record
storage must remain live during the call. Texture dimension providers must
preserve the floating-point environment; native width normalization occurs
before the height call, while the reused numeric kernel runs after both
dimensions have been obtained. Ordinary returned values and explicit scale
stores are covered; exceptional x87 trap/status timing, native SEH/ABI,
arbitrary reentrant destruction and original object-byte padding are not.
No installed-page render or gameplay validation is claimed.
