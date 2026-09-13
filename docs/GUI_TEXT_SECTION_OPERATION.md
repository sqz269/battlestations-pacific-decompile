# Tracked Text section operation

`ensure_gui_text_draw_sections_00ab8530` now has a five-argument overload with one caller-owned `GuiTextSectionOperation`. It preserves AB8530's completed mesh/section/material creator references if a later actual provider throws. The frame borrows the canonical Text lifetime's existing text and shadow fields. It adds no native retain, object, scene tree or resource registry.

This packet supplies the overload and tracked auxiliary Model producer. Existing four-argument callers and the legacy buffer body remain for parent integration. The parent must keep the frame on the same lifetime before entering AB8530 and reject retirement while `has_incomplete()` is true. In particular, the old C++ Text-constructor catch that releases Shadow cannot discard this failed frame.

The owner also supplies `begin_default_type_admission` and `finish_default_type_admission`. The real factory first retains its one implementation shell in its allocation record, then publishes a borrowed constructor-dispatch pointer on the same owner. `implementation()` uses that pointer while construction runs. A make_type exception leaves that owner and its before_destroy callback registered; retirement rejects the pending borrow. Only construct_base, after receiving the same completed unique_ptr from the factory, clears the borrow. These are host retention associations, not reconstructed native fields, extra counters or successful failed-constructor admission.

## Native evidence

Read-only BSP wrappers verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. AB8530 has 208 listed instructions, zero flow gaps, inclusive body `00AB8530..00AB87CB`, length668, final one-byte RET at AB87CB. ECX is the Text; there are no stack arguments. The source interface is MSVC Win32 C++, not an original binary entry replacement.

The full body, its full assembly, all four callers, its direct callee bodies and the five cleanup funclets were read. The report enumerates all37 normal calls and five cleanup tail jumps, with numeric address/native rows. AB8530's callers are ABA8EC (content), AB9D93 (glyph child), AB98D8 (constructor), and ABB601 (copy after AB8910 and before ABB1D0). Every site supplies the same Text in ECX without pushing an argument.

| Span or step | Coverage |
| --- | --- |
| AB8553..AB86C9 shadow creation | Complete supported normal sequence; returned creator tracking and explicit factory boundaries |
| AB86CB..AB87B4 main sections and parenting | Complete supported normal sequence, fresh native field reads |
| AB8565..AB85D0 Model prefix | Actual canonical pool/ModelRecord/reference producer, native name/slot cleanup; post-construction host metadata failure retained |
| CB7F20..CB7F58 unwind actions | Full read; Model raw-slot and name actions implemented by tracked producer, effect-name actions by caller; mesh raw-slot action remains inside existing factory provider |

No Ghidra mutation, export refresh, annotation or game run was performed.

## Order and ownership

Existing Shadow+188 skips the entire creation/repair branch. Otherwise the tracked `GuiWidgetOwnerRuntime` overload allocates from the same Model pool, constructs the frame's one eight-byte temporary name as `Shadow`, runs B75030, and admits its canonical reference in the runtime's existing ModelRecord map. It transfers that one creator into live+188 at AB85AA before destroying the name header. Subsequent work reloads live+188 and clears Model+138 bits0/1.

Host record allocation, Model view preparation and reference registration are marked separately in `model_phase`/`model_failure_phase`. A native constructor exception cleans the conditional name before returning the unconstructed slot. A host registration exception after B75030 leaves the live ModelRecord in the canonical runtime and publishes `constructed_model_owner` plus the still-live temporary name in the frame. It does not return that constructed slot or destroy the live Model. No access to the Model record follows successful publication/name-release callbacks.

The temporary name uses the explicit `buffers.strings` provider. A nonnull Model environment `actual_names` must be that same provider. A null `actual_names` retains the established semantic Model-name provider domain and is labelled as such; this packet does not manufacture a native global string pool.

The returned mesh creator is recorded before model geometry assignment. AB8609 loads D7A260 once with FLD, captures current Shadow, stores17C with FST, disarms raw-mesh cleanup, then stores178 with FSTP. An inline assembly helper preserves that sequence and its signaling-NaN behavior. B75170 receives `(0,mesh,178,17C)` and has RET10h.

Both section branches record the returned section before B73C60 appends and retains it. Only then is the same temporary header constructed as `guidefault.mshd`, and the actual00535320 material factory entered. After return its material creator is recorded; the name cleanup state is disarmed before its header is destroyed. B864C0 publishes/retains the material before the caller releases section, material, then (shadow branch only) mesh creators.

Each release clears its creator cell before `release_native_render_actual_owner`, which decrements actual+04 before resolving a zero-count canonical companion. `last_release_identity` and `release_phase` therefore record a possibly interrupted terminal without restoring or releasing an already-consumed reference. These fields are diagnostic identities and must not be dereferenced after terminal completion.

The main branch reads current+4C, tests current Model+180, reloads+4C for geometry retrieval, captures that mesh and reads its live+58 count. Only count0 adds a section. Missing main geometry is not created. Current shadowed+15C selects current primary or null parent; B6E680 consumes the current Shadow. The byte is reread after parenting, and a clear value reloads Shadow before B6D890(null). No page, node or global snapshot substitutes these accesses.

## FH3 and failure limits

Handler CB7F59 loads FuncInfo DEED34. It has six unwind entries at DEED58:

| State | Previous | Action |
| --- | --- | --- |
| 0 | -1 | CB7F20: captured raw Model slot -> B748C0 |
| 1 | 0 | CB7F28: test/clear temporary bit1, header ->41DD20 |
| 2 | -1 | Same CB7F28 conditional temporary cleanup; not assigned in the observed normal body |
| 3 | -1 | CB7F41: captured raw mesh slot ->B72F70 |
| 4 | -1 | CB7F49: current effect header ->41DD20 |
| 5 | -1 | CB7F51: current effect header ->41DD20 |

States4/5 clean only the effect name on exception. There is no native cleanup action releasing a completed mesh, section, material or already-published Shadow. Their outstanding AB8530-local creators consequently remain in the failed frame. String destruction is noexcept and leaves header bytes untouched; `name_live` and `name_cleanup_armed` distinguish those stale bytes from owned storage. A failure while constructing the string has not armed its cleanup action.

`factory_in_flight` identifies an interrupted existing composed mesh/section/material provider. Its `native_site` is the caller entry site, not a fabricated inner failure instruction. In particular, create_mesh combines AB85E7 allocation, AB85FE construction and host registration; its internal raw slot is not exposed by that existing API. The recorded unwind state on entry is the last known caller state. The provider's existing host rollback is not claimed to reproduce native raw-slot cleanup.00535320 similarly does not expose its internal effect/raw-material acquisitions; renderer current48/B318B0 is still a mandatory actual dependency. This packet does not silently complete those boundaries or replace them with fake effects.

Failed frames cannot rerun, even after callers inspect or externally resolve their references. Complete frames may serve another invocation only on the same widget/text/shadow/services identities and with no pending creators/name. The frame has no automatic resource cleanup. Lifetime admission/retirement guards and eventual explicit diagnostic resolution remain the parent's integration responsibility; no full native SEH or failure-recovery continuation is claimed.

AB8910 continues to call the legacy auxiliary Model helper and retains its analogous hidden Model-prefix boundary. Its renderer/stream work is owned by the separate packet.

## Validation

Validation results and artifact hashes are recorded in `reports/gui_text_section_operation.json`. No permanent test is added. A direct actual Text interruption fixture depends on the parent's lifetime migration: the baseline constructor still enters legacy AB8530 and rolls back its failed lifetime. An unattached or fabricated Text projection would not establish this operation's same-lifetime contract, so no guard-only substitute is used. Build and static/native evidence do not establish a game-rendering result.

The final MSVC Win32 build and CTest1/1 passed. The ignored Model-prefix fixture passed against core SHA256733513ECCD16D517E8BD5845947156B01F1148A90355FF793B0885EAED12E8AA: the second actual name-allocation request throws inside B75030, then the conditional Shadow temporary is released and its raw Model slot returns to the canonical pool. No Shadow or Model creator remains; the destroyed temporary header keeps its original bytes. This validates the new prefix cleanup, not the unexecuted full AB8530 provider-failure path. The emitted argument-capture helper also preserves FLD/capture/FST/state-store/FSTP order.
