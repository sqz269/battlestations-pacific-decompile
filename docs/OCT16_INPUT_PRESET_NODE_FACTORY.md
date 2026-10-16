# Preset tree node factory

| Scope | Coverage | Original interface |
| --- | --- | --- |
| 0069F5F0 | Whole ordinary entry, 126B/44 instructions through 0069F66D | ECX/EDX unconsumed; five stacked words; RET14h; EAX saved allocation |
| 0069F66E | Complete logical catch, 21B/7 instructions through 0069F682 | Private runtime EBP scope; no callable Source Native ABI |
| 00C7EF80 | Complete inert placement effect, 17B/7 instructions through 00C7EF90 | Private runtime EBP scope; effect-elided, no public helper |

The whole Source factory uses the published pair constructor and the existing
current-CRT allocation/free services. It adds borrowed actual RawCtx and
0109EEA4 references: seven stacked Source words and RET1Ch, returning the saved
raw allocation. The existing `STL_Tree_Buynode_StrKeyCI_N2C_0069f5f0` name is
retained. The descriptive Source name is provisional. No Node overlay or
host container is introduced; Native argument shape comes from the assembly.

Allocate raw 2Ch bytes before entering the construction try, with the existing
`SingletonAllocationRequest {object, 2Ch, 2Ch}`. Allocation failure escapes
without reaching this catch. Preserve the explicit null-result branch: null
does no node initialization or pair work and returns the saved result. The
accepted current allocator normally returns nonnull or throws; it is not
replaced with a callback or a fabricated null-producing allocator.

For nonnull, write left, parent and right DWORD bits at +0/+4/+8 in order,
then construct the actual 1Ch pair at +0Ch. Ignore the child's EAX. Only after
successful construction read the CURRENT private color argument's low byte,
store color at +28h, then store nil=0 at +29h. Return the original saved node.
The factory adds no other writes, including to padding +2Ah/+2Bh. Pair layout
is length +0Ch, data +10h and five opaque mapped words +14h through +24h.
The published pair service retains its progressive representation copies,
initial key resets, current-field reads and provider failure boundaries.

On a primary Current-C++ construction exception, free only the saved raw node
and bare-rethrow. There is no key, mapped-payload or pair destructor, rollback,
reset, retry or abandoned-key repair. Source does not use the child's return
value to choose the pointer to free. Both named fragments describe logical
Source scopes, not callable Original catch or unwind interfaces.

| Scope | Native site and actual target |
| --- | --- |
| Owned allocation | 0069f613 CALL 00bf681b |
| Owned pair construction | 0069f64a CALL 0069e7a0 |
| Owned catch raw free | 0069f672 CALL 00bf65ac |
| Owned catch rethrow | 0069f67e CALL 00bf6885 |
| Owned logical placement | 00c7ef88 CALL 00401130 |
| Readonly handler | 00c7ef96 JMP 00bf6b43 |
| Readonly published pair resize | 0069e7be CALL 0041dd40 |
| Readonly published pair copy | 0069e7d3 CALL 00bf7680 |
| Readonly producer in 0069FBC0 | 0069fc44 CALL 0069f5f0 |

The raw Native EH data at DABE60..DABEBF is fully verified. FuncInfo DABE9C has
magic19930522, maxState3, three unwind entries, one try [0,1], catchHigh2 and
one catch-all. State0 unwinds to -1 without an action; state1 unwinds to0 via
C7EF80; state2 unwinds to -1 without an action. Both saved slots -14h/-18h
contain the original allocation before state1 and its null test. The placement
action passes those two saved words, invokes literal 00401130 (one byte C3,
RET), removes eight argument bytes and returns. This proves an inert logical
effect under the admitted private-frame/fault boundary. Its call-frame writes
and fault PCs are not reproduced; no empty placement stub, fake destructor,
public EBP reader or synthetic Native frame is created. C7EF91, the literal
return and EH data are readonly evidence, with no new Source promotion.

Published Main 57b4a23ad686ebc12e9fd9d252ee43347249d487 contains the accepted
E7 Source pin aabd4c5afb43b7a047ff549b2fbd2479f8210372. Fourteen concrete
provider files match that baseline. RawCtx borrows canonical AA8/AA4/AA0
publication/gate/manager cells. The separate feature reference must bind
actual0109EEA4; PE/Ghidra initial zero is not runtime feature truth. Existing
pair, raw-pool, CPU/OS, valid-extent, current-CRT malloc/_callnewh/bad_alloc and
free policies remain explicit boundaries. This is not an Original heap or
exception-metadata replacement. Old generic AV/NativeStringStorage projections,
all prior frozen receipts and the E7 annotation/export closeout remain unchanged.

All node/provider storage must satisfy reached accesses and exclude active
private argument/return/handler frames and borrowed context/cell storage.
Address offsets use wrapping DWORD integer arithmetic. Source preloads private
pair/context/feature argument values before allocation and reads the right
private link argument after the parent store; Native has a different private
load schedule. These are qualified Source argument-frame differences, without
extra pointee reads. The late volatile private color-byte read and material
node store order are retained. Current C++/CRT, GS checks, compiler FH3 tables,
registers/flags and private frames do not establish Original binary ABI,
FH3/SEH/RTTI/fault/heap/runtime or game equivalence. No FBC0 fixup/count/length
path, additional caller or full map owner is promoted.

Default MSVC Win32 build and all three existing suites pass in 7.76 seconds;
they do not specifically exercise this new factory. No new tests, probes,
target Native execution or fixture/game validation are claimed. Full fresh inspection
is 298 code bytes/109 starts plus96 EH bytes: owned union164B/58, with readonly
support/reference134B/51 separately counted. Entire actual CMake code was read:
285 executable bytes/101 instructions, plus six trailing CC padding bytes,
across five raw code sections291B/107 linear starts. Public181B/61, lexical
catch20B/6, handler39B/12 (including two executable entry NOPs) and unused
helpers45B/22 are distinguished. All15 relocations resolve to the intended
current providers, cookie machinery and generated EH maps. Generated88B EH
collapses the inert placement states to maxState2, try [0,0], catchHigh1 and
two NULL unwind actions. Allocation is at state-1 and the saved-node free-only
catch has no hidden pair cleanup. SAFESEH/debug/directive/checksum data and the
unreferenced four-byte weak STL BSS (zero file bytes) are retained separately.
Frozen-pin primary review is pending; the companion report records exact hashes.
