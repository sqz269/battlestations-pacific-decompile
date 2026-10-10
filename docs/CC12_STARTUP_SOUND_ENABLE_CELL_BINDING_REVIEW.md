# Startup sound-enable cell binding review

At accepted Source epoch `0ebcc4745339534a70d87b20f4324b631b43ceba`, the ordinary
`GameStartupHost::run_initialize_phases` argument has the correct cell provenance
and polarity. A completed raw settings load is followed by a read-view refresh;
that refresh reads exactly one byte at the retained raw owner's `+24h` and
normalizes it with `!= 0`. The later startup argument negates that bool:

```text
b = raw settings byte +24h, sampled during copy_read_view
settings_view_.audio.enabled_24 = (b != 0)
sound_->core.startup(!(b != 0))  =>  disabled = (b == 0)
```

This is a normalized snapshot, not a live alias or a byte-for-byte copy of a
possibly noncanonical byte value. Its zero/nonzero meaning matches the admitted
Native caller's `SETZ DL`. No bounded Source mismatch was found, and no C++ fix
or activation change is proposed.

## Actual Source owner and storage

`GameNativeSettingsProcess::Impl` owns `NativeGameSettingsStorage settings{}`
(`src/game_native_settings_process.cpp:36`). The process accessor allocates its
wrapper once and rejects a different retained data mapping; `settings()` checks
the ready phase and returns that same `impl_->settings` member. The raw type is
standard-layout, four-byte aligned, and `BCh` bytes, with `opaque_00` at offset
zero. Thus the one-byte projection read addresses the object's actual byte
`+24h`, without reading adjacent padding or a separate options-file bool.

Initialization calls `initialize_native_game_settings_static_00cd2d80` on this
member. That Source wrapper calls `construct_native_game_settings_008d7710(&p)`;
the constructor writes `byte(p,0x24,1)`. `GameStartupHost` retains the process
pointer and makes an initial read-view copy after initialization at lines
`1470..1472`. Startup later refreshes it again after loading.

`F88980`/`F889A4` are the admitted original owner/field identities represented by
this reconstructed storage. The current Source allocates its process/Impl
objects; it does not prove that this C++ member physically resides at virtual
address `00F88980` or that the original installed process was observed. The
verified relationship here is the exact retained Source owner and its offset.

## Load, refresh, and consumption

| Stage | Current Source evidence |
|---|---|
| Load the retained owner | `GameNativeSettingsApplication::load`, line 123: `load_native_game_settings_008d8190(&p.process.settings(),p.loader,p.operation)`. The loader saves that same pointer as `op.owner` at line 154. |
| Admit successful completion | The application reaches `Phase::ready` only after the loader returns and language rows are copied. A thrown operation sets `Phase::failed` and propagates. |
| Require ready before copying | `copy_read_view`, lines `137..139`, rejects any non-ready phase and passes `impl_->process.settings()` to `copy_native_game_settings_read_view`. |
| Read the actual byte | `src/game_native_settings_application.cpp:21` implements `flag` as a one-byte `memcpy` read followed by `!=0`; line 57 assigns `a.enabled_24=flag(p,0x24)`, with `p=&storage`. |
| Refresh before startup | `src/game_hosts.cpp:1924` calls `settings_host.load()`; line 1925 calls `settings_host.copy_read_view(settings_view_)`; line 2002 calls `sound_->core.startup(!settings_view_.audio.enabled_24)`. |
| Preserve disabled argument | `src/game_sound_runtime.cpp:316` forwards `disabled,0,0` into `construct_sound_system_00a88770`; it does not replace or invert the argument. |
| Set the sound flag | `src/sound_startup.cpp:33` sets `owner.system.sound_enabled = !sound_disabled`. |
| Select output | `src/audio_online_startup.cpp:236` selects `FmodOutputType::nosound` only when `state.sound_enabled` is false. |

The complete intervening host Source is frozen. Its explicit settings override
changes projected width and height through `settings_view_.options_file`; it
does not assign the audio-enable field. The interval also handles frame-clock
binding, summaries/logging, and `SoundServices` construction. That constructor
retains the same settings process for its online publication/SDK binding and
constructs the sound runtime; it does not refresh or replace this audio bool.

The snapshot is proved equal to the raw byte's nonzero predicate **when copied**.
The inspected normal Source sequence has no intervening direct audio-field
assignment. This finite review does not prove freshness against arbitrary
aliased, delegated, concurrent, or external raw-cell mutation. It establishes
Source statement ordering, not original startup reachability or live execution.

## Accepted Native meaning and limits

The existing accepted NOSOUND receipt establishes the original sequence:
`0073DAEE` compares `F889A4` with zero; `SETZ DL` supplies the disabled byte to
`00A88770`; the constructor negates it into sound `+70h`. Only the first stack
byte is consumed, so the Source bool normalization preserves the relevant
truth value without claiming preservation of the Native upper EDX preimage.

| Raw byte at the sampling point | Source disabled argument | Sound enabled | Output branch |
|---|---|---|---|
| `0` | `true` | `false` | NOSOUND |
| Any nonzero byte | `false` | `true` | Ordinary enabled path |

The Source runtime calls the full owner constructor and then its library stage.
The separate legacy `start_audio` helper has matching flag polarity but is not
the call selected by `GameSoundRuntime::startup` in this chain. No drop-in ABI,
raw argument-register identity, FMOD success, or gameplay claim follows.

R167 admits the complete normal raw loader; R170 admits its ordinary retained
owner/application binding. R167's older statement that ordinary startup still
uses a projected loader is historical and superseded by R170 and the current
calls above. The current raw loader preserves `SoundEnabled` as a consumed
keyword with no value read/store (`native_settings_loader.cpp:77`). This review
adds no supported original zero selector or newly demonstrated zero writer.
Historical Native/build/runtime results in the admitted reports were not rerun.

## Frozen scope and validation

The [receipt](../reports/cc12_startup_sound_enable_cell_binding_review.json)
retains 27 complete Source/Git inputs at the stated epoch, 30 exact excerpts,
six captured queries, and offline replay. All current working inputs match their
Git baseline after EOL normalization. The stats worker's in-progress branch was
not inspected; later edits require their own revalidation.

```powershell
python local/cc12_startup_sound_enable_cell_binding_review/replay.py
```

Replay verifies retained-file hashes, Git blob identities, excerpts, and finite
Source anchors/order. An audit hook confines reads to that evidence directory
and forbids writes. There were no Native body/bytes/live-address queries,
installed-executable reads, C++ edits, compiler/new tests, SDK/game/OS changes,
or Ghidra mutations. Source provenance is established at this epoch; original
selector support and live startup/gameplay validation remain unestablished.
