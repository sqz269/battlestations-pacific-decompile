# Font shader names and descriptor loading

The typed font shader selector and descriptor-name fragment now connect the
font's scale to the installed shader source name. They reconstruct selection
and name rewriting; the native renderer cache, VFS search registrations and
effect object lifecycle remain separate dependencies.

`select_font_shader_name_00ab8ce0_fragment` receives the explicit override,
the signed DWORD read natively from `global0109cf04->+28`, and font scale.
A nonempty override wins. Otherwise the native CVTSI2SS conversion and two
ordered equality gates choose `GuiFont.mshd` only for converted value 720
and scale 1. Every other combination chooses `GuiFontBilinear.mshd`, including
a NaN scale. The intrinsic integer conversion retains caller MXCSR rounding.
The host returns a name; the full ECX-context/RET native routine also skips an
existing cached shader, loads/stores a new one and returns whether selection
was attempted, even when loading returned null. Those cache duties are omitted.

## Corrected material-name route

Fresh assembly resolves the direction and scope of the conversion:

1. Renderer virtual `+48`, `00b318b0`, copies the requested name and applies
   ASCII lowercasing via `004bcc00` at `00b3193c`. This call does not itself
   normalize slashes or trim spaces.
2. Material registry `00b2ebb0` copies that name. At `00b2ecb6..00b2ecce`, it
   invokes `004cad40` with search `.mshd`, replacement `.shfx`, and count
   `7fffffff`. The literal bytes were read at `00d5efc8/00d5efd0`.
3. The helper uses case-sensitive `strstr`, rebuilds prefix + replacement +
   suffix, then resumes searching immediately after the inserted text.
   This replaces every matching substring for supported string lengths,
   including matches inside directory components. It is not an extension-only
   conversion and does not try the `.mshd` file first.
4. `00bdf4c0` receives the changed string at `00b2ed1e`. It normalizes and may
   mutate the name further during VFS resolution. On success that resolved
   string reaches `00b46950` at `00b2ed5a`. The original requested name is
   separately assigned to the effect via `00b18f70`.

The containing registry method has two stack slots and RET 8; the generic
replacement helper has ECX string, three stack arguments and RET 0Ch.
`shader_descriptor_name_00b2ebb0_fragment` implements the fixed-name
replacement portion with a std::string output. It explicitly rejects embedded
NUL and lengths above INT32_MAX, preserving output on rejection. It does not
reimplement generic native substring allocation or malformed string behavior.

The earlier description of a required compiled `.mshd` font artifact was an
unsupported assumption. The observed route rewrites that name to `.shfx`.
Furthermore the installed `shaderfx/common/alphachannel.mshd` is Lua text,
so the extension alone is not evidence of shader bytecode. Generated D3D
bytecode is a later compilation product, not inferred from this suffix.

VFS registration still determines which physical or archived source wins.
`00b46950 -> 00b45ee0 -> 00b43b00 -> 00b69d40` hands the resolved filename to
Lua execution. Combiner filenames resolve separately. See
[SHADER_VFS_LOOKUP.md](SHADER_VFS_LOOKUP.md) for exact mutating lookup and the
remaining provider/search dependencies. The installed nonbilinear source still
uses separate constant names where the native caller registers a pair; the
name rewrite alone does not resolve that version/constant mismatch.

## Evidence and validation boundary

Live project `bsp` and `/battlestationspacific.exe` were verified before each
batch. The full registry body `00b2ebb0..00b2eed3` (exclusive end, 803 bytes)
matches the installed PE, SHA-256
`37f556ad7a43d3d46354018093dd878008b10fba5f01e75bf803a2f09327a725`.
The full replacement helper `004cad40..004caf0f` (463 bytes) also matches,
SHA-256 `d884e3db51927e83f8560ef0c494e7db8f006020eee7a76960904535bbef0f7f`.
See `reports/font_material_resource_audit.json` for these and related ranges.

The font probe selects Arial16 from real Fonts.lua, feeds its scale and an
explicit reference value 720 through selection, then lowercases and rewrites
the selected name to `guifontbilinear.shfx`. It supplies a diagnostic physical
mapping to the installed GUI source; it does not claim native search priority.
Full rendering verification is recorded separately in `FONT_MATERIAL_DRAW.md`.

Reviewed selector/registry/helper and VFS handoff names/comments were applied
in Ghidra with previous values recorded locally, existing comments preserved,
the project saved and affected exports refreshed. Native metadata corrections
do not imply that the complete surrounding methods are reconstructed.
