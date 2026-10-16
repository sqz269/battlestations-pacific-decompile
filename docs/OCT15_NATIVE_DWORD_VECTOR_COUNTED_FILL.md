# Native counted construction of DWORD-vector headers

Addresses: `00698040`, `0069809B`, `00698310`, `00C7EC10`.

| Address | Coverage | Source form |
| --- | --- | --- |
| `00698040` | complete normal110B/42starts,two ranges | actual constructor loop |
| `0069809B` | complete37B/15starts | lexical completed-prefix catch |
| `00698310` | complete54B/23starts | three-input stdcall dispatcher |
| `00C7EC10` | complete effects17B/7starts,qualified elision | inert private-frame action,no public helper |

Native core uses ECX output,EDX unsigned count,four stacked source/owner/valuebits/
tag words and RET10h. Sourcevoid excludes incidentalEAX. It captures initial,
current and completed output before testing count. Count0 never loads source.
Each nonnull current reloads the source ARGUMENT before actual557590 construction;
the child consumes CURRENT source header/backing each iteration. Null current
skips construction only. Count decrements,cursor advances10h with DWORDwrap,and
completedend publishes only after successful construction/skip. No value snapshot.

The lexical catch captures initial/completed endpoints. Only a nonempty prefix
loads ownerbits. It calls actual4D29D0 forward on every10h header,without a null
skip,and bare-rethrows. The failing element is outside the completed prefix;
its only cleanup is the actual constructor's own scope. No rollback or extra
destructor is added. Constructor and distinct Tidy are complete accepted Source
from Mainb8aa1de49; their currentCRT allocation/free contracts are unchanged.

Native dispatcher has THREE stacked inputs and RET0Ch. IncomingECX owner/tag
bits are semantically unused by actualTidy. Source therefore uses stdcall3inputs,
not a fabricated owner/context register parameter. Four actual core stackslots
are source,canonical owner0,duplicate sourcebits,canonical tag0. Native tag only
clears low byte of savedECX; Sourcezero highbits are a qualified private choice.
CoreEAX is ignored. Return independently uses capturedoutput+(count<<4),DWORDwrap.

Raw FuncInfoDABA18 has magic19930522,maxstate3,mapDABA00:
`(-1,0),(0,C7EC10),(-1,0)`. TryDAB9EC is `[0,1,2,1,DAB9DC]`;
catchDAB9DC is `[40,0,0,69809B]`. State0 precedes counttest. State1 precedes
the nullbranch/constructor,then state0 and completed publication follow advance.
ActionC7EC10 reads saved private current slots,pushes both,calls literal401130
(one-byte RET),adds8 and returns. Full17B action/literalRET/map proof permits
effect elision only for valid Source privateframes/currentCPP behavior. No public
EBP function,no no-op stub,no fake destructor and no FH3 bridge is introduced.
HandlerC7EC21 and literal401130 are support evidence only.

```text
00698088 CALL 00557590
006980AB CALL 004D29D0
006980BB CALL 00BF6885
00698334 CALL 00698040
00C7EC18 CALL 00401130
00C7EC26 JMP 00BF6B43
00698500 CALL 00698310
006985D0 CALL 00698310
006989C9 CALL 00698040
```

Owned code is218B/87current starts. Support is11B/3starts; three bounded unowned
producer windows are115B/44starts,not whole698400 or698980 coverage. All bytes
equal PE/live program; rawEH is96B. Typed complete ranges/current starts/targets
and physical stack schedules are retained in ignored evidence. Current producer
ownership corrects stale snapshot catch labels. Correct STL library names and
existing comments/provenance remain separate from new descriptive Source names.

FIFO claim succeeded before mutation. AcceptedMainc27d1861b was synced with
reviewed Source/provider hashes unchanged; one Source line appended and the
registration lease released before build. Warm scripts/build.ps1 passed and the
existing three suites passed in9.11s. Report checks8CALLs; Markdown checks9
transfers including the separately proven supportJMP,with no failures/unchecked
claims. Focused emittedCOFF passed13checks: core188B/74instructions(including6
INT3padding after182B livebody/catch),dispatcher39B/18instructions,pure8B pointer
helper and compiler-generated29B EHhandler. RET10h/RET0Ch,conditional argument
load,completion/forward cleanup and independent return are retained. Compiler
EH/security-cookie/privateframe machinery is CurrentCpp,not OriginalFH3 identity.
Primary precommit review passed13checks with no findings; Source/header/provider
hashes stayed frozen. No new tests or packet-native/game execution. Existing
suites do not directly prove this routine's aliases,originalABI or gameplay.

Reached accesses/releases and equality loops must be valid/finite in accepted
currentCRT domain. Public aliases retain constructor-current-source and forward
cleanup order. Original private argument/local stack aliases and asynchronous
mutation are excluded. CurrentCpp exception transport,cleanup-failure replacement
and generated frames do not prove OriginalFH3/RTTI/CRT/heap/fault/SEH/nested-failure/
ambient-register/binaryABI/gameplay behavior. No noexcept bridge,guard,callback,
provider,typed header/container,countbridge or extra ownership model is added.
Older actual narrower/Hook Source,fixtures and reports remain unchanged. This
packet does not implement transfer6980E0,general insertion698400,append698980 or
placement parents.
