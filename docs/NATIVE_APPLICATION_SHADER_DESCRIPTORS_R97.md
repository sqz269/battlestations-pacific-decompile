# Shared application shader descriptors (R97)

The renderer now exposes full `00B43B00` descriptor parsing through its existing
Lua bootstrap/DoFile, VFS, pooled strings, published state definitions and canonical
`0108FEE4` state-list pool. The focused application probe parsed and retired all
179 entries selected from the installed shader preload script. Ordinary startup
does not yet execute the full `0073BF80` material preload/compiler loop.

## Native contracts and source boundary

| Address | Original ABI | Change or exercised behavior |
| --- | --- | --- |
| `00B573F0` | unused ECX, stack Lua table, EAX allocated 1Ch field, RET4 | Track actual writes to type, count and semantic; require explicit native preimages only when an ordinal leaves one unwritten. |
| `00B419B0` | unused ECX, Shader/header/string arguments, RET0C | Existing outer append traversal can select the written-ordinal adapter without a callback. |
| `00B437F0` | unused ECX, string slots/Lua entry, RET8 | Require an actual ordinal-zero mode write before selecting a slot, or use the existing explicit native mode input. |
| `00B439C0` | unused descriptor ECX, slots/Shader/string, RET0C | Existing outer integral-key filter selects the corresponding combiner adapter. |
| `00B43B00` | descriptor ECX, filename/generation, RET8 | Existing full normal body uses the shared application provider context and active Lua/VFS binding. |
| `00B41830` | descriptor ECX, Shader object, RET4 | Real sampler children use the same published definitions and canonical state-list pool. |
| `00B43700` | fresh 110h ECX, EAX same, RET | Initializes established strings, arrays and vtable; preserves other preimages. |
| `00B458A0` | descriptor ECX, RET | Retires sampler/field children through their actual shared domains. |

Field ordinals advance only for accepted integral numeric keys. Combiner ordinals
advance for every entry. Both retain Lua traversal order. Explicit-preimage
callbacks keep their previous timing and meaning. The new adapters never publish
their host-initialized scratch cells unless the corresponding Lua read overwrote
them; missing field scalars stop before allocation, and a missing combiner mode
stops before destination computation. These errors are source input boundaries,
not a claim about the original machine's uninitialized-stack behavior.

The caller retains the initialized descriptor, filename header and persistent
read operation, and retires successful descriptor children before the application
graph. An interrupted parser marks the renderer failed and retains the existing
native operation state. The sampler callable-vtable bridge remains an explicit
source interface. The numeric conversion selector uses the application's current
hardware-SSE2 criterion; original CRT `0109EEA4` storage is not mapped here.

## Validation

- Strict MSVC Win32 build and all three existing CTests passed. No repository
  tests or extra build targets were added.
- Eight saved-analysis function spans match the original PE: 6,613 bytes. The
  single unlisted gap is six bytes of unreachable same-register LEA alignment.
  The report records 263 native calls, including one indirect child deletion.
- A diagnostic built from the current production bootstrap, 52 application
  objects and three libraries selects the verified static `FileNames` list,
  resolves `.mshd` names to `.shfx` through the actual application VFS, then calls
  the new application API with generation 3. This selection is diagnostic code,
  not a reconstruction of native preload iteration.
- All 179 descriptors parsed and retired: 311 samplers, 1,016 vertex/interpolator
  fields, 370 combiner names and 792 render states. No ordinal preimages were
  supplied. Raw descriptor allocations were not blanket zeroed.
- One empty-table regression in an isolated stock Lua interpreter checks both
  rejection paths and Lua stack cleanup, then verifies explicit field preimages
  and the supplied combiner destination. It uses the shared application string
  storage and does not represent native SEH/error-path equivalence.
- The probe and unmodified application each completed two ticks, one Present,
  ordinary singleton drain, a joined worker and final D3D device/API counts 0/0;
  both processes exited 0. The first successful run was archived before adding
  the focused rejection regression. Production code did not change afterward.

See [the machine-readable report](../reports/native_application_shader_descriptors_r97.json)
for exact source/build/runtime hashes, call sites, all per-descriptor results,
preserved Ghidra annotation values, and immutable artifact receipts.

## Remaining work

The full material preload/compiler graph, sampler B-16 stack input production,
automatic startup descriptor loading, original CRT identity, binary ABI and
native FH3/SEH behavior remain open. This turn does not add a unique native
function or claim original-machine descriptor differential, rendered scene or
gameplay validation.
