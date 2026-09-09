# Texture logical-name resolution boundary

The renderer/texture-manager route does not unconditionally rewrite `.tga` to
`.dds`. `Fonts/white.tga` cannot yet be claimed to select installed
`effects/white.dds`: that would require the actual generic VFS search and
alternative-extension registrations, plus provider priority. A caller-supplied
fixture resolver can make that mapping explicitly, but it is not recovered
native behavior.

## Direct route before image decoding

Verified live `bsp`, `/battlestationspacific.exe`, x86 LE image base00400000
before each batch. Existing `TEXTURE_FILE_LOADING.md` identifies primary
renderer vtable00d5f0a8: slot+64 is00b319b0. Fresh decompilation/assembly of that
routine and its immediate manager dependencies confirms:

1. `00b319b0`, ECX renderer, stack name/flags, RET8, copies the name and applies
   ASCII lowercase004bcc00. It calls00b30b40 on renderer+1A74 with
   name/flags/0/1. Fonts pass flags0.
2. `00b30b40`, ECX manager with four stack arguments, normalizes through
   00bee690 and searches cached names/aliases. Existing normalization converts
   backslashes to slashes and trims byte20 spaces; it does not replace file
   extensions. A cache hit may return a retained existing logical resource
   before any filesystem resolution.
3. On a miss, manager virtual+4 is00b31c20 (manager vtable00d5f088). It takes
   stack output-string/request/flags and RET0C; the inspected body does not
   consume flags. It copies the name and passes that mutable copy to generic
   VFS resolver00bdf4c0.
4. The resolved name is normalized again. The manager may reuse an existing
   resolved-name cache entry and add the original name as an alias. Otherwise
   its virtual+8 is00b2c2d0, receiving the resulting name and flags. That loader
   opens the name with mode2 and uses D3DX image-info/content decoding; it does
   not establish a prior `.tga` extension substitution.

The generic manager has SEH, alias containers and resource bookkeeping that
remain unported. The observations above establish the handoff order, not a
complete cache implementation or inferred extension priority.

## Missing texture fallback is error.tga

Before resolving the first name,00b31c20 checks manager byte+1C and calls
00b31bd0 when clear. That initializer builds `error.tga` in manager string
+14/+18, sets the initialized byte, invokes00bdf4c0 on the fallback string,
then calls00b30b40(fallback,0,1,1), storing its result at manager+20. The VFS
resolution can change the stored fallback name.

When00bdf4c0 fails for the requested name,00b31c20 logs
`ERROR: TEXTURE NOT FOUND:%s` and copies the stored fallback string to output.
There is no special white-image fallback and no direct `effects/` prefix in
these routines. Thus failure to resolve a requested white texture does not by
itself authorize substituting the discovered white DDS as native behavior.

## Where alternative extensions can occur

The coordinated `SHADER_VFS_LOOKUP.md` audit establishes that00bdf4c0 is a
mutating resolver, calling00bddc80 after normalization. That lower resolver
first attempts direct resolution00bdd6e0. After failure it separates name and
extension and consults manager structures rooted at+54/+60, including
extension-keyed candidate lists. Helpers00bddaa0 and00bdc680 build/probe names;
successful candidates overwrite the output string. Several ordered passes
exist, so a global basename dictionary or blanket extension replacement would
discard observed behavior.

This texture audit does not reassign a meaning to untraced registrations in
those structures. The concrete next dependency is their initialization and
the providers traversed through00bdd0a0. Without that state, neither `.dds`
priority over a present `.tga`, basename matching across directories, archive
versus loose priority, nor resolution of `fonts/white.tga` is established.

## Installed white asset and fixture boundary

A read-only recursive enumeration found one loose file whose exact stem is
white and whose extension is TGA or DDS:
`effects/white.dds`,11064 bytes, SHA-256
`d98f8d618cfbb3840198fb54e8065446b5a249760b1a8f4bf2dd47f332d28d95`.
No loose white.tga was found by that enumeration. This says nothing about
entries inside archives or mounted provider state.

Font path construction remains as documented in `FONT_RESOURCE_OWNERSHIP.md`:
missing AlphaTexture defaults to nonempty `white.tga`, yielding prefix+name;
explicit empty AlphaTexture yields unprefixed `white.tga`. Neither path rule
itself supplies `effects/` or a DDS extension. The new FontPhysicalResolver
must expose any fixture mapping explicitly and must not call it native VFS
parity. The task stops at this unresolved registration/provider boundary.

## Byte evidence and scope

Fresh read windows matched the installed executable; lengths are evidence
windows and not claims about complete function extents:

| Start | Bytes | SHA-256 |
|---|---:|---|
| `00b319b0` | 256 | `d345c4989f07d375228266493f4102bd5e11291e16f33cbaa6d3bad00b21400c` |
| `00b31bd0` | 80 | `68a4e268d2dbf7f66d0299c6590cafcca3f9543c1e001ce6578b2629241c5eb2` |
| `00b31c20` | 272 | `0ec45665f343dbfa5c33e8c7178fc0041dd9aca316371268664c7653e2f19fa0` |

Ignored fresh exports are under `exports/bsp/owner_textures/font/` for these
routines and00b30b40. This is analysis evidence only. No source, Ghidra state,
installed file, shared metadata, build or test was changed.
