# Text current70 clip refresh

`gui_text_clip_refresh.cpp` supplies the concrete main/shadow material work in
`00AB7A40` over the existing `GuiTextLifetime`, actual Model/Mesh/Section/Material
owners, and `register_native_gui_clip_parameters_00aa9f10`. It then exposes the
unimplemented current child70 call as a suspended continuation. It adds no Text
state, material cache, child owner, callback substitute or factory registration.

The native entry is ECX Text, no stack arguments, RET at `00AB7B0B`, length1;
the inclusive body is `00AB7A40..00AB7B0B`. All 77 instructions were read, including
the register/list checks lost by decompilation. The only xref is DATA at
`00D5C738`, whose bytes `40 7A AB 00` select this entry at Text table
`00D5C6C8+70`. There are no direct callers in the verified current target.
The Ghidra fastcall/prototype inference does not establish a second input.

Project/program verification used the repository wrappers for every analysis
batch: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Ghidra remained
read-only. Names are descriptive hypotheses; the C++ interfaces are not binary
ABI replacements and do not reconstruct native SEH or invalid-storage behavior.

## Material order and same storage

ESI becomes entry Text at `00AB7A42`. EDI captures main node+4C at `00AB7A45`.
For that captured node, the native code:

1. Skips null, then tests model+180 through `00B74650`.
2. Gets the actual mesh through `00B74640(0)` and tests DWORD count+58 through
   `00B72B40`; zero skips this drawable only.
3. Re-reads geometry on the same captured Model through `00B74640(0)`, obtains
   section0 through `00B732C0(0)`, and captures its material+20.
4. Calls `00AA9F10` with ECX original Text and that actual material.

Only after the main call and its temporary-name cleanup does `00AB7A88` load
the live Text+188 shadow slot into EDI. The same sequence applies independently
to that captured shadow. It does not test the Text shadow-enable byte. If main
and shadow refer to the same actual drawable, registration still happens twice.
Main registration may change the shadow slot, ancestor chain or child list;
the later reads observe those changes. Captured resources must remain alive
through their native callback intervals; no extra retain is inserted.

The five direct helpers' actual bodies and their current C++ implementations
were checked. B74650 reads model+180; B74640 returns that field and pops one
unused DWORD; B72B40 reads mesh+58; B732C0 indexes the actual pointer array+54
and pops its index. Actual owners are resolved by the existing storage identity
registries, including the model attachment association. No wrapper pointer is
cast to an unrelated model or semantic geometry snapshot.

The existing full `00AA9F10..00AAA0E8` body owns name allocation/registration/
cleanup. It writes the same widget+E8 and registers `cClip`, destroys that
temporary, then re-reads +E8. Its active arm registers `cClipCenter`,
`cClipBorder`, then `cAspectRatio`; inactive retains prior other parameters.
Names use the actual NativeString pool, while values remain borrowed addresses
in the same widget, captured ClipBox and live aspect global. Those sources must
outlive every later material consumer. This caller does not perform B18A40 or
invent a widget retain. Parameter pool, string storage and actual resource
domains must match the established buffer/lifetime domains.

Supported resources are live canonical Model, Mesh and Section owners and the
actual material profile D5E520. Null drawable/mesh and zero sections are ordinary
skip paths. Corrupt extents, null section/material despite a nonzero count,
unknown profiles or missing associations are outside the supported domain;
they produce an explicit error rather than an invented empty material.

## Child boundary

After both material stages, `00AB7ACE` reads the live child-list head. EDI now
holds Text+64, ESI the current native list node, and EBX is reloaded from the
current sentinel at `00AB7AD8` on each loop. `00AB7AF0..00AB7AF8` loads the
current payload and its current virtual70. After that call, the code checks
the current entry against the reloaded sentinel, reads its next link at
`00AB7B04`, then repeats. ESI no longer holds the original Text; the child
vtable is re-read each iteration and material work is not repeated. The three
BF6713 calls lie behind list-consistency checks; their
library forwarding body was read, but invalid-list CRT behavior is excluded.

`begin_gui_text_clip_refresh_00ab7a40` completes both concrete material stages
and selects the first entry from the same borrowed `transform.children` list.
An empty list returns nullopt at the native ordinary return. Otherwise it
returns a frame immediately before `00AB7AF8`. `frame.child_owner()` resolves
the existing current child owner; it performs no virtual method itself.

The caller must complete that child's actual current70, including any nested
continuations, before invoking `resume_gui_text_clip_refresh_after_child70_00ab7a40`.
Resume consumes the prior frame, advances only after completion, and re-reads
the live list/end/next payload. It returns another pending frame or nullopt at
the native return. It never re-registers the parent materials or substitutes
another child method. Current `GuiWidgetTypeImplementation` has no70 interface,
so **all child profiles remain this explicit dispatch boundary** in this module.

The existing C++ vector projects borrowed child payloads, not native intrusive
list-node identities. Membership and order must stay stable from the first
suspension through the last resume, and frame owners must survive. Duplicate
payloads remain distinct entries by position. The implementation validates the
current entry and reloads subsequent payload/owner after callbacks; it does
not claim arbitrary native list insertion/removal or self-deletion equivalence.
Material callbacks before traversal may change the list normally. Child
implementation/state changes are read at each pending call boundary.

## Coverage and checks

Reconstructed ranges are `00AB7A40..00AB7AF7` before current child70, plus
`00AB7AFA..00AB7B0B` after actual child completion and the repeated selection.
The general routine remains partial at `00AB7AF8`; the no-child path is concrete
through return. No bytes of the native routine are unread. Native list ABI,
invalid-list/storage paths and child virtual dispatch are explicit exclusions.

Strict MSVC Win32 C++17 compilation passed with `/W4 /WX /EHsc /permissive-`
using an isolated new-header overlay and current integration headers. The
report records exact direct-call checks. No tests were added. No executable
path or factory currently reaches this new module, so no game/render or ABI
equivalence is claimed. The integrator owns source registration and combined
build validation.
