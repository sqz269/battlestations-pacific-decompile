#pragma once
#include "bsp/avoid_zone_dyn_hull.hpp"

namespace bsp {
// Native allocation/Win32 services. Site is the original CALL address. Owners
// supply the same allocator as scene/body construction; calls must be safe for
// concurrent task invocations when used by a parallel scene.
struct NativeDynNarrowPhaseCalls {
    virtual ~NativeDynNarrowPhaseCalls()=default;
    virtual void* allocate_00bf55be(std::uint32_t site,std::uint32_t bytes,const AvoidZoneDynHullMemory&);
    virtual void* malloc_00bf9f1a(std::uint32_t site,std::uint32_t bytes,const AvoidZoneDynHullMemory&);
    virtual void free_00bf6989(std::uint32_t site,void*,const AvoidZoneDynHullMemory&);
    virtual void enter_critical_section(std::uint32_t site,void*);
    virtual void leave_critical_section(std::uint32_t site,void*);
};
// Actual scene/body/pool storage from existing constructors, valid shape and
// dispatcher objects, normal successful allocation and contact counts 0..4.
// Candidate is nine floats: local A, local B, normal. Dispatch results hold
// a signed count followed by at most eight candidates. No geometry is supplied
// here: scene dispatcher cells must contain their real callable objects.
void append_native_dyn_manifold_00c35260(void* body,void* manifold,
    const AvoidZoneDynHullMemory&,NativeDynNarrowPhaseCalls&);
void* find_or_create_native_dyn_manifold_00c3f4d0(void* pool,void* first_body,void* second_body,
    const AvoidZoneDynHullMemory&,NativeDynNarrowPhaseCalls&);
std::int32_t match_native_dyn_contact_00c3f650(void* manifold,const float* candidate) noexcept;
void insert_native_dyn_contact_00c3f760(void* manifold,const float* candidate) noexcept;
// ESI scene, EAX first, stack inclusive last / RET4 in the original. These are
// explicit C++ interfaces; the private kernels retain the x87 operand schedule.
void intersect_native_dyn_pairs_00c44090(void* scene,std::int32_t first,std::int32_t last,
    const AvoidZoneDynHullMemory&,NativeDynNarrowPhaseCalls&);

// Complete one-slot IntersectTask2 source table (00403CC0). The stable owner and
// borrowed calls/allocator context must outlive every task. Invocation context
// lives on the caller's stack; no shared progress state or TLS is introduced.
// Original RTTI, exception metadata and dispatcher implementations are separate.
class NativeDynIntersectTaskRuntime final {
public:
    NativeDynIntersectTaskRuntime(const AvoidZoneDynHullMemory&,NativeDynNarrowPhaseCalls&) noexcept;
    NativeDynIntersectTaskRuntime(const NativeDynIntersectTaskRuntime&)=delete;
    NativeDynIntersectTaskRuntime& operator=(const NativeDynIntersectTaskRuntime&)=delete;
    const void* table() const noexcept {return methods_;}
private:
    std::uintptr_t methods_[1];
    AvoidZoneDynHullMemory memory_;
    NativeDynNarrowPhaseCalls* calls_;
    static void __fastcall run(void*,void*);
};
} // namespace bsp
