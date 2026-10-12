# Caller-less shader and texture functions in 00B3-00B4 (cc13-stlreview2)

Packet cc13-stlreview2 part 2, 2026-10-12. The question came from `docs/CC13_BAND_0090_00BF.md` (open
question 2): about 190 tiny caller-less functions in 00B33E90-00B3FFB0 and 00B4xxxx with no RTTI,
assumed to be shader/texture vtable slots.

Status vocabulary: names and Ghidra annotations from static evidence (disk image bytes, live Ghidra
listing). Nothing here is reconstructed, build-tested or run. Every name is a hypothesis.

## 1. The premise was mostly wrong: these are not vtable slots

The set is every function in 00B30000-00B4FFFF that had neither a ledger name nor a tag after
cc13-sweep9x: 191 functions (`local/s2_vt.py` in the worker tree). Each entry address was searched
in the whole disk image three ways:

- every 4-byte value at any alignment, in every section (`local/s2_scan.py`);
- every `E8`/`E9` rel32 operand in `.text` (`local/s2_rel.py`);
- `.rdata` runs of code pointers (vtables) that contain the address (`local/s2_vt.py`).

| result | functions |
| --- | --- |
| no reference of any kind | 179 |
| pointer in an `.rdata` vtable (real virtual slots) | 7 |
| reached only from other unreferenced code (`00B489E0`, and the undefined code at `00B405E0` and `00B48460`, section 4) | 2 (`00B40440`, `00B483A0`) |
| only coincidental 4-byte matches | 3 (`00B40220` in `.rsrc`; `00B40520` inside the instruction bytes at `00ACCA61`; `00B40900` in the flag table at `00D64C34`) |

So 182 of the 191 are
out-of-line copies that nothing reaches: inline constructors, accessors and container helpers that
the compiler emitted as functions and the linker kept. They sit between the class's named functions
in the same order as the source, which is how most of them were attributed below.

Two of the 191 are not even real entries (section 4).

## 2. What was named

169 names, added with `ledger add-name` and applied with `ghidra_annotate.py --apply` under the
write lock (logs `local/ghidra-annotations-20261012T144352Z.json` and `...T144642Z.json` in the worker
tree). The 160 unreachable ones carry the suffix `_Unreferenced`, following the cc13-sweep9x
precedent (`BSP_PointerArrayMinTen_PushBack_Unreferenced`). The evidence record of each states the
negative scan and the body.

### 2.1 Real virtual slots (7)

| address | name | vtable and slot | body |
| --- | --- | --- | --- |
| `00B3CD40` | `BSP_D3D9Surface_LockRect` | `00D619B0` +1Ch (surface: +4 `InitializeBinding`, +0Ch `GetWidth`, +28h `LoadFromSurface`) | `IDirect3DSurface9` `[this+2Ch]`, COM +34h `LockRect` |
| `00B3CD60` | `BSP_D3D9Surface_UnlockRect` | `00D619B0` +20h | COM +38h `UnlockRect` |
| `00B3DD00` | `BSP_D3D9Surface_SaveToFile` | `00D619B0` +24h | `D3DXSaveSurfaceToFileA(name, format, [this+2Ch], NULL, NULL)` (import, not a static D3DX copy) |
| `00B3D060` | `BSP_VolumeTexture_UnlockBox` | `00D618D0` +14h, after `BSP_VolumeTexture_LockBox` | `IDirect3DVolumeTexture9` `[this+10h]`, COM +50h `UnlockBox(level)` |
| `00B3D590` | `BSP_D3D9Texture2D_LockLevel` | `00D6197C`, before `BSP_D3D9Texture2D_UnlockLevel` | `IDirect3DTexture9` `[this+10h]`, COM +4Ch `LockRect(level, ...)` |
| `00B33E90` | `BSP_LogicalTextureBase_GetFlagBit25` | seven texture vtables (`00D5F20C`, `00D5F240`, `00D5F298`, `00D5F2D8`, `00D61888`, `00D618C8`, `00D61960`) | bit 25 of `[this+1Ch]` |
| `00B45330` | `BSP_MaterialEffect_ApplyInstanceGeneratorModesToSections` | `00D61A00` +18h (MaterialEffect; +14h `DispatchEntry`) | if byte `[this+0B4h]`, `BSP_MaterialSection_ApplyInstanceGeneratorModes` on 14 section pointers at `+0C8h` |

The COM slot numbers are the D3D9 SDK interface orders (resource 0Ch-28h, base texture 2Ch-40h,
texture/volume/cube 44h-54h, surface `LockRect` 34h / `UnlockRect` 38h).

### 2.2 Slab-pool members (43)

Eight fixed-size slab pools of 32 slots each keep a free-count word and a word stack of free
indices after the slots. Each has the same five out-of-line members; the slab layout (slot size,
count offset) ties each copy to the pool whose named functions sit beside it:

| slab | slot size | free count / stack | Pop, Push, HasNoFreeSlot | StampSlabIndex (field), AllSlotsFree |
| --- | --- | --- | --- | --- |
| `BSP_D3D9SurfaceSlab` | 38h | +740h / +700h | `00B3D160`, `00B3D190`, `00B3D1D0` | `00B3E340` (+34h), `00B3E360` |
| `BSP_D3D9Texture2DSlab` | 54h | +0AC0h / +0A80h | `00B3D220`, `00B3D250`, `00B3D290` | `00B3E4C0` (+50h), `00B3E4E0` |
| `BSP_CubeTextureSlab` | 34h | +6C0h / +680h | `00B3D2E0`, `00B3D310`, `00B3D350` | `00B3E640` (+30h), `00B3E660` |
| `BSP_MaterialPassSlab` | 8Ch | +11C0h / +1180h | `00B40220`, `00B40250`, `00B40290` | (mis-split, section 4), `00B40A30` |
| `BSP_NativeVertexDeclarationSlab` | 0D4h | +1AC0h / +1A80h | `00B476D0`, `00B47700`, `00B47740` | (mis-split, section 4), `00B47EA0` |
| `BSP_HardwareLayoutSlab` | 44h | +8C0h / +880h | `00B47790`, `00B477C0`, `00B47800` | `00B48000` (+40h), `00B48020` |
| `BSP_NativePhysicalBufferSlab` | 30h | +640h / +600h | `00B48E30`, `00B48E60`, `00B48EA0` | `00B4A220` (+2Ch), `00B4A240` |
| `BSP_VertexStreamSlab` | 78h | +0F40h / +0F00h | `00B48EF0`, `00B48F20`, `00B48F60` | `00B4A3A0` (+74h), `00B4A3C0` |
| `BSP_LogicalIndexSlab` | 28h | +540h / +500h | `00B48FB0`, `00B48FE0`, `00B49020` | `00B4A520` (+24h), `00B4A540` |

- `PopFreeSlot`: `--count`, index from the stack, return `this + index*size`.
- `PushFreeSlot`: index = `(slot - this) / size` by a magic multiply, push, `++count`.
- `StampSlabIndex(n)`: writes `n` into the last dword of each slot. `D3D9SurfacePool_ReturnRawSlot`
  reads `slot+34h` as the slab index, which fixes the meaning.
- `HasNoFreeSlot` is `count == 0`; `AllSlotsFree` is `count == 20h`. The existing
  `BSP_FixedSlotPool64_IsFull` (`00410B10`, `count == 40h`) is the second test under another name,
  so "full" there means "all slots free". That name was left alone.

### 2.3 Constructors, assignment and destructors (25)

- Registry and cache bases stored inline by `BSP_D3D9Renderer_Construct`/`_Destroy`:
  `00B31D00` `BSP_DeclarationRegistryBase_Construct` (vtable `00D5F024`, pure slots +4/+8),
  `00B31D60` `BSP_NativeTextureCacheBase_Construct` (`00D5F038`), `00B31FD0`
  `BSP_MaterialEffectCacheBase_Construct` (`00D5F04C`), `00B321B0`
  `BSP_NativeDeclarationRegistry_Construct` (`00D5F060`), `00B321E0`
  `BSP_MaterialEffectRegistry_Construct` (`00D5F074`), `00B32340`
  `BSP_NativeTextureResourceCache_Construct` (`00D5F088`, the TextureManager slots).
- RefCounted-derived (base vtable `00CEB130`, refcount 1, then the class vtable):
  `00B33EC0` LogicalTextureBase (`00D5F1F4`), `00B3B190` NativeShaderReflectionOwner (`00D61810`),
  `00B40520` NativeInstanceGeneratorBinding (`00D619F8`, id from counter `0108FD30`), `00B41FD0`
  MaterialRenderStates (`00D61A2C`), `00B42020` MaterialSamplerStates (`00D61A34`), `00B42070`
  MaterialThirdStates (`00D61A3C`). Three vtables are stored only by their unreferenced
  constructor, so the class has no other anchor and is named after the vtable:
  `00B3DC50` `BSP_RefCountedD618F8_Construct`, `00B49430` `BSP_RefCountedD61D24_Construct`,
  `00B49B10` `BSP_LogicalStreamD61D30_Construct` (slot +2Ch is `BSP_LogicalStream_PostUploadNoOp`).
- `operator=` of the three material state blocks (`00B42000`, `00B42050`, `00B420A0`: copy +4,
  then `BSP_MaterialRenderRows_Copy` on +8, vtable untouched).
- Destructor bodies that restore a vtable and jump to the base: `00B3C4E0`
  (`BSP_RenderSingleton0108FED8`, `00D61844`), `00B45090` (generic instance generator, `00D61BFC`),
  `00B45190` (building instance generator, `00D61C1C`).
- Vertex layout records: `00B478B0` `BSP_VertexElement_ConstructUnused` (type 11h,
  `D3DDECLTYPE_UNUSED`), `00B48430` (four 12-byte `{RefPtr, dword, dword}` records through
  `eh_vector_constructor_iterator`), `00B483A0` `BSP_HardwareLayoutRecord_Assign`, `00B489E0`
  `_CopyConstruct`.

### 2.4 Accessors and helpers (48)

- Shader source text, strings from the disk image: `00B38210`
  `BSP_ShaderBuilder_AppendNullShadowFadeOut` (`float4 GetShadowFadeOut(...) { return float4(0,0,0,0);`),
  `00B38220` `_AppendAmbientHelper` (`float4 GetAmbient(float3 i_Norm)`), `00B38290`
  `_AppendAmbientAndSrgbHelpers` (adds `inline float4 tex2D_sRGB(sampler2D Tex, float2 UV)`), and
  the byte-identical pair `00B39070`/`00B390C0` `_AppendSystemConstantPair` (`'\n%s\n%s'`).
- Texture and surface COM wrappers without a vtable slot: `00B3D6D0`
  `BSP_CubeTexture_LockFace` (COM +4Ch, 5 arguments), `00B3D700` `_UnlockFace` (+50h).
- Byte maps of the shader reflection owner: `00B343C0`, `00B34570`, `00B3FF70`, `00B3FF80` read and
  write `[this+8+i]`, `[this+3Eh+i]` and `[this+74h]`, the fields the `00B3B190` constructor sets
  (`memset(this+8, FFh, 36h)`). The class is from the layout match: provisional.
- `00B341D0` `BSP_LogicalTextureBase_SetName` (NativeString at `+8`, which `GetNameAddress` returns),
  `00B34860` `BSP_ShaderDescriptor_FindFirstFlaggedSampler` (array `+0C4h`/`+0C8h`, the count that
  `BSP_ShaderDescriptor_FindSamplerByName` walks), `00B400B0` `BSP_MaterialEffect_GetByte13C` and
  `00B40840` `_GetIndexedFloat` (the fields of the named setters beside them), `00B3C570`
  `BSP_CockpitHelper_SetCameraViewMatrix`, `00B47C80` `BSP_VertexDeclaration_ElementAt`, `00B478E0`
  `BSP_VertexElement_GetTypeSize` (the D3DDECLTYPE size table `00D61CC0`: 4, 8, 12, 16, 4, ...),
  `00B47C10` `BSP_HardwareLayout_ReturnRawSlot` (pool global `0108FD70`), `00B4A970`
  `BSP_PointerArray_RemoveAtSwapLast`, `00B4B2C0` `BSP_NativePhysicalBufferPointerArray_Destroy`,
  `00B3A6D0` `BSP_NativeShaderConstantArray_Clear` (Resize(0)), `00B34D60`/`00B34E00` pooled
  string construct and release, `00B347D0` NativeString c_str-or-empty (`0108D6F2`).
- Float helpers: `00B3D0D0` Vector4 construct, `00B40400`/`00B40440` 2x4 and 3x4 transposed copies of
  a 4x4 matrix, `00B405D0`/`00B406B0`/`00B406E0`/`00B40710` a float4 block at `this+4` (At,
  SetFloat4, SetDword4, SetScalar).
- Smart pointer assignment: eight intrusive `BSP_RefPtr_Assign` copies (Interlocked refcount at +4,
  vtable[0] on zero), three `BSP_ComPtr_Assign` copies (`AddRef`/`Release`), and three member
  setters (`00B4AAC0`, `00B4AB00`, `00B4AB40`).

### 2.5 Engine array template members (46)

The `{data, count, capacity}` array the engine uses everywhere: 20 `TMPL_PodArray_Construct`, 14
`TMPL_PodArray_At<size>` (2, 4, 8, 12 and 96-byte elements), 2 `TMPL_PodArray32_End`, three growth
rules `TMPL_PodArray_GrowCapacityMin10/16/6` (`max(n+5,10)`, `max(n+8,16)`, `max(n+4,6)`; ECX is a
register input), and inline-storage variants (`TMPL_InlineArray_At*`, `TMPL_InlineArray32_PushBack`
with count at +80h, `TMPL_InlineArray4_PushBack` with count at +10h). These carry the address in the
name because the element class is not recoverable from an out-of-line copy. They are engine code,
not Dinkumware, so they were named rather than tagged as library.

## 3. Left unnamed (22)

| address | body | why not named |
| --- | --- | --- |
| `00B34320` | byte `[this+0Ch]` | no class anchor |
| `00B34380`, `00B34390` | 2-byte pair ctor (`FFh,FFh` / two args) | no class anchor |
| `00B34400` | three calls through `[this+8]` vtable +64h/+54h/+28h, gated by byte `0108D6F1`, `++[this+4]` | interface not identified (not `IDirect3DDevice9` or `ID3DXEffect` by slot count) |
| `00B34560`, `00B347C0` | count and At(i) of the member array at `[this+24h]` | owner class unclear |
| `00B3D5E0` | returns a copy of a float4 argument | purpose unclear |
| `00B3FFA0`, `00B3FFB0`, `00B40040`, `00B40050` | getters `+18h`, `+0Eh`, `+1F8h`, `+144h+i*40h` | class unclear; `+144h` matrices conflict with MaterialEffect's float array at `+140h` |
| `00B40120`, `00B403F0`, `00B40580` | `[this+10h]=0`; copy `+4`; `[this+50h]!=0` | too small to place |
| `00B40750`, `00B40780` | copy vector3 from `+390h` / `+38h` | owner unclear |
| `00B40FF0`, `00B41010` | entry `+20h` / entry dword of a 60h-byte array at `[this+184h]` | owner unclear |
| `00B436C0` | ctor: two zero dwords, 14 `-1` pairs at `+14h`/`+4Ch` | precedes `BSP_ShaderDescriptor_Construct`; sub-object not identified |
| `00B49170` | `(ECX & F000h) == 1000h` | flag meaning unknown |
| `00B40A20`, `00B47E90` | loop tails | not function entries (section 4) |

## 4. Listing problems found

- **Two mis-split functions.** Ghidra starts `FUN_00B40A20` and `FUN_00B47E90` inside a loop. The real
  entries are `00B40A10` and `00B47E80` (`mov edx,[esp+4]; lea eax,[ecx+88h|0D0h]; mov ecx,20h`),
  which belong to no function. They are the `StampSlabIndex` members of the MaterialPass and
  NativeVertexDeclaration slabs. Fix: create the functions at the real entries and remove the
  fragments; not done here (listing repair is outside this packet).
- **Undefined code islands.** `00B405E0` (a float4-block `SetMatrixRowsAt(i, rows, matrix)` that
  dispatches to `00B404A0`, `00B40440` or an inline 2-row copy) and `00B48460` (push a record into the
  `00B48430` quad and tail-jump to the assign) are code with no Ghidra function. Both are themselves
  unreferenced. `FUN_00B405D0`'s body stops after four instructions, before `00B405E0`.

## 5. Checks

- `_free` `00BF9DC8` read `void __cdecl _free(void * _Memory)` before and after both applies;
  'Non-Returning Functions - Discovered' was not touched.
- No name collides with an existing ledger or Ghidra name (checked against the index).
- `tools/verify_report_calls.py` does not apply: this doc has no call-site report.

## 6. Open questions

1. Whether the engine was built without function-level linking for these objects, or whether these
   copies are COMDATs that `/OPT:REF` should have removed, is not settled. It does not change the
   names.
2. The interface behind `00B34400`'s `[this+8]`.
3. Repair the two mis-splits and define the two code islands (section 4), then re-run the census.
