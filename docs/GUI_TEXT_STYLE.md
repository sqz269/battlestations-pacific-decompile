# Concrete Text style operations

Addresses: 00AB6B50, 00AB6C30, 00AB7200. Descriptive names are hypotheses,
not recovered symbols. The worker used verified read-only queries against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; no Ghidra mutation.

`GuiTextStyleBinding` borrows the same `GuiWidgetOwner`, existing
`GuiTextWidget`, live `NativeNodeBinding*&` shadow slot (+188h), canonical
material services and `NativeNodeParentingRuntime`. It creates no Text state,
model, material, node hierarchy, lifetime registry or forwarding interface.
The Text factory, glyph children and secondary-model lifetime remain external.

| Routine | Coverage | Original ABI and exact ending |
| --- | --- | --- |
| 00AB6B50 | Complete for actual canonical model/material owners | ECX Text; RGBA pointer stack; final 00AB6BC1 RET4, length 3, inclusive end 00AB6BC3 |
| 00AB6C30 | Complete with raw enable byte preserved | ECX Text; stack enable byte in DWORD, shadow-position DWORD, offset float, color pointer; final 00AB6CAA RET10h, length 3, inclusive end 00AB6CAC |
| 00AB7200 | Partial input-domain projection: complete body for hidden widgets or non-hidden states 0..3; unsupported non-hidden other indices select arbitrary native storage at 00AB7215..00AB7220 | ECX Text; state DWORD stack; final 00AB7281 RET4, length 3, inclusive end 00AB7283; raw 132-byte routine |

## Color publication

00AB6B50 first calls the concrete base setter 00AA6870. The standalone base
function supports this call without using the generic base-vtable callback
binder, which correctly does not claim Text's override. The input remains
a borrowed float[4] throughout. The existing `GuiTextWidget::color` projection
mirrors the canonical layout's base-color stores so existing text consumers
observe the same update. No new color cache is introduced. The caller must
associate its one Text projection with its one retained owner; no Text factory
association is fabricated by this binding.

The tail reloads the shadow slot, returns if null, tests its actual geometry
pointer, tests actual geometry+58, reloads the shadow geometry, resolves element
zero and its actual material+20. It multiplies the current shadow alpha at
Text+174h by the incoming color alpha, stores the float result, then writes
only the shadow material diffuse alpha at material+38h+0Ch. Shadow RGB,
material dirty flags and named parameters are untouched.

The shadow's actual storage key is resolved through the already-existing
`GeneratedModelLifetimeRuntime::find_actual_node`, then checked as the same
live `NativeModelReference`: node binding, native storage, native node runtime
and actual material-owner domain must agree. No node-wrapper pointer is cast
to a model-wrapper pointer. The section material similarly resolves through
the canonical actual-owner domain and must be the same live
`NativeMaterialReference` and `NativeMaterialStorage`.

| Function / call site | Callee | Concrete operation |
| --- | --- | --- |
| 00AB6B50 / 00AB6B59 | 00AA6870 | Existing base color/material publication |
| 00AB6B50 / 00AB6B68 | 00B74650 | Same shadow model geometry-presence read |
| 00AB6B50 / 00AB6B79 | 00B74640 | Same model geometry pointer |
| 00AB6B50 / 00AB6B80 | 00B72B40 | Actual geometry DWORD+58 |
| 00AB6B50 / 00AB6B93 | 00B74640 | Reloaded shadow model geometry |
| 00AB6B50 / 00AB6B9A | 00B732C0 | Actual pointer-array+54 element zero |
| 00AB6B50 / 00AB6BB3 | 00B179F0 | Alias to actual material diffuse quartet |
| 00AB6C30 / 00AB6C8E | 00B6E680 | Actual shadow-node parenting |
| 00AB6C30 / 00AB6CA4 | 00B6D890 | Actual root-registration clear after byte reload |
| 00AB7200 / 00AB727C | Current +50h, Text 00AB6B50 | Concrete color publication above |

These callees were read before their contracts were bound. Color50's ESI
captures the Text receiver at 00AB6B57; EDI captures the RGBA pointer at
00AB6B52, survives all calls and supplies alpha at 00AB6BA5. +188h is reloaded
at 00AB6B5E, 00AB6B71 and 00AB6B89, as the implementation does.

## Shadow configuration

00AB6C30 writes +160h position, +15Ch raw enable byte, +164h offset, then
the four shadow-color DWORDs. The offset uses MOVSS, the color uses integer
copies. It passes the Text's current +4Ch node as parent when the original
enable argument is nonzero, otherwise null. After actual parenting it rereads
the stored enable byte and, only when zero, reloads +188h and clears root
registration. Callback reentry may therefore change the branch or shadow slot.
The supplied objects must remain live; reentrant destruction is unsupported.

The canonical `GuiTextWidget::shadowed` field changes from bool to uint8_t.
This preserves raw values such as the byte loaded at 0054A090 from +61Fh;
existing authoring and preset bool assignments remain valid. The fields and
ShadowPos meanings already come from the existing Text reader/constructor.
No second raw shadow flag or conflicting offset declaration is added.

All four direct call sites were inspected: 0054A09A uses the raw +61Fh byte;
0054BC69 and 0054BE4E pass 1, position 0, an offset from 00CE7638 and black
with alpha 1; 00AB9E05 passes 1 in EBX, position 0, default offset and
black with alpha 1. The whole 00AB98F0 listing was filtered for EBX/BL;
00AB9D9D writes EBX=1 before the 00AB9DFC push, with no intervening write.
All four arguments are retained. The callee's RET10h
confirms four stack DWORDs; no count is inferred solely from push listings.

## State-color selection

00AB7200 reads the actual owner flag +77h. Its producer 00A9C380 was reread:
it writes zero for a visible list item and one for a hidden item. The existing
`GuiWidgetSceneFlags::hidden` is therefore used; node visibility and a newly
invented disabled flag are not substitutes.

Hidden selects the existing disabled color at +14Ch. Otherwise the native
address is +11Ch + state*10h, corresponding to normal/focus/selected/disabled
for 0..3. The typed implementation rejects other non-hidden indices because
it does not expose unrelated native object storage as color records. A hidden
widget permits any state because native indexing is bypassed. There is no
has_state_colors gate and no persistent state-index store.

The selected quartet is copied to the native temporary before state 1 adds
the exact double 0.2823529541492462158203125 (00D5C5D0 bytes
`00 00 00 20 12 12 D2 3F`) to RGB and +0.0 (00D7A258) to alpha. This happens
for state 1 even when hidden selected the disabled row. It does not clamp.
The x87 stack operations and per-lane float stores are reproduced, including
the alpha addition; the resulting temporary goes to actual Text color50.

## Raw function and validation

The worker found no Ghidra function at 00AB7200. Full disk instructions through
00AB7281 RET4 were compared with live bytes through inclusive 00AB7283;
00AB7284 begins INT3 padding. Text vtable 00D5C6C8+80h (00D5C748) contains
`00 72 AB 00`; +50h (00D5C718) contains `50 6B AB 00`. The report retains
the raw call row for parent definition and subsequent exact-call verification.
The strengthened verifier ran: 14 rows checked; 13 passed, and only the raw
00AB727C call failed for its missing Ghidra containing-function definition.

The new source and existing `src/gui_text.cpp` strictly compile under MSVC
Win32 `/W4 /WX /O2 /fp:strict`, including the shared byte-field change.
No tests were added and these new operations were not executed. Standard
source registration/build and the raw-function definition belong to primary
integration. There is no native differential, installed-page or render claim.

Coverage requires actual existing model/material/lifetime domains and a valid
caller-provided association between Text projection, retained owner and live
shadow slot. Text type construction/destruction, glyph resource setup and
geometry creation are still prerequisites, not stubbed by these operations.
Native object/SEH/exception ABI, malformed native indices/storage and exact
exceptional floating-point trap/status timing remain outside this typed API.
