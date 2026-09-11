# Force event base destructor

Address00872C90 is an18-byte destructor, not a pointer-adjusting thunk:
`MOV [ECX],D0C88C; MOV [ECX+8],D0C888; JMP BD30F0`.
Its inclusive end is00872CA1. BD30F0 changes only the primary word toCEB130.
There is no ECX adjustment, reference operation, request cancellation or free.
The prior automatic label `CG_adjustor_thunk_00872c90` is replaced with the
descriptive hypothesis `BSP_ForceEvent_DestroyBase`. Existing evidence/comments
are preserved by the annotation audit.

The constant/fading/alternating constructor exception maps all call this body.
See [native owners](NATIVE_GAMEPAD_FORCE_EVENT.md) and
`reports/native_gamepad_force_event.json` for exact maps, callers, original-byte
validation and the boundary between C++ cleanup and unvalidated native SEH.
