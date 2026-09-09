# Texture stream lifetime and D3DX handoff

Read-only audit of `bsp.gpr`, `/battlestationspacific.exe`; target verified
before each analysis batch. Uses the established
[VFS route](TEXTURE_VFS_HANDOFF.md) and
[memory backing semantics](MEMORY_STREAM_HANDOFF.md), without reimplementing
them. Exports and complete-body byte comparisons are under ignored
`exports/bsp/owner_textures/vfs/`.

The central result is that successful native texture loading retains the memory
stream in the logical texture object. Its backing must survive beyond the
synchronous D3DX call: native retained-source recreation reads it later.

## Initial loader and source lifetime

`00b2c2d0` is the texture manager's load virtual at manager table `00d5f088 +8`.
It takes stack native name-string reference and optional callback, returns a
logical texture pointer in EAX, RET 8. Its body does not use the incoming ECX
manager receiver. Do not infer arguments from the unstable decompiler output:
assembly establishes the second stack argument's conditional callback at
`00b2c629..00b2c636` (ECX newly created 2D texture).

The VFS open at `00b2c345` passes a copied filename and mode 2 to global
`0109ceec`, virtual `+4`. It checks returned stream virtual `+18h`; there is no
null guard before that call. On false validity, `00b2c35f` calls stream virtual
`+4` with flag 1 directly, frees the copied name, and returns null. This is
different from the reference-counted successful path.

At `00b2c39f`, ECX is the valid opened stream and `00bef750` returns a new
memory wrapper in EAX. The loader keeps it in ESI and stack local `+20h` (at the
post-call stack baseline). At `00b2c3ae` it decrements the original stream's
reference count, invoking its final-release virtual only on transition to zero.
Therefore a physical file handle can close before D3DX sees the bytes; a shared
source-memory backing remains alive through the newly retained wrapper.

The image-info call at `00b2c3ef` is:

```cpp
D3DXGetImageInfoFromFileInMemory(
    memory.backing.data, low32(memory.size()), &image_info);
```

The caller obtains size by virtual `+30h` at `00b2c3e4` and base data via
`00bef610` at `00b2c3e9`. It does not use the stream cursor. The HRESULT is
ignored: the next instruction reads `image_info.ResourceType` from the stack.
That output structure is not first cleared by this function. A failed decode
therefore is not a proven safe null-return path. Successful resource types 3,
5 and 4 select 2D, cube and volume texture creation respectively; unsupported
types skip creation and release the local memory wrapper.

## Exact D3DX argument order

The current local official SDK header
`build/win32/_deps/dxsdk_d3dx-src/build/native/include/d3dx9tex.h` establishes
the WINAPI parameter order; the original PUSH sequence supplies these values.
No interpretation of decompiler stack-variable names is needed.

For 2D creation at `00b2c565` (and retry at `00b2c5cf`):

| Parameter | Original supplied value |
| --- | --- |
| `pDevice` | Saved renderer `+1A10h` device |
| `pSrcData` | Retained wrapper's backing base from `00bef610` |
| `SrcDataSize` | Low DWORD of wrapper virtual `+30h` result |
| `Width` | `FFFFFFFFh` default, or quality-adjusted width |
| `Height` | `FFFFFFFFh` default, or quality-adjusted height |
| `MipLevels` | Image-info mip count, or quality-adjusted count |
| `Usage` | 0 |
| `Format` | `15h` if source format is `14h`, otherwise 0 (UNKNOWN) |
| `Pool` | 1 (MANAGED) |
| `Filter` | `00070004h` |
| `MipFilter` | `FFFFFFFFh` |
| `ColorKey` | 0 |
| `pSrcInfo` | null |
| `pPalette` | null |
| `ppTexture` | Address of initially null local COM pointer |

The format conversion is R8G8B8 to A8R8G8B8. Quality/name policy is part of the
native loader, not a VFS operation: mip counts above one and renderer quality
settings enter the name-exception branch before dimension/mip reduction.
Detailed `detail.dds`/`noseart`/GUI-unit matching remains in the native evidence;
this handoff does not replace that policy with guessed defaults.

Cube creation at `00b2c6a6` and retry `00b2c6f3` calls
`D3DXCreateCubeTextureFromFileInMemory(device, base, low_size, &local_texture)`.
Volume creation at `00b2c789` and retry `00b2c7ca` uses the identical four-argument
order with `D3DXCreateVolumeTextureFromFileInMemory`. Both use the same memory
wrapper throughout their creation attempts.

All three branches enter/leave the optional renderer guard when global
`0108d6dc` is nonzero. If the initial output pointer is null, a nonzero HRESULT
other than `8876017Ch` or `8007000Eh` triggers `00b29670` and one retry. A zero
HRESULT with null output does not retry. After retry, output-pointer presence
controls logical wrapper construction. This is not a general retry-until-success
loop. The same saved source backing and saved device argument are reused.
Complete device-recreation side effects are outside this handoff.

## Retain before local release

Successful D3DX output is wrapped by `00b3f930` (2D), `00b3ced0` (cube), or
`00b3cfa0` (volume). Their constructors initialize the future retained-stream
field to zero. The loader then calls `00b23640` to assign the local memory
wrapper into the texture:

| Texture kind | Retained field | Assignment call | Texture vtable |
| --- | --- | --- | --- |
| 2D | `+4Ch` | `00b2c624` | `00d61948` |
| Cube | `+2Ch` | Common call `00b2c80e`, destination formed at `00b2c730` | `00d61870` |
| Volume | `+30h` | Common call `00b2c80e`, destination formed at `00b2c807` | `00d618b0` |

`00b23640` has ECX destination pointer slot, EDX source pointer slot, EAX
destination, plain RET. Identity-equal assignment does nothing. Otherwise it
stores the new pointer, increments its count if non-null, then decrements and
conditionally destroys the old pointer. This is native logical-object reference
counting, not COM AddRef on the D3D texture.

Only after the creation branch and optional guard exit does `00b2c833` release
the loader's local wrapper reference (InterlockedDecrement call `00b2c837`).
For success, the logical texture still owns the wrapper and backing. For failed
creation without another owner, this release destroys the wrapper/backing.
The copied filename is freed after that release. A successful 2D path also
sets mip count `texture+3Ch`, invokes the optional callback, and stores low file
size in `texture+24h` before exiting.

Normal-path reference arithmetic after conversion is wrapper count 1 locally,
2 after assignment to texture, then 1 after local release. Backing count is
independent; conversion either shares an existing backing or creates a new one.
Do not release the texture's retained wrapper immediately after D3DX creation.
Allocation-failure paths dereference newly allocated null logical wrappers in
the native body; full failure/SEH parity is not established.

## Retained recreation versus disk reload

The logical texture vtables expose distinct routes:

| Kind | Virtual `+08h`: re-open name through VFS | Virtual `+2Ch`: use retained stream |
| --- | --- | --- |
| 2D | `00b3fa90`, conversion call `00b3faf0` | `00b3e190` |
| Cube | `00b3df40`, conversion call `00b3df7b` | `00b3e1f0` |
| Volume | `00b3e080`, conversion call `00b3e0bb` | `00b3e230` |

Each disk route opens the native name at texture `+8` with mode 2, validates,
converts, releases the original stream, calls D3DX using the local conversion,
and releases that local conversion afterward. The inspected bodies do not
replace the texture's retained-source field. They also release the prior COM
texture before assigning a newly loaded one. Thus a disk reload must not be
assumed to update the bytes subsequently used by retained recreation.

The three retained routes take ECX logical texture, no stack arguments, plain
RET. They read their retained field directly, get size/base, and create into
texture `+10h`; they do not call VFS or `00bef750`, and do not first release an
existing COM pointer in these bodies. Their caller must have arranged the
appropriate device/texture state. 2D `00b3e190` is already named and reconstructed
in this repository; this audit confirms its input ownership dependency.
Cube/volume retained recreation use the simple four-argument D3DX APIs. The
retained methods establish reset-capable source storage, but the complete
device-reset caller sequence is not audited here.

## Texture destruction releases the retained source

The texture deleting-destructor slots route through `00b3f590 -> 00b3f2e0`
(2D), `00b3f410 -> 00b3ead0` (cube), and `00b3f430 -> 00b3eb70` (volume).
Destructor bodies use ECX texture with no stack arguments. Every retained
stream release decrements `stream+4`, calls stream virtual slot zero only on
transition to zero, and clears the texture field after that call returns.

- 2D releases `+4Ch` before renderer notification, COM texture cleanup, cached
  surface releases and base destruction.
- Cube releases `+2Ch` before renderer notification, COM texture release and
  base destruction.
- Volume performs renderer notification first, then releases `+30h`, then COM
  texture cleanup and base destruction.

The 2D destructor's Ghidra export stops after free at `00b3f3e9`, but raw bytes
continue through base destructor `00b33f50`, SEH restoration and RET at
`00b3f40d`. The verified full body ends at `00b3f40e`. No Ghidra repair was made
in this read-only task. Logical-object allocator/pool teardown and all renderer
notification semantics remain unported dependencies of the full lifecycle.

## Smallest integrated boundary

Connect the physical read to the real shared-backing conversion, supply its
base/size to the existing 2D retained-memory creation routine, and retain the
memory wrapper in the typed logical texture until its release. Existing probe
checks can compare installed DDS bytes, create the texture, release the caller's
source owner, then recreate from the texture-held backing. This directly checks
the newly established lifetime instead of treating a temporary host byte vector
as equivalent to native source ownership.

Keep VFS mount selection, archive decoding, texture-quality/name policy,
resource-manager cache aliases, error callbacks, full reset dispatch and logical
allocation as explicit integration boundaries. Native conversion reads once,
ignores short reads, and can leave uninitialized bytes while exposing the full
size; the texture caller adds no corrective check. A host rejection of those
paths is an explicit failure boundary, not proven native behavior.

## Complete-body evidence

The following saved-program ranges matched installed PE bytes. End addresses
are exclusive. These checks establish static identity, not game validation.

| Start | End | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `00b2c2d0` | `00b2c897` | 1479 | `b9cde0abf2b8b2efd6bda382f436c26ce66aa1b19e73350e6a48030f5ad221b3` |
| `00b23640` | `00b2367b` | 59 | `2e77ba6c60539766c2d8e8bedbc7c2eb2be64e3dc3b59b958ec6274373ae7004` |
| `00b3f2e0` | `00b3f40e` | 302 | `2f88788d59d33ebd6a3d1d0c09c597df7ecf7398a8d9dd928cb921d9a35825b1` |
| `00b3ead0` | `00b3eb6f` | 159 | `7a83246a860603f4ec73a4d467f5fa725b4e2edf02e13b131a60cc0ac81f084d` |
| `00b3eb70` | `00b3ec24` | 180 | `d803fe1bd7dab559dd109905a70885730fae685399c5fef2b9befb265dd3ce96` |
| `00b3e190` | `00b3e1ed` | 93 | `2add77ebc7813add3ebd37bb5a7a41cfb509c8969e3c62d3219bcd2ecfdee7d5` |
| `00b3e1f0` | `00b3e222` | 50 | `208b212b16ee653c404ceff97d44d8b8084453c19cc2558ace819dc69fdf7a15` |
| `00b3e230` | `00b3e262` | 50 | `fd8c71f1d35f3b1c2f4ad1774947d0183d070acdde30a52517443266ae6309f0` |
