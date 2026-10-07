# Connected ordinary landing queue source

This packet reconstructs complete ordinary `006C0B50`, `006BF720` and `0077C7B0` together. Queue directly calls append and route; the route directly reuses the existing complete generic nonlocal-peer/locked-send/serializer Source chain. It is a conditional Source facade over actual borrowed fields/providers, without a GameUnitsHost adapter, complete lower/composite constructor, executable ABI, private EH/fault, network or gameplay claim.

Implementation: [native_landing_queue.hpp](../include/bsp/native_landing_queue.hpp), [native_landing_queue.cpp](../src/native_landing_queue.cpp). [The report](../reports/landing_queue_source_cc11.json) records exact body hashes, call sites, compiler ordering and fixture receipts. Earlier boundaries remain in [LANDING_QUEUE_PRODUCER_BOUNDARIES_CC11.md](LANDING_QUEUE_PRODUCER_BOUNDARIES_CC11.md); the canonical approach head and message84 prerequisite are already integrated. No worker Ghidra/ledger/CMake changes occurred.

## Complete evidence and original interfaces

Bounds are exclusive. All 865 bytes matched configured original PE and live `/battlestationspacific.exe` in `C:/Users/sqz269/bsp.gpr`. Disk Capstone and live instruction listings match 289 starts. The proto summary's old append count75 is stale; fresh disk/live listings both contain76, with no missing starts or requested repair.

| Body | End | Bytes/instructions | Native interface |
|---|---|---:|---|
| 006C0B50 queue producer | 006C0D1D | 461/141 | ECX actual block; stack incoming identity; EAX holder; RET4 |
| 006BF720 append | 006BF7E5 | 197/76 | ECX actual embedded block+84; stack pointer to14h record; RET4 |
| 0077C7B0 owner broadcast | 0077C87F | 207/72 | ECX actual owner; stack message,excluded identity; RET8 |

The append input is five DWORDs/14h, not20h bytes; its containing observer/vector prefix is20h. Static assertions cover all consumed record/slot/block offsets and the minimum A4h block prefix. These storage declarations borrow existing live native objects and do not construct an AirOps object or initialize unused fields. Public class methods add a Source facade receiver/context and are not drop-in native entry points.

## Scan, append and separate lock scopes

Queue captures block+4's nullable tracked section, enters it and increments its actual+18 depth. It captures record base+98, checks count+9C, and on a nonempty range rereads count to form the end from the same captured base. Nonmatching records update the next-sequence bits only when their signed sequence is at least the current signed candidate; increment wraps modulo32. This is the actual ordered scan, not a maximum over records after a match.

The first matching record+4 is captured. Nonnull returns that exact identity after releasing the captured queue section, skipping append and the entire slot/message tail. Null stops scanning immediately and appends a duplicate; later matches and sequences are not consulted. A null incoming token can compare on a successful early-return path, but an append requires a live nonnull actual observer-prefix endpoint.

The new five-word record captures current block+80 BEFORE append: `{incoming, captured holder, next-sequence bits, 00000000, current D7A24C bits}`. The final word is the current binary32 cell's raw bits (normally `3F800000`), not integer1 or a forced tuning default. Append calls complete `00694A60` on the incoming identical-address observer prefix and the actual embedded callback owner at block+84 BEFORE reading vector capacity/count. The vector's base/count/capacity are embedded+14/+18/+1C, hence block+98/+9C/+A0.

On full capacity, append publishes `2*capacity+2`, then calculates bytes modulo32 (`capacity*20` decimal). It uses canonical `00BF55BE ->00BF681B` through existing complete `singleton_lifetime_allocate`, whose ordinary Source malloc/new-handler/retry/throw contract remains explicit. No historical saturation rule is used. Copy loops observe current count and base, copying five individual words in order. The current old allocation is released through canonical `00BF6989 ->00BF65AC`/existing complete Source free, then replacement base is published. Input copy precedes a fresh count increment. Null/invalid placement and failed allocation are outside the admitted successful storage domain; native address guards are not replaced with a safe fallback or rollback. Input and reached arrays must stay live/disjoint across registration/allocation/free.

Queue releases its captured block+4 section even if that cell has changed. Only after the append phase does it capture the separate nullable block+C section for the slot phase. No shared lock is invented and no owner or queue cell is cleared by either unlock. Original private lock-unwind/EH behavior remains outside this ordinary Source port.

## Slot publications, notification and routing

Initial slot count+50 is read before base+4C. The 58h cursor remains captured. For each cursor whose+28 equals the original incoming argument, capture byte+34, store state+2C=4 and timer+30 zero bits. If the captured byte was nonzero, copy current `CE3850` bits (normally binary32 five, `40A00000`) to timer+30, then clear byte+34. These are raw MOVSS transfers, with no arithmetic or new worker numeric kernel.

Then reload owner+7C and invoke existing complete event-word `00696350/00696120` with event4/value0 and its real required lifetime/dispatch/callback context. Construct the real hex84 `NativeLandingSlotMessage84` on an ordinary borrowed38h frame using current game/selected-owner, original block/index, actual slot/class-registry and profile/bank/release contexts. Reload owner+7C AGAIN after the constructor and call the complete route with excluded identity null. No ordinary scalar/destructor or frame retention is added: the original queue tail contains no such call.

After each iteration, freshly read count+50 and base+4C to compute the end, while advancing the OLD cursor by58h and the original index by1. Both old cursor storage and newly selected bounds must remain valid and the traversal must terminate. After releasing the captured slot section, reload block+80 for the returned holder. Preappend record+4, post-notification/current holder and returned block+80 are distinct observations; no task+404 cache substitutes for the original incoming argument.

Route first reads the actual current game cell and its+1FE4 mode; it returns unless mode1. There is no null-game fallback. With null excluded identity it captures message type+10: raw82/81/A7/A8/A9 select current message+14; otherwise call current profile+0C with raw4A and select+14 only if AL is nonzero. The producer admits the real hex84 profile/payload. Other profile/codecs need their own domains; raw49/46 acceptance in that profile does not admit their semantics.

The route captures owner+2A8's initial sentinel to obtain its first node, then samples current sentinel on each iteration/validation. The emitted Source omits only `CMP EDI,EDI`'s proved unreachable invalid-iterator branch at native0077C81B; its call site is still byte/call checked. Other reached invalid-iterator service remains required with no invented no-return behavior. Compare excluded identity against node+8, then on a send read owner WORD+174, node+8 again, publish message WORD+18, read that peer's actual+50 target and reload current game+1EF0 for the complete nonlocal-peer send. Revalidate against fresh sentinel before following the current node's next link.

## Existing Source adoption and required domains

`NativeLandingQueueCalls` derives `NativeUnitHealthMessageSerializedCalls`: no health source/body is copied or edited. It directly invokes the existing common-header `00770B50` selector, which preserves local-target exclusion and the secondary type29 gate; inherited `00783DC0` preserves captured enter/current-section leave; inherited `00783C80` preserves actual delivery cursor, tick/prefix, concrete profile writer, overflow/threshold and required current transport+20 operation.

All inherited globals, D2 profile and pure actual field aliases remain mandatory borrowed Source references. The inherited current-game cell must be the SAME cell supplied by the queue message context. Unrelated inherited health methods retain their own admitted caller domains; this class supplies no substitute health world or unused default providers. The existing complete transport flush Source adapters may supply the actual current operation under their own contracts; no flush success, clock, transport profile or retention policy is invented here.

Registration and notification reuse canonical live observer prefixes/arrays/global publication/recursive sections. Actual callback targets, current bank owners, class-registry getter, profiles, current owners/peers/cursors, allocator/new-handler context and all reached backing lifetimes remain required. Controlled field publications may occur while objects remain live. Concurrent writes, structural reentry invalidating reached storage, nonterminating bounds, invalid addresses, allocation failures, private faults/EH and original global/arena/profile binding remain excluded/unbound. No deferred death/cache cleanup is inferred.

## Focused verification

One ignored actual-TU fixture compiled six fresh Win32 objects: queue, current renamed hex84 message, health sender/serializer, event producer, observer edges and probe. `/O2 /W4 /WX` passed; linked executable has embedded `asInvoker`, PE32 machine14Ch, and passed. The three pinned support-library hashes stayed unchanged before/after.

The same scenario first proves nonnull-record early return, then makes that record+4 null and proves duplicate append/growth8 with only preceding sequence3 contributing new sequence4, ignoring later777/900. It uses real registration on block+84 and real event4/value0 dispatch. Controlled Source callbacks publish owner A->B->C, game cells and preappend/event/final holders. During the real constructor's required getter, it rebinds slots to old base+58/count0 while both slots remain live: old cursor advance meets fresh end, leaving the second slot untouched. Complete peer selection, actual tracked send lock and real serializer/hex84 codecs emit84 bits below flush thresholds; wrong old owner/game produces no send. Decode proves fresh sender/class/count/raw/squad fields. Captured slot lock restores correctly after its cell is rebound, queue/send/observer depths restore, and dispatch interval size restores. These callback/registry/profile/CRT observations are SOURCE instrumentation, not native reentry, runtime provider or network proof. The fixture's unused inherited health/flush domains fail explicitly rather than supply no-op operations.

Generated Source COFF separately confirms register-before-vector reads, capacity-before-allocation, preappend+80 capture, event-before-constructor-before-fresh-owner-route, count/base rereads with old cursor, and final+80 load after captured leave. All12 direct native call rows passed; four lock-IAT sites and the profile query are preserved indirect evidence with runtime targets unverified. Diff checks passed. Root owns CMake registration, serialized full main build/CTests and independent integration/probe.

The natural next consumer is complete `009AFE70` at its actual call boundary: freshly read original input-plane+9D4, call this queue and publish the returned holder to approach+34/task+42C. Its earlier approach+0C/task+404 capture remains distinct. Actual holder creation/destruction never becomes implicit here. Complete lower/composite/geometry/math/state/observer lifetime recovery remains separate; current sparse BotTaskHost/GameUnitsHost projections are not admitted adapters.
