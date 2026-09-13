#pragma once

#include <cstdint>

namespace bsp {
class NativeRenderActualOwners;
struct NativeVertexDeclarationCacheContext;

// Borrow the SAME actual registry/cache, string pool and canonical declaration
// companions used by loading. No registry copy, private map or extra refcount.
// +04/+08/+0C retain the existing actual 2Ch-record vector representation.
struct NativeVertexDeclarationRegistryLifetimeContext {
    NativeVertexDeclarationCacheContext& cache;
    NativeRenderActualOwners& owners;
    const volatile std::uint32_t* base_vtable_00d5f024;
};

// Placement-only B32534..B32555 within B32410: clear actual registry+04/+08/
// +0C/+10 in that order, then publish D5F060. The native fragment uses ESI=
// renderer and EBX=0. This new interface receives renderer+1A60 directly.
// Partial renderer constructor: no allocation or other renderer initialization.
void initialize_native_declaration_registry_00b32534_fragment(void*) noexcept;

// Complete B30270; ECX actual vector header, stack DWORD count, RET4.
// Signed growth/shrink comparisons; preserve default records' unknown+08 and
// resource+28. Publish count last on growth, decrement it before each shrink
// destructor. Failure does not destroy earlier newly constructed records.
void resize_native_declaration_records_00b30270(void* actual_header,
    std::uint32_t count, NativeVertexDeclarationCacheContext&);
// Complete B316A0..B316B6; ECX vector header, RET. Resize to zero, then free
// CURRENT data through the cache's shared BF6989 boundary; leave header intact.
void destroy_native_declaration_records_00b316a0(void* actual_header,
    NativeVertexDeclarationCacheContext&);

// Complete B31D40; incoming ECX unused, stack actual declaration, RET4.
// Decrement SAME actual+04; resolve canonical companion only at zero. Its
// terminal path dispatches current native virtual0 and real destruction/return.
void release_native_cached_declaration_00b31d40(void* actual_declaration,
    NativeRenderActualOwners&);

// Complete B31630; ECX registry, RET. Read last resource+CC unconditionally,
// subtract before current virtual+10 release, then reload count/data and
// destroy current last record before decrementing. Null cached resources have
// no native guard. Current D5F024/D5F060 profiles and B31D40 slot are covered;
// other profiles are explicit source errors after the preceding native writes.
void flush_native_declaration_registry_00b31630(void* actual_registry,
    NativeVertexDeclarationRegistryLifetimeContext&);
// Complete B32030..B3208E; ECX registry, RET. Publish D5F024, flush, destroy
// vector. Native state0 unwind destroys the vector if the flush throws.
void destroy_native_declaration_registry_00b32030(void* actual_registry,
    NativeVertexDeclarationRegistryLifetimeContext&);
// Complete B32210..B3222D; ECX registry, stack flags, EAX original pointer,
// RET4. Free through shared lifetime BF65AC boundary only if flags&1.
void* delete_native_declaration_registry_00b32210(void* actual_registry,
    std::uint32_t flags, NativeVertexDeclarationRegistryLifetimeContext&);

// New C++ interfaces, not original binary ABI/FH3 replacements. Actual storage
// and live canonical dependencies are preconditions. No game validation.
} // namespace bsp
