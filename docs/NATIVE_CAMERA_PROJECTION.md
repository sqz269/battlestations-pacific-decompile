# Native camera projection

This packet reconstructs complete B642F0 (176 bytes) and 412E20 (19 bytes).
The existing semantic camera API is unchanged. Descriptive names are
hypotheses, not recovered symbols.

The original builder takes ECX destination and four stack float DWORDs in
fov/aspect/near/far order, returns EAX destination and uses RET10h. The new
fastcall entry takes a raw 64-byte output and writable 16-byte argument-slot
storage in ECX/EDX, returns EAX destination and uses plain RET. Those slots
represent the original callee's scalar argument copies, never direct camera
owner fields. A future full getter must prepare temporary scalar slots using
its own original ordered argument loads/stores; this builder must not be used
to mutate an owner's fov or far-plane fields.

The first source FLD precedes multiplication by immutable double 0.5 at D7A280.
Half-angle is spilled back into the fov argument slot, reloaded and spilled
into the complete tangent helper's stack argument. 412E20 uses FSINCOS,
division, float spill and reload; it is not replaced by a CRT tangent. The
builder takes the reciprocal and publishes eleven positive-zero output words
before spilling the reciprocal scale into the fov slot and reading current
far/near arguments.

The native x87 schedule computes far/(far-near), spills that ratio into the
far slot, divides the reloaded reciprocal by current aspect, and writes the
remaining matrix fields in native order. D7A24C supplies immutable float 1.
The final depth coefficient multiplies retained negative near by the current
rounded far-slot ratio. Output/argument-slot overlap can therefore affect
later reads. These scalar spills and reloads are intentional native behavior.

Both original constants are pinned from readonly .rdata and reconstructed as
exact readonly integer bit patterns. No mutable substitute, validation,
normalization, angle conversion, floating-control policy or exception rollback
is introduced. Invalid or exceptional floating inputs retain native hardware
behavior rather than being repaired.

The strict MSVC Win32 worker build, both existing CTests and all eight native
seed checks passed. The ignored fixture links the complete frozen worker
`bsp_core.lib`, extracts its exact archive member and verifies every byte and
relocation in all three linked library COFF sections, plus the fixture's
sections (171 total). The tangent's 19 bytes match the original exactly. The
complete builder instruction schedule matches after mapping original stack
slots to the explicit EDX slot pointer, relocating the two readonly constants
and helper call, and adapting RET10h to the new interface's RET.

Five paired original/source runs cover finite projection, zero denominators,
a masked signaling NaN, FSINCOS range failure, and destination overlapping
the actual callee argument slots. All 1,045 compared observable bytes match;
eleven whole-code/constant postimages remain unchanged. Compared state includes
the complete 64-byte output and 64-byte scalar/overlap area, returned pointer,
x87 control/status/tag, MXCSR/mask, XMM0..3 and active x87 registers. Full raw
FXSAVE/result records are retained and independently projected. Constant bits,
original/copy readonly PE sections and runtime readonly pages are verified.

The sealed fixture is at
`J:/PROG/battlestations-pacific-decompile-native-camera-projection/local/camera_projection_fixture/sealed.json`
(SHA256 `62e3f3590ca1994d9534da12c70bac95b246c7ed99c63b097e5843b424a7fb8e`).
The frozen full worker library has SHA256
`361341491a6fee45f62a780e8d9e90ccefa9dfdf7b6bd3358a0cf6c58a8441bf`.

All fixture floating exceptions are masked. Unmasked exception dispatch and
access-fault behavior are not runtime-tested. Private callee stack scratch,
instruction/data pointers, inactive registers and XMM4..7 are not compared.
No permanent tests, original caller ABI integration or gameplay are claimed.
Main build registration, integration and saved-analysis annotations remain
the integrator's separate work.
