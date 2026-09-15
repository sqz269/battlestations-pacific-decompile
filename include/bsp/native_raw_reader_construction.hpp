#pragma once

namespace bsp {

// Full BF09A0[00BF09A0,00BF09B0): ECX writable raw10h base, EAX same,
// plain RET. Ordered zero DWORD stores at +0,+4,+8,+C; no allocation,
// release, attached-stream operation or external provider.
void* __fastcall construct_native_raw_stream_reader_base_00bf09a0(void* actual_base10h);

// Full BEA150 construction in the valid, nonfaulting source domain:
// caller-owned writable raw70h storage and the stable actual 00415270 provider.
// Native ABI: ECX reader, EAX same reader, plain RET; this context-free C++
// entry has a new call interface. Base first, ten raw8h headers at +10h,
// then +60=0,+64=FFFFFFFFh,+68=0,+6C=0; return the original pointer.
// Fresh construction storage only: old contents are overwritten, not released.
// No generic throwing callbacks, native FH3/SEH identity, asynchronous-fault,
// hooked-provider, concurrent-mutation or gameplay compatibility claim.
// Evidence: docs/NATIVE_RAW_READER_CONSTRUCTION_BN.md and its report;
// discovery f57a2fba9bef3751f78c8f7ed062c1680f26c573. Names are hypotheses.
void* construct_native_raw_reader_path_storage_00bea150(void* actual_reader70h);

} // namespace bsp
