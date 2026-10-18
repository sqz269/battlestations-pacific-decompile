# Lane oct18-tC, 2026-10-18

The packet is `codex_tail_C.txt` from the cc16-progress worktree: 106 exact
addresses, leased as `oct18-tC-tail`. No host wiring or game execution is part
of this packet. Descriptive names remain hypotheses. The scalar accessors use
MSVC Win32 naked functions to preserve FLD/RET without an intermediate float
spill; numerical units and ownership lifetimes are not inferred.

## Batch 1: raw leaves and compiler boundary

All 14 bodies below are in `src/engine_seg11_35_66_cc13.cpp`, namespace
`bsp::oct18_tail_c`, declared in `include/bsp/oct18_tail_c.hpp`. The existing
engine TU is separately leased and already registered in the build. This avoids
touching another owner's CMake lease. Searches of `include/` and `src/` found
call sites, virtual service declarations or typed projections, but no existing
standalone raw bodies for these leaves.

Evidence: saved per-address Ghidra pseudocode and listings under
`exports/bsp/functions/<address>/`. The live connection was verified against
the configured `bsp` project and `/battlestationspacific.exe` before annotation.

| Address | Behavior | Original ABI |
|---|---|---|
| 00A3E460 | Borrow owner+32h, accepted-invite block | ECX owner, EAX address, RET |
| 00A90ED0 | Empty input enumeration slot | ECX owner, no result, RET |
| 00AA38E0 | Load visibility byte at F5h | ECX layer, AL byte, RET |
| 00AA6BE0 | Store reader to actual F8BC84 cell | ECX reader, RET |
| 00AAB5E0 | Store callback to actual F8BCFC cell | ECX callback, RET |
| 00AD56D0 | Load terrain/foliage field at 3Ch | ECX owner, EAX word, RET |
| 00AD5720 | Load scalar at 0Ch | ECX owner, ST0 float32 extended, RET |
| 00AD57D0 | Load bush texture at 9Ch | ECX owner, EAX word, RET |
| 00AD57E0 | Load bush normal texture at A0h | ECX owner, EAX word, RET |
| 00BBDD30 | Load atmosphere scalar at 20h | ECX owner, ST0 float32 extended, RET |
| 00BBDD40 | Load atmosphere scalar at 28h | ECX owner, ST0 float32 extended, RET |
| 00BBDD60 | Borrow wind direction at 2Ch | ECX owner, EAX address, RET |
| 00BBDD70 | Load atmosphere scalar at 38h | ECX owner, ST0 float32 extended, RET |
| 00BBDDB0 | Load skybox field at 18h | ECX owner, EAX word, RET |

The two callback setters expose the actual cell as an explicit reference, hence
use a new C++ ABI. The other leaves preserve their observed register conventions,
but have no native replacement or application/game validation claim. The layer
visibility method also exists as a typed GUI fragment; the new body reads the
actual raw byte and returns it without converting arbitrary byte values to bool.

`00B83137` is a compiler-generated catch funclet, not a game function: it consumes
the parent's EBP+14h work pointer, calls `_free` at BF65AC and rethrows through
`__CxxThrowException@8` at BF6885. The seven-instruction listing ends at B8314B.
The existing `SceneRegistryVectorInsertFreeCatchBoundary` source projection stays
intact; the function is classified `library_instantiation_out_of_scope` rather
than adding another body or claiming native FH3 unwinding.

Validation: scripts/build.ps1 passed the Win32 Release build and both existing
CTests (reconstructed_math and tool_tests). No new tests were added. This is
source/build evidence; no original-body differential or game run was performed.

## Batch 2: complete ordinary storage bodies

Fifteen additional bodies use the same registered TU and header. Apart from the
two naked entries, their new C++ interfaces accept mandatory, address-labelled
callee bindings. They are implementations over actual borrowed storage, not new
host stand-ins. The dependency contracts come from the pseudocode, call-site
listing and existing named APIs. Checked-iterator failure BF6713 is allowed to
return. Library code remains a dependency; no CRT body is reconstructed.

| Address | Source body | Original ABI / evidence detail |
|---|---|---|
| 00A3EAC0 | active_user_slot | ECX owner, EAX word at 11Ch, RET |
| 00B6DAB0 | node_local_position | ECX node, XYZ stack; exact 12 instructions; replace argument by node+B0h and JMP captured virtual38 |
| 00A2C720 | ai_group_contains_entity | ECX group, entity stack, AL bool, RET4; checked sentinel walk |
| 00A11AF0 | ai_enable_torpedoes | ECX command, RET; retain group identity checks and entity capture across virtual5C |
| 00A15490 | ai_close_attack_tick | ECX command; target leader point, target pass, reloaded landing target, tail A11AF0 |
| 00A15500 | ai_defend_position_tick | ECX command, RET; followers, formation, own leader point, null target |
| 00A10D50 | ai_merge_strength | ECX command, other stack, EAX bool, RET4; own ST0 spills double before other's unspilled ST0 |
| 00A10C60 | ai_merge_distance | ECX command, other stack, EAX bool, RET4; preserve x87 float spills, operand order and unordered comparison |
| 00A07E40 | ai_build_entity_records | ECX group, EDX output vector, class stack, RET4; actual 20h caller temporary |
| 00A7A980 | sound_dsp_resize | ECX array header, signed count stack, RET4; reserve/grow/null-fill/shrink, publish count last |
| 00AC2090 | gui_line_describe | ECX Line, visitor stack, RET4; two-word tagged pairs by value; actual Points literal bound explicitly |
| 00AC2950 | gui_line_reset | ECX Line, RET; retain guards/copy binding, reset count, ensure4, geometry0/element0 resets |
| 00B8F100 | group_world_sphere | ECX Group, EAX group+13Ch, RET; both dynamic uncached aggregation and static affine branches |
| 00BD91F0 | release_whole_file | receiver unused, buffer stack, RET4; explicit BF6989 free binding |
| 00BE7AB0 | cache_file_from_vfs | ECX store, name/flags stack, RET8; original name and flags, memory-view/add, memory/source release order |

The group-sphere body closes a real gap in the pre-existing
`native_point_light_supported_world_sphere` projection, whose dynamic cache-miss
branch remains partial. Historical records are preserved. The new body binds
the actual aggregation callee instead of assuming a static or cached group.

`00AD3A60` and `00ABF590` are classified as compiler-generated MSVC scalar
deleting destructor wrappers. Both test flag bit0, conditionally free, and
return the original receiver. AD3A60 resets the base vtable then frees; ABF590
calls the separate game destructor ABF4F0 then the Section pool free ABEC40.
Their existing ownership projections remain. This classification does not
classify the underlying game destructors as vendor code.

Validation: Win32 Release and both existing CTests passed. Assembly names `in`,
`out` and `dx` initially collided with MASM reserved words; the local identifiers
were corrected and the build rerun successfully. No added tests or host wiring.
Source normal control flow is complete for these bodies. Native FH3/SEH, fault
transport, private call-stack identity and gameplay remain unproved.

## Batch 3: lifetime, aliases, Lua and file operations

Sixteen new ordinary bodies use the same TU/header. Direct JMP aliases keep
their target as an explicit binding and do not claim a second reconstruction
of the target body. The Section and FontRegistry code preserves native fields,
vtable words, caller-owned string/pool operations and raw list identity.

| Address | Source body | Original ABI / specific evidence |
|---|---|---|
| 00ABE630 | section_loaded | ECX Section; JMP AA7170 |
| 00AC6FE0 | widget_describe_alias | ECX widget, visitor stack; JMP AAAED0 |
| 00AD2DF0 | icon_geometry_alias | ECX icon; JMP AB2540 |
| 00B0FC00 | render_release_alias | ECX resources; JMP B0F6E0; the misleading decompile expands the target, not the five-byte thunk |
| 00ABF420 | section_construct | ECX fresh storage, EAX same, RET; type11h base call, preserves unwritten 10D..10F and 128h |
| 00ABEDA0 | section_constructed | ECX Section, RET; optional BCh mesh, same model captured before allocation, no invented null-mesh fallback |
| 00ABF4F0 | section_destroy | ECX Section, RET; table D5C8E8, texture114, string F0, common AA9730 |
| 00ABE960 | normalize_angle | angle float stack, ST0 result, RET4; actual BF857A x87 remainder binding and original three double literals |
| 00AC3690 | font_registry_construct | ECX registry, EAX same, RET; sentinel helper receives registry+4 |
| 00AC36F0 | font_registry_destroy | ECX registry, RET; disk209B through AC37C0 fills the misleading saved free/no-return gaps |
| 00A845A0 | sound_cache_resize | ECX three-word array, signed count stack, RET4; complete ordinary growth and reverse destruction |
| 00A9CCE0 | gui_list_visible | ECX list, DWORD visibility stack, RET4; retain full argument through the linked widget call |
| 00B67D40 | lua_vector4_or | ECX actual 14h LuaObject, output/fallback stack, EAX output, RET8 |
| 00B27C20 | renderer_query_create | ECX renderer, EAX query, RET; retain null constructor result, doubling reserve, borrowed append and current-flag guard leave |
| 00980D30 | warning_remove_listener | ECX manager, channel/id stack, RET8; actual lock depth, map at F8h, checked iterator and deleting slot0 |
| 00BDC8B0 | vfs_load_whole_file | ECX manager, name/size-output stack, RET8, EAX bytes; single read with actual-read pointer null, low32 size published after stream release |

The angle entry adds mandatory ECX/EDX bindings for the raw x87 CRT callee and
the actual CE3828/CE3D18/CE3D28 literal pointers. It preserves the original
FLD/FSTP/FCOMI flow without substituting host `fmod` or reconstructing CRT code.
The Lua reader uses existing actual-object primitives and linked Lua 5.1.1:
release index5 before fallback/numeric reads, release each numeric field before
fetching the next, then publish all four binary32 values. The fallback retains
the original overlap-sensitive integer load/store sequence. No protected error
adapter is silently substituted; native nonlocal error differential remains open.

Query creation rereads the actual optional-guard mode at exit. If it changes
from disabled to enabled during allocation, the original reads an unwritten
owner/flag slot. `QueryGuardScratch` makes that preimage explicit; it is not
silently zeroed. The bound leave helper ignores its saved flag argument, as the
full B33B00 listing shows. Ordinary normal paths are implemented; private FH3
unwinding, fault transport and original allocation-failure frame cleanup are
not claimed. The whole-file loader guards only its copied name, matching its
original state ownership; it adds no stream/buffer cleanup on an escaping read.

`00B51B50` already has a complete address-suffixed ordinary body in
`src/native_render_batch_lifetime.cpp`. Its complete clamp/grow/copy/free/publish
implementation was inspected, compiled and registered as a function without
adding a duplicate. Its canonical allocator interface remains qualified.

`00A9D9D0` is a checked MSVC `std::list` pointer-range assignment specialization:
self-link sentinel, zero count, free old nodes, insert supplied checked range.
Its only non-free callee A9D560 is already tagged `stl_instantiation` and contains
checked iterators, node allocation and STL length checks. There are no GUI field
operations. It is classified `library_instantiation_out_of_scope`; the existing
valid-allocation projection is retained.

Validation: Win32 Release build and both existing CTests passed. No tests were
added and no game was launched. No original ABI replacement or game claim.

## Batch 4: ordinary archive, voice, GUI and platform dispatch

Ten more bodies use actual storage and the same TU/header. They retain mandatory
original-callee bindings; stock CRT, Lua and XLive bodies are not reconstructed.

| Address | Source body | Original ABI / specific evidence |
|---|---|---|
| 00BD8CD0 | archive_write_field | ECX writer, key/value tagged pairs by value, RET10h; indent receives depth, key/terminator receive current root flag |
| 00BB8A90 | mpkg_entry_offset | ECX archive, entry stack, EAX offset, RET4; actual0Ch reader/34h header; cache before current-base free |
| 00BB8BE0 | mpkg_open_entry | ECX archive, entry/unused flags stack, RET8, EAX stream; compressed, memory-copy and reopened adopted branches |
| 00BB8D60 | mpkg_open_named | ECX archive, name/flags stack, RET8, EAX stream; bit0 rejection, ordered24h entries, length-equal case-insensitive first match |
| 00A44000 | voice_packet | ECX owner, packet stack, RET4; accept/state gates, signed distance/10 count, current packet range reloads and self+1C24 voice owner |
| 00A44150 | voice_refresh_flags | ECX owner, mode stack, RET4; two independently captured list owners, mute/reapply, virtualF0 capture before FSTP |
| 00AB7A40 | text_register_clip | ECX Text, RET; main/shadow geometry calls, actual material pointer and child virtual70 walk |
| 00AC4C50 | layer_read_properties | ECX layer, visitor stack, RET4; three tagged pairs/property, explicit unwritten priority preimage, root binding and base read |
| 00AA6340 | gui_manager_destroy | ECX manager, RET; page virtual20/04 calls, current bounds reloads, reference/string/array/tree retirement and singleton clear |
| 00BED3B0 | platform_handle_message | five explicit active/HWND/message/WPARAM/LPARAM stack DWORDs, RET14h, EAX result; incoming ECX is not the receiver |

The MPKG branch order and ownership behavior are retained, including nonzero
compression method dispatch, small stored-entry copy, and large stored-entry
reopen followed by an adopting substream. No invented source release is added
if the final allocation returns null. Existing actual BB8850 header parsing is
reused; zlib stays behind the established raw-inflate constructor binding.

The platform handler keeps the explicit active owner separate from window-extra
storage and preserves message-specific return values and synchronous import
ordering. It reloads HWND after focus/foreground calls, uses full DWORD key
whitelists, and queues the original two-byte character/category payload while
retaining the active argument's high16 preimage. Sound, GUI, media, renderer and
text-queue calls are explicit mandatory bindings. This adds no application
registration or window creation; the new handler has not been executed.

Validation: Win32 Release build and both existing CTests passed. The 37 original
function records in the initial packet all name sources registered in the
current generated MSBuild projects. One of them was only the partial supported
Group-sphere projection; batch2 adds the complete ordinary branch structure.
No whole-body differential, live window/network test, native ABI/FH3/SEH,
allocation-failure transport or gameplay claim is made.

## Follow-up: ten contracts, 2026-10-18

Coordinator session -62 requested these ten rows in the same tree, at max
reasoning effort. Exact addresses and the existing TU/header/document were
claimed as `oct18-tC-contracts`; none overlaps cc13's 00996120-009F6060 lease.
Nine new complete ordinary bodies are appended to the registered engine TU.
The fingerprint decoder keeps its already implemented defined-prefix interface
by the coordinator's explicit decision. No unresolved call is given a no-op
default, substitute singleton, synthetic clock or synthetic resource owner.
Required binding records name the original callees and borrow actual storage.

| Address / source name | Original ABI and recovered contract |
|---|---|
| 00985F30 / `warning_dispatch_chat` | ECX manager, player-name/type/message stack, RET0C. Captured tracked lock+24, eight active/non-observer players at current game+18CC, case-insensitive match or -1. Clear +104/+105, channel lookup at manager+F8. Own integer/string/integer boxes, evaluate into a separate copied-name vector, scalar-delete boxes with flags1, then build 14h-byte Lua variants and a0Ch argument owner for `(playerIndex,message,type)`. Reload current game+1A08 for each Lua call; free arguments, names through their sized pool, and vector blocks. Set +105 only after a nonempty callback run. |
| 00986B00 / `warning_dispatch_hp` | ECX manager, entity/reason/oldHP/newHP stack, RET10. Same lock; two ancestor00923810 calls and ancestor virtual5C gate. Entity-ID/string boxes; checked iterator owner/node pair, current sentinel reload and subscription virtual0C(params). Strict ordered damage `old > threshold > new` / repair `old < threshold < new`; copy matched callback names before destroying boxes and calling Lua `(entity,reason)`. No +104/+105 stores. |
| 0098E910 / `hardware_default_options` | ECX hardware owner, EAX owner+C3C, RET. Actual antialias begin/end at +14/+18, shader+24, LCID+C38, resolution fallbacks through 0098DF90/0098E0E0. Exact append order including EAX0, SoundSW0, Log0, CheatsOnAtStartup0, UseFpsLimit0, FPSLimit30, HiResCapture2 and Firewall1. Sound publication F8A0E0 is a **DWORD**, not a byte. Existing semantic builder omits these fixed lines. |
| 00A3C5E0 / `network_send_worker` | ECX actual4003F0h owner, EAX0/RET on quit. Full1286B ordinary loop: tracked +14C quit/activity protocol, current clock+20 sample, Sleep25, two separately captured +148 locks, sixteen channel deadline/backlog checks, direct queue send/return, profile latency/round schedule, selector/ACK/socket/statistics/error/keepalive paths, progress-triggered rescans and pool return. FCOMIP/JC sends unordered deadlines down the not-due path. Socket results are integer -1 checks, despite float pseudocode. Capture statistics publication before channel lookup as listed. |
| 00A3CAF0 / `network_receive_worker` | ECX owner, EAX0/RET on quit. Full868B loop: activity protocol, discarded current clock sample, Sleep25, captured +148 lock and socket-drain rescans. Special03/00 packets enter indexed820h receive pool; ordinary packets find/activate/classify channels, splice released82Ch send lists, append accepted820h records and count types5/6. Preserve native socket/address/sequence stores, wrapping index arithmetic and unchanged short-packet bytes; no clamp, allocation, timeout or rollback added. |
| 00AB6120 / `icon_visibility_changed` | ECX icon, visible stack, RET4. Full640B traversal with live F4/F8 bounds reloads and checked-iterator hook allowed to return. Show/+3C uses captured renderer virtual68(name,0), either consumed NativeString apply00AB3840 plus terminal texture release, or delayed singleton00AB5700/request00AB5F00(name,icon). Hide/+3D uses placeholder00AB3B30, then virtual80(index) only for the current signed-short state+EC. No delayed branch rejection. |
| 00AB98F0 / `glyph_quad` | ECX Text, ten DWORD stack slots, RET28. Full1747B ordinary flow. Exact x87 prefix and native raw mapped fields; TL/TR/BR/BL/UV/color stores and low16 indices. Conditional code-unit scan and UV blanking, pool construction, checked child-vector append, reference-node creation, scene/draw/visibility/listener/font/shadow/shader/content setup, UTF16 lifetime, attachment, pivot and all three final-position branches. Slots5/6 unused; slot10 supplies the code unit. Re-read current borrowed reference after callbacks; original arg2.x is read again only for the long-reference position branch. |
| 00AD4C30 / `font_load` | ECX font; stack root, DAT, GFX, alpha, language; RET14. Full1361B normal path. `(root+language)+GFX`, root+alpha or white.tga, then `(root+language)+DAT`; preserve five native owner-string assignments and capture renderer/VFS before concatenation calls. Read scalar count, signed height, four floats and three words per24-byte input record into a32-byte glyph. Exact CVTSI2SS/MULSS/CVTTSS2SI, unsigned metric scaling, unchanged bearing/padding, cache-owned texture pointers. Lookup/insertion remain STL bindings; first duplicate freed. Release stream, initialize space, require/copy0091 fallback, initialize LF and release retained GFX-name temporary. |
| 00AEEAF0 / `atlas_parse` | ECX manager; one TextBuffer pointer stack; RET4/AL. Full1429B ordinary flow. EBP/ESI pseudocode anomalies are frame-tracking artifacts, not extra inputs. Validate TextureAtlas token; copy descriptor path from buffer+C. Native computes and immediately discards a canonicalized temporary, then uses the original copied spelling for directory/slash/name. Capture renderer before concat; publish texture before body. Grow actual manager+10 texture and +4 item arrays, allocate/construct/parse items, ignore item-parser status, keep partial effects and return success at EOF. Exact string/token cleanup uses length+1 or strlen+1 as appropriate. |
| 00BE9630 / `decode_fingerprint_payload` | Two stack pointers, ECX unused, RET8. Existing196B decoder's defined source/key words16..255 produce240 bytes. The original final512-byte stack copy also publishes272 uninitialized stale-stack bytes. These are expressly excluded from coverage by the coordinator; no deterministic zero, copied-input or fabricated tail is added. |

The warning Lua temporary is exactly **20 decimal bytes (14h)**;
the checked vectors are10h headers with begin/end/capacity at+4/+8/+C.
0097B8C0's listing consumes three stack pointers with RET0C; incoming ECX is
unused, so its binding uses that transport instead of trusting the old
zero-argument prototype. Integer/entity event boxes retain unwritten padding.

`NetworkWorkerScratch` supplies explicit initial timestamp, sockaddr and packet
preimages. Sampling may leave QPC destinations unchanged, and a short receive
does not initialize unread header bytes; neither condition licenses zeroing
the original frame. `FontLoadScratch` likewise preserves unused key/pair
padding. All raw owners, arrays, lists, virtual tables and bound services must
remain live; sizes, native success-path resources and finite valid lists are
the original preconditions. Native FH3/SEH, unmasked FP exceptions/fault
ordering, allocator/new-handler failure transport and concurrent interleavings
are not established by these new C++ interfaces.

### Fingerprint uncertainty and consumers

The body comment and name/function ledger evidence state that the original's512-byte
result depends on stale stack outside its deterministic240-byte prefix.
0053C740 obtains singleton+4 through00BE9620 and passes it to00ABBE50 for
`Fingerprint_Text`. That routine treats the pointer as a C string, so it can
read the stale tail **if no NUL occurs in the defined prefix**; no bound at512
is imposed by that consumer. 0073BAE0 also calls00BE9620 during font startup
but discards the result and reads neither prefix nor tail. The current C++
interface returns only the defined prefix and does not claim the original
unbounded C-string result or private-frame ABI.

### Validation and packet disposition

All nine bodies compile under MSVC Win32 Release in the existing registered
TU; both existing CTests pass. One focused, local original/source glyph probe
checks the full1747-byte body after verifying every byte against live Ghidra
and the installed executable. It relocates data operands and routes the17
identified direct callees plus virtual methods to recording fixture services.
Twenty-four cases compare12576 vertex/index output bytes and complete child
callback traces: both color layouts, signed/large metrics, index wrap, absent,
nonmatching and null/default code-unit lists, UV blanking, reference replacement
between callbacks, offset mode and short/long reference positioning. All pass.
Probe sources/seeds/logs are retained under this worktree's ignored
`local/tail_c_glyph_*` files; this does not execute real child resources or the
game. The other eight new bodies have listing/build evidence, not native
whole-body differential or live SDK/concurrency proof. No new permanent test
suite, host wiring or original-installation write was added.

The106-row packet now has64 new ordinary bodies, two existing bodies newly
registered (one is the expressly qualified fingerprint prefix), four earlier
library/compiler classifications, and36 other pre-existing function rows left
intact. No follow-up row is skipped or lease-blocked. CRT/STL/Lua/XLive
dependencies stay bound; there are no new library/compiler classifications in
this follow-up. Game activation, native ABI/private EH and live validation
remain separate from the completed source contracts.
