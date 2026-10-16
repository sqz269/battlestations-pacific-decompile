# Actual nested DWORD-vector count insertion

Complete ordinary code covers559 Native bytes/220 starts; two lexical catches
cover69B/25 and one logical cleanup action8B/2. The10B/2 frame-handler
support and148B EH map stay separate. No global coverage increment is inferred.

The actual owning private fill copy precedes receiver reads and count zero.
Bounds and growth retain repeated CURRENT size reads and wrapping/SAR arithmetic.
Growth transfers prefix, constructs inserted copies and transfers suffix. The
growth catch retains POST-FILL completion, ignores suffix return and remains
active through old-range cleanup, fresh CURRENT old-begin free, cap/end/begin
publication and common normal private-begin free. It adds no rollback.

Short spare-tail insertion has its CURRENT-end cleanup scope only around counted
construction and Source end publication; assigned fill follows in outer-only
scope. Long spare-tail insertion publishes the actual transfer return before
backward swap and assignment. Normal private cleanup only frees captured begin;
the outer failure action tidies the CURRENT private header through actual storage
cleanup. No public EBP helper, callback, typed container or fake provider is added.

Exact Native transfers, including separate read-only frame-handler support:

```text
0069842d CALL 00557590
0069846c CALL 006978d0
006984cd CALL 00696890
006984f0 CALL 006980e0
00698500 CALL 00698310
0069851c CALL 006980e0
00698542 CALL 004d49b0
0069854b CALL 00bf65ac
006985b3 CALL 00698350
006985d0 CALL 00698310
006985ed CALL 00698010
0069862b CALL 00698350
00698639 CALL 00698380
00698649 CALL 00698010
00698659 CALL 00bf65ac
00698579 CALL 004d9e40
0069857f CALL 00bf65ac
0069858b CALL 00bf6885
0069860c CALL 004d9e40
00698615 CALL 00bf6885
00c7ec63 JMP 004cdce0
00c7ec6d JMP 00bf6b43
```

All18 primary checks passed. Independent Source/Native peer review passed22
checks with no findings. Default MSVC Win32 build and existing3 suites passed
in13.79s at8180c34d839f8fd356ebfe50545f2a65585af388. One focused Source fixture additionally
passed zero count, growth, both spare-capacity paths, logical values/order,
distinct ownership, actual owning length error and final actual cleanup. Its
Win32 executable embeds an asInvoker manifest; it executes reconstructed Source,
not the original game binary. No new test suite or framework was introduced.

All1056 actual CMake code bytes/395 decoded instructions were inspected; this
includes6 final INT3 padding bytes/instructions. Source EH is192B/six states/
three tries, with an outer lexical catch replacing the Native cleanup action.
The compiler moves state0 before the first begin read, short state3 before its
current-end read and resets0 before raw end publication. Those accesses cannot
throw CurrentCpp exceptions under /EHsc; original fault/SEH/FH3 scope identity
is not established. Growth state1 still includes common normal private free.

Current allocation/free/error interfaces, Source private frames, dead tag bits
and void results remain qualified. Original CRT/heap/newhandler/RTTI/FH3/SEH/
fault/ABI and gameplay behavior are unproved. Existing tests and the Source
fixture do not establish Native execution, arbitrary cleanup-failure behavior
or memory-leak freedom. Library identities and prior Source domains survive.
Saved annotation and Main publication evidence is recorded in the batch receipt.

A separate independent review of the exact CMake object passed22 executed
checks with no Source/behavior findings. It confirmed192B C++ EH and all
code/relocations. The earlier seven-INT3 count was corrected to six; total
physical code remains1056B/395 starts, or1050B/389 excluding that padding.
