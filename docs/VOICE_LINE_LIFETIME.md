# Voice-line lifetime and archive constructor

This packet reconstructs three complete game/CG bodies against the existing
`VoiceLine`, `VoiceClips`, GUI, pooled-string and Lua-reader projections.
Descriptive game-function names are hypotheses. The scalar-deleting wrapper
keeps its correct compiler-generated identity and 00BF65AC keeps `_free`.

| Entry | Native ABI | Inclusive final byte | Bytes |
| --- | --- | --- | --- |
| 005B9160 | ECX=line, RET | 005B91DA | 123 |
| 005B9B80 | ECX=line, flags stack, RET4, EAX=original this | 005B9B9D | 30 |
| 005BAF30 | ECX=fresh 38h line, reader pointer stack, RET4, EAX=this | 005BB01E | 239 |

## Destruction

005B9160 installs the line vtable 00CF0ED4 before doing anything else. It
tests line+14, reloads that pointer, and invokes the widget's virtual +4 with
flag 1. It then clears line+14, including a replacement written by the
callback. Only the original-nonnull arm reloads `[[00E198C4]+A4]` and sets
manager+60 to byte 1. A null initial widget skips the manager lookup entirely.

After those operations, it frees the current clip-vector backing pointer at
line+8 when nonnull, then clears line+8/+C/+10. Clip records are borrowed:
there are no element destruction, record release, sound stop or shortcut-widget
delete calls. Other line words, including +18/+1C/+20/+24/+28/+2C/+34, are
left unchanged. The widget tree owns the shortcut child.

The canonical `VoiceClips` uses `std::vector`. Its actual destructor releases
storage with its matching allocator, then placement construction creates an
empty header at the same location. We do not apply CRT free to a projected
STL buffer. `VoiceClip` must remain trivially destructible. Cleanup runs on
normal exit and C++ exception unwinding: native funclet 00C72200 also invokes
the clip-vector destructor 005B8DA0 during unwinding.

005B9B80 calls the concrete line destructor unconditionally. It frees the
outer allocation only when the low bit of the flags byte is set, then returns
the original pointer even if storage was released. `VoiceLineLifetimeHost`
supplies the real widget virtual destructor, current manager and matching
outer allocation release. The release callback must not rerun the native line
destruction sequence. It may perform normal projected C++ object teardown on
the now-empty vector as required by the actual allocator.

## Archive construction

005BAF30 installs the same vtable and zeroes the fresh clip-vector header.
It does **not** deserialize a clip list or restore playback progress.

The first serializer virtual +10 call has key `{0,"shortcut"}` and field
`{4,&shortcut}`. Tag 4 is the existing Handle conversion, not an integer
read or a read-with-default. The destination is initialized by 005BAF67 to
the address of the field pair in the native call frame. 005BAF6B takes the
address of that same destination. After the call, its raw dword is copied
to line+2C, without target validation. If conversion ignores a missing or
unsupported value, the address seed remains. The reconstruction uses the
address bits of its own Win32 `GuiLuaVariant` pair; it does not invent a zero
default or claim to reproduce an original-image stack address.

Next, a fresh empty native string is read by virtual +10 with key
`{0,"text"}` and field `{0,&text}`. The serializer's return is ignored.
The existing `GuiLuaReader::read_00bd6830` implements both reads, with a
required real `GuiLuaHandleResolver` for the first. Its String projection is
`std::string`; the result is copied into an actual `NativeString` using the
supplied storage and passed to `display_voice_subtitles_005b8510`.

Subtitle construction runs unconditionally, including empty/missing text.
Only after it returns does the constructor write line+18 = -1. The temporary
string then releases its owned storage, and EAX returns this. The constructor
does not assign +1C; the C++ optional retains the caller's prior projected
state. It never zeroes unrelated line fields as an invented initialization.
Native unwind funclets 00C72450/00C7245B clean the vector and temporary string;
the reconstructed scopes provide the corresponding C++ exception cleanup.

## Evidence and limits

Full assembly, rather than the garbled stack-variable pseudocode, establishes
the serialized-constructor argument pairs. Live bytes at 00CF0DF8 and
00CF1010 confirm `shortcut` and `text` respectively. Read-only live prototype
queries confirmed each body extent. Supporting compiler cleanup paths were
inspected at 00C72200, 00C72450 and 00C7245B, without renaming them.

The initial flow audit found two false-no-return gaps after `_free` calls:
005B91BE..005B91C0 and 005B9B95..005B9B97. Live bytes for both are
`83 C4 04` (`ADD ESP,4`). Including those gives 41 and 11 instructions for
the destructor and deleting wrapper. Final instructions are 005B91DA `RET`
(1 byte), 005B9B9B `RET4` (3 bytes), and 005BB01C `RET4` (3 bytes).
The archive constructor has 77 instructions and no gaps. The parent owns
Ghidra repair; this worker performed no mutations or project saves.

Canonical GUI and serializer limitations remain explicit. `GuiLuaReader`
currently converts its narrow String through `std::string(char*)`, so it does
not retain an embedded-NUL Lua length; table Handle resolution currently uses
`resolve_by_table(nullptr)` because its interface does not expose the live
table reference. The caller must supply compatible real services. This packet
does not replace either API with a divergent serializer or assume success.
Exact inherited API/source sites are `GuiLuaReader::read_00bd6830` at
`src/gui_lua_reader.cpp:551`, the String projection at line 419, and the
null-identity Handle table resolver call at line 450. Null table identity is
an unresolved canonical-reader limitation, not faithful native table forwarding.

These are host-projection reconstructions, not native ABI entry points or
original SEH registrations. Validation details are in
`reports/voice_line_lifetime.json`; game/visual and native differential
validation have not been performed.

Release MSVC Win32 `scripts/build.ps1` passed, including the existing
`reconstructed_math` CTest (1/1). One ignored local fixture,
`local/voice_line_lifetime_fixture.cpp`, passed with `/W4 /WX`: it exercises
widget callback mutation, dirty-manager timing, clip cleanup after a throwing
callback, the null-widget branch, and flags 2/3 storage ownership. It does
not validate archive construction or replace native/game validation. No
repository tests or test registrations were added.
