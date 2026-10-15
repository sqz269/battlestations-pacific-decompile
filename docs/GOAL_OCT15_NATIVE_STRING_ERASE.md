# Native string vector unordered erase

Addresses: `0043EA00`.

`src/native_string_vector_erase_unordered.cpp` supplies the complete ordinary
C++ body over the existing actual eight-byte string and twelve-byte vector
storage contracts. It uses `NativeStringRawPoolContext` to reach the existing
concrete singleton, allocation and return providers. It adds no host dispatch,
allocator substitute, iterator model or unresolved-call stub.

| Routine | Body | Coverage | Original ABI |
| --- | --- | --- | --- |
| `BSP_NativeStringVector_EraseUnordered` | `0043EA00..0043EA72` (115 bytes) | complete, inherited provider and Source interface qualifications below | ECX actual vector; stack iterator-pointer-cell address; `RET 4`; no semantic result |

The descriptive native name already existed and remains a hypothesis rather
than a recovered symbol. There was no indexed reconstruction at this address;
the separate checked-vector erase body at `004954F0` is a different routine and
uses a different header layout. The existing actual vector authority is
`NativeStringVectorStorage` (an alias of `NativeMeshWeightNamesStorage`): data
at +0, count at +4, capacity at +8. Its reserve/append producers initialize and
publish that layout. Each element uses the existing length +0 / pointer +4
native string layout. This erase reads only vector +0/+4; it never reads
capacity. The new source asserts the established sizes and field offsets and
does not declare a conflicting record.

The Source interface is:

```cpp
void __fastcall erase_native_string_vector_unordered_0043ea00(
    void* actual_vector, NativeStringRawPoolContext& strings,
    const void* iterator_cell);
```

Source receives the additional raw-pool context in EDX. The original overwrites
EDX from its sole stacked word and has no meaningful incoming EDX argument.
This is a new Source interface, not an original ABI replacement.

The native schedule is preserved:

1. Read current vector count, then data, then the pointer in the iterator cell.
   Compute `last = data + count*8 - 8` with wrapping Win32 DWORD arithmetic.
   Keep both native equal-last tests; their arithmetic uses the same captured
   count/data registers, without rereading the vector between tests.
2. Unless iterator equals that last header, read the last length and call the
   accepted raw-pool string resize with preserve=1. After it returns, reread
   the last length. If nonzero, read destination length, source data and
   destination data in native order, and copy destination length bytes.
   The original CRT helper accepts overlap, so Source uses `std::memmove`.
3. Reread CURRENT vector count then data and recompute its last header. The
   accepted raw-pool single-header destructor implements the native inline
   release: read data once; if nonnull, read length+1 with DWORD wrap, resolve
   the actual pool, and return the captured block/size. The header stays
   untouched. Null data skips the length read and pool getter entirely.
4. Only after release returns normally, reread and decrement CURRENT count.
   The release path can expose changed vector/header storage; captured values
   are not substituted for the native later reads.

There is no empty-vector guard or range/owner validation. A last-element erase
skips assignment and only releases that element. A non-last erase deep-assigns
the last content, then releases the current last element. There is no explicit
capacity, backing pointer, iterator-cell or stale trailing-header write. The
iterator cell may alias external storage: only its initial value is loaded,
and subsequent vector/string writes retain their natural alias effects. No
exception handler, catch, rollback or error conversion is introduced. A getter
exception prevents the later decrement; earlier resize changes remain.

| Native call site | Native callee | Concrete Source contract |
| --- | --- | --- |
| `0043EA27` | `0041DD40` | Existing `resize_native_string_header_0041dd40` raw-pool overload; actual destination, captured source length, preserve=true; child `RET 8` |
| `0043EA3C` | `00BF7680` | Existing CRT body read; supports backward overlap, so `std::memmove`; caller `ADD ESP,0Ch` |
| `0043EA5E` | `00419CC0` | Existing actual singleton getter, no explicit arguments; pending block/size/1 words remain on caller stack |
| `0043EA65` | `00BD1510` | Existing actual raw-pool block return through `destroy_native_string_header_0041dd20`; child `RET 0Ch`; supplied size selects the pool class |

Each callee body was read before these contracts. The string resize and pool
getter pseudocode carry corrected assembly-derived ABI notes. The complete
erase assembly settles the register captures and rereads, pending stack words,
copy count and cleanup. The CRT body explicitly has a backward-overlap branch.

The only live xref is the CALL at `00BDD07D` in `00BDD020..00BDD09D`,
`BSP_VFS_RemoveContentPackageName`. Its full listing was read: receiver is
manager+48h, and the sole stack argument is the address of its local iterator
pointer. It ignores child return state, keeps its cursor on a match, and
reloads vector count/data before the next end comparison. This settles the
iterator-cell contract without inventing iterator output or cursor advancement.
The caller belongs to the primary integrator's separate packet and is not
modified here.

Live queries verify project `bsp`, program `/battlestationspacific.exe`, x86
language and image base through the configured client; the configured project
file is `C:/Users/sqz269/bsp.gpr`. Annotation uses the existing write lock,
records prior values, preserves the existing plate comment, saves the project,
and refreshes the affected export. No game installation changes are made.

Verification results are recorded in
`reports/goal_oct15_native_string_erase.json`. Compilation and existing suite
results do not establish native execution, binary ABI, visual or game behavior.
No new tests are added. Source omits only the accepted zero-byte CRT copy while
retaining all header reads, inherits actual pool/manager/current-CRT provider
limits, and does not reproduce private stack aliasing, instruction fault
addresses, native binary identity or native exception/unwind metadata.
