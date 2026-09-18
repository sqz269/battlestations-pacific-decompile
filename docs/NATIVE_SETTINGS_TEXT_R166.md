# Raw settings path and text persistence (R166)

Addresses: 008D5150, 008D5340, 008D6170.

Three complete normal bodies now implement the raw options path, 18h builder
copy construction and settings writer. They borrow the actual string pool and
catalog publication and operate on the actual BCh settings owner. This supplies
two remaining dependencies of the raw settings loader. The ordinary application
still uses its projected settings path until canonical ownership is integrated.

## Recovered contracts

| Entry | Original ABI/size | Behavior |
| --- | --- | --- |
| 008D5150 | Stack output header, EAX output, RET4; 485 bytes | Call SHGetSpecialFolderPathA with null window, CSIDL 5, create 1; ignore result. Construct pooled directory/root temporaries, concatenate into caller output, release captured root then current directory. Call CreateDirectoryA with null security attributes and ignore result. Append the captured options suffix through the native resize/copy schedule, then return its captured buffer. |
| 008D5340 | ECX destination, stack source, EAX destination, RET4; 232 bytes | Zero/copy three 8h headers in ascending order. Identity abandons prior allocations and leaves all headers zero. Recheck current source length after resize and copy current destination length. |
| 008D6170 | ECX actual settings owner, RET; 805 bytes | Construct builder; capture language and all scalar arguments before the first append. Emit 15 ordered lines through 47 chained append calls. Clone builder, destroy original, construct path, fopen wt, conditionally fwrite/fclose, release path, destroy clone. |

The path body's pseudocode omits reachable copy/release work. Assembly is
authoritative: capture root data/length before concatenation and release those
captured values afterward; the directory release uses its current header. The
suffix append captures old output length, resizes with wrapped addition, then
copies into current output data plus that old length. Dead headers remain.

The native 260-byte personal-folder buffer is initially uninitialized. Source
requires an explicit preimage and preserves the ignored folder-call return;
it does not invent a failure fallback. Valid execution still requires readable,
NUL-terminated data within the admitted buffer domain.

Writer order is Language, Fullscreen, Resolution, Vsync, ShaderModel, Antialias,
Clouds, Foliage, Shadow, Reflection, TextureDetail, ObjectDetail, SoundEnabled,
Firewall, HardwareReported. Resolution includes two signed integers separated
by a space. Byte fields are zero-extended numeric values, not normalized bools.
The language pointer comes from current catalog plus wrapped index*20h, with
the native empty-string fallback only for a null language-data pointer. No
catalog bounds guard is added. Fwrite and fclose return values are ignored.

Source operations retain temporary/header ownership, captured arguments, file
ownership, nested operation progress and native call sites. Escaping C++ callbacks
require explicit cleanup before acknowledgement; replay is rejected. Existing
builder helper construction/cleanup policies and the actual pool bridge's
returning-getter domain apply. This is not recovered native FH3/SEH.

## Validation

- All 1522 code bytes plus 302 data/import-slot bytes match live Ghidra and the
  original PE. All 78 direct CALL rows are checked mechanically; the two OS
  import sites are recorded separately and their arguments checked at execution.
- Strict MSVC Win32 build and all three existing CTests pass.
- A local diagnostic compares copied original bodies with source in seven
  paired groups: 1687 identical observed bytes. The native writer calls the
  copied native path and copy bodies; the source writer calls their source
  counterparts. Existing raw pool/concat/builder helpers are shared by both lanes.
- Path cases cover successful folder lookup and a failed lookup that leaves
  the seeded buffer intact, with failed directory creation ignored. Builder
  copy covers ordinary construction and identity abandonment with explicit
  fixture cleanup of the abandoned allocations.
- Writer cases cover successful open with ignored short-write/close results,
  failed open, and null language data. Checks include exact line order, raw
  byte values 2..255, signed INT_MIN/-1 formatting, file-call gating, preserved
  owner bytes and pool/manager drain.
- A source-only fwrite exception retains the open file, path and copied builder
  after the original builder is destroyed. Replay rejection, explicit close,
  header cleanup and pool drain pass.
- A source integration case uses the default CreateDirectoryA/fopen/fwrite/fclose
  adapters under local/settings_text_r166/io-personal. The resulting options.txt
  is read back in binary mode: all 231 bytes, including CRLF translation, match.
  Only folder lookup is redirected into this workspace-local directory.

Allocation identities are normalized; live string content, owner bytes, callback
arguments and file data are observed. Original CRT internals, original binary ABI,
FH3/SEH, private stack aliases, asynchronous mutation, ordinary application
ownership and gameplay remain unproven. Real I/O here uses the linked host CRT
and Win32 APIs, not the original game's CRT objects or user settings directory.
No installed game files or shared game process were changed.

Existing path/writer names are retained; the copy body is named
BSP_StringBuilder_CopyConstruct. Names are descriptive hypotheses, not recovered
symbols. Saved comments, prior annotations, refreshed exports, source/combined
builds and sealed artifact hashes are recorded in reports/native_settings_text_r166.json.

## Follow-up packets

Compose raw 008D8190 using these persistence functions, the R165 catalog producer,
R160 choice assignment/language selection, and the raw tokenizer/memory owners.
Recover remaining renderer calls and helpers from their actual bodies before
binding canonical settings ownership into application startup.
