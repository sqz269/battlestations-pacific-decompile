# First installed-asset entry point

Read-only investigation on 2026-09-08 local time / 2026-09-09 UTC. Target verified
before each Ghidra batch: project `bsp`, program `/battlestationspacific.exe`,
`x86:LE:32:default`, base `00400000`, using `config/target.json` and its loopback
backend. No Ghidra annotations or original installation files were changed.

## Recommended vertical slice

Start with `interface/textures/menu_dxt1_2.ats` and its sibling
`menu_dxt1_2.dds` under `I:/SteamLibrary/steamapps/common/Battlestations Pacific`.
The descriptor is 1,034 bytes of plain text, beginning
`TextureAtlas menu_DXT1_2.dds`. It contains seven `TextureItem` blocks with
`Param U1`, `V1`, `U2`, `V2`. The first item is
`interface/textures/fe/achievement/ca_of.tga`, with UV bounds `(0, 0, 0.25, 0.25)`.
This is an accessible concrete UI rectangle; reconstruct parsing and texture
upload, then render that rectangle through the existing D3D9 probe.

The DDS is 524,416 bytes: DDS magic, 124-byte header, width/height 1,024,
FourCC `DXT1`, and 524,288 payload bytes. The mip-count field is zero; the
payload equals exactly one 1,024-square DXT1 level. Interpret this as a
single-level candidate, not a verified native mip policy. Its first item spans
256 by 256 texels under the ordinary normalized-UV interpretation. UV orientation,
edge filtering, actual image appearance and native presentation remain unverified.
Exact local hashes and header summaries are in `reports/asset_inventory.json`.

## Native dependency boundary

Existing export `0057cb60` passes `interface/textures/menu.ats` to `00af0060`.
That literal file is absent in the observed installation. The actual files are
`menu_dxt1_1.ats`, `menu_dxt1_2.ats`, and `menu_dxt5.ats`.
Live export `00af0060` logs `Loading atlas: %s`, checks an `.ats` suffix,
constructs a directory search via `00886280`, filters candidates through
`00aef3c0`, and calls `00aef280`. The family-name expansion explains a plausible
route to the suffixed descriptors, but its exact matching rules are not yet
reconstructed; do not implement a guessed glob as native behavior.

`00aef280` allocates a 0x1c-byte object via constructor `00af5600`, checks
availability via `00bdf4c0`, reports `Atlas file not found: %s` on failure,
loads bytes via `00af5850`, and hands that object to `00aeeaf0` before cleanup.
The immediate bounded follow-up is **decode `00aeeaf0` and its smallest token
parser dependency**, using the seven-item descriptor as a real input. It is not
yet a reconstructed atlas parser.

Assembly-confirmed `00af5850` is a thiscall-style routine: ECX is the buffer
object, one stack filename argument, `RET 4`. It stores the filename string at
object+0x0c, uses global file service `0109ceec`, calls service vtable+4 with
(filename-string, 0x32), file vtable+0x30 to obtain byte count, allocates that
count, then file vtable+0x24 with (buffer, count, out-count). It compares the
returned count with the requested count and decrements the file reference count
at file+4, invoking vtable+0 at zero. Object fields observed are length+0,
cursor+4, extent+8, filename+0x0c, data+0x14, and cleared field+0x18; meanings
beyond those accesses are provisional. The value 0x32 is an open-mode argument,
not the bytes-read initializer suggested by pseudocode. The failure/free block
has missing fallthrough in Ghidra disassembly and an `extraout_EAX` artifact;
inspect raw bytes before reconstructing its failure behavior. Full file-service
ownership and archive/search-path resolution are unresolved.

Local raw exports for `00af0060`, `00aef280`, `00aef3c0`, `00af5850` are in
ignored `exports/bsp/functions/`; pseudocode and assembly are retained. No new
native routine was named or compiled as part of this inventory. Other calling
conventions above remain decompiler suggestions until checked against callers.

## Layout and material alternatives

The installed tree exposes loose `interface`, `models`, `shaderfx`, `terrain`,
`scripts`, `sound`, `movies`, and other directories. No archive extraction is
needed merely to read the selected descriptor and DDS. This does not establish
that the engine never uses archives. The current tree includes user mods,
backups, editors, model dumps and generated image/glTF outputs; counts describe
this installation, not a pristine release. The focused inventory finds 12 ATS
files under interface, 1,384 MMOD files under models, and 260 SHFX files under
shaderfx. MMOD decoding was not investigated here.

`debugshader.mshd` and `pf43cc.mvfm` did not appear in the filename inventory;
treat their executable strings as resource-key clues, not proof of disk formats.
`shaderfx/common/debugshader.shfx` (806 bytes) and
`shaderfx/common/error.shfx` (613 bytes) do exist. Both start with
`DoFile ("shaderfx/dx9_lua.inc")`, establishing text/Lua-style shader descriptors
rather than opaque compiled shaders for those examples. This route additionally
requires shader-script interpretation, effect selection and parameter bindings;
the ATS/DDS rectangle offers a smaller initial data-to-image target.

## Evidence status

Installed-file inventory and selected headers are verified. Native atlas/file
entry points are exported and partially assembly-inspected. Parser semantics,
texture upload, native rendering comparison and complete resource lifetime are
pending. No fixture, build, ABI compatibility or game-validation claim follows
from this investigation. A loader built from an explicit narrow adapter should
remain labelled as an adapter until the corresponding original behavior is
reconstructed and compared.
