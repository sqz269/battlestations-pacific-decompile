# Raw settings loader and renderer choices (R167)

Addresses: `008D8190`, `00467CC0`, `00B1FFF0`, `00B20000`, `00B200B0`,
`00B200C0`, `00B295C0`.

## Result

The complete normal `008D8190` loader now composes the previously recovered
language catalog, pooled path, text writer, raw choice vectors, retained memory
owners and scene tokenizer over the actual BCh settings storage. Its renderer
helpers use the actual renderer vectors and current Direct3D9 COM object.
The ordinary application still calls the projected settings loader; canonical
startup ownership and application admission remain a separate follow-up.

| Entry | Native contract | Source |
| --- | --- | --- |
| `008D8190` (1833 bytes) | ECX settings, no stack arguments, RET | `load_native_game_settings_008d8190` |
| `00467CC0` (42 bytes) | ECX tokenizer, keyword on stack, AL boolean, RET4 | `match_native_scene_keyword_00467cc0` |
| `00B1FFF0` (4 bytes) | ECX renderer, EAX renderer+1Ch, RET | `native_renderer_resolution_header_00b1fff0` |
| `00B20000` (4 bytes) | ECX renderer, EAX renderer+28h, RET | `native_renderer_antialias_header_00b20000` |
| `00B200B0` (7 bytes) | ECX renderer, EAX DWORD at +1B48h, RET | `native_renderer_shader_ceiling_00b200b0` |
| `00B200C0` (3 bytes) | RET4 only; model selection is a verified no-op | `select_native_renderer_shader_00b200c0` |
| `00B295C0` (172 bytes) | ECX renderer, format on stack, RET4 | `rebuild_native_renderer_antialias_00b295c0` |

Names describe observed behavior, not recovered symbols. Existing descriptive
names are preserved; `00467CC0` gains `BSP_SceneTokenizer_MatchAndConsumeKeyword`.

## Recovered schedule

The loader first builds the language catalog, copies the renderer's resolution
header into F8895C and takes its shader ceiling. It builds the raw options path
and opens it in `rt` mode. The file branch seeks/tells/rewinds, constructs the
10h backing and retained 14h memory stream, reads the captured length, closes
the FILE, then constructs the actual 838h tokenizer. Read and close results are
ignored. The first five keywords use explicit peek/compare/consume calls;
the remaining eleven use 467CC0. Parsing success bytes are ignored.

Consequential details preserved from the listing:

- `HiResShadow` and `NoLOD` write bytes +1Dh/+1Ch. All boolean options write
  one byte, preserving adjacent padding; `Shadow` writes both +84h and +85h.
- An unsupported parsed resolution becomes 640x480. The subsequent scan keeps
  the **last** matching resolution index. The missing-file desktop scan keeps
  the **first** match and leaves the existing index on a miss.
- `SoundEnabled` consumes only the keyword. Its value goes through the unknown
  token path, which logs and consumes one token. The sound byte is unchanged.
- Backing and stream references are decremented in that order through one
  captured CE2220 import. Their current slot0 is called only at zero; tokenizer
  destruction follows and releases its actual retained stream reference.
- Missing-file handling sets fullscreen before querying the desktop and stores
  width/height before the `Destop size=%d %d` diagnostic. GetWindowRect failure
  does not trigger a replacement rectangle; the explicit preimage remains.
- Registry access is HKLM, `SOFTWARE\Eidos\Battlestations Pacific`, access
  20019h, value `language`. REG_DWORD selects German/Spanish/French/Italian for
  407h/40Ah/40Ch/410h; all other values select English. Returned byte length is
  not validated. Close follows a successful open, regardless of query outcome.
- Missing-file settings are written **before** shader/capability/AA clamping.
- Renderer primary D5F0A8 slot104h is verified as B1FF50, returning +1B18h.
  Capability +28h below 200h clears both shadow bytes and +90h.
- Antialias rebuild clears raw vector +28h, appends zero, then checks samples
  2 through 15 via the current renderer+1990h COM object's slot2Ch. Only HRESULT
  exactly zero appends. Format is fixed; the original format argument word is
  reused for quality output, preserving its contents between calls.
- Final AA matching keeps the last matching index, then applies the native
  signed upper clamp. Empty vectors and negative indices have no added repair.

## Evidence and validation

Project/program were checked against `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Seven complete bodies total 2065 bytes; 2497
live/PE bytes include literals, import cells, renderer capability slot, and the
LCID jump table outside the loader body. All seven listing audits have zero
gaps. The report records every direct CALL and separately records indirect
calls; mechanical direct-call verification also checks function membership.

Strict MSVC Win32 compilation and all three existing CTests pass. One local
diagnostic compares 12 original/source executions, with **20,428 identical
observed bytes**: full raw settings state including padding, vector contents
and capacities, FILE/registry/diagnostic arguments and order, comparisons,
reference counts, and all fourteen COM sample checks including quality
preimages. It covers valid and malformed values, unsupported resolution and
AA, shader fallback and upper clamp, low/high capability, both formats,
all registry language branches, failed desktop lookup, registry open/query/type
gates, and failed writer open. Both lanes use the same previously recovered
dependencies. The catalog is prepopulated for these loader comparisons.

The native loader calls all six copied native helpers. Its five jump targets
and two table references are relocated explicitly. FILE/registry/desktop/COM
boundaries are controlled; allocation addresses are normalized. Private
tokenizer and failed-rectangle preimages are supplied equally to both lanes.
The retained-memory owners, pool, vectors and tokenizer are actual source
implementations, not projected shadows.

A source-only diagnostic exception at the unknown-token log retains the path,
backing, stream and tokenizer after FILE close. Replay is rejected; explicit
cleanup drains all memory and pool ownership. No exception crosses the copied
native FH3 frame. A separate source run reads a real workspace-local CRLF
options file using the default host fopen/fseek/ftell/fread/fclose path and
verifies its resulting language, resolution, shader and AA settings.

## Limits and follow-up

These interfaces add explicit contexts and retained operations. This evidence
does not establish original CRT internals, original binary ABI, FH3/SEH,
private-frame aliasing, asynchronous mutation, token overflow, actual desktop
or registry behavior, hardware Direct3D sample support, or gameplay parity.
The logging target 4254B0 and shader selector B200C0 are verified no-ops in
this executable; their empty defaults reproduce those bodies.

Next bind the loader to canonical application settings, renderer, catalog,
pool, input and online owner lifetimes. Reuse the actual default path/OS/CRT
providers and serialize any live application check with existing game owners.

Evidence: `reports/native_settings_loader_r167.json`; ignored diagnostics and
immutable archives under `local/settings_loader_r167` and `local/evidence-r167`.
