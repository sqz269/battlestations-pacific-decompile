# AA3E00 eye/target correction

The GUI pass camera faces positive Z. Its eye is
`(0.5 + 0.5/width, 0.375 + 0.5/height, -10000)` and its target is
`(eye.x, eye.y, 0)`, with up `(0,1,0)`. The previous reconstruction placed the
negative Z value in the target and zero in the eye, reversing the view direction.
The source helper now uses the original caller's operand identities.

## Caller stack evidence

Verified against existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Let S be ESP immediately before the call at
`00AA3EB1`; there is no intervening stack adjustment between the pointer setup
and the stores listed here.

| Original instruction | Actual identity |
| --- | --- |
| `AA3E49 SUB ESP,0C`; `AA3E4E LEA ECX,[ESP+2C]`; `AA3E56 PUSH ECX` | Stacked target pointer becomes `S+30`. |
| `AA3E57 LEA EDX,[ESP+3C]` | Eye pointer is `S+3C`. |
| `AA3E79 MOVSS [ESP+38],XMM0` | Positive zero at target+8. |
| `AA3E7F MOVSS XMM0,[D19620]`; `AA3E87 MOVSS [ESP+44],XMM0` | Negative depth at eye+8. Actual image bytes are `00 40 1C C6`, binary32 `C61C4000`, exactly -10000; the earlier -9992 comment was also incorrect. |
| `AA3E9B FST [ESP+30]`; `AA3EA3 FST [ESP+34]` | Half-pixel adjusted target X/Y. |
| `AA3EA9 FSTP [ESP+3C]`; `AA3EAD FSTP [ESP+40]` | Same half-pixel adjusted eye X/Y. |

`B63F10` confirms those roles independently. Native EDX is saved in EDI at
`B63F1B`; `B63F73..83` copies EDI's xyz as the eye. `B63F4B` loads the stacked
target pointer. `B63F87..A3` forms target minus eye, before normalization and
basis construction. Original ABI: ECX destination, EDX eye, stacked target
pointer/up.xyz, EAX destination, RET10h. The typed builder preserves those
inputs and the original floating-point schedule.

`AA3EBD..C0` passes the returned matrix to `B71490`, which inverts through
`B63B30`, invokes current camera+34 (`B71460`), then refreshes direction. The
current source uses that same chain over the actual camera. `AA2020`'s original
ortho side/depth volume and the native CullCCW state require no change.

## Effect on the native Text bring-up

The incorrect view reversed X in the actual VS constants and made native glyph
quads face away from CullCCW. This is a caller-argument recovery error, not a
reason to change glyph indices, culling or issue a separate draw. The parent
bring-up's temporary Text Pos Z=-10000 was chosen using the incorrect camera;
that authored diagnostic must return to Z=0 with this correction. It would
otherwise lie at camera-space depth 0 and fail the near plane at 1.

For 1280x720 widescreen, Text translation `(0.05,0.075,0)` should yield clip origin
approximately `(-0.675586,0.801852,0.500375,1)`. Source/system publication and the
prior real D3D dump established the same raw camera and object matrices were
being consumed; the caller's reversed eye/target values caused the bad view.
Live rendering after this correction remains the integrator's validation.

## Scope and validation

Only the shared camera placement, its constant name and explanatory evidence are
changed. Existing tests contain no placement Z expectations requiring updates;
no new test is added. `scripts/build.ps1` passes Win32 Release and the existing
CTest checks 2/2 (11.18 seconds). Current C++ interfaces remain typed ports, with no native
caller ABI or unrestricted FP/fault equivalence claim. No game run is performed
by this worker; the integrator builds and validates the composed native Text
path. Ghidra's previous comment is preserved and an explicit correction appended
through the locked annotation tool; its saved preimage is retained under local/.
