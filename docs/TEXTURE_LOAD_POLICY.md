# Initial 2D texture dimension and mip policy

Static audit of `00b2c2d0`, with saved `bsp.gpr` and program
`/battlestationspacific.exe` verified before each batch. Saved bytes match the
installed executable for the policy fragment and immediate string helper below.
This documents initial loading, not a generic texture-quality replacement or
runtime validation. No code or Ghidra changes were made.

## Inputs and initial values

After `D3DXGetImageInfoFromFileInMemory` at `00b2c3ef`, the loader takes the 2D
branch only if image-info ResourceType is 3. The API result is not checked before
these fields are read; valid decoded image info is therefore a supported-input
precondition for any typed policy implementation.

At this branch's stack baseline, image-info width is `ESP+48h`, height `+4Ch`,
mip count `+54h`, and format `+58h`. The loader initializes requested width and
height to `FFFFFFFFh`, keeps original mip count in EDI, and separately saves
original width/height for subsequent logical texture construction. It does not
unconditionally pass the original width/height to D3DX.

The controlling DWORD is renderer global `00f8d394`, object offset `+1D84h`.
Call it a mip-reduction setting descriptively; its UI name and runtime allowed
range are not established by this audit. Renderer constructor instruction
`00b32769` writes zero from EBX to this field. This proves an initialization
value, not that the runtime setting remains zero or can only be nonnegative.
No runtime setting is guessed from the diagnostic fixture.

Policy gates at `00b2c438..00b2c455` are:

1. Original mip count must be **unsigned greater than 1** (JBE skips otherwise).
2. Renderer `+1D84h` must be nonzero, using a raw DWORD comparison.
3. None of the filename exemptions below may match.

The setting is read again at `00b2c4d7..00b2c4dd` when arithmetic actually runs.
There is no native snapshot spanning the gates and name checks.

## Exact name predicates

The inspected name is the original native string argument to `00b2c2d0`, kept
in EBP. It is not a basename extracted by this function. The loader copies that
name for opening, but these checks use EBP's original string. Upstream resource
normalization/resolution can alter names before this function is called; it
must not be silently performed again inside this policy.

| Exemption | Native operation | Consequence |
| --- | --- | --- |
| `detail.dds` | Whole C-string case-insensitive comparison via `00449af0`, returning inequality | Only equality suppresses reduction |
| `noseart` | Case-sensitive `strstr` through `00bf9440` | Substring anywhere suppresses reduction |
| `interface/textures/gui/units` | Case-sensitive `strstr` through `00bf9440` | Substring anywhere suppresses reduction |

`00449af0` is ECX left native string, stack right string, AL inequality,
RET 4. If the left stored length is zero, it returns whether the right length
is nonzero. If only the right length is zero, it returns true. Otherwise it
calls CRT `_stricmp` and tests for a nonzero result. **It does not require equal
stored lengths** before comparing nonempty strings. Therefore embedded NUL
terminates comparison even when stored length extends beyond it. This is the
native CRT case-insensitive operation; locale behavior is not replaced with a
guessed Unicode or ASCII-only comparison. A nonzero length with null data is
not safely handled by this helper.

For both substring checks the loader first tests the name data pointer, calls
`strstr`, and tests its result. It then subtracts the name base and compares the
offset with -1. Under ordinary valid C-string storage a found substring's offset
cannot be -1, so any actual match exempts the texture. This is not a prefix
check, separator-aware path check, or extension test. Matching stops at an
embedded NUL and uses the literal lowercase forward-slash strings. No lowercasing
or slash conversion happens in these three predicates themselves.

The distinction matters for an eventual port. For ordinary strings:

- `DETAIL.DDS` equals the whole-name exemption.
- `textures/detail.dds` does not equal `detail.dds` and has neither other token;
  the initial loader may reduce it when the setting/mip gates pass.
- `abc_noseart_suffix.dds` matches the substring exemption.
- `NOSEART.dds` alone does not match the case-sensitive substring exemption.
- `interface\\textures\\gui\\units\\x.dds` does not match the forward-slash token
  inside this routine, although upstream normalization can change the input.

These are direct consequences of the inspected predicates, not runtime-tested
installed filename cases. In particular, the separate disk-reload routine
`00b3fa90` uses `strstr` even for `detail.dds`; it must not be assumed to share
the initial loader's whole-string equality policy.

## Arithmetic and outputs

Only when all gates pass does `00b2c4dd` fetch the setting DWORD Q and compute:

```text
shift = Q & 31
w_bits = original_width  >> shift       // logical SHR, not arithmetic SAR
h_bits = original_height >> shift
m_bits = (original_mip_count - Q) mod 2^32

requested_width  = signed32(w_bits) > 1 ? w_bits : 1
requested_height = signed32(h_bits) > 1 ? h_bits : 1
requested_mips   = signed32(m_bits) > 1 ? m_bits : 1
```

The width/height shift reads CL; x86 masks the count to five bits. The mip
subtraction uses **the entire Q DWORD**, not that masked shift count. Final
comparisons are signed JG. These details matter for out-of-normal-range
settings: Q=32 performs a zero-bit dimension shift but subtracts 32 mips;
Q with its sign bit set is still a nonzero setting and can make signed
subtraction/clamping unusual. No validated native range guard was found in
this fragment, so a typed implementation must either preserve these bit
operations or state its narrower input domain.

For normal positive dimensions and small positive Q, SHR truncates division
by a power of two downward, then clamps to at least one. There is no ceil,
nearest rounding, power-of-two normalization, float calculation or dimension
dependent recomputation of mip count. The reduced dimensions also replace the
separately saved width/height locals used for logical texture construction.

If any gate fails, D3DX requested width and height remain `FFFFFFFFh` and the
original mip count is passed unchanged. A one-mip texture does not enter the
name-check or reduction branch. No-policy dimensions being default sentinels
must remain distinct from the logical construction's saved original dimensions.

After the policy, `00b2c522..00b2c52b` sets requested format to `15h` when image
format is `14h` (R8G8B8 to A8R8G8B8), otherwise 0 (UNKNOWN). The complete 2D
call keeps Usage=0, Pool=1, Filter=`70004h`, MipFilter=`FFFFFFFFh`, ColorKey=0,
null optional image-info/palette pointers. The same policy values are reused
for the loader's single possible retry; it does not recalculate the setting.
See [TEXTURE_STREAM_LIFETIME.md](TEXTURE_STREAM_LIFETIME.md) for argument order
and lifetime through creation/retry.

## Next port boundary and remaining evidence

A bounded policy function can take established image fields, the actual loader
name and a supplied renderer setting, then return requested dimensions/mips,
saved construction dimensions, and requested format. Preserve the default
sentinels and distinguish the initial predicate from disk-reload predicates.
No new test framework is needed; a focused check in the existing probe can
exercise a concrete nonzero quality setting when this policy is integrated.

The setting's mutators, allowed configuration range, synchronization with its
second read, exact upstream name chosen by VFS/cache aliases, and malformed
image-info behavior remain unresolved. Current static evidence does not prove
that the game uses a particular quality setting or that all texture loads
receive already lowercase normalized names.

## Byte identity

Ends are exclusive. The first and third rows are instruction fragments, not
whole reconstructed routines. The middle row covers the complete string helper.

| Start | End | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `00b2c405` | `00b2c530` | 299 | `3942cf531e1e2263796abd98d223cfbeaedf0d8353308f73e0481927038bb6fd` |
| `00449af0` | `00449b3d` | 77 | `ef9d50ee8e4b76e6e6b1472f141e554a8100381cc862bfeada4ed3903ae354f0` |
| `00b32769` | `00b3276f` | 6 | `9dcba8b7762f80c5a2873f72b95f3ffd23f4bc6d68c329eed9c89a824a4af1c9` |

All three matched saved Ghidra and installed PE bytes. Literal bytes at
`00d5e7f0`, `00d5e7e8`, and `00d5e7c8` were read directly for the three strings.
Detailed record: ignored `exports/bsp/owner_textures/vfs/texture_policy_evidence.json`.
