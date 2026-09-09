# Initial 2D texture policy implementation

`texture_load_policy.hpp/.cpp` implements the name/quality selection fragment
of initial loader `00b2c2d0`, beginning at `00b2c405`. The native target was
reverified as `bsp.gpr`, `/battlestationspacific.exe` before implementation.
Assembly, literal reads and installed/saved byte hashes are documented in
[TEXTURE_LOAD_POLICY.md](TEXTURE_LOAD_POLICY.md).

The input is a native string view `{stored length, C-string pointer}`, decoded
width/height/mip DWORDs and a supplied stable renderer quality DWORD. Output
contains requested width/height/mips and separately saved width/height. The
caller must supply successful image-info results; this unit does not fabricate
image fields after a failed D3DX decode.

Requested dimensions begin at `FFFFFFFFh` while saved dimensions begin at the
original image values. Original mip count is preserved unless unsigned mip
count exceeds one, the setting is nonzero, and no name exemption applies.

The `detail.dds` exemption ports `00449af0` with the constant right-hand string:
a zero stored name length means unequal without reading name data; otherwise
native CRT `_stricmp` compares the NUL-terminated strings. Stored lengths need
not match. The two remaining exemptions use case-sensitive C-string searches
for `noseart` and `interface/textures/gui/units`, in that order. Exemption
checks short-circuit in native order. No basename extraction, slash conversion,
lowercasing or path normalization is introduced.

A length-zero view may still have non-null data: equality skips it, but the
subsequent native substring predicates can inspect it. A nonzero length and
null data causes false with unchanged output when name checks execute, rather
than a native invalid dereference. A non-null pointer must refer to readable
NUL-terminated storage; native stored length does not bound these C-string
operations. Bytes after the first NUL are ignored. CRT locale behavior is
inherited from `_stricmp`, not replaced by an invented Unicode comparison.

Reduction uses a logical shift by `setting & 31` for both dimensions, wrapping
DWORD subtraction of the **full** setting from mip count, then signed-DWORD
greater-than-one clamps. The implementation tests the unsigned bit pattern
against `(1,80000000h)` to express that signed condition without relying on
implementation-defined unsigned-to-signed conversion. Reduced dimensions also
replace saved dimensions. No float rounding, ceiling division or mip count
recomputation is used; all setting DWORD bit patterns are represented.

The stable-setting parameter supplies both native reads of renderer `+1D84h`.
Native reads once before name checks and again before arithmetic. Concurrent
changes between those reads are outside this typed function; the interface
does not claim to reproduce a renderer data race. The setting's initialization
is zero in native renderer construction, but this function does not assume
that runtime value or constrain it to a guessed configuration range.

This is specifically initial-load policy. Disk reload `00b3fa90` tests
`detail.dds` using a substring operation instead, so the function must not be
reused for disk reload without a separately recovered policy. Format conversion
and the actual D3DX call remain in the surrounding texture creation code; this
unit returns dimensions/mips only. VFS selection, quality mutators, COM lifetime,
reset orchestration and the texture manager's cache are not implemented here.

Native ABI remains the containing loader's stack name/callback and RET 8;
there is no original standalone function at `00b2c405`. The typed bool-returning
interface is an extracted reconstruction fragment, not a binary replacement.
The embedded string helper `00449af0` has ECX left native string, stack right
string, AL inequality, RET 4. Existing evidence covers policy bytes
`00b2c405..00b2c530` (299 bytes, SHA-256
`3942cf531e1e2263796abd98d223cfbeaedf0d8353308f73e0481927038bb6fd`)
and full helper `00449af0..00449b3d` (77 bytes, SHA-256
`ef9d50ee8e4b76e6e6b1472f141e554a8100381cc862bfeada4ed3903ae354f0`).
Both ranges match the installed executable and saved image.

Parent integration added `D3D9RetainedTexture2D::initialize_00b2c2d0_fragment`:
create using selected request dimensions/mips, then retain the supplied memory
stream if COM output exists. Construction metadata remains separate for later
retained recreation. This is a partial initial load: the native retry, optional
callback, registry/cache and logical wrapper allocation remain unported. Host
input completeness and HRESULT reporting are explicit differences.

The existing DDS probe now calls the installed D3DX image-info API using the
official SDK declaration, supplies an explicit diagnostic setting0, and checks
the default FFFFFFFF dimension requests. Initial creation, source retention,
recreation after local-owner release and complete DXT1 payload comparison pass.
The one-mip fixture does not runtime-test exemptions or quality reductions;
those remain assembly/source-backed. The Win32 build, both CTests and full
D3D9 probe pass without a new test target. See
`reports/font_registry_texture_policy_probe.txt`. Native loader evidence comments
were preserved/extended, saved and exports refreshed.
