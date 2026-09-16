# Native particle resource loader

`create_native_particle_resource_0086ba60` reconstructs all 278 bytes of
`0086BA60..0086BB75`. The original ignores incoming ECX, consumes **two** stack
arguments (actual 8-byte name header and an ignored DWORD), returns the resource
in EAX, and uses `RET 8`. The saved decompiler prototype omits these arguments.
The source interface exposes both explicitly. Names are descriptive hypotheses.

## Behavior and ownership

The loader initializes an actual 1Ch text buffer, deep-copies the caller name
through the actual raw string pool, lowercases every ASCII character, allocates
90h, calls the genuine `AF45D0` constructor, then stamps `D0D418`. It passes the
current copied-name bytes (or actual `F8766C` empty name) to genuine `AF5850`
loading, then calls genuine `AF4BA0` parsing on the allocated resource and text.
It ignores both the loader count and parser AL. A null allocation still reaches
the loader/parser path; there is no added successful-empty or null recovery.

Normal return releases the copied name and then the text buffer. The caller
owns the returned resource. `NativeParticleResourceLoaderRawContext` borrows the
same actual string cells, VFS publication, concrete VFS bindings/name resolver,
and raw particle parser. The application can borrow its existing VFS services
through `GameNativeVfsRuntime::borrow_raw_services()`; this function creates no
VFS manager, callbacks, retry loop, private publication, or substitute parser.

The opaque acquired frame stores the actual 8h name and 1Ch text before both
retained child invocations. Its explicit incoming parser builder kind preserves
the parser's native unwritten builder+C word; that value is separate from the
parser context's nested emitter `child_builder_kind`. Replays fail before any
native operation. A completed frame destroys bookkeeping only. Failed frames
and all borrowed domains must outlive unresolved provider obligations; existing
failed VFS frames currently require process lifetime. Native cleanup leaves
stale header/pointer fields and is never replayed by a source destructor.

## Exception evidence

The original handler is `C9558B`, FH3 FuncInfo `DC7578`, magic `19930522`,
maximum state 3, flags 1, unwind map `DC7560`:

| State | Previous | Action | Native cleanup |
| --- | --- | --- | --- |
| 0 | -1 | `C95570` | Tail to `AF5620`, text cleanup |
| 1 | 0 | `C95578` | Tail to `41DD20`, copied-name cleanup |
| 2 | 1 | `C95580` | `BF65AC`, allocation free only |

The constructor runs at state 2. After construction and `D0D418` stamping, both
file loading and parsing run at state 1. Therefore a constructor exception
frees its allocation after the constructor's own cleanup, while loading or
parsing exceptions **retain the constructed resource** and release the native
name/text temporaries. The acquired frame reports the original failure site,
native state, and constructed-resource identity. Secondary cleanup exceptions
terminate, matching the source family's existing exception-domain contract.

## Evidence and validation

The complete original body (85 instructions), 60 bytes of FH3/map data, and all
37 bytes of action/handler code match the installed PE and current saved Ghidra
program byte-for-byte. All 11 body calls and three action call/tail rows pass
the live call verifier. Prior Ghidra documentation is preserved in the report.
The full `./scripts/build.ps1` passes with MSVC Win32 `/W4 /WX /MD /fp:strict`.
After live/disk seed verification, all three existing CTests pass, including
the native math differential test. The final probe links this worktree's
newly built `bsp_core.lib`.

The copied-original/source fixture passes three complete physical-file cases:
empty file (zero AF5850 count and false parser AL), rejected header (nonzero
count and false parser AL), and minimal valid `ParticleSystem`. It compares
resource profile, reference count, lowercase owned name, all initialized
fields, genuine cleanup/singleton-pool drain, and source replay rejection. The
original path uses the correct two-argument `RET 8` call with different ignored
ECX/second-argument sentinels. Fixture files live only under ignored `local/`.

The fixture relocates the complete 278-byte original loader and bridges its
calls to genuine existing providers. It reserves actual VFS table/publication
addresses in a suspended fixture child before Windows startup allocations,
using the repository's existing RO handoff plus two explicitly owned mutable
publication bands. It does not execute native child bodies or the original
FH3 handler. Build receipts and hashes are in the report. These are new source
interfaces, with no original register ABI, hardware-SEH, exceptional-path
differential, executable replacement, or gameplay claim.
