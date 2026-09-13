# Native material-effect loading

Addresses: `00B2EBB0`, `00B18F70`. Descriptive names are hypotheses, not recovered symbols.

`B2EBB0` now has an actual-storage C++ sequence using the existing native string pool, descriptor/program operation and renderer effect cache. It copies both native locals, replaces every case-sensitive `.mshd` substring with `.shfx`, and calls the required mutable-name resolver against the current actual VFS manager. A found name allocates `178h`, runs `B407A0`, publishes the creator before `B46950`, ignores the loader's AL, and assigns the original name at effect `+B8`. `B18F70` preserves its identity skip, live source/destination reloads, resize and copy behavior.

The missing-name arm constructs and emits the original diagnostic, then calls the current renderer's `48h` slot with `error.shfx` through the concrete `B318B0` route. That return is an acquired cache reference; it is not assumed to be a fresh effect with count one. Native recursive fallback behavior has no added sentinel or successful-null substitute.

The native ABI is unused registry ECX, name and ignored word on the stack, owned EAX and RET8 for `B2EBB0`; effect ECX, name on the stack and RET4 for `B18F70`. These C++ interfaces do not replace the binary ABI or native FH3 encoding. CRT allocation exceptions and valid nonnull allocation results are the supported domain; the original null-allocation path subsequently dereferences null and is not converted to success.

Every invocation has a persistent acquisition frame. The completed creator remains visible before any later throwing operation. The exact resolved eight-byte header resides in that frame. A program-child host failure retains its name allocation so the child cannot retain a dangling argument; this intentionally differs from native local unwinding and does not claim exception parity. The failed frame cannot be discarded or replayed. On success the native reverse name cleanup still occurs.

After complete pass population, the program operation transfers into the same canonical effect record. Its descriptor callable bindings remain alive until actual descriptor deletion. The canonical companion borrows the effect's actual `+04` count. No extra native retain, private cache, secondary reference count, or semantic effect object is introduced. A host registration failure preserves the raw creator, stable metadata record and any constructed companion; it does not roll back partially published native state.

Validation evidence is in `reports/native_material_effect_loading.json`: 860 bytes across the two full spans match the installed PE, and all 36 direct calls are checked against current Ghidra bodies. Both indirect calls are separately identified. Combined build and focused integration results are recorded in `reports/native_material_effect_integration.json` after integration.

The cold path still requires actual `BDF4C0` VFS resolution, constructor renderer `64h` texture loading (`B319B0`/`B30B40`), `B43B00` descriptor reading, the `B3C3A0` compiler, and actual state caches. The existing constructor requires a callable renderer binding; an original numeric vtable is not executable host code. This batch does not establish a complete renderer binding or a successful cold effect/Text render. Existing Text/material callers retain their documented renderer binding frontier pending those dependencies.

Follow-up packets: reconstruct the actual texture-cache/loader path and renderer binding; wire the actual descriptor reader and compiler children; complete actual mutable VFS candidate resolution; then migrate material/Text callers with persistent inner cache acquisitions and validate against installed assets.
