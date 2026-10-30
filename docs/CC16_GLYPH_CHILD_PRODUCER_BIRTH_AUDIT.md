# Glyph child producer, reference birth and failure-provenance audit

This is an evidence/design packet only: two new files, zero leased Native addresses and no production change. Baseline is `9f3f19e63c51eef23f13606ba330567a640d516b`, merging fetched main `2e5f561a85210064a5fa6fb3ef42afbe3fa071c6`. All twelve historical files from changed Text `96dd22fd0`, raw layout `611d95b43` and Frame lifetime `d4e6aa48d` remain identical Git blobs. No new runtime, Native/body, vendor, helper, duplicate, ABI or activation credit is issued.

The three requested producer bodies already have concrete Source implementations. The missing connection is genuine per-child preparation, reference birth, custody and failure provenance. An arbitrary raw pool slot is not a qualified Text/Model reference. No additional Native constructor/helper-body recovery is needed for these three entries.

## Bounded Native evidence and existing seams

| Native body | Inclusive end | Bytes / instructions | Existing concrete API and file |
| --- | --- | --- | --- |
| `00AB78A0` | `00AB79DB` | 316 / 109 | `NativeGuiTextPool::allocate_raw_slot_00ab78a0()` in `src/gui_text_factory.cpp`; declaration `include/bsp/gui_text_factory.hpp` |
| `00AB9650` | `00AB98EE` | 671 / 123 | Raw-Model overload of `construct_native_gui_text_default_storage_00ab9650(void*, NativeGuiTextRawModelDefaultStorageFrame&, NativeGuiTextRawModelDefaultStorageContext&, NativeGuiTextRawModelDefaultStorageAcquired&)`, `src/native_gui_text_default_storage.cpp` / matching header |
| `00AB76F0` | `00AB76FB` | 12 / 4 | `return_gui_text_failed_slot_00ab76f0(void*, NativeGuiTextPool&)`, `src/gui_text_factory.cpp` / matching header |

The bounded audit read every instruction in these three saved listings: 999 physical bytes/236 decoded instructions, ten calls and no x87 forms. The seven-byte jumped-over padding at `AB7999` was read separately. The constructor's seven-state FH3 map was checked against live Ghidra and the installed PE: handler `CB809C`, FuncInfo `DEEE88`, map `DEEEAC`, states 0..6 with parents -1,0,1,2,3,4,5. Complete body hashes, six saved export hashes, call encodings and map/action bytes are in [the report](../reports/cc16_glyph_child_producer_birth_audit.json).

The supported installed PE is 12,223,752 bytes, SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Before live batches `bsp.py ghidra count` verified project `bsp`, program `/battlestationspacific.exe`, x86:LE:32:default and image base `00400000`; live and saved counts were 77,632. Configured project is `C:/Users/sqz269/bsp.gpr`. No project mutation, new export or execution occurred. Export pins identify saved evidence, not a new runtime/ABI claim.

`AB78A0` consumes ECX actual pool and returns EAX raw slot, plain RET. It enters the pool's real section, increments its tracked depth, grows native slab/table storage when needed, pops a free WORD index and returns `slab + index*1F8`. A slot has 1F4 payload bytes plus its existing hidden slab-index DWORD at +1F4. The native slab initializer preserves payloads; allocation does not calloc/zero a Text. There is no local EH unlock: allocation failure preserves the entered lock/depth and prior effects. Existing Source deliberately follows that schedule.

Borrow `game::GameNativeGuiTextProcess::pool_00f8bdf0()` from `include/bsp/game_native_gui_text_process.hpp` only after complete process startup. The Glyph allocation argument must denote that SAME pool storage (`&pool.storage()` in Source), with the SAME allocator-list/slab/CRT domain. Do not cast arbitrary `void*` into the C++ pool companion, copy its storage, initialize another F8BDF0 pool or adopt installed-game CRT slabs. The process/pool outlives all payloads, slots and trim/list activity.

`AB9650` consumes ECX fresh actual 1F4 destination, calls genuine `AA9390(type3)`, installs D5C6C8, writes the exact derived defaults/current constant/quartet values and calls genuine constructor-phase `AB8530`. EAX is the same destination, plain RET. Unwritten bytes, base E4..EB and the hidden pool trailer are preserved. Its existing RAW overload is the whole relevant body; identity-only/typed `GuiTextLifetime` fragments are not substitutes.

`AB76F0` receives the raw slot in ECX, pushes that exact argument, selects the SAME F8BDF0 pool and calls `AB75A0`, then plain RET. It does not run AB8250, construct/dispose a companion, repair a slot or infer current payload ownership. The caller controls whether this failure-return epoch is legitimate.

## Steps 85, 93 and 122 are different events

| Step | Existing record/API | What it establishes | What it does not establish |
| --- | --- | --- | --- |
| 85 | `NativeGuiTextRawModelSectionStorageFrame/Context/Acquired`, `native_gui_text_storage_construct.hpp` | Actual constructor-phase section/Model/provider progress and source cleanup/admission state | `provider_call` is NOT an Original/Native exception-origin credential |
| 93 | Raw `AB9650` overload and its `NativeGuiTextRawModelDefaultStorageAcquired` | A genuine normal constructor return on the SAME actual destination, with its real retained 85 graph | No canonical Text reference, registry publication, inferred slot-return authority or activation |
| 122 | `bind_native_gui_text_raw_model_reference`, `native_gui_text_reference.hpp/.cpp` | Separate qualified companion over completed DEFAULT raw93, SAME actual +04/count/profile and retained contexts | No new Native retain; bind failure is extra host admission, not a failed Native constructor |

The 85 failure enum is exactly `none`, `provider_call`, `host_metadata`. Its header explicitly says an entered provider boundary cannot classify an Original/Native exception. The raw93 record reuses that enum. A source `host_metadata` failure propagates without the corresponding outer Native unwind; interrupted cleanup and independent creators remain retained. Genuine source cleanup has its own pre-existing exception limitations. This audit does not grant Native origin from the enum, public booleans, active call site, exception type or `exception_ptr`.

`make_native_gui_text_default_factory_storage_00aa1380`, in `native_gui_text_default_factory_storage.hpp/.cpp`, is an existing preparation/normal-return recipe over the same process, raw93 construction, persistent retirement and companion/reference storage. It is DEFAULT AA1380 only. It does not bind122 and intentionally does not infer AB76F0 on failure. Do not replace Glyph's separate allocation/constructor Native intervals with that whole wrapper or forge its completion diagnostics.

A normal nonnull raw93 return needs a separate one-shot bind using the matching companion bytes, `NativeGuiTextRawModelReferenceContext`, `NativeGuiTextReferenceFrame` and disposal. Its private preflight checks completed genuine DEFAULT payload, the actual current type/profile aliases, positive actual +04 and SAME canonical internal188 Model. Later Glyph operations mutate these fields, so the initial qualified birth must occur before append and later native child callbacks where required. Binding does not add a Native credit; it borrows the existing initial credit. A completed factory/constructor record alone is not registry publication.

## Independent internal188 Model custody

`NativeGuiTextRawModelRecord`, in `native_gui_text_storage_construct.hpp`, separately contains actual Model identity, `NativeModelStorageConstructAcquired`, persistent lifetime frame/acquired, optional `NativeModelStorageReference`, geometry association, and tree/release frame/acquired. This is independent of both the Text payload and its companion.

The constructor's `sections.model`, publication/retirement Model domain and all actual pool/node/world/scene/type/import/point-light/string/provider identities must match the already genuine raw Model pipeline. The model dispatcher/renderer/GuiDefault/effect-cache/Lua/sampler/VFS and complete recursive frames are prepared before the first AA9390 allocation callback. No A1-only camera graph, fabricated Model/Font, default child or new ownership system closes those prerequisites.

`NativeModelStorageReference(void*, NativeModelStorageIdentityContext&, NativeModelStorageConstructAcquired&, NativeModelStorageLifetimeFrame&, NativeModelStorageLifetimeAcquired&)`, in `native_model_storage_lifetime.hpp/.cpp`, requires its same completed real Model constructor, matching context, no prior metadata binding, current real slots, positive actual +04 and a live real pool slot. It then calls the registry's ordinary `owners.bind` and marks `construction.metadata_bound`.

Text disposal/slot return does not prove disposal of all internal Model credits or physical-slot custody. Keep the independent Model/context/optional companion/construction/lifetime/association/release/provider graph through surviving credits, entered failures and completed flags0 cases. `retired()` alone does not prove physical return. Text's final companion disposal callback must preserve these independent objects and opaque providers while any such obligation remains. No automatic C++ destruction, guessed unbind or extra Native retain supplies that proof.

## Existing reservation capability and its gap

`NativeRenderActualOwnerRegistry::prepare_binding(raw_identity)` reserves one aligned, nonnull opaque key with a null/unavailable value. It reads no Native +04 and performs no Native construction/refcount/cleanup. It may allocate or throw on metadata lock/insertion and rejects any existing key, including an already prepared one. Its move-only `BindingAdmission` has private issuer identity/entry fields; this is authentic registry metadata, not a Native lifetime or exception-origin credential.

`publish(BindingAdmission&&, RenderCommandReference&)` requires the same registry and exact still-null entry, then validates that the companion borrows the SAME actual +04 atomic and its count is positive. After successful locking/validation it fills that entry and consumes the token without insertion, rehash, retain or callback. It is NOT a general nonthrow guarantee: mutex locking can throw, and invalid/mismatched capabilities/aliases/counts terminate under the existing contract. Do not add a noexcept wrapper, terminal cleanup or forced admission to hide those qualifications, and do not hold a registry lock across Native callbacks.

The current Text binders (`native_gui_text_reference.cpp`, ordinary and raw constructors) and Model reference constructor (`native_model_storage_lifetime.cpp`) still call allocating `owners.bind`. They accept no `BindingAdmission` token. Preparing a null entry then calling today's binder is not reserved publication: its duplicate key makes ordinary bind reject. There is presently no demonstrated nonthrowing reserved Text/Model birth seam.

## Glyph071 failure interval and current home7

Glyph's Native schedule is allocation, store allocation to actual home7, arm state0, call nonnull raw constructor, disarm, replace home7 with the returned child and append its actual home address. On the actual constructor escape covered by state0, the failure funclet reads CURRENT home7 and calls AB76F0. Preserve that current-home read; a captured allocation/constructor destination snapshot is not the cleanup argument. Do not add AB8250, an implicit companion destructor, retry, replacement repair or pool reseeding.

Glyph071's current generic C++ catch returns CURRENT home7 for every escaping constructor-provider C++ exception while state0 is armed. That includes an added source host admission/bind failure unless the interface distinguishes it. Raw93 explicitly permits host_metadata escapes that retain the graph, and bind122 is separately extra host admission. Neither may masquerade as a Native constructor failure and cause slot return. Even `provider_call` alone is not enough to authorize that action.

A safe preparation boundary is inside the allocation provider AFTER the real AB78A0 returns but BEFORE it returns to Glyph/core state0. Capture the actual allocation's independent custody first, then perform permitted metadata preparation/reservation there; failure keeps that record/slot/domain retained while Native state0 remains unarmed. A qualified companion can only be born after the genuine constructor establishes its count/profile. Added work after raw93 return must not escape through Native state0 as if the Native constructor itself failed. Model identity may be learned only within constructor callbacks, so reservation alone does not prove all later admissions/locks nonthrowing; provenance remains necessary wherever that cannot be established.

## Changed96 has the same parent-EH cut

The canonical independent audit and Root hold `96dd22fd0` UNADMITTED. The operand/x87 schedules are not the identified issue; exception provenance is. Source28 builds exclude96. No claim of admission/build/runtime credit follows from the earlier Source commit.

At frozen `src/native_gui_text_changed_storage.cpp`, parameter helper catch line54 always invokes raw41DD20; inner catch246 always invokes raw436430 on the copied wide header; outer catches281/293 always invoke raw436430 on their localized/widened headers. These catches currently treat ANY C++ escape in an armed region as permission for corresponding Native cleanup. Direct known-body calls do not supply that missing cause distinction.

The post-constructor AB8530 helper in `native_gui_text_main_section_storage.hpp` admits only its qualified current-nonnull188 main continuation: actual late15C must remain zero, actual188/hierarchy/current graph must match. Its implementation marks failed and rethrows ALL escapes, explicitly retaining and inferring no Native EH/name/pool/base cleanup. It can therefore fail extra source admission after prior effects. AB8530 runs before the inner wide copy/state0, but its host failure can still reach the armed OUTER wide catch; later record/admission failures can reach both inner and outer catches.

Exact existing status/cause surface:

- Raw85/raw93: the above diagnostic `Failure` enum, active site/state and acquisition fields; `provider_call` is not Native origin.
- Main-section Frame: `phase()`, `matches(...)`, `matches_created_normal(...)`; Acquired has escaped boundary/site and `std::exception_ptr failure`. These are preparation/normal-return/retention witnesses, not Native exception credentials.
- Shader Frame: public `Phase`/`Boundary` progress and prepared raw-context/reference-frame getters, plus private raw-credit preparation/normal-return machinery. None makes a thrown callback a Native exception.
- Glyph buffers and SingleLine: private Frame `phase()` and `requires_process_retention()` plus Acquired `exception_ptr`/boundary diagnostics. Retention status does not authorize parent cleanup.
- Glyph children: sticky boundary/progress fields; its header explicitly denies Native-vs-host inference from the stopped boundary.
- Raw B865A0: borrowed context/key/acquisition observation, no exception-origin record at all.
- Changed96: actual raw operand scratch and bindings, currently no private passive cause propagation across its parent catches.

Thus no existing public cause API closes the correspondence. Neither an entered call, matching frame, completed prior stage, boolean diagnostic, SDK exception code nor a guessed C++ allocation-failure class proves origin. Root's separate known ShaderFrame struct/class-key normalization is unrelated to this provenance defect.

## Finite next design, for upstream contract review only

Do not implement a standalone producer adapter or another Native body. Review these coupled existing seams first:

1. `native_gui_text_reference.hpp/.cpp` and `native_model_storage_lifetime.hpp/.cpp`: a reserved-birth overload/path that consumes an authentic existing registry reservation while preserving completed-constructor/current-profile/count/slot checks, same credit and disposal order. Existing `prepare_binding/publish` qualifications remain; do not assert nonthrow or force owner admission.
2. `native_gui_text_default_factory_storage.hpp/.cpp`, `native_gui_text_default_storage.hpp/.cpp` and `native_gui_text_storage_construct.hpp/.cpp`: reuse real preparation/custody/constructor records, preserve the step85/93/122 distinction, and expose any needed private passive cause at actual additional-admission versus modeled-callee epochs. Do not relabel `provider_call` or public flags as authority.
3. `native_gui_text_glyph_quad_storage.hpp/.cpp`: a coupled actual producer binding using the same initialized process pool, genuine raw constructor/retained graph and qualified reference birth. Any added failure inside state0 must remain distinguishable before deciding AB76F0. Genuine qualified Native escape still uses exact CURRENT home7; added host/bind/preparation or unclassified escape retains the unresolved graph without inventing a cleanup action.
4. `native_gui_text_changed_storage.hpp/.cpp` and the exact retained-record adapters it calls: propagate the SAME private per-invocation cause through parameter/inner/outer parent EH boundaries. Genuine qualified Native exceptions still run their exact armed Native actions. Added host/bind/preparation and unclassified failures retain the whole active graph/headers/frames without pretending a Native cleanup obligation exists. Preserve original exception identity; track actual armed/disarmed/returned cleanup state to prevent replay of normal returns.
5. Review `native_render_context.hpp/.cpp` only if the above requires a stronger usable reservation/commit contract. Do not weaken its real alias/capability checks, invent unconditional noexcept, change terminal policy or turn metadata reservation into a Native owner/retain.

A proposed passive channel must be private, issuer/invocation-bound and sticky across nested callbacks. Bind it to the exact active exception identity as well as operation identity/generation, with host or unclassified as the default; an old record cannot authorize a different escape. A cleanup-secondary escape needs its own distinct cause record and must not inherit the triggering exception's permission. It grants no new Native ownership; host/unknown cause cannot be reset/promoted by mutable diagnostics or exception-type guesses. A normal/private credit witness is not an exception witness. If a concrete supplier lacks a trustworthy cause contract, keep that path unclassified and gated. No broader new cleanup/terminal policy or synthesized retirement follows. These are candidate API/file changes requiring Root review, not implemented interfaces.

The same future graph must supply distinct retained operations for recursive children. AB9650 writes `FC=1`; subsequent callbacks remain current, so genuine ABA8D0 wrapped selection is still required and cannot be forced to FC0. Do not add a depth/count/input cap or fabricate default Font/Model/string/child to make the graph appear connected.

No production edits, new Source claim, CMake/application/ledger/Ghidra mutation, tests/build/probe/game/native execution or process control was performed for this evidence packet. Only the two audit artifacts are committed. Whole actual child-provider/changed-content admission remains false; the next work is coupled real Source/metadata binding and provenance, not new Native body recovery.
