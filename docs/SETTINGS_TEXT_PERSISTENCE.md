# Options text persistence
Addresses: 008D6170, 008D5150, 008D64A0, 008D5340, 004264A0, 00BD1BB0

`008d6170` writes options.txt. The earlier description of hardware derivation
was incorrect. The serializer `008d64a0` calls it at `008d64a9`, before its
first archive virtual. That call is now required by `SettingsWriter`, with
`OptionsTextSettingsWriter` binding it to reconstructed text persistence.

The output order is Language, Fullscreen, Resolution, Vsync, ShaderModel,
Antialias, Clouds, Foliage, Shadow, Reflection, TextureDetail, ObjectDetail,
SoundEnabled, Firewall, HardwareReported. Language uses the table lanfile
pointer at entry+4h, selected by settings+4h and stride20h. The two resolution
integers use a single space (`00ce3a90`); every line ends LF (`00ce4390`).
Integer append uses the stream's initial `%d` (`00ce3a34`), not locale grouping.
`settings_text.cpp` captures all text before the first filesystem host call.

The path builder calls SHGetSpecialFolderPathA with null HWND, CSIDL_PERSONAL5
and create=true, appends `\Battlestations-Pacific`, calls CreateDirectoryA,
then appends `\options.txt`. Directory creation's result is ignored. Native
`008d5150` returns its caller-supplied string in EAX, RET4 at `008d5332`.
Its decompiler drops string-copy branches; complete assembly was checked.

Writer `008d6170` has ECX=settings, no arguments, RET at `008d6494`. After
formatting, it resolves the path (`008d640c`), opens with `wt` (`008d6424`),
and only for a nonnull FILE calls fwrite(data,1,length,file) at `008d6448`
then fclose at `008d644e`. It ignores write/close results. A failed open
does not prevent the enclosing archive settings writer from continuing.

`Win32SettingsTextHost` supplies real Windows/CRT services. Tests explicitly
override the personal directory to a verified `local/options-text-probe`
path; no installed game file or user's existing options file is written.
The isolated Win32 fixture wrote and read back227 bytes, checking all15 lines
and CRT LF-to-CRLF conversion, including a negative signed integer. The
existing settings test also checks that text persistence precedes archive
sections. The standalone Win32 build and both existing CTests passed.

Limits: these are C++ projections, not native ABI hooks. Native pooled
temporary strings/stream SEH are represented by owning std::strings; arbitrary
allocator reentry is not modeled. Boolean fields use normal bool values.
Language index must be valid. Native assumes successful personal-folder
resolution; the concrete host reports failure instead of using the native
uninitialized buffer. A supplied personal-directory override is explicit
host policy for an isolated application or check.
