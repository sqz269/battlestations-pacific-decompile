# Raw singleton lifetime manager discovery

The discovery below records its original snapshot. Since then, the raw vector leaves, allocation/length throw, reserve/constructor, insertion/registration, tracked-section release and unregister/reorder packets have been integrated; current packet states are in `config/parallel_work.json`. The complete raw manager and registry getters are now available under explicit borrowed-publication and source C++ exception contracts, with compiled cleanup review in [NATIVE_SINGLETON_PUBLICATION.md](NATIVE_SINGLETON_PUBLICATION.md). The executable's former no-op menu lifetime callbacks have also been replaced by application-owned effect-manager destruction through the existing typed source interfaces, with a 180-frame shutdown run verified in [GAME_SINGLETON_RUNTIME_OWNERSHIP.md](GAME_SINGLETON_RUNTIME_OWNERSHIP.md). Raw mixed-owner destructor dispatch and canonical executable migration remain open; the bounded executable binding does not complete them.

The next coherent complete-function packet is **`native_singleton_vector_leaves6`: BCF910[19], BD0160[30], BD0180[47], BD0220[42], BD0500[48], BD0560[46], totaling 232 bytes**. These raw vector leaves have a concrete existing CRT memory-service boundary. They remove full-function dependencies from both insertion and exceptional cleanup. They do not make the raw manager or registry getter B1B730 ready by themselves.

The full manager construction/registration/destruction dependency family is **16 functions / 1,645 bytes**, plus **BCFCA0[115] unregister and BD0D70[474] reorder**, for **18 functions / 2,234 bytes** of raw lifetime operations. This is larger than the starting six-address proposal. Already complete raw BD1860 and 411EE0 are reusable dependencies, not missing work. Original CRT internals and current registered-owner dispatch remain explicit service/integration boundaries, rather than being hidden inside a typed-manager cast.

This read-only discovery starts at `a90f7a0a`. It owns only this document and [the report](../reports/native_singleton_manager_raw_discovery.json); no C++, tests, builds, ledgers, Ghidra annotations or installed files were changed. Descriptive names below are hypotheses or existing repository names, not recovered symbols.

## Evidence and complete boundaries

Every live query used `tools/bsp.py ghidra`, which verifies project `bsp`, program `/battlestationspacific.exe`, x86 language and image base before its batch. The configured project file is the existing `C:/Users/sqz269/bsp.gpr`. Fresh guarded captures cover **33 complete function bodies / 3,353 bytes**; Capstone decoded every byte of each body. **43 disk-backed spans / 3,884 bytes** match the saved image, including padding, unwind actions/maps, imports and profile slots. Three separate four-byte saved virtual spans at 1090AA0, 109DD64 and 109EEA8 are analysis-image evidence only, not live game state or disk bytes.

Immutable evidence is in the worker's `local/singleton_manager_raw/`. Its seal contains 242 artifacts and 34 source/configuration pins, with both exact and LF-normalized source hashes. The report records the absolute path, PE hash, transfer graph, signatures, boundaries and service qualifications.

`sealed.json` SHA-256: `d992c6b6d68ae04a03e4be9a8b3d09657e8e5a0dd3482d56f9171ef127ad2d8d`.

| Entry | Complete inclusive extent / bytes | Current source boundary |
| --- | --- | --- |
| 415350 | 415350..4153B8 / 105 | Typed domain getter |
| BD0960 | BD0960..BD09AF / 80 | Typed constructor |
| BD0600 | BD0600..BD06A4 / 165 | Empty-constructor reserve fragment |
| BD08D0 | BD08D0..BD0958 / 137 | Same-owner fragment; no foreign-owner interface |
| BD0BC0 | BD0BC0..BD0C28 / 105 | Typed append |
| BD0C30 | BD0C30..BD0C56 / 39 | Typed registration |
| BD0700 | BD0700..BD08B3 / 436 | Count-one insertion fragment; general count paths missing |
| BCFEB0 | BCFEB0..BCFF04 / 85 | Internal typed allocation helper; source bad-alloc boundary |
| BD0500 | BD0500..BD052F / 48 | Internal copy fragment, no original result/stack ABI |
| BD0560 | BD0560..BD058D / 46 | One-value stores, no full count-fill entry |
| BD0160 | BD0160..BD017D / 30 | No reconstructed source |
| BD0180 | BD0180..BD01AE / 47 | No reconstructed source |
| BD0220 | BD0220..BD0249 / 42 | No named full source; actual EH dependency |
| BD0400 | BD0400..BD04C4 / 197 | Typed destructor with noexcept callback boundary |
| BCF910 | BCF910..BCF922 / 19 | Typed count method |
| 41CC80 | 41CC80..41CCBF / 64 | Existing typed/raw-layout sources need actual allocation pairing and capture correction |
| BCFCA0 | BCFCA0..BCFD12 / 115 | Typed unregister |
| BD0D70 | BD0D70..BD0F49 / 474 | Typed reorder; null-owner validation branches excluded |

BF681B, BCFEB0 and BD0590 end in genuine exception-throw calls on their terminal paths. That differs from the stale returning-free flow gaps at **BD0689**, **BD0801**, **BD04A8** and **BD0230**, each containing the missing `ADD ESP,4`. Complete installed decoding includes those instructions and the field stores after them. No repair was performed in this packet.

## Raw representation, getter and registration

The native manager allocation is **14h bytes**: untouched DWORD+0, begin+4, end+8, capacity-end+0Ch and actual tracked-section pointer+10h. Each vector element is a four-byte registered pointer. The checked iterator is eight bytes, owner+0 and position+4. Inspected complete bodies neither install a manager vptr nor store destruction/validation callbacks in this allocation. Offset0's broader library meaning remains unclaimed.

`SingletonLifetimeDomain` and `ConcreteSingletonLifetimeManager` instead contain a C++ vptr, callback aggregate, slot aggregate, owned-section pointer and projection. `OwnedCriticalSection` also contains projection storage beyond the actual 1Ch-byte section. Their raw pointer cannot be cast to read native manager+10h. The next migration must bind the canonical existing 01090AA0 ownership; constructing a second private domain would split registered-object destruction.

415350 has no consumed input, returns EAX and plain RET. It captures current 01090AA0 once. A nonnull fast path returns that capture without allocating or locking. A miss allocates14h, stores the allocation spill, arms state0, calls BD0960 for nonnull storage and publishes returned EAX. The null branch publishes zero. It does not reread publication for the return and has no publication lock or atexit call. Its failure action C5E0D0 reloads allocation from `[EBP-10h]` and frees it without clearing publication. FuncInfo D8418C points to the one-entry map D84184: state0 to -1, action C5E0D0.

BD0960 consumes ECX raw manager, returns the captured owner in EAX and RET. It writes zero only to +4/+8/+0Ch, arms state0 before reserve256, calls full BD0600, then complete raw BD1860, and finally writes the returned actual section to +10h. It leaves +0 and the preexisting +10h untouched before that last store. Its state0 cleanup is **CC5490 -> BD0220**, reloading `[EBP-10h]`; FuncInfo DFF4E8/map DFF4E0. If reserve throws, the empty slot fields are cleaned; if section creation throws, the reserved slot storage is freed. The original section allocator has no compensating cleanup for an exception inside InitializeCriticalSection.

BD0C30 is ECX manager, stack object, RET4. It validates current end against begin **before** testing the current object argument for null. A returning handler continues, then the actual argument slot is passed by address to BD0BC0. No duplicate check, retain or refcount increment exists. BD0BC0's fast path reads the current value slot, stores at captured end and advances that captured end by four. The growth path captures end before validation, creates a same-owner iterator and passes an actual eight-byte result buffer to BD08D0. Incidental return-register values are not a declared object-return contract.

## Reserve, general insertion and exact helper contracts

BD0600 is ECX vector, stack requested capacity, RET4. It compares unsigned request to 3FFFFFFFh and invokes BD0590 if too large. Current capacity is zero when begin is null, otherwise the 32-bit `SUB; SAR 2` result. Equal or smaller requests do nothing. Growth allocates through BCFEB0, captures end in EDI, validates against current begin, captures begin in EBP, then validates that captured begin against current end. If handlers return, those retained identities are not recomputed. Nonzero captured distance is copied through memmove_s with equal destination-size and count.

After copying, reserve **reloads current begin and current end to capture retained count**, frees the then-current begin, reloads requested capacity from its original stack word and publishes replacement begin, capacity-end, end in that order. The populated path therefore survives the returning free. Neither a cached pre-copy size nor the old decompiler's early return is correct. There is no native C++ exception frame freeing replacement storage if an intervening returning service throws.

BCFEB0 consumes ECX unsigned count; EDX is zeroed by callers but not an input. Zero allocates zero bytes. Positive count uses `FFFFFFFFh / count >= 4` to admit count*4; overflow constructs/throws bad_alloc. Reusing the actual allocation service preserves an explicit source CRT exception boundary, not original RTTI or throw-stack identity.

BD08D0 consumes ECX container and stack `(result, iterator_owner, position, value_slot)`, returns result in EAX, RET10h. It captures begin first. Null begin or zero captured size makes index0 and **skips iterator-owner validation**. Otherwise it validates end/begin, then validates owner nonnull and equal to the captured container. A returning handler does not repair or substitute owner automatically: index still uses captured begin, the original owner is forwarded to BD0700, and **ECX remains the original container**. Full BD0700 never reads that iterator-owner argument. Thus foreign-owner continuation inserts into the original container using the original position; it does not redirect mutation to the foreign owner.

After insertion BD08D0 captures new begin, validates current end, forms captured-begin+index*4, validates that result against current bounds and writes **result+4 position before result+0 owner**. Preserve aliases to the result buffer and the final returned buffer pointer. The current typed source does not expose this result object or the foreign-owner branch.

BD0700 consumes ECX container and stack `(iterator_owner, position, count, value_slot)`, RET10h. It reads `*value_slot` immediately into its own argument storage **even when count is zero**. For nonzero count it checks max-count minus current size, grows capacity by 1.5 with the native overflow-to-zero candidate, and raises the candidate to size+count as necessary. Reallocation copies prefix, fills copies, copies tail, reloads current old size, frees current old begin and publishes begin/capacity/end. The native schedule has no replacement-allocation cleanup frame.

The two in-place paths are distinct. If unsigned tail length is smaller than count, it copies the entire tail to position+count*4, fills the excess region at current end, increments current end and assigns `[position, old_end)`. Otherwise it copies the last count slots past old end, stores the returned end, shifts the remaining middle range backward and assigns count slots at position. BD0160 and BD0180 are real missing callees; they cannot be discarded merely because initial registration appends one object.

The six recommended leaves retain these exact contracts:

| Entry | Complete behavior / proposed source ABI |
| --- | --- |
| BCF910 | Null begin gives0, else raw32-bit `(end-begin) SAR2`; `uint32_t __fastcall(const void* owner, void* unused_edx)`, RET |
| BD0160 | Assign from `*value_slot` anew on every four-byte step until pointer equality; cdecl `(first,last,value_slot)`, RET; no ordering/bounds repair |
| BD0180 | Compute signed element distance, derive destination-begin even for nonpositive distance; call memmove_s only when positive; cdecl `(first,last,destination_end)`, EAX derived begin, RET |
| BD0220 | Capture current begin, free nonnull capture, then zero owner+4/+8/+0Ch in order; fastcall `(owner,unused_edx)`, RET; retain+0/+10h |
| BD0500 | Compute raw signed-shift distance, precompute advanced destination, memmove_s only for nonzero count, ignore error result, return precomputed destination; stdcall `(first,last,destination)`, RET0Ch; incoming ECX unconsumed |
| BD0560 | Unsigned count loop, reread `*value_slot` for every store, return initial destination+count*4; stdcall `(destination,count,value_slot)`, RET0Ch; incoming ECX unconsumed |

Use exact 32-bit pointer arithmetic rather than C++ subtraction of unrelated pointers. Source ABI qualifications cover volatile-register/fault-site differences and actual CRT service behavior; a new wrapper must not silently capture a fill value once or turn a signed test into unsigned.

## Teardown, reentrancy and exceptional ownership

BD0400 arms state0 before the first BCF910 call. It repeatedly captures end, validates the tail through returning BF6713 calls, loads the registered pointer and reduces the **current** vector end before dispatch. For a nonnull object, BD047F loads the object's current profile, BD0481 loads its current slot0, then BD0485 calls that target with **ECX=object, stack flag1**. It adds no per-object unregister. It rechecks count after every call, so a destructor can append, null earlier slots or change the vector storage; newly appended objects participate in the same drain. Null holes are popped without dispatch.

Normal completion calls 41CC80 on the address of manager+10h, then frees current begin, then zeros +4/+8/+0Ch. State0 remains active throughout. **CC5450 -> BD0220**, reloading `[EBP-18h]`, is the exceptional action; FuncInfo DFF490/map DFF488. A throwing registered destructor therefore reaches vector cleanup without normal lock teardown, and the manager allocation/publication are not freed/cleared by BD0400 itself. The existing typed `destroy_registered` function pointer is noexcept and excludes this behavior.

41CC80 captures both the original owner-slot address and its current section once. Null capture returns without a slot clear. It tests the captured section's physical depth+18h as signed-positive, captures the LeaveCriticalSection import for the loop, decrements before each Leave, then calls DeleteCriticalSection, actual free on the captured section, and only afterward clears the original slot. The `random_threads.cpp` source uses `delete` on a referenced pointer and repeatedly accesses that reference; the typed manager source owns a larger projection. Neither should be silently substituted for this exact malloc-backed/captured raw section teardown.

WinMain **8F8449..8F846C** captures current01090AA0, calls BD0400, frees that captured manager, then clears publication. Publication remains reachable during destructor callbacks. No registry-specific atexit registration occurs in the manager/getter family. The atexit call reached by operator-new failure belongs to the CRT bad-allocation support object.

BCFCA0 ignores a null argument, scans with current count/bounds validation and writes null to the first matching slot without shrinking. BD0D70, called by A88770 sound initialization, removes the first matching object and reinserts it after the first remaining anchor. Both searches must succeed; returning validation does not introduce recovery. Its explicit null-owner branches and captured iterator/end schedule belong in the eventual raw family even though neither is a direct B1B730 construction call.

## Actual CRT and registered-owner boundaries

The existing `singleton_lifetime_allocate/free` are real malloc/new-handler-retry/free services. BF65AC is a returning jump to BF9DC8, whose complete native body has both small-block-heap and HeapFree paths. Those CRT internals are not reconstructed by a host std::free call. Likewise BF681B's bad_alloc objects, init flag and atexit entry are library-owned state; preserving a real allocation boundary is different from asserting raw static-CRT closure.

Full native BF67A7 memmove_s returns success for zero count. Bad pointers or insufficient destination size set current errno and call BF66EF with five zeros; after a returning handler it returns EINVAL/ERANGE. Its valid path uses BF87E0 memmove. Vector helpers ignore its returned status. Actual host memmove_s is a concrete existing service, but original errno and encoded-handler ownership need an explicit agreement for malformed-storage claims.

BF6713 itself passes five zero cdecl words to **BF66EF**, which loads current encoded handler **109DD64**, calls completed raw **C04FDE**, and tail-calls the decoded nonnull handler with the original five arguments. Null decode calls **C04EF3(2)**, then tails **BF65BB `__invoke_watson`**. C04EF3 ignores its argument and clears DWORD109EEA8. Watson's complete captured machine body has a RET if the terminal OS call returns, despite its saved noreturn signature; a source fatal/no-op/throw substitute must not be inferred.

The only bounded direct writer of109DD64 is **BF65B1**, a ten-byte cdecl store. Its observed caller BFBDFB `__init_pointers` obtains encoded null through **C04FD5 -> completed C04F67**, then supplies that value to BF65B1. The initializer has other CRT pointer-family calls which are outside this manager packet. Saved zero bytes in109DD64 do not establish a live initialized null handler. Current callbacks and `SystemInvalidParameterRuntime` are interfaces, not a completed BF6713/BF66EF ownership implementation. In particular, `game_hosts_menu.cpp` currently passes no-op lifetime callbacks; their mere existence does not satisfy the raw boundary.

BD0590 is also a complete native throw-site, not a completed raw provider: it constructs the literal `vector<T> too long` through408720/411700 and throws through BF6885, with string cleanup4072D0. The existing source uses the real `std::length_error` service. A future packet must either state that source exception boundary or separately reconstruct the native exception machinery; it cannot claim the original105-byte body was already ported.

Finally, the native BD0400 slot0 call has **no EDX context**. Completed registry B1B710 source requires added EDX `NativeResourceRegistryDeleteBindings` carrying actual F8D41C, pooled-string storage and invalid-parameter service. Its constructors still write original **D5E59C** identity. A source raw manager cannot dereference that identity as a rebuilt C++ vtable or directly call the new scalar without supplying its owned bindings. The canonical integration needs a fixed, evidence-backed mapping from reached owner profile/slot identities to completed source terminals, covering every owner admitted to that manager. A generic placeholder callback or registry-only mapping for a mixed manager does not close this edge.

## Concrete next packet and B1B730 integration

Implement the six leaves in `include/bsp/native_singleton_vector_leaves.hpp` and `src/native_singleton_vector_leaves.cpp`, with `docs/NATIVE_SINGLETON_VECTOR_LEAVES.md` and `reports/native_singleton_vector_leaves_audit.json`. The primary owns CMake registration, ledgers, Ghidra annotations and packet metadata. Functions should use the fixed existing actual free/memmove services, retain raw owner storage and expose the ABIs in the table above. No allocation domain, generic callback or std::vector replacement is needed for this packet.

Then complete raw BCFEB0/reserve/general insertion/checked insertion/append/register, with the explicit invalid-handler and length-error service contracts. Follow with raw getter/constructor/destructor/section release plus unregister/reorder and the canonical registered-owner dispatch binding. Constructor and destructor C++ cleanup must model the newly identified BD0220 actions, while excluding original FH3/SEH stack compatibility and arbitrary mutable unwind-spill aliases unless separately proved.

Full B1B730 remains200 bytes. Its verified native path obtains raw manager+10h, enters the actual section and increments physical+18h, rechecks currentF8D41C, allocates/constructs, publishes, **calls415350 again**, then reloads currentF8D41C for BD0C30. It decrements/leaves captured section and returns publication reloaded after Leave. A registration exception leaves publication in place. Its existing full registry constructor and raw guard providers can be reused after the manager and terminal ownership contracts are concrete. No second private manager or typed-object cast is a valid shortcut.

This packet establishes finite native/source evidence and a ready complete-function leaf packet. It makes no build, fixture, original caller-ABI, hardware-fault, runtime or game-validation claim.


Primary revalidation checked all 242 sealed artifacts and 34 source pins, then
reread all 46 finite spans through the guarded BSP client: 43 disk-backed spans
(3,884 bytes) and three separate saved virtual DWORDs. Every captured span
remains equal. The only current source drift is unrelated mission/menu wiring
in game_hosts_menu.cpp; its inspected no-op lifetime invalid-parameter callback
and registration remain unchanged, so it still does not close native CRT
ownership. The immutable primary evidence is local/singleton_raw_primary/.
This is completed discovery, not completion of the raw manager family.
