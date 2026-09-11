# Canonical Text glyph references and placement

Addresses: `00531130`, `00531380`, `00AB98F0`. Names are behavior hypotheses. Ghidra is read-only for this packet.

| Routine | Coverage | Native ABI / bounds |
| --- | --- | --- |
| `set_gui_text_glyph_reference_00531130` | Partial projection: canonical typed string assignment and both borrowed-pointer stores; native pooled UTF16 header allocation, callbacks and SEH excluded | ECX Text; `(UTF16 wrapper, listener, reference Text)`; `RET 0Ch`; `00531130..005311BE`, final instruction `005311BC`, length 3 |
| `begin_gui_text_prompt_glyph_offsets_00531380_fragment` | Partial: first offset block `00531903..00531935` | Containing function `00531380..00531AF1`, final `RET` at `00531AF1`, length 1 |
| `copy_gui_text_prompt_glyph_offsets_00531380_fragment` | Partial: stores `00531989..005319A6`, `005319FA..00531A17`, `00531A6B..00531A88` | Receives the first block's captured x, as the native local does |
| `position_gui_text_glyph_child_00ab98f0_fragment` | Partial: `00AB9EC2..00AB9FAD`, after completed child creation, recursive content, attachment and current `+30` pivot | Containing function `00AB98F0..00AB9FC2`; ten stack arguments; final `RET 28h` at `00AB9FC0`, length 3 |

The field extension belongs to the existing `GuiTextLifetimeFields`; no second Text state, tree or identity map is introduced. The lifetime source and owner implementation are unchanged. Factory activation, model cloning and the earlier optional child tail remain separate requirements.

## One borrowed reference convention

Native `00531130` stores an opaque borrowed listener at `+1AC` and a borrowed reference **Text** at `+1B0`. The consumer `AB98F0` obtains that reference's main model, font name, text length and resolved position. Its use is not an event-object pointer. All nine native calls pass the target Text itself as the reference; the helper still accepts any valid canonical reference or null, matching the native stores.

The new C++ convention keeps `pointer_1ac` as `void*` and changes the existing `pointer_1b0` to `GuiWidgetOwner*`. It is a pointer to a real canonical owner in the **same** `GuiWidgetOwnerRuntime`, never a native object address reinterpreted as a C++ companion. Resolution checks the runtime's existing owner entry for that layout, type Text / type id 3, the owner's existing `text_lifetime()` association, and the companion's matching owner binding. The producer validates before mutating the destination. Null remains null; wrong domains or absent companions throw instead of manufacturing a reference. The referenced owners and companions must remain alive; neither pointer is retained or released by this producer.

The current-use audit covered this worktree and the parent/content/lifetime workers' sources. Before the type change, the only executable `pointer_1b0` consumer was the null gate in `gui_text_geometry.cpp`; the lifetime fields supplied its null default. That gate still uses the same slot. There was no raw-pointer-to-Text conversion to preserve. The old prompt host projection still has native raw-offset writes and is not silently bound to this new convention.

## Producer order and string boundary

Full `00531130` assembly establishes the order. If the incoming wrapper is the same object as Text's `+1A4` wrapper, string work is skipped. Otherwise it calls `004C53E0(source_length, 1)` at `0053114A` (`RET 8`) and, for a nonempty source, copies code units through the terminator. Every branch then stores listener first and reference second, returning with `RET 0Ch` at `00531188`, `005311A2` or `005311BC`.

The C++ producer accepts the canonical `std::u16string` object, so identity-based self-assignment remains visible. A different source must have its terminator at `size()`; embedded terminators are rejected before mutation. Assignment runs before both pointer stores, including for empty strings. Typed string allocation failure leaves the pointer stores unexecuted. The native resize's equal-length shortcut, actual pool allocation/return callbacks, raw header aliases, and uninitialized bytes after an early embedded terminator are **not** claimed by this projection. There is no fake string allocator or callback service. Visibility, base listener `+DC/+79`, offsets and geometry are untouched by `00531130` itself.

All nine call setups were inspected: `005318F2`, `0053196F`, `005319E0`, `00531A51` in `00531380`; `0054D13D` in `0054C050`; `005D2B03`, `005D2B50`, `005D2B91` in `005D26D0`; and `0062168B` in `0061FBE0`. The report retains the caller and all three arguments for each. Register provenance was checked from complete listings, including the last ESI assignment before `0062168B` and the full EBX/BL filter in `00531380`.

## Offsets are not constructor defaults

Native Text constructor `AB9650` initializes byte `+1B4` and clears pointers `+1AC/+1B0`, but does not write `+1B8/+1BC`. The copy constructor `ABB2C0` also clears those pointers, copies byte `+1B4`, and leaves both offset floats unwritten. The canonical fields therefore have no float initializers. `glyph_offsets_written` is separate C++ validity metadata; its initial false value is not an added native store. Code that imports another proven producer must write **both** floats before setting this marker. Neither reference assignment nor mode-byte changes establish float validity.

The observed prompt producer computes x with native `FLDZ; FDIV double[CEC380]`, stores mode byte one, spills x to float, then stores x at `+1B8` and the current float at `CED318` into `+1BC`. The first helper preserves this sequence and returns the captured x. The other three blocks reuse that same captured local across intervening color calls; they do not divide again. Each block reloads the y constant after its x store and marks validity only after both stores. Full EBX provenance shows `MOV EBX,1` at `005313A8`, no subsequent replacement before these BL stores.

The helpers cover these direct stores only. The caller's preceding `00531130`, `AA6BC0(0,0)` base listener clear and following current `+50` color remain in caller order. In particular, a caller must fetch the current button binding again after those earlier calls, just as the native screen does. No helper advances the whole screen entry routine.

## Final child position fragment

Call this fragment only after the real child operations through current `+30` at `AB9EC0` have completed. It cannot serve as proof that any earlier factory, clone, recursive content or attachment operation succeeded.

At `AB9EC2`, the native tail reloads the current reference and calls `AA6750` to capture its resolved position. Offset mode then reads and spills both current `+1B8/+1BC` floats before adding them to reference x/y. The C++ fragment requires their validity marker at this point; it does not substitute zero. In non-offset mode, native `AB9F1F` reloads the reference to inspect current text length. Length one uses reference x directly. Other lengths read **original argument 2's current x**, calculate `(x - double[D06880]) / double[CEC380] + reference_x`, and spill to float at `AB9F79` before the following subtraction. Original arguments 5 and 6 remain unused.

| Live input | Verified native bits / operation |
| --- | --- |
| `D06880` | double `403a000000000000`, 26, subtracted before normalization |
| `CEC380` | double 960; also the prompt producer's divisor |
| `D7A358` | double `3f847ae140000000`, subtracted from non-offset x |
| `D5C7A8` | double `3f8c432ca0000000`, subtracted from non-offset y |
| `D7A220` | double `4059000000000000`, 100, subtracted from z on every branch |

The service record borrows the actual live constant storage; it supplies no copied fallback values. Inline x87 operations preserve the tail's load, extended arithmetic and explicit float spills, including the intermediate x spill and both offset spills. There are no finite-value clamps or invented conversion guards. Existing `resolved_position` and `local_position_for_resolved` supply the established widget arithmetic and its existing limits; this packet does not claim floating-trap or native call-stack equivalence.

The final `AA8240` operation uses the child's current parent/pivot to derive local position, writes the existing transform, then calls the real owner's `recompose_00aa7220()` and `refresh_bounds_00aa70e0()` in that order. It uses actual node/geometry owners rather than a transform callback adapter. Bounds are reevaluated after recomposition. A failure during those existing operations does not roll back prior stores or imply completion of the enclosing glyph writer. No ownership transfer or child cleanup is added by this fragment.

## Verification

MSVC Win32 C++17 `/W4 /WX /O2 /fp:strict` compilation passed for the new source plus unchanged `gui_text_lifetime.cpp` and `gui_text_geometry.cpp`, using the parent worktree's dependency headers as an include overlay. The stronger report verifier passed 27 numeric call rows with zero failures; four current `+50` targets also rely on the checked Text vtable. No tests were added. These helpers are not connected to an executable Text factory; no game/runtime, native differential or rendering validation is claimed. The parent performs combined build registration and integration.
