#include "bsp/effect_admission.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Effect admission requires MSVC Win32 x87 and native pointer widths.
#endif

namespace bsp {
namespace {
static_assert(sizeof(EffectManager) == 8 && sizeof(void*) == 4);

// Exactly the translation copies, delta spills, and arithmetic at 86A66F..DA.
// ECX=original XYZ; EDX=actual world translation; stack=distance output.
__declspec(naked) void __fastcall distance_kernel(const float*, const float*, float*) {
    __asm {
        sub esp, 24
        fld dword ptr [edx]
        fstp dword ptr [esp]
        fld dword ptr [edx + 4]
        fstp dword ptr [esp + 4]
        fld dword ptr [edx + 8]
        fstp dword ptr [esp + 8]
        fld dword ptr [ecx]
        fsub dword ptr [esp]
        fstp dword ptr [esp + 12]
        fld dword ptr [ecx + 4]
        fsub dword ptr [esp + 4]
        fstp dword ptr [esp + 16]
        fld dword ptr [ecx + 8]
        fsub dword ptr [esp + 8]
        fstp dword ptr [esp + 20]
        fld dword ptr [esp + 16]
        fld dword ptr [esp + 12]
        fld dword ptr [esp + 20]
        fld st(1)
        fmulp st(2), st(0)
        fld st(2)
        fmulp st(3), st(0)
        fxch st(1)
        faddp st(2), st(0)
        fmul st(0), st(0)
        faddp st(1), st(0)
        mov eax, dword ptr [esp + 28]
        fstp dword ptr [eax]
        add esp, 24
        ret 4
    }
}

// Native FCOMIP threshold^2,distance then JA: equal/unordered take virtual path.
__declspec(naked) bool __fastcall threshold_kernel(const float*, const float*) {
    __asm {
        fld dword ptr [edx]
        fmul st(0), st(0)
        fld dword ptr [ecx]
        fxch st(1)
        fcomip st(0), st(1)
        fstp st(0)
        seta al
        ret
    }
}

class CapturedSection final {
public:
    explicit CapturedSection(SystemSingletonCriticalSection* section) : section_(section) {
        if (section_) {
            singleton_enter_critical_section(*section_);
            ++section_->recursion_18;
        }
    }
    ~CapturedSection() {
        if (section_) {
            --section_->recursion_18;
            singleton_leave_critical_section(*section_);
        }
    }
    CapturedSection(const CapturedSection&) = delete;
    CapturedSection& operator=(const CapturedSection&) = delete;
private:
    SystemSingletonCriticalSection* section_;
};

// Same storage pattern as the canonical lifetime manager's tracked section.
// The projection borrows the real first 1Ch bytes; it has no shadow counter.
struct OwnedSection {
    CRITICAL_SECTION native;
    std::uint32_t recursion_18;
    SystemSingletonCriticalSection projection{&native, recursion_18};
};
static_assert(sizeof(CRITICAL_SECTION) == 0x18);
static_assert(offsetof(OwnedSection, recursion_18) == 0x18);
} // namespace

bool admit_point_effect_0086a650(EffectAdmissionTemplateView owner,
    const std::array<float, 3>& point, CameraTransform& reference,
    EffectAdmissionDispatch& dispatch) {
    if ((reference.valid_flags & 2u) == 0) refresh_camera_world_00b6db70(reference);
    auto* cursor = owner.rows_08;
    auto* const end = reinterpret_cast<void* const*>(
        reinterpret_cast<std::uintptr_t>(cursor) + owner.count_0c * 4u);
    float distance;
    distance_kernel(point.data(), reference.world.data() + 12, &distance);
    bool any = false;
    for (; cursor != end; ++cursor) {
        void* const row_owner = *cursor;
        const auto row = dispatch.project_row(row_owner);
        if (row.gated_10 == 0) {
            any = true; // native does not write this row's +1C
            continue;
        }
        const bool admitted = threshold_kernel(&distance, &row.threshold_18)
            || dispatch.virtual_1c(row_owner, point, reference) != 0;
        any |= admitted;
        // Callback may have replaced *cursor; never write through the old view.
        dispatch.project_row(*cursor).admitted_1c = admitted ? 1u : 0u;
    }
    return any;
}

void unwind_effect_manager_base_00865fb0(EffectManager& owner,
    EffectManager* volatile& global) noexcept {
    global = nullptr;
    owner.original_vtable_identity_00 = 0x00ce3818u;
}

EffectManager& construct_effect_manager_00866230(EffectManager& owner,
    EffectManager* volatile& global, EffectManagerLifetimeAccess& access) {
    owner.original_vtable_identity_00 = 0x00d0d3ccu;
    try {
        owner.section_04 = access.create_section_00bd1860();
    } catch (...) {
        unwind_effect_manager_base_00865fb0(owner, global); // C94CC0
        throw;
    }
    return owner;
}

EffectManager* effect_manager_singleton_00866440(
    EffectManager* volatile& global, EffectManagerLifetimeAccess& access) {
    // Fast path returns the first load (866455); slow path reloads after unlock.
    if (auto* existing = global) return existing;
    {
        CapturedSection lock(access.manager_00415350().section_10);
        if (!global) {
            void* raw = access.allocate_00bf681b(
                {SingletonAllocationKind::object, 8, sizeof(EffectManager)});
            EffectManager* created = nullptr;
            if (raw) {
                created = ::new (raw) EffectManager; // deliberately uninitialized
                try {
                    construct_effect_manager_00866230(*created, global, access);
                } catch (...) {
                    created->~EffectManager();
                    access.free_00bf65ac(raw); // C94D08, before unlock C94D00
                    throw;
                }
            }
            global = created; // no rollback if subsequent registration throws
            auto& manager = access.manager_00415350();
            access.register_00bd0c30(manager, global); // reload after getter
        }
    }
    return global;
}

EffectManager* delete_effect_manager_008669d0(EffectManager* owner, std::uint32_t flags,
    EffectManager* volatile& global, EffectManagerLifetimeAccess& access) noexcept {
    auto* const original = owner;
    owner->original_vtable_identity_00 = 0x00d0d3ccu;
    access.destroy_section_0041cc80(owner->section_04);
    unwind_effect_manager_base_00865fb0(*owner, global);
    if ((flags & 1u) != 0) {
        owner->~EffectManager();
        access.free_00bf65ac(owner);
    }
    return original;
}

ConcreteEffectManagerLifetimeAccess::ConcreteEffectManagerLifetimeAccess(
    SingletonLifetimeDomain& domain) noexcept : domain_(domain) {}

SystemSingletonLifetimeOwner& ConcreteEffectManagerLifetimeAccess::manager_00415350() {
    return domain_.get_manager_00415350()->system_owner();
}

void* ConcreteEffectManagerLifetimeAccess::allocate_00bf681b(
    const SingletonAllocationRequest& request) {
    return singleton_lifetime_allocate(request);
}

void ConcreteEffectManagerLifetimeAccess::free_00bf65ac(void* raw) noexcept {
    singleton_lifetime_free(raw);
}

SystemSingletonCriticalSection* ConcreteEffectManagerLifetimeAccess::create_section_00bd1860() {
    void* raw = singleton_lifetime_allocate(
        {SingletonAllocationKind::critical_section, 0x1c, sizeof(OwnedSection)});
    if (!raw) return nullptr;
    auto* section = ::new (raw) OwnedSection;
    // No invented unwind free if InitializeCriticalSection itself fails.
    InitializeCriticalSection(&section->native);
    section->recursion_18 = 0;
    return &section->projection;
}

void ConcreteEffectManagerLifetimeAccess::destroy_section_0041cc80(
    SystemSingletonCriticalSection*& projection) noexcept {
    if (!projection) return;
    auto* section = static_cast<OwnedSection*>(projection->native_section);
    while (static_cast<std::int32_t>(projection->recursion_18) > 0) {
        --projection->recursion_18;
        singleton_leave_critical_section(*projection);
    }
    DeleteCriticalSection(&section->native);
    section->~OwnedSection();
    singleton_lifetime_free(section);
    projection = nullptr; // actual owner's pointer is cleared after free
}

void ConcreteEffectManagerLifetimeAccess::register_00bd0c30(
    SystemSingletonLifetimeOwner& owner, EffectManager* manager) {
    static_cast<ConcreteSingletonLifetimeManager*>(owner.native_owner)->register_object(manager);
}

} // namespace bsp
