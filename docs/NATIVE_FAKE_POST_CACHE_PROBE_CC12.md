# Opt-in fakemotion preload and warm-hit probe

`GameNativeGuiEffectProbe::run(bool include_fakemotion = false)` adds a bounded acquisition check using the existing effect application and canonical shader-cache bracket. The default remains the two genuine font acquisitions. This packet builds the probe and preserves its Source/object evidence; it does not claim a successful fakemotion runtime acquisition or POST draw.

The Original bracket and ownership contract is in [NATIVE_POST_CACHE_ORDER_CC12.md](NATIVE_POST_CACHE_ORDER_CC12.md). In particular, `0073C1BE` closes the binary cache before the later `+664` producer, while preload effect credits survive in the application vector. The probe follows that close-before-warm-use relationship. It does not replace native Lua preload iteration, the production application vector, or its shutdown schedule.

## Scope and normal order

The two existing `resources` slots remain `GuiFont.mshd` and `GuiFontBilinear.mshd`. Two additional persistent slots hold the fakemotion cold and warm credits. Only the cold slot constructs a native pooled name; the warm call borrows the cold slot's **exact actual8h header address**. Both slots and their pointers live in the retained heap `Impl`.

| Step | Opt-in behavior and recorded evidence |
| --- | --- |
| Cache creation | Existing renderer operation only, one-shot, existing compiler source mode 0. Record actual cache identity and both mode bytes; do not write them. |
| Existing fonts | Same two guarded acquisitions in the same order. Record operation, owner, cache publication and cursor before/after each call. |
| Cold fakemotion | Operation index 2, real `fakemotionblur.mshd`, same `GameNativeGuiEffectApplication::acquire_effect_00b318b0`. Require a completed registered genuine cold creator, registry publication and exact caller credit, with no fallback call. |
| Forward consumption | Require unchanged canonical cache/mode domains and record count, with a strictly increased cursor within the same owner. These endpoints are not a complete lookup trace. |
| Actual sampler inputs | Opt-in read-only hook records each genuine compiler invocation's builder/pass/root/mode identities and the current root/mode shape, as described below. |
| Normal cache close | Keep the cold credit in its slot across the existing `release_shader_cache()`. Require the current publication null and the actual effect count unchanged. No second create or lifetime extension. |
| Warm fakemotion | Operation index 3, same application and preserved cold operation, distinct stable warm frame, same actual input header and effect pointer. Require `00B31436`, one added actual caller credit, no key-resolution frame, no cold loader, no registry insertion and no cache reopening. |
| Explicit normal teardown | Retire warm, cold, bilinear and font credits in reverse order through existing `00B32010`/canonical owners, verifying one actual count decrement each. Destroy the single fake name once, then the two existing names. Parent graphs remain until the ordinary shared native drain. |

The probe logs actual identities and captured counters; diagnostic identities never become extra owners. After cache close it does not dereference the saved cache pointer. After terminal reference retirement it checks companion presence before any further raw-owner inspection. Exceptions retain the entered frames, names, credits and parent graph; there is no cleanup/retry fallback or graph reconstruction.

## Actual Source0 admission and the observation hook

The installed physical `postprocess/fakemotionblur.shfx` declares one pixel `SceneTexture` sampler, type 2, index 0, and no `TextureSource` key. Its mode 0 combiner is `dummy.shfx`, whose physical file has no sampler table. Original `00B57B77` clears EBX and `00B57B8F` initializes the sampler's `+14` source word to zero. The current reader only replaces it when the actual Lua `TextureSource` key is an integer. The receipt preserves 46 bytes matched between live Ghidra and the installed PE, plus those current physical descriptors and relevant Source inputs.

Physical files alone do not establish the selected VFS/override result. For the opt-in path, the probe uses the **existing** `GameNativeGuiSamplerApplicationBindings::evidence` hook solely for observation and bounded shape checks. The callback runs in the existing compiler `final_states` host scope before the Original root/mode sampler call sites `00B3C018` and `00B3C024`. It records those following sites and actual builder, pass, root descriptor, mode descriptor and sampler pointers for each compiler invocation. These are host callback positions, not new native entry addresses.

Every observed fake root must have exactly one pixel `SceneTexture`, source 0, type 2, index 0, and only mode 0 named `dummy.shfx`. The **actual mode descriptor** must have zero samplers. The two persistent observation slots cover the existing operation's at-most-two variant loads; each accepted root has only one nonempty mode. The final retained root descriptor is checked again after cold acquisition. At least one actual compiler sampler invocation and actual binary-cache cursor advancement are required for this proof.

The callback returns an entirely empty `GameNativeGuiSamplerCallEvidence`: no entry word, release scratch, capture or provider override. The existing actual cached predecessor capture and its qualification guard remain active, and still publish/consume the real per-invocation word. The default font-only path installs no hook. In the opt-in path, font calls return empty evidence without changing their behavior; only calls while the fakemotion cold stage is active are inspected.

If an actual selected root or mode requires Source1, the bounded shape guard preserves the failure before those sampler calls. The Source1 release/provider work then remains a named dependency for that different reached case. This packet does not toggle compiler source mode, replace a sampler provider, or treat physical descriptors as runtime selection proof.

## Integration and validation

Root owns the separate `game_hosts.cpp` opt-in wiring (`BSP_NATIVE_POST_CACHE_PROBE=1`) and slot-aware actual launch. This packet changes only the probe implementation/header and this new doc/report. Existing no-argument calls still compile to the font-only behavior. No CMake, shared startup report, loader, compiler, cache, owner implementation or private accessor changed.

The final `scripts/build.ps1` Win32 Release build passed with warnings treated as errors, along with the three existing checks (`reconstructed_math`, `native_math_differential`, `tool_tests`). No new tests or synthetic GPU fixture were added. The first build exposed a local variable shadow warning; that failed attempt and the intermediate passing builds remain separate evidence. The final build includes the actual-input hook, explicit inspected/uninspected failure diagnostics and its object copy.

`reports/native_fake_post_cache_probe_cc12.json` pins source preimages, final sources, the actual compiled probe object, build command/log/results, relevant current providers, physical descriptors and the 46 native bytes. Preparation verified the preceding contract receipt's 118 artifacts and two committed snapshots. None of those sealed files changed.

Runtime status is **pending a coordinated device-ready launch**. No worker GPU process was launched. The required terminal evidence remains: genuine cold fake result, all accepted actual root/mode observations, normal cache close with retained credit, same-owner `00B31436` warm hit, exact two-credit retirement, shared graph closure and process exit. Build/static evidence does not establish those events, native FH3 behavior, Original ABI replacement, a `+664` construction, material draw or full scene.
