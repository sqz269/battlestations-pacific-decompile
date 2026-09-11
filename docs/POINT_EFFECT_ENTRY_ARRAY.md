# Point-effect retained entry arrays

`src/point_effect_entry_array.cpp` reconstructs the complete bodies at
`008670A0` (reserve) and `008672A0` (resize). Both operate on the existing
`PointEffectReferenceArray` from `point_effect_instance.hpp`: the actual
pointer/count/capacity words at offsets `00/04/08`, with size `0C` on Win32.
There is no second container, count, allocation owner, or implicit destructor.
The constructor `008680B0` calls resize for its array at instance `+0C` after
the cache stage described in `POINT_EFFECT_INSTANCE.md`.

## Target and ABI evidence

Every live query went through `bsp.py ghidra`, which verifies project `bsp`,
program `/battlestationspacific.exe`, x86 language and image base `00400000`.
The configured project file `C:/Users/sqz269/bsp.gpr` was checked to exist.
This worker made no Ghidra mutations. Saved entry names were `FUN_008670a0`
and `FUN_008672a0`, without saved entry comments. Proposed descriptive names
remain hypotheses; the integrator handles annotations and export refresh.

Both native functions receive the header in ECX and one signed DWORD on the
stack, preserve the used nonvolatile registers, and return with `RET 4`.
Reserve's last instruction is `00867191` (inclusive end `00867193`); resize's
is `0086731B` (inclusive end `0086731D`). EAX has no result contract established
by these callers. The reconstructed free functions have a new C++ ABI.

## Reserve

| Native instructions | Behavior |
| --- | --- |
| `008670BC..008670D3` | Clamp signed request to at least one; return if current capacity is already sufficient. |
| `008670DB..008670E1` | Double EAX twice, then call `BF55BE` with that DWORD byte count. No overflow check. |
| `008670EB..0086712E` | Ascending copy, reloading the live count and backing pointer. Null-test destination, initialize null, load source, publish nonnull entry, increment its actual `+04` reference word. |
| `00867130..00867165` | Ascending old-slot destruction. Capture slot and entry; decrement actual `+04`; call virtual `+00` at zero; clear the captured slot after return. Reload live count and pointer on the next iteration. |
| `00867167..0086716A` | Reload the current backing pointer and call ordinary `BF6989` free. |
| `0086717C..0086717E` | Publish replacement pointer first, then the captured clamped capacity. Do not write count. |

Ghidra's incorrect no-return flow at `BF6989` omits `0086716F..00867181`.
Live bytes matched independent decoding of installed disk bytes for the tail:

```text
0086716F  mov edx,[esp+1C]  ; saved replacement
00867173  mov eax,[esp+30]  ; requested/clamped capacity
00867177  add esp,4
0086717A  pop edi
0086717B  pop ebp
0086717C  mov [esi],edx
0086717E  mov [esi+8],eax
00867181  pop ebx
```

The complete executable body includes this tail despite the incomplete saved
decompilation. The integrator must repair flow and refresh the export before
treating a future pseudocode view as authoritative.

`BF55BE` forwards to the established `BF681B` malloc/new-handler retry service;
`BF6989` forwards to ordinary free. The implementation uses the existing
`singleton_lifetime_allocate/free` boundary with equal native/host byte counts
on Win32. It does not substitute `std::vector`, `new[]`, an allocator callback,
or a new allocation domain. Allocation failure propagates before this function
writes the header or any reference; a native new handler can itself alter
external state, and subsequent count/pointer reads remain live.

The reserve EH handler at `C94E17` loads descriptor `DC6C70` and jumps to
`BF6B43`. Its one map entry at `DC6C68` is `(-1,C94E00)`. The funclet
`C94E00..C94E16` calls `401130`, whose body is a single `RET`. The main body
initializes state to -1 and never activates state0; its copy loop writes -1
again at `86712A`. There is no evidenced allocated-prefix rollback or replacement
free action to invent. The canonical retain/release helpers are nonthrowing.

## Resize and lifetime

`8672A8..8672AE` calls reserve only when signed requested count exceeds current
capacity. `8672B3..8672D4` captures the initial current count and writes null to
each newly exposed nonnull destination address; it reloads the backing pointer
for each slot. It does not publish the larger count during this loop.

`8672E0` decrements the actual count before loading the removed slot. The slot
address and entry are captured before decrementing the entry's actual `+04`
word; zero invokes actual virtual `+00` through its canonical companion.
`867309` clears the captured slot after terminal callback return. The next
comparison at `86730F` reloads live count, allowing terminal reentry to change
iteration. The final `867316` always stores the requested count, even if a
reentrant resize changed count to a smaller value in the meantime.

Entries use the established `RenderCommandReference` interface bound to the
owner's existing atomic count and real terminal action. The implementation
never initializes an owner count or creates a companion. A callback may retire
that companion; code accesses only the captured slot after it returns. Caller
ownership must keep the actual header and captured slot storage valid through
reentry. The canonical terminal contract is nonthrowing; this module does not
introduce native exception dispatch for arbitrary throwing vtables.

## Domain and limitations

MSVC Win32 is enforced. Header offsets and pointer size are compile checked.
Slot addresses and count changes use unsigned DWORD arithmetic to avoid adding
C++ signed-overflow or null-pointer-arithmetic behavior. Requests are not
silently range checked: reserve accepts negative input via the native min1
clamp, and the byte product wraps as it does in native code. Valid backing spans,
live entries, and nonnegative resize requests remain caller requirements;
malformed headers or undersized wrapped allocations can fault in native code.

These are complete typed implementations of the two bodies under the existing
companion/CRT service contracts. They are not binary replacements, a complete
point-effect constructor, or gameplay validation. Array destructor `8675B0`,
entry factories, and manager insertion are outside this packet.

## Validation

Direct MSVC Win32 `/std:c++17 /O2 /W4 /WX /fp:strict /MD /EHsc` compilation
passed. The ignored `local/point_effect_entry_array_probe.cpp` is a focused
lifetime fixture using canonical helpers and borrowed actual count words.
It passed duplicate-reference retention, a real CRT allocation failure before
publication, null growth, descending shrink, and terminal reentry whose nested
resize is followed by the outer final-count store. It linked the strict new
source object with `bsp_core.lib` and embedded its manifest. After seed-byte
verification, `scripts/build.ps1` passed both `reconstructed_math` and
`native_math_differential`. These existing CTests do not exercise the new array
bodies; the separate lifetime fixture does. Results are recorded in
`reports/point_effect_entry_array.json`. The worker did not edit source
registries; the integrator must register this new source before the normal
CMake build includes it.
