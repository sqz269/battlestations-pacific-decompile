# The actual class-model publication path

Addresses: `00879590`, `00879AA0`, `0043EBD0`, `007188A0`; audited caller `009598D0`, factory call `00965143`, handler `00C96610`.

**The installed executable does populate `class+50h` and call class slot `+20h`.** `00879590` calls the game-resource loader at `00879763` and writes its EAX result to `[EBP+50h]` at `00879768`. Its receiver is the same class passed by `00879AA0`; that caller reloads `+50h`, invokes the current class slot `+20h` at `00879ABA` when nonnull, and writes activation byte `+44h` at `00879ABC`.

The concrete vehicle route is:

```
00964790 VehicleClass_GetOrCreate
  00965143: current class slot+10h, stacked argument0
  D1ACF8+10h = 009598D0 (Destroyer class profile)
009598D0: if byte44 is zero, call00879AA0 at009598DF
00879AA0 -> 00879590 -> 007188A0 -> 00B80720
  00879768: class+50h = returned resource
00879AA0: if current50 is nonnull, invoke current class slot20
  D1ACF8+20h = 0082FE30 (established ship model-data consumer)
  then byte44 = 1
```

All five ordinary bodies were checked against disk and live Ghidra bytes. The full 112-byte `009598D0` caller is audited, including its later `+94/+98` entry loop; that loop is not implemented here. Current Ghidra also has the computed `00965143 -> 009598D0` reference. The saved 44-byte `D1ACF8` table independently establishes both concrete slots. These are source/static and controlled-fixture facts, not proof that the current game host already invokes this native class path.

This corrects the historical conclusions in `MODEL_HANDLE_PRODUCER.md` and `MODEL_REACHES_UNIT.md`. The writer lies outside the former vehicle-class scan window, and the slot20 dispatcher receives its descriptor as a parameter. A GeomMesh item at `shape+24h` is a separate, valid relationship; it does not disprove a class-owned game-resource container at `+50h`.

## Implemented source

| Entry | Bytes | Native ABI and behavior |
| --- | ---: | --- |
| `00879590` | 533 | ECX class, stacked enemy flag low byte, RET4; conditionally resolve name, load and publish resource50 |
| `00879AA0` | 36 | ECX class, stacked flag, RET4; load, optional current slot20 bind, latch44 |
| `0043EBD0` | 14 | ECX class, stacked flag, RET4; nonzero44 returns, otherwise tail-jump879AA0 |
| `007188A0` | 28 | ECX actual name header, EAX resource, RET; capture factory7175D0, get manager4C1400, invoke completeB80720 |

`native_damageable_class_model` supplies these four complete ordinary bodies (611 bytes). The new loader wrapper composes the existing actual factory, manager and cache-loader implementations. The producer calls the actual VFS resolver using the current publication and uses the same raw name pool as the resource manager. The class's model-data binding remains an explicit required implementation; an unknown class is never treated as successfully bound.

Loading is skipped when the current name length at38 is zero or the current resource at50 is nonnull. For a nonzero flag low byte, a nonnull name buffer is searched with `strstr` for the **first dot**. A nonnegative offset builds prefix + `_enemy` + extension. Only a true result from full `BDF4C0` replaces the class name, using the resolver's current candidate header. A false resolver result leaves the class name intact even if it changed the candidate. The final current C-string, or the original empty literal for a null data pointer, is copied into a temporary and passed to full7188A0. Its result is published before returning that temporary. There is no extra retain, model fallback or post-publication rollback.

## Cleanup and interface limits

Handler C96610 selects FuncInfo DC87D8 and its ten-row map DC87FC. Six existing eight-byte funclets at C965E0/E8/F0/F8 and C96600/08 return suffix, extension, prefix, joined name, candidate and load name through41DD20. States0..3 own the first four temporaries in reverse order. State4 describes candidate plus those four but is not installed by this caller. States5/6/7/8 own candidate plus respectively prefix/extension/suffix, extension/suffix, suffix, or nothing else. State9 owns only the load-name temporary. Normal cleanup advances the state before each return; source unwind does the same and terminates if a second cleanup exception occurs.

The immovable acquired frame owns only temporary headers and the existing nested resolver/loader invocation frames. It is single-use. Their existing rule to retain a failed nested resolution frame still applies. Substring, concatenation and assignment reuse their existing `NativeStringStorage` release boundary: lazy getter failure inside a noexcept release is not certified as native FH3 behavior. The source does not implement original exception objects, hardware-fault recovery, private stack aliases or an ABI bridge. Names are descriptive hypotheses.

## Verification

The strict MSVC Win32 build and both existing CTests pass. One ignored fixture executes five source/original pairs through the full guarded activation caller: plain cache-hit load, failed enemy-name resolution, an already populated model, an empty name, and an already activated class. All normalized class images match. Native wrapper and caller bytes run with full canonical string, factory/manager getter, raw cache and VFS resolver services. Their shared source dependencies are explicit; the fixture is not an independent oracle for those dependencies.

Two source-only failures verify that a real loader metric exception returns the owned load-name block without publication, while a class-binding exception preserves the published resource reference and leaves byte44 zero. The fixture's class binding is a complete small fixture-class method, not the ship's unreconstructed full slot20 body. Cache hits use an actual constructed game resource and real cache insertion/lookup/retention. Successful enemy-file lookup, disk/resource parsing misses, lazy singleton creation, native FH3 and gameplay are not dynamically proved. Empty-resource dismantling at fixture shutdown is explicitly fixture cleanup; the peer's `35a49082` resource-container terminal remains an unmerged dependency.

The initial enemy fixture lacked the extension-tree and group-list sentinels required by full VFS resolution. Supplying those actual empty structures fixed the fixture without production changes. `F878E0` is a loader-zeroed byte beyond its PE section's raw data; it is compared as initialized-image data, not claimed as an on-disk byte.

Evidence: `reports/native_damageable_class_model_be.json` and immutable inputs under ignored `local/damageable_class_model_be/`. Actual host class admission, the full class model-data binder, vehicle entry activation and gameplay remain open.
