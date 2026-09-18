#pragma once
#include "bsp/native_string.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
// The owner name is provisional, inherited from the AllSeenHints reader.
struct alignas(4) NativeProfileHintsOwnerStorage {std::byte bytes[0x50];};
static_assert(sizeof(NativeProfileHintsOwnerStorage)==0x50);
struct NativeProfileHintsOwnerCalls {
    virtual ~NativeProfileHintsOwnerCalls()=default;
    virtual void* manager_00415350(NativeStringRawPoolContext&);
    virtual void* allocate_00bf681b();
    virtual void free_00bf65ac(void*) noexcept;
    virtual void register_00bd0c30(void* actual_manager,void* actual_owner);
    virtual void enter_section(void* actual_section);
    virtual void leave_section(void* actual_section);
};
struct NativeProfileHintsOwnerContext {
    NativeStringRawPoolContext& strings;
    NativeProfileHintsOwnerCalls& calls;
    void* volatile& actual_publication_00e17664;
    const char* initialization_label_00ce3a38; // verified "_auto_.log", including NUL
};
struct NativeProfileHintsOwnerOperation final {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    void* owner{};
    void* allocation{};
    void* registered_owner{};
    void* result{};
    alignas(4) std::byte temporary[8];
    void* captured_temporary_data{};
    std::uint32_t captured_temporary_bytes{};
    std::uint32_t guard[2]{}; // CE37FC / captured actual section pointer
    std::uint32_t native_site{};
    bool temporary_returned{},guard_held{},guard_depth_restored{},owner_published{},registered{};
    std::int32_t unwind_state{-1};
    ~NativeProfileHintsOwnerOperation();
    NativeProfileHintsOwnerOperation()=default;
    NativeProfileHintsOwnerOperation(const NativeProfileHintsOwnerOperation&)=delete;
    NativeProfileHintsOwnerOperation& operator=(const NativeProfileHintsOwnerOperation&)=delete;
    // Resolve actual temporary/allocation/publication/registration/guard
    // ownership first. This changes no native storage and unlocks nothing.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Complete normal 426250: ECX actual50h owner, EAX owner, RET. Partial writes;
// the temporary 8h header is resized to10, receives current length+1 bytes
// from CE3A38, and returns its captured data/size through the actual pool.
void* construct_native_profile_hints_owner_00426250(void*,
    NativeProfileHintsOwnerContext&,NativeProfileHintsOwnerOperation&);
// Complete426300: ECXowner, RET. Unconditionally clear E17664, then write
// base profile CE3818. Other owner bytes and manager registrations untouched.
void destroy_native_profile_hints_owner_00426300(void*,NativeProfileHintsOwnerContext&) noexcept;
// Complete426320: ECXowner,stackflags,EAXcapturedowner,RET4. Inlined destructor;
// bit0 additionally frees captured owner. Publication is not cleared again
// after the free callback. Correct library/domain names are not replaced.
void* delete_native_profile_hints_owner_00426320(void*,std::uint32_t,
    NativeProfileHintsOwnerContext&) noexcept;
// Complete normal4C1E90: no native inputs,EAXpublication,RET. Initial captured
// fast return; slow path captures manager+10,enters/increments,rechecks,creates
// raw50h,publishes,then re-resolves manager and CURRENT publication to register.
// Release the original section and reload the publication for the slow return.
// Escaping source callbacks retain progress, including a possibly held guard,
// in the caller-owned operation. Native FH3/SEH cleanup is not projected.
void* get_native_profile_hints_owner_004c1e90(
    NativeProfileHintsOwnerContext&,NativeProfileHintsOwnerOperation&);
} // namespace bsp
