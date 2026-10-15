# Complete raw POD8 array operations

This packet reconstructs all five complete raw-array functions over their actual consumed owner prefix: data pointer at+0, unsigned size at+4, capacity at+8. It is not the checked-vector proxy/begin/end/capacity header, a virtual object or a host container. `NativePod8RawArrayStorage` exposes those 0Ch bytes without implicit initialization. Source entries accept the actual owner address, with an unused EDX adapter word; that word is not an Original parameter.

| Native physical span | Source entry | Observed behavior |
| --- | --- | --- |
| `00414010..00414067`, 88B | `set_capacity_native_pod8_raw_array_00414010` | Unconditionally publish capacity=n, allocate wrapped n*8, copy current size, conditionally free late current data, publish allocation; size unchanged |
| `00414070..004140C8`, 89B | `grow_native_pod8_raw_array_00414070` | Unconditionally capacity=capacity*2+2 moduloDWORD; same complete reallocation schedule |
| `00418EA0..00418F22`, 131B | `append_native_pod8_raw_array_00418ea0` | Grow only for size==capacity, then reload data/size/value and append; increment actual size DWORD |
| `00418F30..00418F98`, 105B | `resize_native_pod8_raw_array_00418f30` | Reallocate only when unsigned n>size AND n>capacity; always set size=n afterward, including shrinking branches |
| `00418FB0..0041900C`, 93B | `reserve_native_pod8_raw_array_00418fb0` | Reallocate only when unsigned capacity<n; size unchanged |

All five consume ECX as an actual pointer. Grow has no stack input and plain RET; the other four consume one stacked value/count and RET4. No semantic EAX result is established. Existing `TMPL_Pod8Array_*` library-role hypotheses and prior evidence are retained. The historical ReserveIfLarger label at18F30 does not remove its unconditional size write; new elements are not initialized.

## Concrete allocation/free and ordering

Whole live thunk listings establish `BF55BE -> BF681B` for allocation and `BF6989 -> BF65AC -> BF9DC8` for free. There is no virtual dispatch or missing provider. The two actual private Source bridges bind the existing `singleton_lifetime_allocate({object, bytes, bytes})` and `singleton_lifetime_free` functions. They do not install callbacks, fake heap state or a substitute allocator. Current malloc/new-handler retry/throw and matching free are the declared Source service boundary; original static CRT heap, global new-handler, exception identity and private child frames remain separate.

Each reallocation stores capacity before calling allocation. All arithmetic is literal DWORD ADD/LEA, including multiplication wrap and zero-byte requests. There is no element-count overflow check, capacity normalization, early null success or rollback. Copy length is the CURRENT size, not the new capacity. The loop reloads size at every unsigned comparison and data at every copied element. It tests the current destination cursor for null; a zero cursor skips both source reads and stores but still advances index and cursor. After the loop it reloads the current owner data for the conditional free, then publishes the captured allocation.

If the allocation provider throws, this entry performs no later data/size publication and adds no rollback. Provider or real new-handler side effects remain; unchanged prior fields are not promised when a handler mutates the actual owner.

Append reloads size/data after any reallocation, then loads the physical incoming value slot only if its computed destination is nonzero. Its value is consumed after old storage is freed. A value aliasing retired storage is not cached or repaired; callers must keep every reached source live. A zero computed destination skips both source reads/stores and still increments size.

## x87 and alias semantics

The exported pseudocode's integer assignments are misleading: every actual copy uses x87 FLD/FSTP for each binary32 word. The Source retains those exact instructions and their ambient rounding/status/exception behavior. Word0 is loaded, its source address is captured before the first store, word0 is stored, then word1 is freshly read and stored. A word0 store can change the word1 source content. The owner data field is not reloaded between those two words; it is reloaded for the next element. No memcpy, bitwise pair, cached value or numeric conversion policy replaces this schedule.

All actually reached owner/stack/data/control accesses must be backed and loops must terminate under the literal DWORD schedule. Malformed fields and native aliases are not made safe. Source child frames and current CRT/EH remain qualified; literal kernel byte identity does not prove arbitrary original private-frame aliases or Native runtime compatibility.

## Coverage and verification

Every live batch used target-verifying `bsp.py ghidra` against existing project `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86/base00400000. Complete five-body assembly, all reached direct dependencies, old names/comments and typed body ranges were read before implementation. No Ghidra changes or reimport occurred.

The five physical spans total506 bytes. Ghidra's admitted body ranges total503 bytes because PushBack excludes unreachable three-byte padding at418ECD..418ECF; its literal `8D 49 00` bytes are preserved in Source. All five complete Source sections match466 bytes outside ten actual CALL displacement operands. Every masked operand is verified as REL32 to a positive defined allocation/free bridge; both bridges have real public current-provider bindings. Live byte captures equal the installed PE bytes. No branch, x87 instruction, field operation, return, padding or unknown relocation is masked.

Strict MSVC x86 C++17 `/MD /O2 /W4 /WX /EHsc /fp:strict /permissive-` compilation passed without warnings. One ignored combined executable linked the new object with the real current integrator archive and passed all five Source entries: actual growth/free; masked-x87 signaling-NaN quieting; alias-sensitive second read; resize/shrink/reserve/no-growth; append with/without growth; wrapped byte count reaching actual allocation0; null computed-destination skip; and real current allocator failure retaining the capacity published before the call. New slots were explicitly initialized by the fixture caller before later copying. The fixture uses `/MANIFEST:EMBED`; no framework, mock, callback or new test target was added.

The physical reference census finds zero incoming CALL/JMP/address candidates for each function in scanned `.text/.rdata/.data`; it does not establish dynamic reachability or adoption. The standalone probe calls the new Source APIs directly. `reports/goal15h_pod8_raw_array.json` retains exact spans, bytes, relocations, Original ABI, direct call rows and evidence limits. Temporary captures, comparer and probe are under `local/goal15h_pod8/`.

Primary integration owns CMake registration, final registered-object/archive proof, the required Win32 build/existing suites, annotations/save/exports and main integration. The worker changed no shared CMake, registry or Ghidra state. Source compilation, exact kernel bytes and the focused Source fixture are separate from captured-Native execution, Original CRT/RTTI/EH/SEH identity, installed binary equivalence and game validation; those are not claimed.
