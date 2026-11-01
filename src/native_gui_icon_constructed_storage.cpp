#include "bsp/native_gui_icon_constructed_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw Icon current74 supplier requires MSVC Win32 inline assembly.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Phase = NativeGuiIconConstructedStoragePhase;
static_assert(sizeof(void*) == sizeof(Word));
void* at(const void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
Word read(const void* p) noexcept { return *static_cast<const volatile Word*>(p); }
void stage_association(const volatile Word& sentinel, Word (&homes)[4], void* mesh) {
    const volatile Word* source = &sentinel;
    __asm {
        mov ecx, source
        mov eax, homes
        fld dword ptr [ecx]
        fst dword ptr [eax + 0ch]
        fstp dword ptr [eax + 8]
        mov edx, mesh
        mov dword ptr [eax + 4], edx
        mov dword ptr [eax], 0
    }
}
void stage_position(Word (&homes)[3]) {
    __asm {
        xorps xmm0, xmm0
        mov eax, homes
        movss dword ptr [eax], xmm0
        movss dword ptr [eax + 4], xmm0
        movss dword ptr [eax + 8], xmm0
    }
}
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
} // namespace

void construct_native_gui_icon_geometry_storage_00ab2540(void* actual,
    NativeGuiIconConstructedStorageFrame& f, NativeGuiIconConstructedStorageContext& c,
    NativeGuiIconConstructedStorageAcquired& a) {
    require(&f.context_ == &c && f.phase_ == Phase::fresh && a.phase == Phase::fresh &&
        f.association_.phase() == NativeModelStorageGeometryPhase::fresh && !a.mesh.started,
        "Icon74 requires its SAME fresh retained frame/acquisition");
    require(&c.geometry.actual_owners() == &c.owners,
        "Icon74 requires the SAME genuine canonical mesh/Model owner domain");
    f.phase_ = a.phase = Phase::entered;
    a.actual_icon = actual;
    try {
        a.observed_74 = *static_cast<const volatile std::uint8_t*>(at(actual, 0x74));
        if (a.observed_74 == 0) {
            a.captured_model = reinterpret_cast<void*>(read(at(actual, 0x4c)));
            a.model_captured = true;
            a.active_call_site = 0x00ab256b;
            a.mesh_entered = true;
            void* const mesh = c.geometry.create_native_mesh_00b73b60(a.mesh);
            a.captured_mesh = mesh;
            a.mesh_returned = true;
            stage_association(c.unchanged_00d7a260, f.arguments_, mesh);
            a.arguments_staged = true;
            a.active_call_site = 0x00ab25aa;
            // Metadata lookup AFTER the actual scalar spills; no x87 value is
            // live. The receiver remains the preallocation captured raw Model.
            auto* const model = dynamic_cast<NativeModelReference*>(
                &c.owners.resolve_actual(a.captured_model));
            require(model && static_cast<void*>(&model->model_owner().storage.node) == a.captured_model &&
                &model->model_owner().environment.retained_owners == &c.owners &&
                &model->model_owner().environment.constants.unchanged_00d7a260 == &c.unchanged_00d7a260,
                "Icon74 requires the captured Model's SAME authentic canonical reference and sentinel");
            a.association_entered = true;
            model->set_raw_geometry_00b75170(f.association_, a.association);
            a.association_returned = true;
            a.active_call_site = 0x00ab25b3;
            const auto decrement = c.decrement_00ce2220;
            a.captured_decrement = decrement;
            a.decrement_entered = true;
            const long result = decrement(static_cast<volatile long*>(at(mesh, 4)));
            a.creator_result = result;
            a.decremented = true;
            if (result == 0) {
                a.creator_profile = read(mesh);
                const auto* const table = static_cast<const volatile Word*>(
                    c.data.data_at(a.creator_profile, sizeof(Word)));
                require(reinterpret_cast<Word>(table) == a.creator_profile,
                    "Icon74 requires the genuine captured current mesh table alias");
                a.creator_target = table[0];
                require(a.creator_profile == 0x00d62d60 && a.creator_target == 0x00bd30e0 &&
                    c.owners.find(mesh) == a.mesh.companion,
                    "Icon74 already-zero mesh requires its SAME genuine captured current0 companion");
                a.active_call_site = 0x00ab25c3;
                a.creator_zero_entered = true;
                dispatch_native_render_actual_owner_zero(c.owners, mesh);
                a.creator_zero_returned = true;
            }
            // No mesh payload/companion access after a possible terminal call.
        }
        stage_position(f.position_);
        a.position_staged = true;
        a.active_call_site = 0x00ab25e3;
        a.position_entered = true;
        set_native_gui_widget_local_position_00aa7dc0(actual, f.position_, f.transform_,
            c.transform, f.bounds_, c.half_00d7a280);
        a.position_returned = true;
        f.phase_ = a.phase = Phase::complete;
    } catch (...) {
        a.escaped_call_site = a.active_call_site;
        a.failure = std::current_exception();
        f.phase_ = a.phase = Phase::failed;
        throw; // Source UNKNOWN retention only; no Native unwind action.
    }
}
} // namespace bsp
