# Bright-pass deleting destructor

`00B101A0..00B101BD` is the complete 30-byte deleting wrapper in slot +4 of
profile `00D5E1B4`. The name is descriptive, not a recovered symbol. The
`B107F0` producer pushes `224h` at `B10F6B`, allocates at `B10F70`, initializes
the count and two child pointers, stamps this profile at `B10F91`, publishes
the owner at service +24, and invokes the established bright-pass initializer
`B54940` at `B10FAE`. The profile's first two words are `BD30E0, B101A0`.

The wrapper preserves the original ECX owner in ESI, calls the existing
`B0F5E0` base destructor at `B101A3`, then tests bit zero of the stacked flags
byte. It calls CRT free at `B101B0` only on that bit, restores the stack and
returns the original pointer in EAX with `RET4`. A destructor exception does
not reach the free. Returning the pointer does not extend freed storage's
lifetime. The source reuses `NativeRenderEffectLifetimeContext`, its canonical
post-effect companions and holder cleanup, and the established allocation
boundary; it introduces no reference count or owner registry.

Ghidra had a `CALL_RETURN` override at `B101B0`, omitting the three-byte
`ADD ESP,4` at `B101B5` and producing a false return value after free. The
primary repaired only that call-site override under the shared write lock,
saved the program and retained before/after evidence in
`reports/cc10_bright_pass_flow_repair.json`. The callee's no-return flag was
not changed. The full live 30-byte body matches the installed PE.

The new C++ interface adds an explicit context and is not a drop-in native
ABI replacement. It relies on the existing base's admitted child profiles
and lifetime domain. This wrapper does not register the new pass, complete
`B107F0`, admit resource-graph destruction, or prove native FH3/SEH or gameplay.
Validation is recorded in `reports/native_bright_pass_lifetime_cc10.json`.
