# CC13 template-copy shape sweep

Packet `cc13-stl1-template-shapes` (worker cc13-stl1, 2026-10-12). This packet produced metadata only:
tag-ledger rows, Ghidra renames and bookmarks. It wrote no C++. Nothing here was reconstructed,
build-tested or game-validated. The names are inventory tags for compiler-generated or template
code. They are not recovered symbols.

## Scope and result

The scope was the untagged `FUN_` functions in the 44 segments cc13 covered: 20, 17, 23, 26, 27, 29, 6, 15,
12, 19, 2, 9, 78, 81, 84, 85, 86, 88, 89, 90, 91, 92, 93, 94, 96, 43, 45, 46, 61, 64, 65, 76, 77, 79, 80, 82,
83, 25, 22, 16, 28, 4, 10 and 5.

| count | before | after |
|---|---|---|
| `bsp.py state` untagged FUN_ (whole image) | 17966 | 14559 |
| non-thunk FUN_ in the 44 segments | 7610 | 4212 |
| renamed by this packet | | 3368 |
| bookmarked only (`stl_probable`) | | 248 |
| skipped because another owner holds a lease | | 42 (all in cc13-camera2's 0042b000-00435000) |

`reports/cc13_template_copy_sweep.json` holds the per-segment before/after counts and the address list for every
shape.

## Method

1. `tools/template_shapes/st1_shapes.py` disassembles each target with Capstone over the on-disk `.text`. It runs from the
   function start to the next function start (EH funclets included) and stops at the first INT3. It normalises
   each instruction:
   - a call or jump target becomes `L<offset>` (inside the body), the stock name (`_free`, `operator_new`,
     `_memmove_s`, ...), the inventory prefix (`STL_inst`, `CG_*`, ...), `T:<tag category>` or `F` (another
     untagged function);
   - constants of 0x400000 or more become `A`.

   The *coarse* signature also masks every small constant. This lets one shape group template copies that differ
   only in element size, node `_Isnil` offset or member offset.
2. The script groups identical coarse signatures. It also signs all 29235 already named or tagged functions, so
   that each group can be matched to existing reference names (`STL_allocator_deallocate_*`,
   `BSP_PointerVector_Tidy`, `STL_Tree_Construct_*`, ...).
3. I reviewed every group with 5 or more members, 170 groups in all, from its normalised body. Where a group
   called `F`, `st1_callee.py` checked which group the callees belong to. For example, every tree
   constructor calls the tree head `_Buynode` group, every list constructor calls the list head `_Buynode` group,
   and every tree `clear` calls a `_Erase` group. Only groups whose shape was certain went into
   `tools/template_shapes/st1_decisions.txt`.
4. Smaller groups and singletons were taken only through these rules:
   - **twin**: the coarse body equals a reference whose names are all `STL_`, `CG_` or `TRIV_body` (48 functions).
   - **The repo's own mechanical rules** (`tools/build_tag_ledger.py`):
     - `sub/add ecx,N; jmp` gives an adjustor thunk.
     - A function that calls 00bf6713 and is no more than 96 bytes gives `stl_instantiation`.
     - `test [esp+N],1` followed by `_free` and `ret 4` gives a scalar deleting destructor.
     - A constant-return trivial body gives `trivial_body`.
     - A function that calls 00bf6713 and is over 96 bytes gives a `stl_probable` bookmark only.
   - **Family regexes** (`st1_family.py`) for shapes that cannot be mistaken for anything else: the
     `std::_Tree` destructor (89), `_Erase` (2) and `allocator::allocate` (4).

## Categories and names

Two new tag categories were added to `tools/build_tag_ledger.py` `CATEGORIES`. That script never derives them,
and it keeps them as already-applied rows.

- `stl_shape`: Dinkumware (VC9) container, algorithm and allocator copies, named `STL_<Shape>_<addr>`.
- `tmpl_shape`: non-std template helpers. These are the game or middleware pointer array (`TMPL_PointerArray_*`,
  the same class as the existing `BSP_MitPointerArray_*` names), ref-counted pointer reset and release, owning
  pointer delete-reset, and raw-pointer free. They are named `TMPL_<Shape>_<addr>`.

The existing categories are reused with their existing prefixes: `trivial_body` (`TRIV_body_`),
`cg_scalar_deleting_dtor`, `cg_adjustor_thunk` and `stl_instantiation` (`STL_inst_`). Each row's
`evidence_id` (`st1_<key>`) resolves in `config/tags/evidence.json` to the instruction pattern that defines the
shape.

| shape (prefix) | n | defining pattern |
|---|---|---|
| STL_Tree_Destroy | 261 | erase(begin,end) through a stack iterator, `_free(head)`, zero head and size |
| STL_IterWrap | 254 | `call <stl-tagged iterator helper>; mov eax,esi; ret` (operator++ or operator-- returning *this) |
| STL_TagDispatch | 225 | zero-byte iterator-category tags pushed, then forwarded to a `_Uninit_*` or `_Copy_*` worker |
| STL_allocator_deallocate | 194 | `mov eax,[esp+4]; push eax; call _free; pop ecx; ret 8` |
| STL_allocator_allocate | 189 | max_size check, `operator new`, `std::bad_alloc` throw |
| STL_Tree_Construct / BuyHeadNode / BuyNode | 152 / 51 / 27 | head `_Buynode`, `_Isnil=1`, self links, size 0 |
| STL_Tree_Rotate / Min / Max | 141 / 66 / 65 | red-black link rotation; left or right walk to the `_Isnil` byte |
| STL_Vector_Tidy / TidyDestroy / Size / InsertN | 122 / 48 / 53 / 30 | VC9 vector at +4/+8/+0xC |
| STL_Tree_InsertUnique / InsertHint / LowerBound / Erase / Clear | 59 / 44 / 15 / 41 / 43 | |
| STL_Tree_CopyTree / CopyConstruct / CopySubtree / Assign | 24 / 22 / 20 / 7 | `_Copy`, plus the EH rethrow variants |
| STL_List_Construct / BuyHeadNode / BuyNode / Clear / Tidy | 35 / 36 / 26 / 42 / 49 | VC9 list (head +4, size +8) |
| STL_Fill / FillN / FillObjects / Copy / CopyBackward | 36 / 31 / 19 / 25 / 36 | dword loops and `_memmove_s` copies |
| STL_UninitCopy / UninitFillN / DestroyRange / allocator_construct | 53 / 5 / 15 / 42 | |
| STL_String_Tidy | 10 | `cap>=16 -> _free; cap=15; size=0; buf[0]=0` |
| STL_inst (twins and the <=96-byte SCL rule) | 54 | |
| TMPL_PointerArray_Reserve / SetCapacity / PushBack / Grow | 34 / 33 / 29 / 16 | `operator new(cap*4)`, copy, `_free` old |
| TMPL_RefPtr_Reset / Release, OwnedPtr_DeleteReset, FreeAndClear, FreePointer | 45 / 32 / 38 / 29 / 21 | |
| TRIV_body | 317 | ret, constant return, getter or setter |
| CG_scalar_deleting_dtor / CG_adjustor_thunk | 63 / 44 | |

## Ghidra application

`tools/template_shapes/st1_apply.py` reuses `tools/ghidra_tag.py`'s `apply()` under `ghidra_lock`, in four saved batches. It
renames only functions that still carry `FUN_` names, sets an `Inventory: <category>` bookmark, and leaves plate
comments untouched for these categories. Result: 3368 renamed and 248 bookmarked, with 0 errors and 0
kept_existing. Every renamed function was `FUN_<addr>` before the rename, so a revert restores that name. The
per-batch logs are `local/st1-ghidra-tags-*.json` in the cc13-stl1 worktree, and they are lost when that
worktree is removed. After the run, `bsp.py snapshot` and `bsp.py index` were refreshed.

## Uncertainty and what was left alone

- Getters and setters were tagged `TRIV_body` because that is the repo's existing trivial_pattern convention.
  Some of them are game vtable slots, for example `GetType` constants. A later reviewed name supersedes the tag.
- `STL_TagDispatch` covers several `_Uninit_copy`, `_Uninit_fill_n` and `copy_backward` dispatchers. The shared
  prefix does not say which algorithm each one is.
- `tmpl_shape` rows are template copies of game or middleware classes, not std. The shape is certain, but the
  owning class is not identified.
- The following were left untagged even though they repeat:
  - GUI pool classes (`00aad040`, `00aad120`, `00aad5f0`, `00ab1340`);
  - pooled-string and NativeString pair helpers (`00431f20`, `00436490`, `0044c6e0`);
  - observer/unregister pairs (`0042ec20`, `0042e700`);
  - zero-initialising constructors (`00b2f470`, `00b344c0`);
  - indexed getters (`00b34590`, `00b3e370`, `00b40900`);
  - type tests (`0042e840`, `00aa1cc0`);
  - EH placement constructors with pool or free calls (`004580e0`, `0043c050`);
  - every group I could not tie to a single container operation.
- 3490 singletons and small groups did not match a rule. Many of them are real game functions.

## Handoff

The sweep is complete for the 44 segments. The scripts and the reviewed decision table are committed under
`tools/template_shapes/`. Run them from the repo root; their intermediates go to `local/st1_*.json`. To extend
the sweep:
- edit `SEGS` in `st1_shapes.py` and run it;
- run `st1_auto.py`, `st1_rules.py` and `st1_family.py`;
- review the new groups with `st1_compact.py`, `st1_callee.py` and `st1_ex.py`;
- add `ex|category|prefix|key` lines to `st1_decisions.txt`;
- claim a lease, then run `st1_build.py --write` and `st1_apply.py <lo> <hi>`.

`st1_auto.py` and `st1_rules.py` are quick scripts with relative paths, so run them from the repo root too.

Open question for the lead: should `TRIV_body` getters and setters be excluded from future sweeps so that game
vslots stay visible as `FUN_`?
