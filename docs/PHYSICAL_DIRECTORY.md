# Physical directory provider fragment

`PhysicalDirectory` implements native root-plus-suffix construction and the
ordinary non-indexed existence/virtual-name resolution route. It supports
provider constructor flag+28=false and an empty indexed tree (+34=0). It is
not the complete native directory provider, mount manager or archive search.

## Evidence and dispatch

Live `bsp`, `/battlestationspacific.exe`, x86 LE base00400000 were verified
before each analysis batch. Fresh byte windows below matched the installed PE.
Ignored exports are under `exports/bsp/owner_textures/font/`; missing function
definitions at00bf0fb0 and00bf39c0 were read through raw bytes and Capstone,
without creating or changing Ghidra functions.

| Start | Bytes | SHA-256 |
|---|---:|---|
| `00bf3970` | 65 | `83322b019ce0e47cdec0a1fbce1d04a3d9593917cb2064655650fb18ef603a7b` |
| `00bf0fb0` | 75 | `a7052b969ac9845090521079e1a3c025b01679e131f13ab309efa5f8c84fdb47` |
| `00bf39c0` | 188 | `aeec8ead04fffb35d1c32d0d8b842a8b740c1561b6746715379afc82bbdc7a1a` |
| `00bf3f70` | 586 | `a5f7056012120dd19df00543204ef9879a3c9864b2c5d903f5a83c2716a721b9` |
| `00bf4d30` | 156 | `f6faa5e0f7616105b9ad7029676d61514a8b0aac125a26ea90b288ae59935738` |
| `00d69168` table | 44 | `647f1cb3a32c73e34215133240a985a1726f2d034a1f7a1cad561d589e5617ab` |

Lengths are evidence windows; no function-completeness claim is implied by
the constructor window. The path/resolve/exists windows end at their RETs.

| Provider slot | Target | Established role |
|---|---|---|
| +0 | 00bd30e0 | Not recovered here |
| +4 | 00bf4dd0 | Deleting destruction wrapper around00bf4c70 |
| +8 | 00bf4ba0 | Physical file open; prior PHYSICAL_FILE audit |
| +C | 00bf43b0 | Overlapped file open; not ported |
| +10 | 00bf3f70 | Existence, including cached/indexed modes |
| +14 | 00bf47e0 | Enumeration; not ported |
| +18 | 00bf39c0 | Exists then replace name with physical path |
| +1C | 00bf3970 | Construct physical path |
| +20 | 00bf3a80 | Timestamp-related operation; not ported |
| +24 | 00bf0fb0 | Exists then copy original logical suffix |
| +28 | 00bf46b0 | Overlapped completion processing; not ported |

The parent manager audit establishes that its direct-resolution callback uses
provider+24, while its existence callback uses provider+10. Provider+18 is a
different operation: substituting it for+24 would incorrectly return a physical
root path to the virtual mount resolver.

## Root and path construction

Constructor00bf4d30 calls base00bb5590, then installs table00d69168. Base copies
the supplied root string directly to provider+8/+C, initializes reference count
+4=1 and field+10=FFFFFFFF. No slash conversion, separator insertion or
absolute-path expansion appears in that base constructor. The concrete
constructor initializes last-success string+20/+24 empty, takes byte+28 from
its argument and creates an empty tree at+2C/+30 with count+34=0.

`00bf3970`: ECX provider, stack output-string/suffix, RET8 at00bf39ae, EAX output.
It calls004261a0 to concatenate root+suffix, then scans from the root's stored
length to output length, changing '/' to '\\' only in that appended range.
The root is untouched, and no separator is inserted. For example root ending
in no separator does not magically form a child path. Caller configuration
must supply the intended root bytes and desired trailing separator.

## Existence and returned name

`00bf3f70`: ECX provider, one suffix argument, AL result, RET4. Empty stored
length returns false without changing the previous cache. Otherwise00435c40
compares suffix to last-success+20 using equal lengths then CRT `_stricmp`.
A match immediately returns true, even if the file has since disappeared.
Nonzero constructor byte+28 also immediately returns true for nonempty names;
that mode is outside this typed fragment.

With a nonmatching cache, flag false and indexed count zero, provider+1C builds
the physical path. `GetFileAttributesA` at00bf4157 returns success whenever its
result differs from FFFFFFFF. No directory-bit rejection occurs. On success
00bf416b copies the original suffix into last-success; on failure00bf4173
clears it. The fragment retains this positive-name cache instead of inventing
cache invalidation or converting the query to an open/read attempt.

When indexed count is nonzero, the native body performs a separate name-tree
route (00bf3feb..00bf412e). That lookup, enumeration/population and updates are
not reconstructed, and cannot be silently treated as the ordinary OS query.

`00bf0fb0`: ECX provider, stack suffix/output, AL result, RET8. It calls+10 at
00bf0fbb. False leaves output unchanged. True copies the **original suffix**
to output unless they already alias, then returns true. It does not copy
the constructed physical path, cached string or normalized spelling.
`00bf39c0`, separately, calls+10 then+1C and replaces its mutable input with
the physical path on success. Only the+24 behavior is exposed as `resolve`.

## Typed boundaries and validation

The host supplies an immutable root and models only the supported flag/tree
state. `supported()` is false for embedded NUL or root length above INT32_MAX;
method inputs and concatenation length receive the same explicit safety guard.
Valid empty root is allowed. No dot-segment/path canonicalization or sandbox
policy is inferred from native concatenation. String allocations may throw.
The type is stateful because existence updates the cache; it adds no locking
or claim about the native manager's synchronization.

`build_path_00bf3970` is for physical access, while `resolve_00bf0fb0` is for
virtual mount callbacks. A mount layer must retain that distinction and its
actual ordered registration rules. This provider fragment alone establishes
neither `.tga` to `.dds` fallback nor the mounted root for installed assets.

Source and assembly inspection and diff checking were performed here. No
build, new tests, native runtime validation, Ghidra mutation, installed-file
write or shared metadata change was performed; parent owns integration.
