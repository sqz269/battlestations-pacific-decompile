# Raw-pool provider for native path canonicalization

Address: 00BEE390..00BEE512. Source-only candidate; descriptive names are
hypotheses. This packet adds a mandatory raw provider composition to the
existing complete actual-header algorithm. It does not replace that algorithm.

The original routine receives output in ECX and input in EDX, returns the
captured output in EAX and uses ordinary RET. The new overload is an ordinary
C++ interface, with no original register, stack, FH3/SEH or fault equivalence.
Read-only analysis used C:/Users/sqz269/bsp.gpr, /battlestationspacific.exe.

## Required composition

`NativePathCanonicalizerRawContext` requires the actual raw AA8/AA4/AA0 string
pool context, distinct actual 0109DB91 fallback role, SAME live canonical
0109EEA4 feature-word reference and current-locale BF9611 library binding.
Every argument is explicit. Its constructor only borrows those references and
binds the existing concrete runtime-services metadata from those SAME three
pool/control cells. No pool getter, feature load or native operation runs in
context construction; there is no semantic owner, replacement gate or default.

The raw overload is:

```cpp
void* canonicalize_native_path_00bee390(void* output, const void* input,
    NativePathCanonicalizerRawContext& context);
```

Both existing and new overloads reach ONE shared source algorithm. The existing
`NativePathCanonicalizerContext` and BDB970 discard wrapper retain their API
and constructor binding. Only the final-constructor helper is overloaded. Raw
completion directly calls `construct_native_string_header_0041e870_exact`,
which forwards that SAME feature reference to exact41DD40 and genuine BF7680.
It does not use the older bool-preserve raw facade or std::memmove facade.

The parent retains the whole context and every borrowed pool, manager,
allocator/library, fallback, feature/CPU/FP/DF and lower-provider domain through
the complete graph, including failure disposition. Context metadata does not
give native raw8 storage birth, prove an extent or adopt an existing allocation.

## Preserved schedule and evidence

| Native evidence | Preserved source order |
| --- | --- |
| BEE39A..BEE3C1 | Initial count selects uninitialized stack260 below256, otherwise wrapping count+1 allocation. |
| BEE3C3..BEE3DF | Read DATA after allocation, select actual0109DB91 on zero, copy through first NUL, reload count. |
| BEE3E1..BEE4C2 | Keep captured DWORD end, slash/backslash, dot and parent cursor/barrier operations unchanged. |
| BEE4C4..BEE4CF | MOVSX byte into required current-locale BF9611; retain returned low byte. |
| BEE4E6..BEE4F0 | Write final NUL, then call genuine41E870 constructor using captured output. |
| BEE4F5..BEE512 | Only after normal constructor completion, release heap scratch and return output. |

No validation, ASCII lowering, retry, retain, cleanup, catch or failure recovery
is added. Inconsistent counts/NUL extents and unresolved-parent expansion keep
the existing valid-native-access obligations. A lowercase or output-constructor
escape still retains heap scratch; the lower exact constructor may have changed
pool/header/copy state. The caller retains the incomplete attempt and all debt.
The raw output constructor clears an existing output header without releasing
its old allocation, exactly as the existing algorithm's native call requires.

For the AEEAF0 parser, directory and canonical output remain DISTINCT raw8
objects. Canonicalization output is immediately returned under the parser's
actual cleanup schedule; it is never assigned back to directory by this packet.
No parser, texture cache, CMake, publication authority or startup is changed.

## Source-only validation and integration

The ignored packet manifest pins the base commit, whole old/candidate HCPP,
every finite unique seam and its old/new inverse. Original complete HCPP copies
are supplied under inverse/. Reversing the seams reconstructs each original
file exactly. The shared body is byte-identical after reversing its sole final
constructor-dispatch seam, and the BDB970 suffix is unchanged. Existing exact
constructor/resize/provider/CMake inputs have read-only whole-file pins.

This packet performed static source/evidence inspection and deterministic
seam/inverse/body-identity checks only. It did not compile, add tests, run the
Game, execute original code, mutate Ghidra, alter tracked files or claim ABI,
fixture, runtime, activation or gameplay proof. Existing historical tests do
not validate the new overload. Integrator review and authorized strict MSVC
Win32 build/existing relevant checks remain required before activation.

Both cpp dependencies are already registered with bsp_core: existing BEE390 in
cmake/startup.cmake and exact raw41E870 in CMakeLists.txt. No build graph edit is
proposed. Apply only these owned seams after checking current whole-file pins;
if pins drift, rebase and review the unique seam/inverse proof before building.
