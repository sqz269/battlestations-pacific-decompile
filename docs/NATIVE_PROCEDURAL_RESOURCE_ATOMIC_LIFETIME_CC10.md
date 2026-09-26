# Procedural resource count lifetime (CC10)

B19980 now starts the single `std::atomic<int32_t>` lifetime at its existing
count-one initialization. Previously it wrote raw DWORD+4=1, while the existing
procedural companion, canonical registry and shared release borrowed that word
as a live C++ atomic. The corrected source constructs the atomic exactly where
the count was initialized, before those consumers can borrow it. No later count
initialization, retain, resource wrapper or identity map is added.

The complete original B19980 body is 22 bytes, B19980..B19995 inclusive; ECX is
actual resource storage, EAX returns the same storage and RET has no arguments.
Its B19988..B1998E instruction is the seven-byte `MOV [EAX+4],1`. The source
preserves CEB130 profile store -> single count1 construction -> D5E554 profile
store. C30470 and BBC6F0/BBC810 inherit that count without another write. This
packet's ledger claim is only the count-store source-lifetime fragment; the
already reconstructed full bodies are reused. Names remain hypotheses and
the C++ APIs are not original register/private-stack ABI replacements.

The MSVC Win32 source requires atomic size4, alignment4 and always-lock-free
operations. Direct B19980/C30470 callers must supply four-byte-aligned,
exclusive backing for a new constructed lifetime, with no active count users
or canonical companion. Concrete creators supply fresh, sufficiently aligned
CRT34h backing. No whole-resource C++ overlay or callable C++ vtable is installed;
all bytes outside the existing field writes remain untouched.

Creator fields and order remain B19980's profile/count/profile, C30470's D79B54
then DWORD zeros+10/+14/+18/+8 and byte+1C, then D64478/D644B4. In particular+0C
and tail+1D..+33 remain uninitialized. Direct native nearest constructors
C30210, BBC670 and BBC7C0 are evidence dependencies only; this packet neither
reconstructs nor admits the C30210 resource family.

The existing actual `NativeProceduralResourceReference` borrows producer+4.
Caller code binds it once to the SAME `NativeRenderActualOwnerRegistry`; bind
does not initialize, retain or release the count. On zero, existing actual
release resolves the canonical companion, which reads current profile/virtual0,
uses genuine BD30E0, then reads current virtual4 and invokes BBC6D0/BBC7F0.
Those source scalar/destructor/CRT providers are reused without modification.
The native resource scalar tests its stacked LOW flags byte after C304A0; its
source API uses a value argument and does not claim mutable native stack aliases.

Companion allocation or binding failures remain caller-owned source obligations.
Failed companion construction leaves the initialized count-one resource.
A completed unbound companion cannot be directly deleted in its bound phase.
Its supplied retirement callback must explicitly track whether this caller's
exact canonical bind succeeded. For a zero-child resource, release of the
original reference through the companion can perform genuine terminal disposal
and retire without unbinding if bind never succeeded. Successful binding must
unbind that SAME identity/companion before companion disposal. No freed payload
may be inspected. Absent that caller contract, retain backing, companion and
diagnostics for explicit disposition. These obligations add no automatic
production rollback or native creator unwind claim.

The focused ignored fixture links the frozen production library; it compiles
no production translation unit and uses no alternate resource allocator,
aggregate-zero raw backing, count reset or registry adapter count write.
Each genuine BBC6F0/BBC810-created zero-child resource follows count1 -> existing
companion bind1 -> genuine 4DDB20 Interlocked retain2 -> actual release1 ->
terminal0 -> concrete scalar/C304A0/base/CRT disposal -> exact registry unbind.
The immutable borrowed profile views contain the exact eight native bytes from
the live/PE-equal D64478/D644B4 spans. Numeric native target identities resolve
to source providers; no copied-native machine code or native vtable is invoked.
After terminal free the fixture inspects only surviving registry/host metadata.

Strict MSVC Win32 `/W4 /WX /fp:strict` build and all three existing CTests passed.
The fixture has active assertions, embedded asInvoker manifest and passed both
profiles at PID77520, exit0, with empty stderr. No failure was injected and no
native FH3/SEH handler, application, startup or original game was executed.
This demonstrates the qualified source count/ownership composition, not native
ABI, original runtime or game parity. Win32 Interlocked and C++ atomic share the
same qualified four-byte producer storage in this tested MSVC target.

Compiler evidence freezes all 291 complete BSP COFF sections from five genuine
library members, with full bytes and relocation maps. Fifty-two selected
library sections match linked PE bytes outside relocations. Standalone B19980,
C30470 and the actually linked creator helper each emit exactly one count-one
store and preserve the ordered native field writes. Their full sections are
byte-equal to the cached pre-change sections: the correction establishes a C++
lifetime while retaining the emitted machine effects. Standalone constructor
sections are emitted evidence; this fixture reaches the inlined constructor
through the creator helper. Duplicate companion COMDATs selected from probe.obj
are explicitly excluded from production linked-body identity claims.

The first identity inspector stopped on that duplicate COMDAT ownership case.
Its original input/diagnostic and emitted sections are preserved; the revised
inspector distinguishes actual map providers without another build or run.
Source/header/compiler/system-header/library/object/probe/manifest/executable
inputs, receipts and old cached artifacts are frozen. Earlier table/readiness
archives and 1,187 prior indexed artifacts remain unchanged. See the
[report](../reports/native_procedural_resource_atomic_lifetime_cc10.json) for
hashes, exact native ranges, library selection and frozen archive identity.
