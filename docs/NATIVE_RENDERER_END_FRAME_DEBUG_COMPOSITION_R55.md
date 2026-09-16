# End-frame debug consumer composition (R55)

`end_native_renderer_frame_00b2d8e0` now calls the recovered `B2BB90` sphere
consumer and `B2B580` sprite consumer directly. The native order remains:

1. Optional surface save through renderer slot `+110`.
2. `B2BB90` at `B2D9DD`.
3. Current-profile renderer slot `+C4`, bound to the existing `B28D00` lines body.
4. `B2B580` at `B2D9F0`.
5. The existing render-state writes, XLive, EndScene, cache/counter work and Present.

`NativeRendererEndFrameRemaining` now contains only actual-owner queue execution
`B1EBE0`. Its two obsolete rendering callback methods were removed. The existing
typed `RenderCommandQueue` executor is not sufficient evidence for substituting
an actual native queue/command owner graph.

## Borrowed frame contract

Four appended context pointers supply each consumer's existing context and
persistent frame. They borrow the same actual renderer, model, geometry, names,
device, entry cache and camera domains used by the rest of end-frame. A pair can
be null when that child's initial current count is zero. A nonempty child needs
its existing prepared persistent frame, including the sprite consumer's current
canonical camera-pool binding and lifetime admissions.

The caller retains both frames and their admitted domains across failure. This
parent does not create, prepare, replay, retire or roll back child frames. It
performs the direct call at the original point; a child exception propagates to
the existing outer optional-guard cleanup. The child implementations retain their
own documented partial effects and constructor-only unwind behavior.

This modifies a reconstructed C++ context/interface. It makes no claim of original
ECX/RET4, private-stack, FH3/SEH, incidental-register or hardware-fault ABI parity.
Application activation still requires the genuine remaining queue provider and
construction of the complete owner graph.

## Verification

- Fresh live Ghidra/PE comparison of full `B2D8E0[749]` and the four existing
  rewind/leaf bodies (1089 bytes total), with live direct-call verification.
- Strict MSVC Win32 `/MD /W4 /WX /fp:strict` build and all three existing CTests.
- The existing end-frame fixture is copied unchanged and rebuilt against the
  current full libraries. Original/source buffer rewind, exact leaf encodings,
  recursive guards, retained partial failure, signed count and inactive
  context-free end-frame wrappers pass. Its copied rewind calls use the existing
  source optional-guard bridge; no original FH3 path is executed.
- The linked probe's parent body is inspected to confirm the direct sphere,
  lines and sprite consumer calls appear in native order. This is linkage and
  machine-code evidence; the fixture does not reach those active calls.

Active end-frame, loaded XLiveRender, application rendering, visuals and gameplay
remain unvalidated. Prior child fixtures retain their individually stated scope.
