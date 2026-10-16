# Actual Screen constructor readiness, packet 69

The actual constructor is still incomplete. `initialize_native_gui_page_00ac6600_prefix`
stops at AC6714 with EH state1; neither semantic `GuiPageLuaEvaluation` nor a
`GuiScreenLayerImplementation` is a completed native124h page. Genuine raw
Lua owner/reader/root providers now exist. The first missing raw callee is the
script-path pair AC5600/AC5770, independently assigned by Root as packet71.
The next useful parent packet is the script continuation `[AC6714,AC67CC)`,
stopping before reader construction and the incomplete raw property dispatcher.

This is read-only evidence. Requested Root `f025c2315d3e370a7730839e898cad438a4e3d70`
merged with exit0 into clean worker commit `dd434c96459fe74ac8c4afc7db16c5df0356068d`,
retaining the worker's committed66. Only this doc/report and ignored local
evidence paths were leased/written; no Source, CMake, ledger or Ghidra mutation,
activation, build or new execution proof belongs to69.

## Whole Original and corrected ABI

Before each live batch verified existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Used bounded `lookup`, `show`, `show --asm`, live
funclet assembly, `proto` and `ghidra bytes`; no export/listing repair was needed.
AC6600 is `[00AC6600,00AC6879)`,633B,162 instructions, with RET0C at AC6876.
The listing shows a 4E8h local allocation and FH3 handler CB8A93→DEFA98.

The actual ABI is ECX=raw124h page, stack `(actual8h name header pointer,
actual node pointer, share-scene DWORD slot)`; only the third argument's low byte
is stored. EAX returns the same page. AC662C loads name from ESP+508;
AC66A4 reads the flag byte at ESP+510, AC66AB loads node at ESP+50C, then
AC670F calls AA6720 with it. Caller AA593F/40/41 pushes flag/node/name and
AA5944 calls AC6600. Old prose describing argument2 as share_scene/argument3
as unused and a call at AA5931 is stale. The live Ghidra prototype still says
`undefined(void)`; that is not authoritative ABI evidence and was not changed.

## Established prefix and real dependency map

`native_gui_page_construction.hpp:40..58`, cpp`:35..98` cover exactly
`[AC6600,AC6714)`. They call actual AA9390(type1), stamp D5BE38, initialize
EC/F0/F5/F8/FC, copy the actual name at100 with native reload ordering, write
108/10C/110/114/118/11C/120/121 and20/24, then AA6720 publishes the SAME node
at4C and clears node138 low bits. Padding, E4/E8 and pool124 retain preimages.
The acquired record is explicitly awaiting continuation, not publishable.

| Reached portion | Current genuine Source status |
|---|---|
| AC6718→B66BD0, AC672B→B6A020(mask65) | `native_lua_objects.hpp` has REAL `NativeLuaStateStorage`, size4C8h and actual14h tracked objects. `construct_native_lua_state_00b66bd0` and `open_native_lua_state_00b6a020` exist. They are distinct from `PcStorageLuaOwner`, whose header explicitly rejects native4C8h layout/ABI. |
| AC6748→AC5600; AC678D→AC5770 | First actual raw gap. Existing implementations are `std::string gui_page_script_path_00ac5600` (`gui_layout_loader.cpp:530`) and a std::string/`GuiLeafHost` wrapper (`engine_seg49_64_78_cc13.cpp:1363`). Root assigned genuine raw reconstruction to71. |
| AC675A/AC67A0→B69D40(flag0) | `run_native_lua_file_00b69d40`, genuine actual Lua owner/file/override flow, exists in `native_lua_file_loading`. It requires real VFS dispatch/DoFile/string services and preserves Lua current-handler/fatal errors. |
| AC67DA→B67980; AC67E3→4425C0 | Raw globals and `NativeLuaReaderStorage` exist. The reader is actual14h, preserves proxy04 preimage, consumes the by-value globals argument, and uses actual tracked14h objects. |
| AC6804 reader+4; AC681F reader+8 | `enter_key_00bd8e20` and `pop_back_00bd7130`/BD7A20 projection exist on the same raw reader. No registry-reference/GuiTable substitution is needed. Actual key kind0 and literal D5CB24=`GuiScreen`. |
| AC6812 page+18 | D5BE38+18 actually contains AC4C50. Only semantic `GuiScreenLayerImplementation::before_properties` exists for it. Raw whole AC4C50 is missing. Its AAA710 dependency currently composes only `[AAA710,AAACD4)`; enumeration/children/epilogue remain missing. Do not fill these calls with a base result/default/stub. |
| AC6825→B6D890(node4C,0) | Genuine raw root propagation exists over the same actual nodes/scene attachments. It happens while Lua is still open. |
| AC683E reader vector tidy; AC6857 Lua close | Existing raw reader destructor matches CE44FC→vector cleanup→CE374C. B669A0 actual owner close exists. Neither operation is permission to close/reuse interrupted raw Lua execution. |

`GameNativeLuaServices` borrows actual process Lua globals, VFS strings/manager,
fundamentals getter and Source service bindings (`game_native_lua_services.cpp:12..60`).
`NativeLuaServiceBindings::Activation` scopes every synchronous native Lua entry;
`bootstrap()` and `files()` expose the real required inputs. No replacement
fundamentals/empty VFS/script or duplicate Lua state owner is needed. Existing
bootstrap/file APIs accept `NativeStringStorage&`; proposed path71 uses a raw
pool context. Both must borrow the SAME application's existing publications,
not separate string domains. This narrow API cannot independently authenticate
all wrapper/context backing; normal caller composition must provide that identity.

Another stale prefix comment says raw AA9730 is unavailable. It now exists as
`destroy_native_gui_widget_base_00aa9730` (`native_gui_widget_lifetime.cpp:132`),
including a fresh-prefix cleanup contract. Its current child/timed dispatch,
types/tables and list-remove bindings remain required real inputs. Availability
of the body does not supply those providers or retrospectively add unwind to
the existing retention-only prefix.

## Exact parent order and cleanup obligations

From S=ESP after AC6600's prologue saves, the raw Lua owner is at S+30,
path header at S+10, reader at S+18, with a temporary20-byte globals argument.
AC6718 constructs owner, AC6723 arms state2, AC672B opens mask65. Then:

1. Push obfuscation0, construct BY-VALUE8h `_Common` name using41E870 and actual
   D5CB64; AC5600 consumes that header and writes the caller's raw path temp.
2. Arm state3, run B69D40(flag0). Capture the current path data BEFORE state2;
   then set state2 before CURRENT length+1 and pool getter/BD1510. Consume exactly
   once; a throwing getter is not a reason to retry stale header contents.
3. AC5770 copies actual page+100 into its by-value name and writes SAME path
   temp. Arm state4, run B69D40(flag0), then state2 BEFORE the second path return.
4. AC67CC begins globals/reader work; state5 guards reader; state6 protects only
   its base profile while explicit vector tidy runs. After property read/leave and root detach,
   normal tidy/close leaves the page live and returns at AC6876.

The real seven-state map DEFA98→DEFABC is:

| State → predecessor/action | Exact action |
|---|---|
| 0→-1 / CB8A40 | saved actual page → AA9730 |
| 1→0 / CB8A4B | page+100 →41DD20 |
| 2→1 / CB8A5C | actual4C8h owner →B669A0 |
| 3→2 / CB8A67 | reused path8h header →41DD20 |
| 4→2 / CB8A72 | SAME reused path8h header →41DD20 |
| 5→2 / CB8A7D | actual reader →441A20 |
| 6→2 / CB8A88 | actual reader →410600 (CE374C-only base store) |

This evidence does not itself implement native FH3 or establish valid cleanup
after foreign C++ exceptions cross Lua C frames. A partial continuation must
retain actual storage and consumed-action metadata and fail-stop on interruption;
it must not unconditionally replay the whole map or return the page pool slot.

## Proposed parent Source packet72, after71

Recover exactly `[AC6714,AC67CC)`,184B, through both real script runs. Own new
`include/bsp/native_gui_page_script_continuation.hpp` and
`src/native_gui_page_script_continuation.cpp`; minimally append
`awaiting_reader_continuation` to the prefix Phase enum without renumbering old
values and correct stale comments in `native_gui_page_construction.hpp/.cpp`.
Own unique next doc/report and exact AC6600 continuation/EH addresses above;
Root coordinates resulting ledger/CMake registration. No71 address/file edits.

Suggested typed API: continuation over `NativeGuiPageConstructionAcquired&`, a
caller-retained `NativeGuiPageScriptContinuation&`, and a context borrowing
existing `NativeStringStorage&`, SAME raw path context, actual bootstrap/file
services and actual `_Common` literal. Retain REAL `NativeLuaStateStorage`4C8h,
SAME8h path header, by-value name and71's genuine nested failure/cleanup frames.
No padded callable object, std::string path, hidden string domain or fake call
implementation. Coordinate exact71 public interfaces before editing/linking.

Admit only prefix awaiting AC6714/EH1; establish all persistent backing before
first getter/native/Lua entry. Execute the exact order above. On normal return,
set prefix continuation AC67CC/EH2 and `awaiting_reader_continuation`: Lua stays
OPEN, no reader yet, and the page is still unfinished/unpublishable. Keep a
sticky metadata-only interruption predicate and consumed cleanup states. Preserve
raw Lua current-handler/fatal policy; no new catch/pcall/restoration/default.

Dependencies:71 actual raw path pair (now owned separately), existing actual
Lua bootstrap/files/services, same string/pool/VFS lifetime and prefix. The
packet does not invoke AC4C50/AAA710, create/read a fake page, or claim successful
full-page admission. A focused real-service script-stage check plus canonical
build is appropriate after Source;69 itself ran no new tests. Later reader/
property/children/retirement work remains explicit before AA52A0 can register
the finished native page. Worker70 separately audits raw retirement; no overlap.

Root approved72 in principle after69 closes, conditioned on71's agreed real
API/implementation and a concrete72 frame/lifetime proposal before Source edits.
Source C++ interruption retention at that fragment boundary is explicitly not
Original whole-parent EH equivalence. Root owns CMake/integration coordination.
