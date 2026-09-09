# Template / compiler-generated / trivial function inventory (game-code range)

Scope: internal functions in `0x00401000-0x00be0000`, excluding Lua (`0x00a61000-0x00a7a000`) and zlib
(`0x00bc9000-0x00bce000`). 32075 functions (30944 still `FUN_`). Sizes were measured by
linear sweep over the disk image (capstone), bounded by the next function start. Ghidra was used read-only.

## Headline

| Category | Low | Point | High |
| --- | ---: | ---: | ---: |
| STL (Dinkumware VC8) template instantiations | 6945 | 10266 | 14592 |
| Scalar deleting destructors (`??_G`) | 1456 | 1456 | 1706 |
| Vector deleting dtors / array ctor helpers | 152 | 152 | 152 |
| Adjustor / vcall thunks | 116 | 116 | 176 |
| Static-object dtor stubs + atexit initializers | 58 | 58 | 112 |
| Ghidra `thunk_` (jmp thunks) | 311 | 311 | 311 |
| Tiny trivial bodies (<=32 bytes, not in rows above) | 2806 | 3301 | 4116 |
| In-house engine (Mit) template instantiations | 1200 | 2000 | 3000 |
| **Remaining hand-reconstruction candidates** | **9093** | **14419** | **18540** |

Roughly half of the in-scope functions fall out of declarations (STL, engine templates, compiler helpers,
accessors) rather than needing routine-by-routine recovery. The STL share is the dominant and least
certain number; the compiler-helper rows are exact pattern counts.

## Size histogram (measured body bytes, in scope)

| Bucket | Count |
| --- | ---: |
| <=8 | 966 |
| <=16 | 1159 |
| <=24 | 1044 |
| <=32 | 2452 |
| <=48 | 5939 |
| <=64 | 3680 |
| <=96 | 4066 |
| <=128 | 2829 |
| <=256 | 4723 |
| <=512 | 2893 |
| <=1024 | 1360 |
| <=4096 | 964 |

## How the STL estimate was built

Two anchors were established first:

- `00bf6713` is **not** an EH prolog: it is `_invalid_parameter_noinfo` (it calls `_invalid_parameter(0,0,0,0,0)`).
  It is the `_SECURE_SCL` checked-iterator failure hook, so its **6664 callers** are functions containing
  Dinkumware iterator/range checks. `00bf681b` is `operator new`; `00bf65ac` (FID-named `_free`) is
  `operator delete`; `00bf5695` is `_String_base::_Xran`; `00408720` is a `basic_string::assign` instantiation.
- Six STL throw-message strings (`vector<T> too long` @00ce37e0, `list<T> too long` @00ce38f8,
  `invalid map/set<T> iterator` @00ce44e0, `map/set<T> too long` @00ce47bc, `vector<bool> too long`,
  `deque<T> too long`) are pushed from **567 distinct functions** (found by scanning `.text` for
  `push imm32`). Each is one `_Insert_n`/`_Buy`/`reserve` (vector), `_Insert` or `erase` (`_Tree`), or list
  node-buy instantiation. They imply on the order of 110 vector types, 135 map/set types and 40-75 list types.

The union of the two signals is 7231 functions. 108 random functions were decompiled and classified by hand,
12 per stratum (size bins 17-32, 33-48, 49-96, 97-256, >256 bytes; separately for flagged and unflagged
functions), to measure signal precision and the STL share the signals miss:

| Bin | Pool | Throw-site | SCL callers | Unflagged | SCL precision | STL rate in unflagged | STL point (low-high) |
| --- | ---: | ---: | ---: | ---: | --- | --- | --- |
| 17-32 | 2474 | 0 | 8 | 2466 | 1.0 (0.9-1.0) | 0.33 (0.17-0.53) | 822 (426-1315) |
| 33-48 | 5809 | 0 | 2846 | 2963 | 1.0 (0.9-1.0) | 0.33 (0.14-0.61) | 3824 (2976-4653) |
| 49-96 | 7426 | 0 | 1185 | 6241 | 0.9 (0.75-1.0) | 0.39 (0.22-0.59) | 3500 (2262-4867) |
| 97-256 | 7498 | 298 | 1565 | 5635 | 0.6 (0.4-0.8) | 0.07 (0.01-0.22) | 1631 (980-2790) |
| >256 | 5137 | 269 | 1052 | 3816 | 0.1 (0.03-0.3) | 0.03 (0.0-0.1) | 489 (301-966) |

Observations from the samples:

- Every sampled SCL caller of 96 bytes or less was an STL method (`begin/end`, iterator `*`, `++`, `--`, `+=`, `-`,
  `==`, `_Tree` iterator `*`/`++`/`--`, `list` iterator `++`, checked `_Copy_opt`, `_Fill_n`).
- SCL callers above 256 bytes are mostly game functions with inlined iterator checks (1 of 12 was container code),
  so the signal must be size-gated.
- Unflagged small functions are still about one-third STL: `_Tree::_Min/_Max/_Lbound`, `_Lrotate`, `_Erase`,
  list/tree constructors (`_Buyheadnode`), `vector::_Tidy`, `allocator::allocate`, `std::fill`, `~_Tree`/`~list`.
- The 33-48 byte bucket (5770 functions) is almost entirely STL and engine-template code; the 97-256 and >256
  buckets are dominated by game logic.

Structural cross-check: ~300 container instantiation types x 25-35 emitted methods each = 7500-10500, consistent
with the sampled point estimate.

## Compiler-generated helpers (exact pattern counts)

- **Scalar deleting destructors**: 1456 functions match `test byte ptr [esp+4],1 ... call operator delete ... ret 4`
  (median 30 bytes, max 768 where the destructor body is inlined).
- **Vector deleting destructors / array-member constructors**: 82 call `eh_vector_destructor_iterator`, 70 call
  `vector_constructor_iterator` / `eh_vector_constructor_iterator` / `eh_vector_copy_constructor_iterator`.
- **Adjustor/vcall thunks**: 116 (`sub/add ecx, N; jmp`); **static dtor stubs**: 54 (`mov ecx, imm32; jmp dtor`);
  only 4 functions call `_atexit`, so most dynamic initializers are either folded or register through another path.
- **Ghidra thunks**: 311.
- Out of scope but relevant to the global denominator: 27369 `Unwind@` and 617 `Catch_All@` EH funclets.

## Tiny trivial functions

4116 functions have a measured body of 32 bytes or less and no STL signal (after removing deleting dtors and
thunks). 579 match exact accessor patterns (`mov eax,[ecx+X]; ret`, one-store setters, `ret`-only,
constant returns, member-address). Sampling of the 17-32 byte remainder shows forwarders, two-field setters,
small vtable-setting destructors and `new T(...)` factories, with about one third being STL helpers (counted in the
STL row, so 815 were subtracted here).

## Mechanical tagging rules

- `stl.iterator_or_container`: calls 00bf6713 (_invalid_parameter_noinfo) AND measured size <= 96 bytes (precision ~0.95 (24/24 sampled); count 4039)
- `stl.container_probable`: calls 00bf6713 AND 97 <= size <= 256 AND calls nothing outside {CRT helpers, other tagged STL} (precision ~0.6-0.8; count 1565)
- `stl.throw_site`: body contains 'push imm32' with imm32 in the six STL throw-string addresses (68 e0 37 ce 00 etc.) (precision ~1.0; count 567)
- `stl.allocator_allocate`: 'if (n) if (0xffffffff/n < K) throw bad_alloc; operator new(n*K)' - calls 00bf6340 with bad_alloc vftable then 00bf681b (precision ~1.0; count not enumerated)
- `stl.tree_walk`: loop 'while (node->isnil_byte == 0) node = node->left|right' with isnil at node+0x15/0x19/0x1d/0x29/0x31/0x99; recursion+free variant = _Tree::_Erase; rotate variant = _Lrotate/_Rrotate (precision ~1.0; count not enumerated)
- `stl.vector_tidy`: 'if (first) operator delete(first); first=last=end=0' (precision ~0.9; count not enumerated)
- `stl.fill_copy`: pointer-stride loop copying a value or range with no other calls (std::fill/_Fill_n/_Copy_opt/_Destroy_range) (precision ~0.8; count not enumerated)
- `gen.deleting_dtor`: calls operator delete, contains 'test byte ptr [esp+4|8],1', ends 'ret 4' (precision ~1.0; count 1456)
- `gen.adjustor_thunk`: 1-3 insns 'sub/add/lea ecx' then 'jmp' (precision ~1.0; count 116)
- `gen.static_dtor_stub`: 'mov ecx, imm32; jmp' (precision ~1.0; count 54)
- `gen.thunk`: Ghidra isThunk (precision 1.0; count 311)
- `trivial.accessor`: <=2 insns ending in ret: getter/setter/const-return/ret-only/member-address (precision ~1.0; count 579)

The full address lists for every mechanical rule are in `templates_and_generated.json` under `address_lists`
(SCL callers split by size, throw sites, deleting dtors, adjustor thunks, static stubs, vector dtors, trivial patterns).

## Caveats

- Estimates for STL and engine templates carry sampling error (12 per stratum); ranges are stated, not a single number.
- Size measurement is a linear sweep; functions with data-in-code or tail jumps into shared blocks may be under-measured.
- The classification says nothing about ABI or byte-for-byte matching; it identifies what can be regenerated from
  declarations (`std::vector<T>`, `std::map<K,V>`, class definitions with virtual dtors) instead of reconstructed.
- No Ghidra annotations were changed.
