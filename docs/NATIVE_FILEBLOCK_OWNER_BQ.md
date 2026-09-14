# Native FileBlock owner and loading-job preparation BQ

Addresses: 00BE0A30, 00BDCB30, 00BDEBE0, 00504790, 00CC6740, 00CC6748, 00CC6753, 00CC6210, 00CC6218, 00CC6223, 00C68F90, 00C68F9B, 00C68FB4

Four reconstructed complete ordinary bodies cover 513 bytes and close the direct FileBlock owner/preparation source dependencies of the loading queue. The new explicit-service interfaces retain partial state on escaping calls. They do not implement the native outer FH3 unwind actions. Production loading-queue binding remains separate work.

The owner is **1Ch bytes**, not28h. Its actual layout is profile0, reference count4, zero DWORDs8/C/10, and the native name header14/18. BE0A30 takes ECX owner and stack name/gate DWORD, returns that owner, RET8. It writes baseCEB130, count1 and derivedD68494; initializes the remaining words; copies the input name unless its header is identical; invokes the existing BDF950 identifier; then reads current0109CEEC and enters that manager through the BP scope body. Identity still clears the embedded header first. Only the low gate byte affects the scope.

BDCB30 takes ECX owner, RET. It stamps D68494 and reloads current0109CEEC for exit. It does not remember the construction manager. After exit returns, it captures name data before advancing native state1 to0, obtains current length+1 and uses the raw throwing pool getter/return service. Base BD30F0 then only stamps CEB130. The name header and reference count retain their values. BDEBE0 takes flags, RET4, returns the captured owner and frees it only when bit0 is set after normal destruction. There is no reference decrement.

504790 takes ECX actual job, RET. Existing job+20 or zero name length+14 skips. It captures the name-header address, allocates1Ch before reading the current count for substring(start13,wrapping length-18), constructs FileBlock(temp,1), and publishes returned EAX at job+20 before returning the current temporary name. It adds no prefix/minimum-length validation. The substring and identifier are existing reconstructed bodies, with their documented string-storage and internal cleanup limits.

The ordinary source preserves current DWORD read order, captured pointers before callbacks, output publication and explicit native call sites. Frame bookkeeping is a new source interface, allocated before invoking the native-shaped body. Publish and retain each immovable frame and all borrowed storage if a call escapes. Active/failed destruction terminates. It does not replay, free retained allocations, reset names, or issue a compensating scope exit.

## Native exception evidence and source limits

| Owner | FuncInfo / unwind map | Native actions |
|---|---|---|
| BE0A30 | E00DB0 / E00DA0 | state1 to0: CC6748 destroys name through41DD20; state0 to-1: CC6740 stamps baseBD30F0 |
| BDCB30 | E00650 / E00640 | state1 to0: CC6218 destroys name; state0 to-1: CC6210 stamps base |
| 504790 | D922D0 / D922B8 | state1 to0: C68F9B tests/clears completed-temp flag then destroys temp; state0 to-1: C68F90 frees captured allocation. The third map row routes state2 to-1 through the same temp action; no ordinary state2 store occurs in the complete body |

The constructor arms base cleanup before name construction and name cleanup only after copying. The destructor advances to state0 before the ordinary getter/return, so a failure there only leaves base cleanup armed. Job preparation arms allocation cleanup after allocation and temp cleanup only after substring returns; normal publication precedes state-1 and temporary return. No outer owner action performs a VFS exit during construction failure.

These maps describe original execution. The current source deliberately retains outer state on any escaping call because the BP/BO observer interfaces can retain borrowed native names and nested frames. For example, after an observer throws, the native constructor would unwind name/base and the job would unwind temp/allocation; this interface retains them. That is an explicit incomplete exception contract, not native EH parity. Source identifier/substring internals can still perform their existing cleanup. NativeStringStorage releases remain noexcept, while the ordinary owner/job final name returns use NativeStringRawPoolContext so lazy getter failure can propagate. Omitted zero-length memcpy, original CRT/FH3/TLS/CLR identity and arbitrary native stack aliases remain limits.

One ignored source fixture uses the actual string pool, raw VFS manager fields and native gate helpers. With the native observation byte disabled, it checks allocation/substring/publication, destruction after switching the VFS publication, flags2/1, self-name construction, low gate byte and adjacent storage. Another mode injects a deliberate observer exception: allocated owner/name/temp and nested frames remain live, the already-linked gate remains, and job publication/depth updates have not happened. It intentionally keeps fixture storage alive until process exit. This is a source-retention test, not concrete PakRegistry behavior or native-unwind execution. The concrete observer bodies have the separate BO/BP fixture evidence.

The report records strict compilation, fixture receipts, native byte/membership and call checks, saved annotations, repairs, and combined-build status separately. No original game executable is invoked or changed. Gameplay and production loader wiring remain incomplete.
