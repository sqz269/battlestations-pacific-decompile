# Post-sound startup continuation review

Recommend one bounded Source packet: `cc12_particle_manager_raw_lifetime_access`,
covering the existing particle/foliage-manager base constructor `00AF06A0` and
destructor `00AF0740`. Their current access contract requires the semantic
`SingletonLifetimeDomain`; the application now shares the actual `01090AA0`
publication cell. Existing borrowed raw/legacy adapters can bridge this restriction
without constructing another manager. This is a readiness proposal for Root review,
with no C++ or startup-activation authorization.

The review starts at main `ea3c9fc17f79f93b8c842a08e41ef531b6eea0fc` and accepted
Source3348. The accepted runtime still stops in FMOD before window/device creation.
Every later step below is established by inspected Source or retained Native
evidence, not new runtime coverage.

| Source continuation after successful core sound | Present boundary |
| --- | --- |
| Dialog startup, VFS parser registration, alternate-owner gain stores | Separate operations can still throw. |
| Settings-derived window request and `configure_platform_window_00becee0` | The helper returns renderer slots; the host performs device/cache work afterward. |
| `GameDeviceHost::create` and actual renderer cache initialization | Conditional on a created window and requested renderer; full device/default-surface guards remain. |
| Power policy, online manager, render resources | Explicit markers; restore/lifetime, parent IPC and post-effect/material prerequisites remain. |
| Input backend, locale, fonts and `0073BAE0` GUI entry | Existing Source composition; omitted Native stages are not implied complete. |
| GUI manager/pages and title entry | GUI getter uses projected state; page/widget scene nodes remain diagnostic IDs, followed by sprite/text bridge drawing. |

The Source phase labels are not the Native schedule. Existing application listing
and retained original-image bytes place the `0x34` manager allocation at
`0073E005/0073E00E`, null branch at `0073E027`, receiver move at `0073E029`, and
`00AF0B10` call at `0073E02B`, **before** locale construction `0073E059` and
fonts/GUI `0073E13C`. The Source's later `Phase 8 world_effects_startup` marker is
therefore not an approved activation location.

Own exactly these two C++ files in a future approved packet:

- `include/bsp/native_particle_model_manager.hpp`
- `src/native_particle_model_manager.cpp`

Keep the publication and constant references. Replace the access object's lifetime
field with the existing borrowed `SoundLifetimeAccess`, using this constructor shape:

```cpp
NativeParticleModelManagerAccess(
    NativeParticleModelManagerStorage* volatile& publication,
    SingletonLifetimeDomain& legacy_domain,
    const volatile std::uint32_t& one) noexcept;
NativeParticleModelManagerAccess(
    NativeParticleModelManagerStorage* volatile& publication,
    void* volatile& actual_manager_01090aa0,
    const volatile std::uint32_t& one) noexcept;
```

Existing `{publication, legacy_domain, one}` Source construction remains supported.
The raw overload only borrows cells; it performs no allocation/getter/publication.
The access-context representation may change; the actual `0x34` manager layout and
its existing weak arrays, derived constructors/destructors and scalar frees stay
unchanged. No CMake change or helper rewrite is required.

Both bases must preserve the first getter's captured section, a distinct second
getter, then evaluation of current `F8C274`. Use the existing captured-section
helper and saved manager view, whose raw branch reaches actual manager `+10`,
tracked depth `+18`, `BD0C30` and `BCFCA0`. Construction publishes before the second
getter; destruction clears only after unregister returns. Keep the captured first
section through cleanup and preserve existing Source failure/profile behavior.
Add no substitute registry, null shortcut, rollback, retry or failure-time clear.

This pair is independently ready because its complete Native bodies survive in
existing evidence and both current source files exactly match their prior reviewed
hashes. The implementation exists in a complete 14,073-byte Core archive member,
but is outside selected Source3348/Core1057. Its constructor/base names are absent
from the accepted PE map; name absence alone is not a universal code-identity proof.
All 4,207 tracked source/header paths were retained and searched: the access and
constructor/scalar-delete names occur only in the manager's own header/source.

Activation remains gated on a canonical `F8C274` cell and context lifetime,
`D5D7EC/D5D7F8` deletion admission, and the actual caller allocation/failure schedule.
Current raw deletion bindings do not admit this manager. Render-resource activation
also remains separately gated by the `B107F0` producer for constructor-unwritten
`+70`; shader preload still requires its material/compiler caller context. The
projected GUI manager and fake scene-node IDs are additional frontiers, not substitutes.

The separate Git check passed for 4,229 complete blobs. Portable ZIP-only replay
passed for 3,352 accepted pins, all retained source searches, the 624-file/1,257-edge
quoted-include closure, 23 existing Native spans totaling 1,462 bytes, and ten
application call slots. CMake membership is traced through the normal build script's
`CMAKE_PROJECT_INCLUDE`; the lexical include closure is not relabeled as compiler
input. Seven accepted Source/Git EOL differences and one report worktree/Git EOL
difference preserve both complete byte versions.

Replay with `python portable_replay.py evidence.zip` after copying those files
from `local/cc12_post_sound_startup_continuation_review/`. No new executable run,
Ghidra analysis/export query, C++ edit, build, test, fixture, SDK/audio/OS probe,
FMOD bypass or simulated success occurred. Raw Source admission would not establish
original register ABI, private FH3, hardware-fault or gameplay equivalence.

[Complete receipt](../reports/cc12_post_sound_startup_continuation_review.json).
