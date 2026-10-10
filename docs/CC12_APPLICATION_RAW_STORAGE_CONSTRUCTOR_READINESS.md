# Application raw storage and constructor readiness

**NO_CPP_GO for a dormant raw Application owner; NO_FRAME_ACTIVATION_GO.**
The storage layout and normal field-store order are now bounded. An owner is
still blocked by the singleton base's publication/registration and failed
construction lifetime. Merely adding 1Ch storage and reproducing the final
stores would skip observable base behavior and leave an escaped receiver with
no qualified retirement contract. This review changes no C++ or native analysis.

The Source baseline is `c958f65da2436812f995512ed8a4f720e3f79c81`, after merging
requested main `6ee63daeacb2a318ecaa91b4f28a8a810042273a`. All 4,208 tracked
src/include files, their actual Git blobs, working bytes, and build-support
inputs are retained. The previous publication-authority proof's 124 pins are
copied unchanged and replayed. Its seven historical selected Source epochs and
two prior full Source corpora remain separate; no historical build gets new
credit. Current Source bytes are unchanged from that proof and agree with
requested main in src/include.

## Exact admitted Native scope

Typed complete metadata was captured before any new body read. Both functions
have one owned contiguous range, no gaps, no local branches and one call. Full
listings and exact bytes were then retained, with a repeated metadata capture
at the same modification 79. Each CLI query verifies project `bsp`, program
`/battlestationspacific.exe`; typed records identify
`C:\Users\sqz269\bsp.gpr`. Function count remained 64,730. Correct existing
names, comments and unresolved Ghidra prototypes were retained without edits.

| Entry | Complete owned range | Bytes / instructions | Admission cap |
| --- | --- | --- | --- |
| `00737970` Application constructor | `00737970..00737999` | 42 / 15 | 256 bytes |
| `00BEA970` loop base constructor | `00BEA970..00BEA985` | 22 / 8 | 128 bytes |

Assembly establishes ECX receiver, preserved ESI, EAX equal to the original
receiver on normal return, and plain RET with no explicit stack arguments.
The saved `undefined ... (void)` prototypes do not encode those register facts.
No Native callee, destructor, unwind descriptor, image window, or WinMain body
was freshly expanded. These 64 fresh bytes match the older copied report's
exact bytes. The older report's PE comparison is prior evidence, not a new
installed-image observation. No native execution occurred.

## Same receiver, preimage and normal stores

The copied older report contains the exact 57-byte `008F8410..008F8448` caller
slice. `LEA ECX,[ESP+20]` precedes constructor call `008F8419`; after the two
Initialize argument pushes, `[ESP+28]` at `008F8425` is that same address.
The loop, shutdown and direct destructor calls reuse `[ESP+20]`. This proves
identity within that saved caller slice. The **1Ch extent is inherited from
the earlier full-stack audit** in `WINMAIN_STARTUP.md` and the saved constructor
annotation; this slice and the newly admitted constructor bodies do not
independently re-prove stack allocation size or whole WinMain extent. There is
no newly established heap Application allocation.

`00737970` first calls `00BEA970` on the same receiver. That base first calls
`00BEA810`; only after it returns does the base install DWORD `00D68BC8` at
offset 00 and clear **one byte** at 04. After the base returns, the outer
constructor executes these stores in order:

| Offset | Width | Store / existing Source meaning |
| --- | --- | --- |
| 00 | DWORD | `00CFEAB0`, final numeric Native table identity |
| 08, 0C, 10 | Three DWORDs | Zero actual pointer-vector data/count/capacity |
| 14 | DWORD | Zero actual Game pointer cell |
| 19, 1A | Two separate bytes | Zero, in that order |
| 18 | Byte | One |

Saved complete `BEA810` evidence establishes an earlier `00D68BC4` table store,
manager lookup, captured lock enter/depth increment if present, receiver
publication at `E1AE90`, a second manager lookup, a reload of the **current**
publication for registration, and release using the originally captured lock.
That 145-byte record remains a prior epoch; it was not queried or newly decoded.
The registry is not necessarily handed a cached original receiver if the
publication changes during the intervening calls.

No direct store in the two admitted bodies initializes 05..07 or 1B. Saved base
evidence provides no whole-object clear. Those bytes retain incoming preimage;
their values are unspecified here. Callee/callback effects are not an absence
proof. A future storage declaration must preserve this distinction and the
different preparation stages; aggregate zeroing is not a recovered native
constructor. Offset 04 is not a demonstrated subsystem pointer. Numeric table
identities are not callable C++ vtables and cannot authorize manager teardown.

## Current Source and lifetime constraints

`NativeApplicationPointerVectorStorage` already supplies the actual 12-byte
header at Application+08, with signed count and capacity. Its reserve and shader
preload contracts concern that same storage, including current-value reads and
retained operation frames on failure. The header is not a full Application owner.
`ApplicationShutdownState` stores a projected unsigned count and copied pointers;
its release loop does not establish raw-header ownership or alias identity.
Likewise, `ApplicationFrameState` is a two-boolean projection. Its older comment
calling +04..+14 subsystem pointers is not fresh layout evidence.

The current `GameStartupHost::application_construct` resets projected flags and
sets `constructed_`; destruction only clears that flag. Complete Source searches
and positive definitions establish no raw Application storage or E1AE90 authority
there. Existing raw-manager getter, registration and captured-guard components
are real providers, but their availability does not compose this constructor.

The two admitted shells have no local exception cleanup. If `BEA810` does not
return, neither the loop's +04 store nor the outer vector/Game/flag stores has
run; nevertheless prior evidence places publication before registration in that
base. Failed base construction therefore cannot be equated with unexposed raw
storage. A later fault can also leave a partly written receiver. No automatic
reset, unregister, free, retry, or standard C++ destructor policy is justified
by these shells. Original EH/SEH behavior remains unproved.

Any future owner must remain at a stable address through registration, startup,
all consumers, shutdown, explicit diagnostic resolution and removal from every
publication/registry. The retained `GameNativeGameApplicationFrame` borrows the
actual +14 cell by reference; it does not create or clear that cell, even when
Game cleanup is pending. Its owner and referenced services must outlive the
frame's failure records and all final observations. No projected copy of +14,
temporary header, moved owner, or cast of `GameStartupHost` supplies that lifetime.
The normal Source startup order ends Application destruction before manager
drain; its current boolean implementation does not establish safe raw teardown.

The independent `E1AE78` string cell remains a separate producer/ownership and
retirement contract. This packet creates neither that cell nor a duplicate and
does not couple its retirement to Application construction/destruction. The
prior name schedule and String domains keep their own qualifications.

## Next bounded gate and verification

Next packet: `cc12_application_singleton_base_failure_contract_readiness`.
Before an owner implementation, determine the base's publish/registration
failure states against existing actual manager, guard and registration providers,
including which storage and E1AE90 state must remain retained. Proposed Native
scope is typed metadata first for `00BEA810` (full hull only if at most 192 bytes)
and its already saved handler anchor `00CC7240` (full hull only if at most 64
bytes). Any handler-derived EH table, cleanup body, destructor or other callee
requires its own concrete bounded proposal; no such expansion is authorized by
this review. This gate can establish a constructor failure contract, not grant
frame activation or resolve normal Application retirement/callable tables.

The finite artifact replay checks current complete Source/Git archives, six
whole-corpus queries, twelve bounded excerpts, the unchanged previous proof,
typed ranges, 64 exact fresh bytes and 23 decoded instructions, and copied
57-byte caller / 145-byte base records under separate epochs. A second replay
checks actual Git blobs and working bytes. Physical relocation is checked with
only the declared finite bundle and report. This is static evidence validation;
there were no C++ changes, builds, tests, probes, OS/game calls, GPR mutations,
ledger changes, runtime activation or binary/ABI-equivalence claim.
