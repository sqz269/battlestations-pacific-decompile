# Input-settings `std::_Tree` instantiation cluster (0069b000-006a2000)

Addresses: 0069b000, 0069b090, 0069b190, 0069b910, 0069c0f0, 0069c1c0, 0069c220, 0069c280,
0069c300, 0069c370, 0069c550, 0069c640, 0069cc70, 0069ccf0, 0069cd50, 0069cdb0, 0069ce30,
0069cec0, 0069d0a0, 0069d1a0, 0069d2d0, 0069d3f0, 0069d6b0, 0069d9e0, 0069dc70, 0069df70,
0069dfe0, 0069ef20, 0069f210, 0069f400, 0069f5f0, 0069f690, 0069f940, 0069fa40, 0069fbc0,
0069fdb0, 0069fe70, 006a01f0, 006a02b0, 006a04a0, 006a0770, 006a07d0, 006a0aa0, 006a0ba0,
006a0e90, 006a1050, 006a1170, 006a1230, 006a1290, 006a1460, 006a15e0, 006a1560, 006a17e0,
006a1aa0, 006a1c40, 006a1e70, 006a1f80

Everything below is read off Ghidra pseudocode and assembly. No Ghidra object was mutated;
this packet was read-only. Every name is a hypothesis, not a recovered symbol.

## Correction to the smoke packet

`docs/INPUT_SETTINGS_SMOKE.md` places 006a1230 (clear) and 006a1aa0 (range erase) in the same
family as 006a01f0 (int-keyed unique insert). They are not. 006a01f0 reads `_Isnil` at node+0x15
and 006a0770, which both 006a1230 and 006a1aa0 call, reads it at node+0x29. Those are two
different instantiations with different node sizes. 006a1230 and 006a1aa0 belong to the
case-insensitive string-keyed tree, and 006a0e90 is the only one of the three that shares
006a01f0's instantiation.

The smoke packet's other finding holds and is now proved outright: 0069f400 is
`_Tree::_Insert`, not a length throw. Its body allocates a node, links it, runs the whole
red-black fixup and returns an iterator; the `_Xlen` throw is one guarded branch at the top.
Four more functions carry the same wrong `stl_throw_site` tag for the same reason: 0069ef20,
0069f210, 0069fbc0 and 006a02b0 are the `_Insert` of the other instantiations, and 0069f690,
006a04a0 and 006a07d0 are `erase(const_iterator)`, tagged for their `out_of_range` branch.

## Node and tree layout

All eight instantiations share the MSVC 7.1/8.0 `_Tree` shape. Only the payload width differs.

| Tree object | Node                                             |
| ----------- | ------------------------------------------------ |
| +0x00 allocator/comparator | +0x00 `_Left`                     |
| +0x04 `_Myhead`            | +0x04 `_Parent`                   |
| +0x08 `_Mysize`            | +0x08 `_Right`                    |
|                            | +0x0c `_Myval`                    |
|                            | `_Color` then `_Isnil` after `_Myval` |

`_Myhead->_Left` is `_Lmost`, `_Myhead->_Parent` is `_Root`, `_Myhead->_Right` is `_Rmost`.
Iterators are the checked two-word form `{_Mycont, _Ptr}` with the container at +0x00; every
checked member rejects a null `_Mycont` through `LIBCRT_unmatched_00bf6713`.

`sizeof(value_type)` is not inferred from the offsets alone. Each `_Insert` opens with the
`_Xlen` guard `_Mysize > allocator_max_size() - 2`, and that immediate divides `0xffffffff`
by the value size, so it pins the width exactly.

| Tag  | `_Myval` | `_Color` | `_Isnil` | node | `_Xlen` bound | key                                   |
| ---- | -------- | -------- | -------- | ---- | ------------- | ------------------------------------- |
| N14  | 4        | +0x10    | +0x11    | 0x14 | `0x3ffffffd`  | signed int at +0x0c (value is the key) |
| N18A | 8        | +0x14    | +0x15    | 0x18 | `0x1ffffffd`  | signed int at +0x0c, mapped at +0x10   |
| N18B | 8        | +0x14    | +0x15    | 0x18 | `0x1ffffffd`  | signed int at +0x0c, mapped at +0x10   |
| N1C  | 12       | +0x18    | +0x19    | 0x1c | `0x15555553`  | signed int at +0x0c, pooled string at +0x10/+0x14 |
| N2C  | 28       | +0x28    | +0x29    | 0x2c | `0x9249247`   | pooled string at +0x0c/+0x10, `__stricmp`, mapped 20 bytes at +0x14 |
| N20  | 16       | +0x1c    | +0x1d    | 0x20 | not in range  | unknown                               |
| N24  | 20       | +0x20    | +0x21    | 0x24 | not in range  | pooled string at +0x0c/+0x10, `__stricmp` |
| N28  | 24       | +0x24    | +0x25    | 0x28 | not in range  | pooled string at +0x0c/+0x10, `__stricmp` |

The pooled string is the two-word handle `{int count, char *buffer}` this codebase uses
everywhere. `_Erase` and `erase` release it through
`BSP_SizedStoragePool_GetSingleton_Provisional` / `BSP_SizedStoragePool_ReturnBlock_Provisional`
with size `count + 1`, so the buffer is NUL-terminated pool storage.

N18A and N18B have identical layouts and are still separate instantiations: their `_Insert`
routines call disjoint helper sets. 0069f400 uses `_Buynode` 00554f50 and rotates
005530d0/00553120, while 0069f210 uses `_Buynode` 0069d2d0 and rotates 0069c220/0069a610.
The same holds for 0069cd50 against 0069cdb0, which are byte-identical `_Lbound` bodies that
the linker did not fold, so identical-comparison folding is evidently off in this build and
duplicate bodies do not prove a shared instantiation.

## Method map

Convention is `__thiscall` unless stated. `RET` with no operand means no stack arguments.
Confidence is high for every row unless the note says otherwise.

### N14, `set<int>`-shaped

| Address  | Ghidra name              | Method                       | RET  | Evidence |
| -------- | ------------------------ | ---------------------------- | ---- | -------- |
| 0069b910 | FUN_0069b910             | `const_iterator::operator--`  | RET (`__fastcall`) | `_Rmost` on a nil `_Ptr`, else `_Max(_Left)` or the climb-while-left-child, all on `_Isnil` +0x11 |
| 0069d6b0 | FUN_0069d6b0             | `operator--(int)`             | RET (`__fastcall`) | three instructions: call 0069b910, return the saved iterator |
| 0069ef20 | STL_xlen_throw_0069ef20  | `_Insert`                     | 0x10 | `_Xlen` at `0x3ffffffd`, `_Buynode` 005550d0, link, red-black fixup, returns the iterator |
| 0069fa40 | FUN_0069fa40             | `insert(const value_type&)`   | 0x8  | signed-int descent, byte flag at slot+8, delegates to 0069ef20 |

### N18A, `map<int, T4>` behind 006a1560

| Address  | Ghidra name             | Method                                    | RET  | Evidence |
| -------- | ----------------------- | ----------------------------------------- | ---- | -------- |
| 0069b190 | FUN_0069b190            | `const_iterator::operator--`               | RET (`__fastcall`) | decrement on `_Isnil` +0x15; called by 006a01f0 and 006a0e90 |
| 0069f400 | STL_xlen_throw_0069f400 | `_Insert(bool,_Nodeptr,const value_type&)` | 0x10 | `_Xlen` at `0x1ffffffd`, `_Buynode` 00554f50, rotates 005530d0/00553120, root recoloured black, returns `{tree,node}` |
| 006a01f0 | FUN_006a01f0            | `insert(const value_type&)`                | 0x8  | pair return with the flag byte, duplicate path writes 0 |
| 006a0e90 | FUN_006a0e90            | `insert(const_iterator, const value_type&)`| 0x10 | validates the hint's `_Mycont`, three hint cases, falls back to 006a01f0 |
| 006a1560 | FUN_006a1560            | `map::operator[](const int&)`              | 0x4  | inlined lower_bound, builds `{key,0}` on a miss, returns node+0x10 |

### N18B, `map<int, T4>` behind 006a1460

| Address  | Ghidra name             | Method                                     | RET  | Evidence |
| -------- | ----------------------- | ------------------------------------------ | ---- | -------- |
| 0069c220 | FUN_0069c220            | `_Lrotate`                                  | 0x4  | `_Right` becomes parent, `_Isnil` +0x15; called by 0069f210 and 0069f690 |
| 0069c370 | FUN_0069c370            | `const_iterator::operator++`                | RET (`__fastcall`) | `_Min(_Right)` or climb-while-right-child on `_Isnil` +0x15 |
| 0069cec0 | FUN_0069cec0            | `operator++(int)`                           | RET (`__fastcall`) | wrapper around 0069c370 |
| 0069d9e0 | FUN_0069d9e0            | `operator++(int)`                           | RET (`__fastcall`) | second emitted copy of the same wrapper |
| 0069d2d0 | FUN_0069d2d0            | `_Buynode`                                  | 0x14 | five callee-cleaned arguments, sole caller 0069f210 |
| 0069d3f0 | FUN_0069d3f0            | `_Erase(_Nodeptr)`                          | 0x4  | destroys a subtree with no rebalancing; callers 0069dc70 and 0069fe70 |
| 0069dc70 | FUN_0069dc70            | `clear()`                                   | RET (`__fastcall`) | `_Erase(_Root())` then resets `_Root`, `_Mysize`, `_Lmost`, `_Rmost` |
| 0069f210 | STL_xlen_throw_0069f210 | `_Insert`                                   | 0x10 | same shape as 0069f400 with the N18B helper set |
| 0069f690 | STL_xlen_throw_0069f690 | `erase(const_iterator)`                     | 0xC  | `out_of_range` guard, delete fixup over `_Color` +0x14, `--_Mysize`, free; RET recovered from undefined bytes |
| 0069fdb0 | FUN_0069fdb0            | `insert(const value_type&)`                 | 0x8  | decrements with 0069ab10, delegates to 0069f210 |
| 0069fe70 | FUN_0069fe70            | `erase(const_iterator, const_iterator)`     | 0x14 | whole-range fast path inlines the 0069dc70 clear, else loops 0069c370 with 0069f690 |
| 006a0ba0 | FUN_006a0ba0            | `insert(const_iterator, const value_type&)` | 0x10 | hint overload calling 0069f210 and 0069fdb0 |
| 006a1460 | FUN_006a1460            | `map::operator[](const int&)`               | 0x4  | lower_bound then 006a0ba0, returns node+0x10 |

Two more postfix decrement wrappers call 0069b190 and cannot be pinned to N18A or N18B from
their callees: 0069c640 and 0069d1a0. Method high confidence, instantiation medium.

### N1C, `map<int, PooledString>`

| Address  | Ghidra name             | Method                                     | RET  | Evidence |
| -------- | ----------------------- | ------------------------------------------ | ---- | -------- |
| 0069c300 | FUN_0069c300            | `_Lrotate`                                  | 0x4  | `_Isnil` +0x19; callers 006a02b0 and 006a07d0 |
| 0069dfe0 | FUN_0069dfe0            | `find(const int&)`                          | 0x8  | lower_bound then returns `{tree,_Myhead}` on a miss |
| 0069f940 | FUN_0069f940            | `_Buynode`                                  | 0x14 | callers 006a02b0 and 006a1290, `_Color` read at +0x18 |
| 006a02b0 | STL_xlen_throw_006a02b0 | `_Insert`                                   | 0x10 | `_Xlen` at `0x15555553`, `_Buynode` 0069f940, rotates 0069c300/0069a780 |
| 006a07d0 | STL_xlen_throw_006a07d0 | `erase(const_iterator)`                     | 0xC  | delete fixup over `_Color` +0x18, releases the pooled string at +0x10/+0x14, frees the node |
| 006a0aa0 | FUN_006a0aa0            | `_Erase(_Nodeptr)`                          | 0x4  | subtree destroy, releases `count+1` bytes of pool storage per node |
| 006a1170 | FUN_006a1170            | `insert(const value_type&)`                 | 0x8  | pair return over the int key, delegates to 006a02b0 |
| 006a1290 | FUN_006a1290            | `_Copy(_Nodeptr, _Nodeptr)`                 | 0x8  | `_Buynode` from the source node then recurses on `_Left` and `_Right` |
| 006a17e0 | FUN_006a17e0            | `insert(const_iterator, const value_type&)` | 0x10 | hint overload calling 006a02b0 and 006a1170 |
| 006a1c40 | FUN_006a1c40            | `operator=` copy body                       | 0x4  | `_Root() = _Copy(right._Root(), _Myhead)`, copies `_Mysize`, recomputes `_Lmost`/`_Rmost` |
| 006a1f80 | FUN_006a1f80            | `map::operator[](const int&)`               | 0x4  | lower_bound, default pooled string on a miss, returns node+0x10 |

### N2C, `map<PooledString, T20, CaseInsensitiveLess>`

| Address  | Ghidra name             | Method                                     | RET  | Evidence |
| -------- | ----------------------- | ------------------------------------------ | ---- | -------- |
| 0069b090 | FUN_0069b090            | `const_iterator::operator--`                | RET (`__fastcall`) | decrement on `_Isnil` +0x29 |
| 0069c0f0 | FUN_0069c0f0            | `const_iterator::operator++`                | RET (`__fastcall`) | increment on `_Isnil` +0x29; callers 006a04a0 and 006a1aa0 |
| 0069c1c0 | FUN_0069c1c0            | `_Lrotate`                                  | 0x4  | `_Isnil` +0x29; callers 0069fbc0 and 006a04a0 |
| 0069cc70 | FUN_0069cc70            | `operator++(int)`                           | RET (`__fastcall`) | wrapper around 0069c0f0 |
| 0069cdb0 | FUN_0069cdb0            | `_Lbound(const key&)`                       | 0x4  | descends remembering the last node not less, `__stricmp` on +0x0c/+0x10, null buffer sorts smallest |
| 0069f5f0 | FUN_0069f5f0            | `_Buynode`                                  | 0x14 | sole caller 0069fbc0 |
| 0069fbc0 | STL_xlen_throw_0069fbc0 | `_Insert`                                   | 0x10 | `_Xlen` at `0x9249247`, `_Buynode` 0069f5f0, rotates 0069c1c0/0069a560 |
| 006a04a0 | STL_xlen_throw_006a04a0 | `erase(const_iterator)`                     | 0xC  | `out_of_range` guard, `_Lmost`/`_Rmost` fixups via 0069a530/0069a9e0, delete fixup over `_Color` +0x28 |
| 006a0770 | FUN_006a0770            | `_Erase(_Nodeptr)`                          | 0x4  | subtree destroy, releases the pooled key buffer per node |
| 006a1050 | FUN_006a1050            | `insert(const value_type&)`                 | 0x8  | `__stricmp` descent, delegates to 0069fbc0 |
| 006a1230 | FUN_006a1230            | `clear()`                                   | RET (`__fastcall`) | `_Erase(_Root())` then resets head, size and both extremes |
| 006a15e0 | FUN_006a15e0            | `insert(const_iterator, const value_type&)` | 0x10 | hint cases decided by `BSP_NativeString_LessCaseInsensitive` |
| 006a1aa0 | FUN_006a1aa0            | `erase(const_iterator, const_iterator)`     | 0x14 | whole-range fast path inlines the 006a1230 clear, else loops 0069c0f0 with 006a04a0 |
| 006a1e70 | FUN_006a1e70            | `map::operator[](const key&)`               | 0x4  | `_Lbound` 0069cdb0, hint insert on a miss, returns node+0x14, the mapped value after the 8-byte key |

### Members of instantiations that are otherwise outside the range

| Address  | Ghidra name  | Method                       | RET  | Tag  | Note |
| -------- | ------------ | ---------------------------- | ---- | ---- | ---- |
| 0069cd50 | FUN_0069cd50 | `_Lbound(const key&)`         | 0x4  | N2CB | byte-identical to 0069cdb0, different callers (0069d7c0, 006a4ca0) |
| 0069b000 | FUN_0069b000 | `const_iterator::operator--`  | RET (`__fastcall`) | N20 | `_Isnil` +0x1d |
| 0069c550 | FUN_0069c550 | `operator--(int)`             | RET (`__fastcall`) | N20 | wrapper around 0069b000 |
| 0069d0a0 | FUN_0069d0a0 | `operator--(int)`             | RET (`__fastcall`) | N20 | second copy of the same wrapper |
| 0069c280 | FUN_0069c280 | `_Lrotate`                    | 0x4  | N24 | `_Isnil` +0x21 |
| 0069ce30 | FUN_0069ce30 | `_Lbound(const key&)`         | 0x4  | N24 | `__stricmp` descent on `_Isnil` +0x21 |
| 0069df70 | FUN_0069df70 | `find(const key&)`            | 0x8  | N24 | `_Lbound` 0069ce30 then the reject compare |
| 0069ccf0 | FUN_0069ccf0 | `_Lbound(const key&)`         | 0x4  | N28 | `__stricmp` descent on `_Isnil` +0x25 |

### Helpers just outside 0069b000-006a2000

Not claimed and not named here: `_Rrotate` 0069a610 (N18B), 0069a780 (N1C), 0069a560 (N2C);
`_Max`/`_Min` fixups 0069a530 and 0069a9e0 (N2C); `operator--` 0069ab10 (N18B); and the whole
N18A helper set in segments 17 and 18, `_Buynode` 00554f50, `_Lrotate` 005530d0, `_Rrotate`
00553120, `operator++` 005531f0, iterator compare 00553070. The `_Tree` destructors 006a2060,
006a24b0 and 006a2b00 sit immediately above the range and call range erase 006a1aa0 followed by
a free of `_Myhead`.

### Not tree members

0069d6f0, 0069d7c0 and 0069d8c0 are tagged `stl_instantiation` and sit inside the cluster, but
they are checked `vector::end()` bodies: they compare `_Mylast` at +0x08 against `_Myfirst` at
+0x04 and return `{this, _Mylast}`. The index reports call edges from 0069d7c0 to 0069cd50 and
from 0069d6f0 to 0069c0f0 that the decompiled and disassembled bodies do not contain; 0069d7c0
is 17 instructions ending in `RET 0x4` whose only call is to `LIBCRT_unmatched_00bf6713`. Those
edges look wrong and are worth a check before anyone relies on them.

## Owning game code

Every owner is in segment 31, the input-settings segment, and the strings identify the roles.

- **006a7be0**, the controller and keyboard datatable loader, called by
  `BSP_Application_Initialize` (0073d410) and by 006ab6b0. Strings:
  `Scripts\datatables\ControllerInputNames.lua`, `Scripts\datatables\KeyboardSetup.lua`,
  `Scripts/datatables/ControlPresets.lua`, `Inputs`, `DEVINPUTS`, `Pairs`, `Groups`,
  `Conflicts`, `InputNames`, `Min1SensHacks`, `AxisPairs`, `BaseSensitivities`,
  `Sensitivities`. It drives both the N18A index 006a1560 and the N1C index 006a1f80, so it
  owns one `map<inputId, int>` and one `map<inputId, PooledString>`, the second being the
  identifier-to-display-name table.
- **006ab820**, under 00698a10, the `Scripts\datatables\Inputs.lua` binding loader. 00698a10's
  strings are `Inputs`, `groups`, `press`, `inputs`, `helper`, `InvertPlaneY`, `InvertCameraY`,
  `SwapStickMap`, `SwapStickGeneral`, `SwapStickPairs`, `InputModifiers`. 006ab820 drives the
  N18B index 006a1460 and the N18B increment 0069c370.
- **006aa640**, under 006aba50, the input-settings save-data path. 006aba50's strings are
  `inputSettings`, `sensitivitySettings`, `deviceType`, `deviceIdx`, `slider`, `reverse`. It
  drives the N18A index 006a1560, so the persisted per-device sensitivity table is that
  `map<int, T4>`.
- **0069e9e0**, the front-end control-string builder, called by 006a33f0 and 006a9850. Strings
  `FE_pc.cs_unknown`, `(DEVINPUTS)`, `|. |^globals.down`, `|. |^globals.up`. It uses the N1C
  `find` 0069dfe0 and the N2C increment 0069c0f0, so it reads the identifier-to-name map and
  walks a string-keyed map.
- **006aa460**, reached from `CG_scalar_deleting_dtor_006ab800`, and 006a2930, under 006a2c40,
  006a3100 and 006a3cb0, are owner-class destructor and copy paths that call the N2C range
  erase 006a1aa0 and the N1C copy assign 006a1c40.

## Flow breaks and disassembly gaps

Every break has the same cause: Ghidra treats `_free` (00bf65ac) as no-return, so the code that
follows the last `CALL 0x00bf65ac` inside a function body is never disassembled. Six functions
are affected. None were fixed.

The three `_Erase(_Nodeptr)` subtree destroyers each lose 11 bytes, the loop back-edge, so the
exported C frees one node and returns while leaking every left subtree.

| Function | Undefined range     | Bytes                                  | Decoded |
| -------- | ------------------- | -------------------------------------- | ------- |
| 0069d3f0 | 0069d414-0069d41e   | `83 c4 04 80 7e 15 00 8b fe 74 e2`      | `ADD ESP,0x4` / `CMP byte ptr [ESI+0x15],0x0` / `MOV EDI,ESI` / `JZ 0x0069d401` |
| 006a0770 | 006a07b1-006a07bb   | `83 c4 04 80 7e 29 00 8b fe 74 c5`      | `ADD ESP,0x4` / `CMP byte ptr [ESI+0x29],0x0` / `MOV EDI,ESI` / `JZ 0x006a0781` |
| 006a0aa0 | 006a0ae1-006a0aeb   | `83 c4 04 80 7e 19 00 8b fe 74 c5`      | `ADD ESP,0x4` / `CMP byte ptr [ESI+0x19],0x0` / `MOV EDI,ESI` / `JZ 0x006a0ab1` |

The three `erase(const_iterator)` bodies lose their whole epilogue, which is why none of them
has a `RET` in Ghidra's listing at all. The three epilogues are the same code; only the register
holding the tree and the pop order differ.

| Function | Undefined range     | Ends at  | Decoded |
| -------- | ------------------- | -------- | ------- |
| 0069f690 | 0069f901-0069f936   | `RET 0xC` at 0069f934 | `MOV EAX,[EBP+0x8]` / `ADD ESP,0x4` / `TEST EAX,EAX` / `POP EDI` / `POP EBX` / `JBE +6` / `ADD EAX,-0x1` / `MOV [EBP+0x8],EAX` / the iterator writeback `MOV ECX,[ESP+0x64]` `MOV EAX,[ESP+0x60]` `MOV EDX,[ESP+0x68]` `MOV [EAX],ECX` `MOV [EAX+0x4],EDX` / SEH unlink / `ADD ESP,0x54` / `RET 0xC` |
| 006a04a0 | 006a0732-006a076b   | `RET 0xC` at 006a0769 | same, with the tree loaded by `MOV ECX,[ESP+0x14]` / `MOV EAX,[ECX+0x8]` and `POP EBP` before the branch |
| 006a07d0 | 006a0a62-006a0a9b   | `RET 0xC` at 006a0a99 | byte-identical to the 006a04a0 epilogue |

Each undefined range is followed by `CC` padding up to the next function. The guarded
decrement (`TEST` then `JBE` over `ADD EAX,-1`) is `if (_Mysize != 0) --_Mysize`, and the three
words written back are the returned iterator `{tree, node}` into the caller's hidden slot.

Two further gaps that are not breaks: in 0069f690, 006a04a0 and 006a07d0 the listing shows
8-byte steps after `MOV dword ptr [ESP+0x2c],0xf` and `MOV dword ptr [ESP+0x30],0xd6926c`.
Those are the real instruction lengths, not holes.

## Uncertainty

The element types are not recovered. N18A and N18B are `map<int, T>` with a 4-byte mapped type
that could be an int, an enum, a float or a pointer; nothing in the tree code distinguishes
them. N2C's mapped value is 20 opaque bytes. The N20, N24 and N28 instantiations are
represented in this range by one or two members each, so their method sets are incomplete and
their owners were not traced. The instantiation assignment of 0069c640 and 0069d1a0 is medium
confidence; every other row is high confidence.
