# Native system constant registry

`00b5bf70` constructs the system-constant singleton's ordered vector of 52
20h records. It invokes singleton base `00b5b9e0`, installs vtable D62A3C,
clears vector fields +4/+8/+Ch, constructs each record through `00b5bbc0`
and appends through `00b5bed0`. The record constructor sets runtime field +0
to FFFF, dimension product +4, dimensions +8/+Ch, array count +10h, name
string +14h/+18h and source semantic id +1Ch. Getter `00b5b890` returns the
vector at singleton+4. Runtime name lookup `00b5b960` scans in order using
equal stored string lengths followed by case-insensitive comparison.

All 52 construction argument sets were checked against assembly pushes, including
constant EBX=1 and ESI=0. Names were checked at constructor string references.
The complete constructor body matches original disk and saved Ghidra bytes.
The ordered table and hashes are in `reports/shader_system_registry.json`;
`src/shader_system_registry.inc` preserves names, dimensions, counts and ids.
The typed factory is a constructor fragment: singleton publication, allocator
behavior, native string lifetime and other record runtime fields are omitted.

The saved and original global at E13078 equals 77. Header generation explicitly
annotates constants beginning below c77; later records remain declared but are
compiler assigned. The nominal sum of record register spans is331, which is
not the number of live hardware registers required by every shader.

Key corrections to the earlier diagnostic registry:

| Constant | Native declaration shape | Starting register |
|---|---|---|
| cViewProjMat | float4x4 | c15 |
| cElapsedTime | float4 | c34 |
| cFogColor | float4 | c37 |
| cFogDirColor4 | float4[4] | c38 |
| cAmbientCube | float4[6] | c46 |
| cShadowMapSizeData | float4 | c76 |

The expression cElapsedTime[1] indexes a vector component; it does not imply
an array. The generated debug draw now uses the full native-order table and
cutoff77, uploads identity at c15 and zeros the first77 registers in both stages.
Its center remains FF407FBF and outside sample FF000000. The pixel compile
profile is now ps_3_0 to accommodate the recovered register indices; vertex
remains vs_2_0. This is an explicit probe choice, not recovered native profile
selection. Build, both CTests and full D3D9 probe pass.

Descriptor fields remain projected. Runtime source-semantic dispatch, actual
constant values, profile selection, native registry mutations and material
execution still require reconstruction before game equivalence can be claimed.
