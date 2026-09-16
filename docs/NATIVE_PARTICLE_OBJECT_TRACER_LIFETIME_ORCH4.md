# Native Object and Tracer particle type lifetime

Packet `orch4_particle_object_tracer_k11`, based on `9f2483c2e`. These are complete source bodies over actual Win32 storage. Names are descriptive hypotheses. Original register ABI, hardware SEH and game execution are not reproduced by the new C++ interfaces.

| Address | Bytes | Original ABI | Reconstructed behavior |
| --- | ---: | --- | --- |
| `00869B10` | 26 | ECX pointer cell, RET | Free captured nonnull pointer, then clear the current cell. |
| `00AF8350` | 95 | ECX vector, stack signed capacity, RET4 | Existing reserve algorithm with a genuine fixed-CRT overload. |
| `00AF83B0` | 80 | ECX vector, stack signed count, RET4 | Reserve if needed; zero newly exposed cells; reduce current count without releasing pointees. |
| `00AF8940` | 113 | ECX Object definition, RET | Existing reverse model release algorithm with a callable-native-vtable overload. |
| `00AF89C0` | 23 | ECX vector, RET | Resize0, free current backing, retain stale pointer and capacity. |
| `00AF8A40` | 182 | ECX Object definition, RET | Two parameter slots, current model vector, base destruction. |
| `00AF8BB0` | 30 | ECX Object, stack flags, EAX original Object, RET4 | Destroy, then free iff flags bit0. |
| `00B0A4E0` | 555 | ECX Tracer definition, RET | Fifteen ordered parameter slots, current buffer, base destruction. |
| `00B0A820` | 30 | ECX Tracer, stack flags, EAX original Tracer, RET4 | Destroy, then free iff flags bit0. |

## Storage, reloads and ownership

Object publishes `D5DB00`, returns the individually captured parameter slots at `+80/+84`, and clears each only after the return completes. The actual model vector is `{pointer,count,capacity}` at `+8C/+90/+94`. Model release captures the last cell and its pointee, decrements the pointee's actual `+04`, and calls its current virtual0 only on zero. It clears the captured cell after the call, then reloads count before decrementing and before the next iteration. A callback can replace the current vector pointer or count; cleanup uses the resulting current vector. Null model cells skip the clear store.

Tracer publishes `D5E048` and returns slots in this exact order: `B4,B8,BC,C0,C4,C8,CC,D0,D4,D8,DC,E0,E4,8C,94`. It then frees the captured current `+98` buffer if nonnull and clears that cell. It does not clear unrelated fields. Both scalar wrappers retain the incoming object address for EAX, even after free, and do not free if destruction throws.

All parameter work calls the parent's genuine `AFFDF0/B00090` providers, using the same actual `NativeWeakPoolStorage` companion for `F8D344`. Base destruction calls genuine `B00FB0` with the same raw string publication context. No host `NativeStringStorage`, replacement owner, or injected destructor/allocator callback is introduced. The two pre-existing resource APIs retain their host binding interfaces and share the original algorithms with the new overloads.

The Object model terminal remains an explicit callable Win32 vtable boundary. Numeric image model profiles are not callable source code. The concrete next binding is `NativeRenderActualOwners::resolve_actual(raw)` and the canonical `RenderCommandReference`/`NativeModelOwner` destruction domain in `native_render_context.hpp` and `native_model_owner.hpp`. This packet does not create another renderer owner or bind those profiles implicitly.

## Exception states and repaired evidence

Object FH3 `DF2CD8` has map `DF2CC8`: state1 -> state0 via `CBAE38`/`AF89C0` on `object+8C`; state0 -> -1 via `CBAE30`/`B00FB0`. Normal execution sets state0 before vector cleanup and -1 before base cleanup. Tracer FH3 `DF3C08` has map `DF3BF8`: state1 -> state0 via `CBBA28`/`869B10` on `object+98`; state0 -> -1 via `CBBA20`/`B00FB0`. Its normal inline buffer cleanup retains state1 until the base call. Source guards are `noexcept`: a second exception during true unwind terminates, while a first exception propagates after the required cleanup.

The parent repaired the complete Object tail `AF8AD3..AF8AF5`, Tracer gap `B0A6E1..B0A6E9`, scalar ADD ESP4 gaps `AF8BC5..AF8BC7` and `B0A835..B0A837`, vector tail `AF89D2..AF89D6`, and pointer leaf gap `869B1F..869B27`. This worker performed no live Ghidra mutation. Full live/PE spans, SHA-256 hashes, bytes, exact call sites and prior function annotations are in [the report](../reports/native_particle_object_tracer_lifetime_orch4.json). The existing `AF8940` alignment gap after an unconditional jump is not a missing fallthrough.

The 22 captured spans cover all nine bodies, both unwind maps/metadata, all six unwind/handler stubs, two profile prefixes and three allocator/free thunks. `verify_report_calls.py` validates 57 direct call/tail rows with zero failures; two indirect calls are explicitly excluded from direct-target checking. The two compiler FH3 handler jumps have no Ghidra function owner and are recorded separately with their complete live/disk byte evidence.

## Validation

The genuine common provider prerequisite `0fb04a9e8` was fast-forwarded before validation. `scripts/build.ps1` passed Release Win32 `/W4 /WX /fp:strict` with all three CTests; all eight native seeds matched live/disk bytes. No permanent tests were added.

`local/object_tracer_probe.exe`, linked with `/MANIFEST:EMBED` against this exact worktree's new `bsp_core.lib`, passed. It executes copied complete Object182B, Tracer555B, model-clear113B and resize80B normal bodies. Forty-one direct call sites link copied internal bodies or genuine source pool/base/CRT providers; the copied clear uses the actual Kernel32 `InterlockedDecrement` import. Comparisons cover full owner images (only differing allocation addresses masked), all fifteen Tracer pool return indices, captured model cell clearing after a callback replaces the current vector/count, and preserved dangling pointer/capacity. The source-only throwing-model check verifies vector/base unwind; source reserve/grow/shrink uses fixed CRT allocation. Probe source and evidence remain temporary under `local/`.

The original FH3 exception path and scalar flags variants were not executed by this probe. Their full byte spans, call sites and source flow were reviewed and build-tested. This does not establish gameplay, arbitrary numeric model-vtable compatibility, or a drop-in register-ABI replacement.
