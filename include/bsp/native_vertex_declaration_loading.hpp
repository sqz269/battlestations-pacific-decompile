#pragma once

#include "bsp/render_command_queue.hpp"
#include <cstdint>

namespace bsp {
class NativeStringStorage;

// Borrow the SAME actual canonical pool, native string domain and decoder
// globals. Tables contain actual {length,data,value} 0Ch records (8 usages,
// 17 types in the installed image). The current counts are signed and live.
// No pool/lock initialization, replacement tables or private owner domain.
struct NativeVertexDeclarationLoadingContext {
    NativeStringStorage& strings;
    void* pool_0108fd38;
    const volatile std::uint32_t* type_sizes_00d61cc0;
    const volatile std::uint32_t* declaration_vtable_00d61d1c;
    void* usages_0108d678;
    void* types_0108d5a8;
    volatile std::uint32_t& initialized_0108d6d8;
    const volatile std::int32_t& usage_count_00e13074;
    const volatile std::int32_t& type_count_00e13070;
    void* atexit_context;
    // BF6FF5 boundary: register the actual shutdown token CE0C30 or CE0C10
    // in the application's lifetime domain. Its integer result is ignored.
    // Dependencies outlive registered callbacks. No fallback registration.
    int (*register_atexit)(void*, std::uint32_t native_shutdown) noexcept;
};

// Complete actual native bodies. New C++ interfaces, not original ABI stubs.
// B47680 ECX raw1AC4h slab; stacked index; EAX same slab; RET4. Initialize
// only count/free stack and each D4h slot's trailing slab index at+D0.
void* initialize_native_vertex_declaration_slab_00b47680(void*, std::uint32_t) noexcept;
// B48560 ECX initialized actual pool; EAX raw slot; RET. Native lock remains
// held on allocation failure. Capacity/first-free updates are not rolled back.
void* allocate_native_vertex_declaration_slot_00b48560(void* actual_pool);
// Complete TEN-byte thunk: overwrite ECX with108FD38; JMP B48560. Incoming
// ECX D0h from the decoder is ignored; this is not a static destructor.
void* allocate_native_vertex_declaration_slot_00b488c0(void* actual_pool_0108fd38);
// B47D90 ECX actual0Ch array header; stacked actual20-byte record; RET4.
// B48330 ECX actualD0h owner; type/usage/offset stack; RET0Ch. No enum/range
// validation. Append main then usage, update packed cursor, recompute stride.
void append_native_vertex_element_00b47d90(void* actual_header, const void* actual_record);
void recompute_native_vertex_declaration_stride_00b47d20(void* actual_owner,
    const volatile std::uint32_t* actual_type_sizes_00d61cc0) noexcept;
void append_native_vertex_declaration_00b48330(void* actual_owner,
    std::uint32_t type, std::uint32_t usage, std::uint32_t offset,
    const volatile std::uint32_t* actual_type_sizes_00d61cc0);

// Full B2DBD0..B2E93B: original unused incoming ECX, stacked actual8h name
// header plus ignored DWORD, EAX actual declaration or null, RET8. All54
// aliases and native token grammar; actual allocation and array storage.
// Return creator reference+04=1. Syntax failure invokes CURRENT D61D1C+04
// scalar delete(flags1), without decrementing+04. Parse-time exceptions do
// NOT delete the allocated declaration. Constructor failure returns raw slot.
// Unsupported changed virtual profile is an explicit source error. Source
// string/CRT/atexit boundaries and native SEH limits are documented.
void* decode_native_vertex_declaration_00b2dbd0(const void* actual_name_header,
    NativeVertexDeclarationLoadingContext&);

// Concrete shutdown continuations for the actual borrowed token globals;
// reverse destruction leaves native headers and guard bits unchanged.
void destroy_native_vertex_format_types_00ce0c10(NativeVertexDeclarationLoadingContext&) noexcept;
void destroy_native_vertex_format_usages_00ce0c30(NativeVertexDeclarationLoadingContext&) noexcept;

class NativeVertexDeclarationReference;
struct NativeVertexDeclarationCompanionDisposal {
    void* context;
    void (*retire)(void*, NativeVertexDeclarationReference&) noexcept;
};

// Canonical host companion for the SAME actual owner+04 reference atomic.
// Bind exactly one into the caller's NativeRenderActualOwners domain; this
// companion neither initializes nor retains native storage and owns no map.
// Native dependencies survive final release; retirement removes the binding
// after actual destruction/slot return, without touching returned storage.
class NativeVertexDeclarationReference final : public RenderCommandReference {
public:
    NativeVertexDeclarationReference(void* actual_owner, void* actual_pool_0108fd38,
        const volatile std::uint32_t* type_sizes_00d61cc0,
        const volatile std::uint32_t* current_vtable_00d61d1c,
        NativeVertexDeclarationCompanionDisposal);
    ~NativeVertexDeclarationReference() override;
    NativeVertexDeclarationReference(const NativeVertexDeclarationReference&) = delete;
    NativeVertexDeclarationReference& operator=(const NativeVertexDeclarationReference&) = delete;
    void* storage() const noexcept { return storage_; }
    // Read-only source-domain check when another consumer reuses this ONE
    // canonical companion. No native state or reference operation is added.
    bool matches_context(void* pool, const volatile std::uint32_t* type_sizes,
        const volatile std::uint32_t* vtable) const noexcept {
        return pool_ == pool && type_sizes_ == type_sizes && vtable_ == vtable;
    }
    void release_zero_references() noexcept override;
private:
    enum class Phase { bound, destroying, retired };
    void* storage_;
    void* pool_;
    const volatile std::uint32_t* type_sizes_;
    const volatile std::uint32_t* vtable_;
    NativeVertexDeclarationCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    void require_current_profile() const noexcept;
};
} // namespace bsp
