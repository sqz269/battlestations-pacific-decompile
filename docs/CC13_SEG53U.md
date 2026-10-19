# CC13 seg53U: segment 53 and 47 leftovers

Packet cc13-seg53U, under the cc9 gameplay terms approved on 2026-10-16 (local\cc13_gameplay_terms.txt).
The input was 59 functions with no tag and no reconstruction record:

- seg53, 00851E10-008740E0: effects, submarine, torpedo, turning gun;
- seg47, 007B2DD0-007D1DC0: power lost, engine fire, spinning, path points.

Main had no tag or record for any of them (0c5f2bbe3). None of the 59 lies in a struck cc9 range
or is one of the frozen cc9-physics13 bodies.

New code: `src/cc13_seg53U.cpp` and `include/bsp/cc13_seg53U.hpp` (namespace
`bsp::cc13_seg53U`). These are new C++ interfaces over the Win32 layouts. Nothing in them is wired into bsp_game.
Names are hypotheses, not recovered symbols.

**Torpedo and submarine:** none of the 59 bodies has torpedo or submarine logic. 00864380 is the
destructor of the unit gunnery AI and has no behaviour beyond releasing memory.

## Triage

| class | count | action |
| --- | --- | --- |
| engine container templates (seg53) | 26 | `tmpl_shape`, evidence `cc13seg53u_tmpl`, bind-only |
| STL bodies (seg53) | 14 | `stl_shape`, evidence `cc13seg53u_stl`, bind-only |
| STL bodies and STL-shaped record helpers (seg47) | 15 | `stl_shape`, evidence `cc13seg53u_stl47`, bind-only |
| reconstructed | 4 | `src/cc13_seg53U.cpp` |

**seg47 STL bodies.** The 15 include the plane bow-wave record helpers 007CD2F0, 007CDDB0 and
007CF4B0, and 007CCD30. Each is an uninitialized_fill, a fill-at, a vector insert-n or a vector
tidy:

- 007CD2F0 copies a retained definition pointer per element.
- 007CCD30 destroys each 2Ch-byte element through its vtable +4h with 0, then frees the block and
  zeroes first, last and end. It is a `std::vector` tidy over polymorphic elements. Its caller in
  cc13_seg47b reaches it through a host hook, which a binding can satisfy.

**seg53 engine containers.** The 26 are the image's own containers:

- `{data, count, cap}` pointer, dword, qword and 12-byte arrays: reserve, grow 2n+2, push_back,
  resize with zero fill, clear, destroy, at;
- the particle resource-pointer push_backs;
- the zero and handle constructors, and the handle compare;
- the buffer frees;
- the +48h vector non-empty test;
- a pooled NativeString destroy.

The cc13 sweeps already name all of them by shape, and they follow the precedent of the earlier
`tmpl_shape` rows. 0086A280 and 0086AA10 are called through host hooks in scene_graph2_cc13,
which a binding can satisfy.

## Reconstructed

| address | function | ABI | notes |
| --- | --- | --- | --- |
| 00859950 | `gun_build_sync_delta_00859950` | `__thiscall(ECX = gun; record)`, RET 4 | The turning gun's sync-delta slot. Its address is stored in the vtable dwords 00CFBEA0, 00CFC0D8, 00CFC310 and 00CFE6C8 (scan-bytes). |
| 00864380 | `unit_gunnery_ai_destruct_00864380` | `__thiscall(ECX = ai)`, RET | Only caller: the scalar deleting destructor 00864660. |
| 007B7290 | `bot_task_rocket_debug_draw_007b7290` | `__fastcall(ECX = task)`, RET | Rocket bot task slot +04h (dword 00D0591C). Draws only when byte +35Dh is set. |
| 007D06A0 | `model_collect_numbered_note_nodes_007d06a0` | ECX = model, EDX = prefix, `vector*` on the stack, RET 4 | Only caller: 007D0820 BSP_Plane_HideOtherMeshHolders. |

### 00859950

1. **Early return.** When the owner +3Ch is set and owner->vtable[5Ch](6) is true, it returns.
2. **Role value.** The value is `*record ? [*record + 20h] : 8`. It is passed to
   00927850(ECX = [gun+3Ch]). On this path the owner is not null-tested.
3. **Sync call.** It calls the cdecl 0077B280 with 13 arguments:
   `(gun, record, role, 0.0f, 1, 300.0f, 3, 800.0f, 10, 500.0f, 20, FLT_MAX, 200)`.
   - The floats come from 00CE3AE8, 00CE3950, 00CFFD60 and 00D7A248.
   - The integer arguments are formed as 14h-0Ah, 14h-11h and 14h-13h.
   - `role` is the whole argument dword. At 00859999 the image writes AL over the low byte of its
     own `record` argument slot, so the upper three bytes are the record pointer's. The interface
     passes that dword as it is.

### 00864380

1. **Vtables and owned objects.** It re-installs the vtable words 00D0D360 (+0h) and 00D0D348
   (+1Ch). It deletes +5Ch and +60h through vtable[0](1). It frees +68h's buffer with `_free`
   00BF6989 and then +68h itself, and runs operator delete on +64h without a null test.
2. **The tree at +B0h.** For each node, the mapped object at +10h is deleted through vtable[0](1).
   The iterator is advanced by 00862110 and the old node erased by 00863BF0. 00863BF0 is
   std::_Tree erase(iterator). It was renamed from STL_xlen_throw_00863bf0 to
   STL_Tree_Erase_00863bf0 in the ledger and in Ghidra (cc13-seg53F). Every checked-iterator test is kept.
3. **The vector at +C0h..+C8h.** It is freed and zeroed.
4. **Tree clear and base.** The tree is cleared through 00864090, the head is freed, +B4h and +B8h
   are zeroed, and the base 007293D0 runs.

### 007B7290

1. **Trivial call.** It always calls 0099B6E0 first. When byte +35Dh is set, it then draws
   steps 2-5.
2. **Point marker.** A 00860BE0 marker at the task point +370h: EDX is the blue static 00F8721C,
   the radius is 10.0, the second pointer is the red static 00F87208, and `detailed` is 1.
3. **Tip marker.**
   - The length is 0042B2F0(owner position +FCh minus the point).
   - The tip is the owner position plus len × (ECh, F0h, F4h), where the owner is task+3FCh,
     refreshed through 00414DB0 when byte +C8h is clear.
   - The marker at the tip uses the white static 00F871F4 for both pointers.
4. **Green line** (static 00F87230 {0, 1, 0.5, 1}). It is drawn through 0085F990 from the tip to
   `tip + 0.5 * len * owner+9E8h * (DCh, E0h, E4h)`.
5. **Cyan line** (00F87244 {0, 0.5, 1, 1}). It is drawn the same way with +9E4h and
   (CCh, D0h, D4h).

**Precision and encodings.**

- Every intermediate is stored as a float, and the model rounds once per operation.
- The x87 encodings were read as bytes, not from the listing:
  - DC C9 at 007B7585 and 007B7709;
  - D8 C9 at 007B740E, 007B7543, 007B756D, 007B7591 and 007B7715.
- The products are commutative, so operand order changes nothing.

**Static colours.** Each function-local static colour is initialised once behind its guard bit.
The image tests the white, green and cyan guards a second time, but by then the guards are always
set.

### 007D06A0

For i = 0, 1, … the function does the following:

1. **Build the name.** It builds `std::string(prefix)` with 00408720, formats `_itoa(i, buf, 10)`
   and appends it with 007B7CC0. The string uses the SSO layout: +4h buffer, +14h size,
   +18h capacity 0Fh.
2. **Find the node.** It looks the name up with 0071AD50. The first missing name ends the loop,
   after the string is freed (when the capacity is 10h or more).
3. **Push it back.**
   - **Fast path:** when the data pointer is non-null and size < capacity, the node is stored
     inline.
   - **Slow path:** otherwise the image checks that first <= last and calls 00505D50(out, vector,
     last, &node).

## Adoption notes (checked at the host callers)

No cc9 host *function or switch* implements any of the four. The first version of this section said
there was no match by address either, searching only the game_hosts and cc9 .cpp files for
0077B280, build_sync_delta, 00864380, UnitGunneryAi, 007B7290, rocket debug and 00859950. That was
overstated (corrected 2026-10-18, cc13-seg53F). cc9 already documents 00864380 under the class
name **UnitGunneryPass**:

- `include/bsp/unit_gunnery_pass.hpp:28` has `kUnitGunneryPassDestructAddress = 0x00864380u`.
- `:51-52` have `kUnitGunneryPassVtable` 00D0D360 and `kUnitGunneryPassObserverVtable` 00D0D348.
  These are the same two words this file installs. `kUnitGunneryAiVtable` / `kUnitGunneryAiVtable1C`
  in cc13_seg53U.hpp are now defined as those constants, not duplicated.
- `docs/UNIT_GUNNERY_PASS.md:596`, the coverage row
  "`00864380` | `00864380-0086454F` | partial: the vtable re-install and the four member
  destructions".

**Adoption note for cc9 (UnitGunneryPass, 00864380): MINOR, documentation only.** Row 596 can point at
`unit_gunnery_ai_destruct_00864380` (src/cc13_seg53U.cpp) for the full body 00864380-008644FA. The
partial row stops at the four member destructions. The full body adds:

- the tree at +B0h: each mapped +10h deleted through vtable[0](1), then erased node by node
  through 00862110 and 00863BF0, now named STL_Tree_Erase;
- the +C0h vector free;
- 00864090 clear and head free;
- the 007293D0 base.

There is no behavioural host to compare against: the destructor is not called from host code. I
did not edit the cc9 files.

Within cc13, 007D06A0 is called through `node_append_numbered_children_007d06a0` (cc13_seg47b),
and that hook matches this body. 00859950, 007B7290 and 007D06A0 are new code only.
