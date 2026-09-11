# Scene record string storage (`record+90Ch`/`+910h` and the sized storage pool)

Addresses: 004e1d70 0041dd40 00419cc0 00bd1120 00bd1510 004c6890 004f1d70 004e82f0

`docs/MISSION_LOAD_PATH.md` modelled the scene path at `record+90Ch`/`+910h` as a `std::string` and
listed the sized-storage-pool plumbing as a follow-up, because the model's boundary was never
written down. This packet writes it down. The conclusion is that the model is **right for the
values** this path observes and **wrong about representation in three specific ways**, so
`src/mission_load_path.cpp` is not corrected; the boundary is recorded here instead.

## The native string is a bare `{length, pointer}` pair

`0041DD40 BSP_NativeString_Resize` is already reconstructed (`src/native_string.cpp`,
`docs/APP_INIT_ALLOC_STRINGS.md`). What matters for the record:

| Offset | Field |
| --- | --- |
| `+0h` | `uint length`, **excluding** the terminator |
| `+4h` | `char* data`, or null |

There is no capacity field and no small-buffer area. The block is always `length + 1` bytes, so
every length change reallocates, and the terminator is written at `data[length]` by the resize
itself. `Resize(len, preserve)` takes the header in ECX and `(uint length, char preserve)` on the
stack, RET 8.

The record carries five of these pairs the load path touches, all built by the record constructor
`004DA2A0`:

| Offset | Content | Writer |
| --- | --- | --- |
| `+90Ch`/`+910h` | the scene path | `004E1D70` |
| `+918h`/`+91Ch` | `FileSystemLua` | `004F1D70` |
| `+920h`/`+924h` | `Title` | `004F1D70` |
| `+928h`..`+97Fh` | eleven script names | `004F1D70` |
| `+980h`/`+984h` | `LocKitPaths` | `004F1D70` |

The eight side blocks add ten more per block (`Name` and `ClassName` for each of five units).

## What `004E1D70` does with the scene path

The create arm is the `assign` idiom, spelled out:

```
Resize(strlen(path), 0)                                  ; ECX = record+90Ch, preserve = 0
if (*(record+910h) != 0)
    memcpy(*(record+910h), path, *(record+90Ch))
```

The reuse arm has the same `assign` inlined at `004E1F5x..004E1FA5`: it compares `strlen(path)` with
`*(record+90Ch)` and does nothing when they match, releases and nulls both fields when the new
length is zero, and otherwise allocates `len + 1` from the pool, releases the old block with
`old_len + 1`, and stores pointer, length and terminator.

The duplicate test the same arm runs is `strcmp(existing+910h ? existing+910h : 00E18B1C, path)`,
where `00E18B1C` is a zeroed global used as the empty string. That substitution is why a
`std::string` model gives the right answer: a null `data` compares exactly like `""`.

## The pool contract

`00419CC0 BSP_SizedStoragePool_GetSingleton` returns the pool in EAX, takes no arguments and does no
stack cleanup, so callers push the arguments of the *following* `__thiscall` before calling it. That
is why the decompiler renders it with arguments it does not have, in this function and in every
other caller.

| Call | ABI | Contract |
| --- | --- | --- |
| `00BD1120 AllocateBlock` | `__thiscall(pool; uint size, uint unused)`, RET 8 | `size >= 0x96` falls through to `malloc`; smaller sizes come from the size-classed free lists |
| `00BD1510 ReturnBlock` | `__thiscall(pool; void* block, uint size, uint unused)`, RET 0xC | the release is **sized**: the caller must pass the same `size` the block was allocated with |

Every release in this path passes `length + 1`, matching the allocation. A model that keeps only the
string value and not the allocated size cannot free through this pool at all; that is the reason the
pool argument threading shows up in every string-touching reconstruction.

## Where the `std::string` model stops being true

The model in `src/mission_load_path.cpp` (`SceneRecord::scene_path`) reproduces every value this
path reads or compares. It diverges in representation:

1. **The empty string has a null pointer, not a pointer to `""`.** `Resize(0, …)` releases the block
   and writes null into both fields. Native callers either guard with `if (data != 0)` before the
   `memcpy` (which `004E1D70`, `004C6890`, `004F1D70` and `004E82F0` all do) or substitute
   `00E18B1C`. A host that hands `c_str()` to native code must reproduce the null, and a host that
   reads a native record must not assume `data` is dereferenceable.
2. **A same-length assign does not reallocate.** `0041DD4A` returns immediately when the requested
   length equals the stored length — even when `data` is null, which leaves a non-zero length with
   no buffer. Nothing in this path can reach that state (length and pointer are always written
   together), but a model that reasons about buffer identity or allocation counts is not equivalent.
3. **There is no capacity, so every length change reallocates.** `reserve`, `shrink_to_fit` and
   `std::string`'s growth factor have no counterpart. Allocation counts differ from any
   `std::string` implementation.

None of these change a value the mission load path observes, so the reconstruction stands as
written. The three points above are what a future ABI-compatible layer has to implement instead.

## Corrections

- **`docs/MISSION_LOAD_PATH.md` listed `scene_record_storage` as an open question about whether the
  `std::string` model is wrong.** It is not wrong about values; it is a representation
  approximation with the three boundaries above. Recorded here rather than by changing the code.
- **The field at `record+918h`/`+91Ch` and `+920h`/`+924h` had no owner.** They are `FileSystemLua`
  and `Title` from the header's property bag, written by `004F1D70`; see
  `docs/SCENE_RECORD_SIDE_BLOCKS.md`.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `native_string_abi_layer` | 0041dd40 0041e870 004261a0 | include/bsp/native_string.hpp | A header-layout-compatible string type that reproduces the null-for-empty and no-capacity rules, so a reconstruction can hand a real record to native code |
| `scene_record_script_table` | 004f1d70 004dfb70 | docs/SCENE_RECORD_SCRIPT_TABLE.md | Which `StageScript*` key lands in which of the eleven pairs at `record+928h` |

## Uncertainties

- **The `unused` arguments.** `00BD1120`'s second and `00BD1510`'s third are never read in this
  build. Both reconstructions already record that; this packet did not re-derive it.
- **Whether `record+90Ch` is ever read with a null `data`.** Every read in this path is guarded. A
  reader outside the load path was not searched for.

## no_ghidra_function

none
