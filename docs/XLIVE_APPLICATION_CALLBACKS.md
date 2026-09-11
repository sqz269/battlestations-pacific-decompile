# XLive application callbacks

`src/xlive_application_callbacks.cpp` reconstructs the three address identities
installed by `0073DC70`: `00735510` at manager `+20`, `00735520` at `+24`, and
`00737D60` at `+18`. Names below are hypotheses. These are typed C++ operations,
not replacements using the native register ABI. No SDK emulation is included.

| Address | Native ABI and evidence | Reconstructed operation |
| --- | --- | --- |
| `00735510` | ECX=user index; no stack arguments; `PUSH 6`, `PUSH 8001h`, `PUSH ECX`, `CALL A4D3F2`, plain RET at `73551D` | Forward `(user, 8001h, 6)` to XLive |
| `00735520` | Same, value 4; plain RET at `73552D` | Forward `(user, 8001h, 4)` to XLive |
| `00737D60` | Incoming ECX ignored; no stack arguments; FS exception frame; plain RET at `737DF3` | Copy selected cached name to display name, then player name |
| `00A3EAE0` | ECX=manager; `MOV EAX,[ECX+11C]`, `SHL EAX,7`, `LEA EAX,[EAX+ECX+90]`; RET at `A3EAF0` | Compute the DWORD offset of the selected name pointer |

Each terminal RET is one byte. The two 14-byte context callbacks are missing
function starts in the current Ghidra model and are attributed to the preceding
function. Their raw bytes and final instruction boundaries are recorded for the
integrator to define under the write lock. The other two starts exist. There are
no false no-return gaps in these four bodies. This packet did not write Ghidra.

`A4D3F2` is `JMP [CE2698]`. Independently reading the original game PE resolves
that IAT cell to ordinal **5277**. The installed XLiveLessNess export names it
`XUserSetContext`; the Microsoft ordinal-only export consumes three DWORD
arguments and ends with `RET 0Ch`. The native callbacks do not consume a return
value. `XLiveApplicationContextAdapter` calls that ordinal with a Win32 stdcall
function pointer and deliberately exposes no result contract. It borrows
`XLiveLibrary::module_handle()` and neither loads a DLL nor owns SDK lifetime.

`XLiveApplicationCallbackInvoker` dispatches the three stored image identities
to these C++ operations; it never executes an image address. Its
`invoke_state_callback(const void*)` entry uses ECX=0, as established at the
existing startup, pump and sign-in call sites. `invoke_callback` accepts the
incoming ECX explicitly. An unrecognized identity raises an error. The required
globals interface borrows the current manager and current game/profile storage;
the integration must provide the real live owners.

The name callback loads the current manager at `737D78` and obtains its source
pointer before string allocation. The getter's shift/add wrap as 32-bit unsigned
arithmetic. Manager `+90..+10F` is one 128-byte cached name, not an array of four
names: index 1 starts at `+110`, in other manager fields. The typed callback
supports offsets that resolve to `+90` and raises an explicit error for other
regions. The recovered profile-change caller invokes `+18` only when the selected
index is zero. Unknown cache bytes and a missing known terminator also raise an
error, without interpreting backing zeros as native data.

The existing C++ spelling `signin_state_11c` is misleading for this operation.
The getter reads `[ECX+11C]` directly; it has no stack selector. At `A3ED14` the
same field is loaded and at `A3ED43` pushed as `XStorageBuildServerPath`'s user
index. UI processing copies `+3B4` into it at `A408AA/A408CA`; the reset path
writes 1. This supports a selected-user index/sentinel interpretation, not the
SDK sign-in status written separately to `+8C` at `A3ECCD`. The shared field name
is retained for compatibility. At `A3E66A/A3E672` the profile-change caller
explicitly rejects nonzero `+11C` before loading and invoking callback `+18`.
Thus the apparent multi-user name-array overlap is real pointer arithmetic,
with a user-zero gate on this callback's recovered route; the exact meaning of
every nonzero selector is not asserted.

The fresh `0041E870` string operation is projected using `NativeString` and its
canonical resize: scan known bytes through NUL, allocate, then copy exactly the
original source's current length+1 bytes. A reentrant allocator may change those
bytes or the singleton; the source reference stays captured. Definedness is
checked again before copying, and the cache's unused tail is not read. Empty
names keep the native null string buffer. Guard failures are explicit host
boundaries; if a post-allocation guard fails, owned storage is released.

After construction, `737D8D` reloads the game and calls the canonical
`set_profile_display_name_007f9340` on game `+650`; `737DAE` independently reloads
the game before `set_profile_name_007f9290`. The returned bindings include each
current game's `+1FF0` name mirror. The existing profile setter projection owns
its string/mirror behavior; allocator-driven singleton reentry *inside* those
setters remains outside that existing projection. The callback does not cache a
single profile for both calls or roll back a completed first setter.

Native normal cleanup at `737DBF..737DE0` releases a nonnull temporary using its
length+1 through the sized pool. Exception handler `C86288` selects FuncInfo
`DB5868`, whose one-state unwind table at `DB5860` targets `C86280`; that thunk
passes the temporary to canonical destructor `0041DD20`. The C++ temporary owner
releases the same storage on normal return or an exception after construction.

Validation: Win32 Release `/W4 /WX /fp:strict`, both existing CTests and the eight
native seed checks pass. One ignored recording fixture verifies context argument
forwarding, source capture across allocator reentry, separate profile reloads,
known-prefix handling, selected-offset wrap/guards and temporary cleanup without
rollback. It mutates only fixture-owned profiles and invokes no SDK. Raw live
Ghidra bytes match the installed game for all four bodies. These are build and
fixture results; no real account/context changes, SDK callback execution or
game-runtime validation were performed.
