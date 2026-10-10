# Native persistent sound-state writer review

No new original writer of zero or supported input selecting NOSOUND before the
sound constructor is established. The demonstrated persistent-byte writer is
still `008D7736`: `MOV byte[ESI+24h],1`. The admitted CRT initializer `00CD2D80`
passes the actual static object `00F88980` to that constructor. Its byte `+24h`
is `00F889A4`, the cell read at `0073DAEE`.

| Verified direct query | Returned evidence |
|---|---|
| Config base F88980, limit 80 | 58 DATA references |
| Sound byte F889A4, limit 20 | Only 73DAEE READ |
| Constructor 8D7710 callers | CRT CD2D80 and options constructor 5F6030 |
| CRT CD2D80 references | CE3054 initializer DATA reference |
| Loader 8D8190 callers | Application initializer 73D410 |

Every live typed query verifies the `bsp` project and
`/battlestationspacific.exe`, x86 Win32/image base `00400000`, against the frozen
configuration. `C:/Users/sqz269/bsp.gpr` is the configured project file; the bridge
does not independently expose its full live file path. No GPR file/body/listing/
prototype/flow mutation or inline bridge was used. Direct references do not
enumerate indirect or external writes, and a base DATA reference alone proves
neither a store nor copy direction.

The accepted startup windows were frozen before releasing `73D410` for Atlas:
the actual-global loader call at `73DAA5` precedes the byte read at `73DAEE` and
sound call at `73DAFD`. Constructor 446-byte and CRT 22-byte physical hashes
match the admitted evidence. The complete original PE identity and scoped
windows are retained for offline replay; no extra bodies are analyzed from the
image. The admitted CRT ordering supplies value one before the startup read,
not a zero selector. This packet does not repeat the full CRT/startup CFG audit.

The [accepted NOSOUND receipt](CC12_NATIVE_NOSOUND_SELECTION_READINESS.md)
remains unchanged: `SoundEnabled` consumes its name without reading/storing a
numeric value. A following `0` or `1` becomes an unknown token. The genuine
NOSOUND branch requires zero in the actual byte; adding a Source/CLI selector or
forcing a constructor argument would be an intentional change.

All 31 selected Source paths from that receipt remain identical after EOL
normalization. Forty complete current Source/Git paths are frozen. The raw
owner explicitly stores `+24=1`; the raw loader preserves the ignored-token
behavior; the compatibility read view reads this byte. Current profile archive
Source loads audio fields `20/28/2C/30`, with no `enabled_24` store. It is a
semantic `GameSettingsBlock` projection, so this is not a new full raw Native
profile-write/ABI audit. The admitted raw profile importer affects controls
`40..43` and game difficulty `6AC`, not this sound byte.

Current options Source and the accepted menu document call `005F65C0`
`reload_from_settings` and describe settings-to-screen reload. It is not an
established persistent writer. Its saved body has 267 pseudocode lines and spans
`005F65C0..005F6EE0` (2,337 bytes). The bounded constructor view calls `8D7710`
twice but hides the ECX receivers. No register/copy recovery was attempted.
The audio UI exposes volume rows; runtime audio application passes the existing
enabled value to the sound host and is not a persistent-byte setter.

The next Astra boundary is the exact receiver/copy direction, covered fields,
value provenance and ownership in `005F65C0`, especially base references at
`65EB/6619/6624/6E53/6EB7`, with the related constructor `005F6030`. Update/list
event callers provide runtime context but do not prove every route occurs after
`73DAEE` or successful startup. `008D6DC0`'s full raw write path and further base
users remain separate frontiers. No zero writer is demonstrated either before
startup or only afterward, and exhaustive absence is not claimed.

The [complete receipt](../reports/cc12_native_sound_state_writer_review.json)
pins commands, query times, Source/Git preimages, bounded saved windows, original
PE identity and pure offline replay. This packet adds no Source implementation,
compiler/tests, SDK calls, game startup, ABI/gameplay proof or environment change.
