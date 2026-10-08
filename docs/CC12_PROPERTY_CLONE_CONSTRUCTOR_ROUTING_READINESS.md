# Selected raw-property clone constructor routing: readiness only

This Source0 packet establishes the routing contract for the Type2/4/5/7/8/9 arms of `008F4F60`. It implements no clone, adds no function/fragment/name credit and does not qualify a native class, owner, CRT, exception path or gameplay behavior. The six selected arms total **390 bytes**. The other arms of the 854-byte parent remain outside this reconstruction claim.

Evidence was read from the existing `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, through verified read-only `bsp.py` queries. Every live query verifies project, program, language and image base. No compiler, Source API, Native target, provider process, old helper or Root active/accepted fixture was executed or accessed. Only this document and its matching JSON report are tracked changes.

## Exact selected routing

The common prefix `[008F4F60,008F4F8D)` borrows the actual source record in ECX, saves it in ESI, reads its unsigned tag at `+4`, and dispatches through the twelve-entry table at `008F52B8`. It does not check a null source. Each selected arm pushes `38h`, calls native `operator_new` at `00BF681B`, and passes the successful fresh allocation as constructor ECX. Constructor argument slots below are measured from **constructor-entry ESP**, after the CALL return address was pushed.

| Tag | Exact arm, exclusive end | Constructor / call site | `[T+4]`, `[T+8]`, `[T+C]` in order | Constructor `+30` result |
| --- | --- | --- | --- | --- |
| 2 | `[008F5075,008F50B5)` | `008EF1B0` / `008F5099` | Actual nullable text pointer loaded from source `+0C` | Zero |
| 4 | `[008F50B5,008F50ED)` | `008EF230` / `008F50D1` | Opaque DWORD from `+28`, opaque DWORD from `+0C` | Zero |
| 5 | `[008F502D,008F5075)` | `008EF2B0` / `008F5059` | Actual nullable text pointer from `+1C`, word from `+18`, word from `+08` | Preserves allocation bytes |
| 7 | `[008F5134,008F5168)` | `008EF270` / `008F514C` | Actual pointer **source + `0C`**, addressing twelve inline bytes | Zero |
| 8 | `[008F5168,008F51AE)` | `008EF2F0` / `008F5192` | Actual pointer from `+20`, byte count from `+24`, literal full DWORD `1` | Preserves allocation bytes |
| 9 | `[008F51AE,008F51FA)` | `008EF360` / `008F51DE` | Element count from `008EF7F0(source)`, actual pointer from `+20`, literal full DWORD `1` | Preserves allocation bytes |

Type4's first argument remains uninterpreted identity bits; this arm and the bounded raw constructor do not dereference it. Type7 uses LEA and passes a pointer, without a float conversion. For Type9 the root is saved in EDI; `1` and the input pointer are pushed before `008F51D6` calls the count helper with source ECX. Its returned EAX is then pushed as the count, and EDI becomes constructor ECX. For tag9, the helper computes **unsigned source byte count `>> 2`**. It supplies no zero/divisibility check. The bounded Source domain therefore requires `byte_count = 4*n`, with `1 <= n <= 3FFFFFFFh`, and a genuine readable span of that size. Nonmultiples would lose their low two count bits and are outside this contract.

Type8 and Type9 always pass copy flag `1`; their retaining branches are not reached by these clone arms. Type8 requires a positive byte count. Text inputs may be null or actual stable NUL-terminated storage under the corresponding constructor contract. All roots, borrowed payloads and the active frame must be disjoint, ranges must not wrap, and source data must remain stable through all allocation/helper calls. Current CRT copy/duplicate paths require DF clear. Neither a semantic container nor a literal native address is an actual payload/provider binding.

## Allocation, ordinal and owner limits

After each constructor returns, the arm rereads source `+34`, writes it to returned EAX `+34`, restores the saved frame/registers and returns the fresh root with a plain RET. The original entry thus uses source ECX and no explicit stack arguments on these paths; Ghidra's stored `undefined ... (void)` signature does not describe this transport. No returned-flags contract is inferred.

**Allocation failure is not a safe null return.** Every selected allocation-null branch joins `008F528A`, loads the source ordinal, clears EAX and executes `MOV [EAX+34h],EDX` at `008F5290`. That path would write address `34h`. Successful nonnull allocation is a required boundary. Separately, an unsigned tag above eleven reaches `008F52A3` and returns zero without allocation. These paths must not be conflated.

The prologue installs an FS exception frame with state `-1` and handler address `00CA4B6D`, inside the current `Unwind@00CA4B62` candidate. Type5/2/8/9 save the allocation and install states `0/1/3/4`; Type4/7 leave the common state. This observation does not recover unwind cleanup, new-handler behavior, historical CRT identity or exception transport.

No selected arm copies source `+30` or publishes a destination owner. Types2/4/7 happen to zero it in their constructors. **Types5/8/9 preserve the fresh allocation's `+30` bytes**, which must remain unobserved as an owner before publication. An initialized phase word or ownership byte cannot replace that missing binding.

Actual insertion `008F33F0` publishes the destination **bag pointer** to record `+30` at `008F345F`, after key/map insertion. It tests record ordinal `+34` before this write. If the ordinal is zero, it copies old bag `+10C` into the record and increments that counter; otherwise it leaves both unchanged. Bag `+110` is a separate owner backlink. Zero is both a valid first ordinal and the unstamped sentinel, so a zero-ordinal clone may be restamped. The semantic model's `max(counter, ordinal+1)` operation is not evidence of this native insertion behavior.

## Observed callers and missing raw binding

Only two current call references were returned for the clone entry. In `008F41F0`, call `008F4263` receives record pointer `entry+8`, after an iterator-entry null check. Key `entry+4`, or the original fallback identity `00F89450`, accompanies the clone to `008F33F0` at `008F426C`, with destination bag EDI. The inspected site has no record-pointer or returned-clone null guard. The fallback identity and real key/map/iterator services still require an actual Source binding.

In `008F54F0`, lookup `0043B8B0` determines BL; `TEST BL` / `JZ` at `008F55DA/55DC` selects the missing-entry arm at `008F5614`. Call `008F561D` receives the source entry's record. The caller redundantly rereads the source ordinal and stores it to the clone at `008F562E`, then uses key accessor `00484D20` and insertion at **`008F563B`**. Existing-record assignment `008F0700` and recursive merge are separate dependencies.

Frozen `scene_property_bag.cpp` copies model entries, strings, vectors and integer bag indices. It does not allocate or bind this native 56-byte record or its original ABI. `scene_property_bag_merge.cpp` invokes abstract `host.clone_record`; current Main `src/` and `include/` contain only that call and the pure virtual declaration. `ScenePropertyCopySpec` describes intended semantics without supplying a raw binding. No stub, fake global, numeric function-pointer dispatch or model-copy substitution was added.

## Admission and independent next contracts

The frozen Main reports admit bounded raw constructors **Type2/4/7/9**. Type5/8 await separate fresh primary complete-helper qualifications. This audit does not consume those active/accepted fixtures or promote either type. Standalone duplicate `00438E40` remains **Source0** independently of a parent's bounded helper use. No constructor admission grants whole-clone, native-private-CRT or class-lifetime credit.

Named dependencies remain explicit: other constructors `008EF140/170/1F0/780/3E0/460` for Types0/1/3/6/10/11; count helper `008EF7F0`; allocation/free/new-handler/SEH services; full insertion `008F33F0`, map insertion `008F28F0`, key and iterator producers; and the six recursive lifetime entries `008F3F30`, `004E6730`, `008F0DE0`, `008F0640`, `008F59E0`, `008F5410`. Existing Source interfaces may cover narrower operations, but this packet does not close their connected clone contract.

The existing whole Type6 raw constructor and ordinary empty-child fragment bind distinct real roots. The current empty-bag producer allocates/initializes its output and rejects nonzero source count. That does not establish nonempty recursive clone, native exception transport, owner publication or disposal. Context-bearing lifetime Source requires genuine producer provenance, matching current CRT and live canonical key/node/pool ownership. `CE89D4`, `D16504` and `D162C4` remain data identities, not callable Source vtables.

Ready independent follow-ups, after actual lease checks and primary review:

- Scope ordinary successful clone fragments to admitted Types2/4/7/9, with canonical current allocation, exact argument/helper/ordinal order, stable real sources and explicit child-before-root disposal. Keep `+30` unpublished and do not claim the whole native clone or its ABI/unwind.
- Finish the independent primary Type5/8 fixture packets; their outcomes must be admitted separately before adding them to such a fragment.
- Audit full raw insertion at `008F33F0` with disjoint files, proving canonical keys/nodes, map insertion, failure/order limits, owner publication and zero-sentinel ordinal behavior.
- Independently audit Type10/11 constructor slices `008EF3E0` and `008EF460`; do not extrapolate stride, overflow, copy or partial-write rules from Type9.
- Defer nonempty Type6/whole recursive clone closure until real producer, map/key/node, ownership and cleanup contracts are connected.

## Retained evidence and verification

The fresh ignored family is `local/clonerouting/` in worktree `J:/PROG/battlestations-pacific-decompile-cc12_property_clone_constructor_routing_readiness`. Its exact all-file manifest covers **248 members**, including scripts, query stdout/stderr/results, local failures and corrections, frozen inputs and verification bookends; only the manifest itself is excluded to avoid recursive hashing. No later family mutation is permitted.

`all_files_manifest.json`: SHA-256 **`9e886996a297c4ffc9b69d89e3ddf03b9adda8faf595b6b5cb8ca0acdd14aafc`**, 44,944 bytes. All 24 current Source pins and all 40 frozen inputs matched at closure. Thirteen Native byte slices, 698 bytes total, matched before/after. Static byte checks verified all six allocation/constructor calls, ordinal stores and returns, the exact switch table, and the null-tail opcodes. Project/program verification remained successful for every live query. An initially malformed local lease-check command and its successful correction are both retained; there were no Native-query failures.

Mutable Main reports/config/docs are historical frozen context, not continuing pins of their original paths. These are static/readiness checks only: no new Source, build, fixture, native-ABI, class, lifetime, runtime or game admission follows.
