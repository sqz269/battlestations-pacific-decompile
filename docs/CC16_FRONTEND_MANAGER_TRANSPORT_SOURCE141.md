# Actual frontend and GUI manager transport

Source141 implements the frozen Source125 six-file transport contract, with
this document and its report as the two new files. Baseline
`836e2736c913cebaedb5eade0cc4c599e1b14302` is the normal merge of Root
`a6cdb63c60c2d8435b2117ffcc86b546a0b39cde` into the worker's committed Source140.
It includes Source135 V2 and Source137 Page operations. Source140's separate
Root-owned observed-zero friend remains outside this patch. No Host, Page,
Font, CMake, ledger, lifetime or activation file changes here.

## Explicit once-bound route

`GameFrontendHost::bind_native_gui_pages(Page&, Manager&)` cold-binds the
existing Page, manager, GUI resource application and actual VFS raw string
context once, before startup. The existing manager/resource bindings must be
present, with no prior resource operations. The manager has a private matching
frontend/Page/resource tuple and its existing published manager/Page lifetime
binding/string context. This is Source transport history, not Native object
birth or readiness authority. The caller supplies the genuine same Page and
manager graph; `Page.load_page` also rejects a receiver that is not its own
actual published manager. No public boolean, profile or count authenticates it.
Cold binding also requires the manager's actual-resource registry to be the
SAME canonical owner registry used by the resource application's renderer.

The bound tuple selects a dedicated raw startup function. It reuses the same
existing font-registry, descriptor and fingerprint prelude operations, then
returns the actual `004C12B0` getter result and enters the fixed resource list.
It creates no semantic page registry, page handle, layout widget, scene Group
substitute or parallel Native owner bank. The unbound legacy startup and its
projection remain the default. The existing `gui_startup` HCPP is unchanged.

## Recovered calls and slot order

The complete saved `AA5E20` listing (442 instruction starts), `AA0E00` and
`AA0E50` listings were read. Typed Page APIs are the published Source137
operations. This packet supplies the actual caller transport, not a claim of
Original whole-frame ABI/EH identity.

| Entry in existing `kGuiManagerResources` | Actual transport |
| --- | --- |
| `data/interface/textures/whiteGui.tga` | Real VFS false skips `+28`; accepted renderer return publishes `+28`, including null. |
| `interface/textures/common/transparent.tga` | Load, release CURRENT old `+2C`, then publish result, including null. |
| `_Mouse` | Whole Page `AA5840(actual_manager, actual_name8, 1, 0)`; store `+4C`; cleanup name; reload `+4C` for true visibility. |
| `MousePtrFE_Icon` | Name construction, CURRENT manager `+4C` parent, whole Page-owned `AA7E00`; store `+54`, then `+50`; cleanup; defer hiding. |
| `MousePtrGUI_Icon` | Same CURRENT `+4C` parent; store `+58`; cleanup; CURRENT `+58` false visibility, then CURRENT `+50` false visibility for FE. |
| `_Highlight` | Actual returned Page Widget is captured as the following Child parent, matching EDI; it is not stored in a manager slot. Cleanup then true visibility. |
| `hl_FrameBox`, `hlCircle_FrameBox`, `safezone_43_FrameBox`, `safezone_169_FrameBox` | Whole raw child lookup on the captured Highlight Page, store `+74/+78/+7C/+80`, cleanup each name, then reload that CURRENT slot for false visibility. |

Only after the final visibility returns does the private manager operation
write actual byte `+84=0` at `AA6305`. It never reads the previously unwritten
byte as readiness. Fixed counters admit this one Source sequence; they confer
no Native allocation, object-class or lifetime proof.

Every non-texture resource has a nonmoving retained actual `NativeString`
header and two physical mutable DWORD lookup homes. The raw VFS string pool
constructs the name. Before child lookup the homes are staged as actual
`[name_header_pointer, 1]`; the second HOME is unread by `AA7E00`. Group calls
receive actual returned Page Widgets, and child results are unretained borrows.
Normal stores precede name cleanup; visibility follows cleanup as in the
recovered caller. Entered failure keeps the header, homes, Page records and
remaining debt. No raw pointer enters `ManagerState`, `GuiLayoutPage/Widget`,
`page_handles`, or the semantic loader's `error.what` catch/continue path.

## Texture credit publication

`load_initial_texture_into_actual_manager` uses the same existing retained
VFS resolution/cache/renderer operation as the legacy resource entry. Private
manager admission occurs before Native entry. The operation records its actual
manager, result, pending handoff, publication and old-slot release stages.

At normal renderer64/B319B0 return, white publishes at the `AA5EA4` epoch.
Transparent invokes the existing whole `release_native_gui_resource_slot_0041da80`
on actual `manager+2C` only after renderer return (`AA5EF2`), so it captures
CURRENT old storage there, releases it and clears it before the new `AA5F1A`
publication. It does not retain a cached pre-load old value.

All throwing admission precedes publication. The actual slot store and Source
credit handoff have no intervening callback or throwing call. The operation
clears its ownership and records manager transfer exactly once, with no copy
or additional AddRef. A white VFS rejection advances only private sequence
history and never writes `+28`. Accepted-null still records an actual null
publication. Resource retirement skips transferred references and never clears
actual manager slots; actual `AA6340` owns their later release. Failed or
pending handoff cannot be retired or replayed. Legacy resource ownership is
unchanged.

Publication occurs before temporary cleanup and logging in the retained load
operation. Its pre-existing independent temporary-string scopes remain
qualified: Original `AA5E20` keeps white's enclosing name through the final
byte84 write, while this application cleans each operation's own name. Using
existing `41E870` string construction and whole callees does not recover the
Original enclosing FH3/SEH stack, instruction/FLAGS/fault schedule or ABI.

## Actual pointer operations and failure custody

Once bound, the public manager getter returns its actual manager. Each getter
or pointer operation owns a retained call record before Native entry and is
serialized against startup and other pointer calls. Actual `AA0E00` writes
byte `+48` first; only false then reloads actual CURRENT `+50` and uses the
Page-owned CURRENT `+34/AA8530` visibility API. Actual `AA0E50` reloads `+50`
and calls the same current-slot operation without touching byte48. There is no
`GuiManagerPointerState` alias or projected visibility call on the raw route.

Raw startup/pointer/resource failures mark the actual owned records before
effects, retain partial slots/credits/name/frame backing, record the exception
and rethrow. The existing frontend/manager/resource failure aggregates include
entered or failed transport. No guessed Native string/texture cleanup, rollback,
retry, new termination policy or public success/birth verdict is added.

Normal resource retirement is not a last-use acknowledgement. Root must keep
the frontend tuple and all borrowed Page/manager/provider backing alive through
real callback, queued, worker and drain last use, and route every failure to its
retention fence before destruction. Source132 lifetime, Source126 Text/FrameBox,
actual Icon services/callbacks, Font activation/retirement, Parent/Material/Section,
worker custody/quiescence and all nine gates remain external. Other semantic
screen, title, sprite and input consumers are not migrated or activated here.

The three changed registered TUs are strict MSVC x86 object-compiled with
`/std:c++17 /EHsc /MD /O2 /fp:strict /W4 /WX`, then built through the official
Win32 build and its existing two checks. Receipts retain actual COFF/library
provenance. No new tests/probes, Game run, installed-file mutation or Ghidra write.
Build/static evidence establishes neither runtime nor whole-lifecycle parity.
