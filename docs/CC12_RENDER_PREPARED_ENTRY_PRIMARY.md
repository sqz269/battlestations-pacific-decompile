# Prepared canonical registry entries: primary acceptance

The canonical `NativeRenderActualOwnerRegistry` can now reserve an opaque raw
identity before Native construction and publish its completed owner into that
same entry. Worker `8579262d4` adds a move-only admission token, prepare,
publish and cancellation. It adds no persistent particle companion or caller.

Preparation validates null/alignment and inserts a null mapped value under a
short lock without reading Native +04. The token stores its registry, key and
stable mapped-value address, which survives unordered-map rehash. Publication
first verifies that exact still-null entry, then checks the Native +04 atomic
alias and positive count, writes the existing mapped value and consumes the
token. It performs no insertion, retain/release, constructor or Native callback.
Invalid capability/alias/count terminates. Mutex acquisition can throw, so
publication is not a universal no-fail contract. Cancellation erases only its
own still-null metadata and never cleans up Native storage.

The registry must outlive admissions, and pending entries must not be erased,
extracted or replaced. Token/raw lifetime is caller-serialized. Ordinary Source
bodies and the enclosing registry's one-pointer storage are unchanged. Pending
entries contribute to size; lookup is unavailable until publication.

Root fresh before/after Win32 builds passed all three existing checks. The
complete 1,989-Core/80-App census preserved 174,277 code sections, changed two
existing code sections and added 11, with no removals. MSVC changed ordinary
unbind to call its existing erase helper and expanded unbind_callback inline.
Root verified their complete code/data/EH/CRT/import relocation closures
(16 nodes/18 edges before; 20/31 after) against its actual linked images and
matched all selected registry payloads and exact symbolic relocations to the
worker. DLL bodies and runtime are outside this static import proof.

Whole objects differ in 366 Core and 21 App members; all App code sections
remain exact. Whole EXE, map and linked .text change. No whole-artifact or
timestamp-only identity claim is made. Root retains 22 complete physical
COFF/symbol/aux/relocation indices per phase, preserving BSS without a file
payload when its raw pointer is zero.

Root copied the finite proof, passed its ZIP-only guarded replay, and checked
4,226 frozen/after actual Git Source blobs against the closed worker commit and
current Root. The retained worker probe is one manifested Source scenario:
growth, unavailable/duplicate/move/cancel behavior and valid publication with
zero observed C++ allocation attempts. Its counters do not monitor every heap,
and Root does not repeat probe execution. The probe constructs no Original
Native model. Public prepare/publish have no direct Core/App code callers.

The new `Source3354-render-prepared-entry` epoch retains 3,354 inputs and four
build artifacts separately from the earlier particle epoch. Persistent particle
record placement, profile bind/admission, publication and factory cleanup still
require a concrete caller contract. Application activation, Original ABI and
gameplay remain unvalidated.

Evidence: `reports/cc12_render_prepared_entry_primary.json` and
`local/cc12_render_prepared_entry_primary/Root_review.json`.
