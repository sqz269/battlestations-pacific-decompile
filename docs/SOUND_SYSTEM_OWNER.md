# Sound owner construction

Packet `orch3_sound_owner` reconstructs constructor behavior at 00A81480,
00A7B190, 00A7FD40, 00A858F0, 00A88650 and 00A880E0, plus the two small
singleton-base unregister bodies 00A7B230 and 00A88180. Names are hypotheses,
not recovered symbols. Source: `include/bsp/sound_system_owner.hpp` and
`src/sound_system_owner.cpp`; evidence/validation: `reports/sound_system_owner.json`.

The projection binds the existing `SoundSystemState`, `SoundConfigurationState`,
`SoundManagerLevels` and `SoundClassOwnership`. There is one scalar configuration,
one enabled byte, one global level and one class pointer table. Full 00A88770
startup integration belongs to the parent packet, which calls these constructors
in native order; this packet does not replace that startup routine.

## Evidence and native interfaces

Every live query/export used the verified repository bridge against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. This worker made no Ghidra
mutations. Assembly was checked for register object inputs, stack pointer changes,
x87 matrix copying, and the false no-return free call. All constructor interfaces
below are native ECX=this, no stack arguments, EAX=this, plain RET. The Ghidra
fastcall/void prototypes of several helpers are incomplete.

| Address | Last instruction | Behavior |
| --- | --- | --- |
| 00A7B190 | 00A7B220 RET, 1 byte | Sound singleton base publication/registration |
| 00A81480 | 00A816A4 RET, 1 byte | Complete base constructor behavior |
| 00A7FD40 | 00A7FDFC RET, 1 byte | Embedded listener owner defaults |
| 00A880E0 | 00A88170 RET, 1 byte | Auxiliary singleton publication/registration |
| 00A88650 | 00A886BC RET, 1 byte | Empty auxiliary tree owner |
| 00A858F0 | 00A85A49 RET, 1 byte | Resource cache owner and error-resource request |
| 00A7B230 | 00A7B2C8 RET, 1 byte | Sound singleton-base unregister; no meaningful EAX result |
| 00A88180 | 00A88218 RET, 1 byte | Auxiliary singleton-base unregister; no meaningful EAX result |

The parent defined/exported missing pointer-element function starts 0054D440 and
004BA0D0. Both contain `MOV EAX,ECX; MOV [EAX],0; RET`, ending at 0054D448 and
004BA0D8. The native EH iterator constructs three 4-byte elements with each helper;
the projection uses two standard arrays of three null pointers, without porting
the compiler's EH iterator or the element deleting functions.

The parent corrected the CALL_RETURN override at 00A85A2D, disassembled its
24-byte continuation and refreshed exports. The decompiler now recovers the
return-this epilogue. Ghidra's stored function body still ends at 00A85A31, so
the saved assembly export omits 00A85A32..49. Verified raw bytes establish the
remaining stack cleanup, register restoration, FS exception-list restoration,
`ADD ESP,60h` and final RET. Function-body extension remains an analysis-metadata
follow-up, not an excuse to omit the proven return path from the reconstruction.

## Base defaults and untouched fields

00A81480 first calls 00A7B190, then writes vtable D5B000 and the thirteen existing
`SoundConfigurationScalars` defaults. Those values remain defined in the existing
configuration header; this module does not duplicate their table.

| Manager fields | Constructor writes |
| --- | --- |
| +38/+3C/+40 | Empty sound-type vector and zero logical capacity |
| +4C, +50 | Global level 1.0, byte 0 |
| +58..+64 | Four zero dwords; their semantic types remain open |
| +68/+69, +6C, +70 | Two zero bytes, float 1.0, enabled byte 1 |
| +74..+7C, +80..+88 | Two arrays of three null pointers |
| +8C/+90/+94, +98/+9C/+A0 | Empty entry/class tables and zero capacities |
| +A4/+A8 | Listener vtable CEB130 then D5AEC4; reference count 1 |
| +AC/+B0/+B4 | Empty listener-name table and zero capacity |
| +B8/+BC/+C0 | Three float zeros |
| +C4..+100 | Identity 4x4 matrix, copied through existing 004134F0 |
| +104/+108, +10C | Null selected pointer, ordinal 0, current listener -1 |
| +118/+11C/+120/+124 | Four raw dwords `{0,0,1,0}`, not float identity data |
| +128..+13C | Empty channel-group and system-DSP vectors |
| +140 | Null master-channel-group handle |
| +144..+154, +160..+16C | Five and four zero dwords; semantics remain open |
| +158/+15C | Raw dwords 100 and 00432380 |

The final three stores repeat nulling +74/+78/+7C. +158/+15C later become the
existing min/max-frequency outputs in 00A88770, but their base defaults are **not**
1 and 440000. 00432380 is an image function address used as a raw initial word;
this constructor does not call it.
The five-word +144 array ends at +154; it excludes the separately represented
+158 frequency word.

No base stores initialize +44, +48, +54, +110, +114, +170 or +174, nor the padding
around byte fields. The constructor API preserves already represented caller
values, including FMOD handles, prior-listener ordinal and derived speaker fields.
The C++ wrapper's initial null +54 and other value-initialized projection storage
are host conveniences, not recovered native writes. This is fresh construction:
all canonical containers are initially empty, class capacity is zero, and the
class owner is bound to the same levels object. Reconstructing over live storage
is outside this constructor interface.

## Singleton and empty-tree ownership

00A7B190 and 00A880E0 write their base vtables D5ABAC/D5B448, capture the first
manager's critical section +10, enter it when nonnull and increment its +18
recursion word. They then publish `this` to F8BBD8/F8BBE8, get the manager again,
reload the global and register through existing 00BD0C30. The original captured
section is decremented/left. The projection uses the real shared
`SingletonLifetimeDomain` and its actual pointer registry, not an observation log.
There is no existing-singleton guard: native constructors overwrite the globals.

00A88650 changes the auxiliary vtable to D5B460. 00A88270 allocates a 1Ch tree
node, initializes its three links to null, byte +18 to black (1), and +19 isnil
to 0; it returns EAX at 00A882A6. The caller stores the head at owner+8, sets
isnil=1, makes all links self-referential and writes owner+C count=0. Owner+4's
allocator/proxy word is not initialized here. `std::map` represents the empty
tree invariant. Native node +C/+10 holds a string and +14 a value word; the
value meaning and comparator remain unresolved. No map mutation, native lookup,
node allocation ABI or STL implementation is claimed reconstructed.

Head-allocation exception cleanup is specifically proven: handler CB6268 loads
FuncInfo DEC408, whose one-state unwind map DEC400 is `{-1,CB6260}`. Funclet
CB6260 loads ECX from EBP-10 and tail-jumps to 00A88180. The C++ factory therefore
unregisters the auxiliary base if standard tree allocation throws after
publication, before its temporary unique_ptr frees the projection.

The explicit unregister APIs implement only the small singleton-base bodies.
They unregister the **current global value**, clear that global unconditionally,
release the captured lock and set the supplied owner's vtable to CE3818. They
do not substitute for full sound, FMOD, resource-cache or populated-tree teardown.
The parent owns 00BD0D70, which reorders already registered owners during startup.

## Error-resource owner and required loader

00A858F0 initializes its 2Ch-record cache header +4/+8/+C and size-accounting word
+10 to zero, sets vtable D5B210, and constructs the temporary options below.
It creates the native path string `sound/gui/error.fsb`: length 19, allocation
20 bytes, memcpy including terminator. At 00A859F6 it calls 00A84740 with
ECX=owner and four stack arguments `(path*, options*, 1, 1)`; native RET10 is
proven at 00A84C23/00A84C87. The result is stored to owner+14 before path release.
The path is released with the same 20-byte size, then the options vector is
resized to zero through 0093F950 and its storage freed. Its records are 10h trivial
dword groups; standard vector destruction supplies that storage operation.

The options occupy **48h bytes**, not 44h: base ESP+1C, vector header at +3C/+40/+44.
Words/floats +00=0, +04=1.0, +08=1.0, +0C=10.0, +10=1e9, +18=0, +1C=0,
+24=-1, +30=0 and +34=0 are written. Bytes +14/+15/+16, +20/+21/+22 and +38
are zero. +28/+2C and padding are unwritten; optional fields explicitly retain
that unknown status. Image D217E8 bytes `28 6B 6E 4E` prove the 1e9 float.

`SoundResourceLoadHost::load_00a84740` is a required, unresolved **game** boundary.
It is not a trivial FMOD import or a reconstructed no-op. The native helper
normalizes names, searches cache aliases, resolves/creates resources and optionally
clones, appends cache records and adds the resource's virtual +C size to owner+10.
Vtable D5B210 identifies +4 resolver 00A82EA0, +8 creator 00A835B0 and +C
clone/retain 00A854C0. Those are the next asset-loading packet, alongside cache
append 00A84530. The supplied host can mutate this same cache and returns the
actual retained resource pointer. Its +14 preimage is unspecified before the call.

Native 00A85A50 later decrements returned +14's reference and dispatches vslot0
at zero, clears +14, then enters cache teardown 00A85500 (00A84C90/00A845A0).
Those teardown bodies are analyzed dependencies, not implemented here. Opaque
game-resource pointers therefore require the actual host teardown before the
projection is discarded. Standard container destruction only frees host storage.

Constructor failure has a separate required cleanup call. Handler CB5F98 loads
FuncInfo DEC080, whose three entries at DEC068 unwind through CB5F90 (path
0041DD20), CB5F88 (options 0093FDB0), and CB5F80 (base owner 00A85500). If path
construction or the loader throws, the factory destroys the temporary scopes,
then calls `SoundResourceLoadHost::destroy_failed_owner_00a85500` while the cache
and accounting still exist, then propagates the exception. The host must release
the acquired game resources; this required boundary has no default no-op.
It is base-cache teardown, not derived 00A85A50 release of a successful +14 result.

## Validation boundary

MSVC Win32 Release build and both existing CTests passed after all eight native
seed byte checks matched the installed image. An ignored focused host probe
checked canonical state binding, defaults, untouched caller fields, identity
transform, real registration order and lock recursion, empty tree, exact loader
arguments/options, string allocation/release, retained loader mutations and
unregister holes. One additional ignored throwing-loader case verifies path
release before the cleanup callback, intact acquired cache state at that callback,
and propagation of the original exception. Options destruction is established
by its C++ scope and native unwind evidence, not separately instrumented.
No permanent tests were added. The auxiliary allocation-unwind fix is
assembly/EH-evidence reviewed, not a forced-allocation-failure runtime test.

Validation uses ignored `local/sound-owner-extra.cmake` because the integrator
owns the source registry. Register `src/sound_system_owner.cpp` during integration.
These are typed behavior interfaces, not ABI-compatible replacements. Native
physical layout, allocator preimages, general SEH equivalence, audio playback,
resource loading and in-game behavior were not validated by this packet.

## Follow-up: concrete sound resource runtime

The subsequent SOUND_RESOURCE_RUNTIME.md packet supplies the recovered cache,
asset creation and teardown through actual VFS/FMOD services. Its installed
fixture now executes the full normal sound constructor, actual cached error FSB
and standalone FEV lifecycles, and installed Lua initialization: 133 successful
FMOD calls. Platform pretranslation/focus remain explicit fixture observers;
no audible playback, native ABI or gameplay claim follows. The current singleton
and +54 cache stay published until resource cleanup finishes. Earlier unresolved
loader/cleanup statements above describe this document's original snapshot.
