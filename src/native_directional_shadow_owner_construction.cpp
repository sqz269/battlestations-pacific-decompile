#include "bsp/native_directional_shadow_owner_construction.hpp"
#include "bsp/camera_configuration.hpp"
#include "bsp/native_camera_configuration_leaves.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_camera_pool.hpp"
#include "bsp/native_frame_target_owner.hpp"
#include "bsp/native_material_effect_runtime.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_shadow_viewport_matrix.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_date_route.hpp"
#include "bsp/native_world_configuration_leaves.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native directional shadow construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(const void* p, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
std::uint32_t word(const void* p, std::uint32_t offset = 0) noexcept {
    const void* slot = at(p, offset);
    std::uint32_t result;
    __asm {
        mov eax, slot
        mov eax, dword ptr [eax]
        mov result, eax
    }
    return result;
}
void put(void* p, std::uint32_t offset, std::uint32_t value) noexcept {
    void* slot = at(p, offset);
    __asm {
        mov eax, slot
        mov edx, value
        mov dword ptr [eax], edx
    }
}
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }
std::uint32_t key(const void* p) noexcept { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
std::uint32_t zero_depth() noexcept {
    std::uint32_t result;
    __asm {
        fldz
        fstp dword ptr result
    }
    return result;
}
std::uint32_t one_depth() noexcept {
    std::uint32_t result;
    __asm {
        fld1
        fstp dword ptr result
    }
    return result;
}
void finish_base_fields(void* owner, const volatile std::uint32_t& one,
    const volatile std::uint32_t& scalar) noexcept {
    const auto* one_cell = &one;
    const auto* scalar_cell = &scalar;
    __asm {
        mov eax, owner
        xorps xmm0, xmm0
        mov edx, one_cell
        movss xmm1, dword ptr [edx]
        movss dword ptr [eax + 038h], xmm0
        movss dword ptr [eax + 03ch], xmm1
        movss dword ptr [eax + 040h], xmm0
        movss dword ptr [eax + 0390h], xmm0
        movss dword ptr [eax + 0394h], xmm0
        movss dword ptr [eax + 0398h], xmm0
        movss dword ptr [eax + 039ch], xmm0
        mov edx, scalar_cell
        movss xmm0, dword ptr [edx]
        movss dword ptr [eax + 008h], xmm0
    }
}
} // namespace

// One capability enters here. Factory/derived/base share it and its persistent
// raw lanes; no nested public admission, native child ownership or host map.
struct NativeDirectionalShadowConstructionExecution {
    using Block = NativeDirectionalShadowConstructionBlock;
    using Admission = Block::Admission;
    using BodyState = Block::BodyState;
    using BodyPhase = Block::BodyPhase;
    Admission& admission;
    NativeDirectionalShadowConstructionContext& context;
    Block& block;

    static void validate(void* storage, bool factory, void* light,
        NativeDirectionalShadowConstructionContext& c, const Admission& token) {
        if ((!factory && (!storage || key(storage) % 4u)) || !light)
            throw std::invalid_argument("shadow construction requires actual aligned508h storage and light");
        token.validate_prepared();
        auto& cache = c.texture_cache;
        auto& texture = c.lifetime.texture_2d;
        if (token.camera_environment_ != &c.cameras || token.node_constants_ != &c.node_constants ||
            token.block_->viewport_registry_ != &c.viewports || &c.cameras.nodes != &c.lifetime.nodes ||
            &c.renderer_00f8d394 != &cache.textures.current_renderer_00f8d394 ||
            &c.renderer_00f8d394 != &texture.renderer_notification.actual_renderer_00f8d394 ||
            &cache.strings != &cache.textures.strings || &cache.strings != &cache.dates.physical.strings ||
            cache.textures.cache != &cache || &cache.textures.current_vfs_0109ceec !=
                &cache.dates.physical.manager_0109ceec ||
            &cache.synchronization_0108d6dc != &cache.textures.synchronization_0108d6dc ||
            &cache.synchronization_0108d6dc != &texture.renderer_notification.synchronization ||
            texture.renderer_notification.actual_native_string_pool != &cache.strings ||
            &cache.textures.owners.texture_context() != &texture ||
            &c.lifetime.frame_target.actual_surface_context != &texture.surfaces ||
            static_cast<const volatile void*>(&c.renderer_00f8d394) !=
                static_cast<const volatile void*>(&texture.surfaces.actual_renderer_00f8d394) ||
            c.renderer_table_00d5f0a8 != texture.actual_renderer_profile_00d5f0a8)
            throw std::invalid_argument("shadow construction requires the same actual provider domains");
        if (!c.renderer_table_00d5f0a8 || !c.white_name_00ce77a4 ||
            !c.lifetime.table_00d5b5d8 || !c.lifetime.table_00d5e5f8 ||
            !c.lifetime.table_00d61948 || !c.lifetime.table_00d5e600)
            throw std::invalid_argument("shadow construction requires actual table and literal bindings");
        for (const auto* name : c.camera_names_00d5b5bc_00d5b5a8_00d5b594_00d5b580)
            if (!name) throw std::invalid_argument("shadow construction requires all four original names");
        // Binding checks/host assignments only, before admission consumption.
        // Private raw-pool cell identity stays an explicit caller obligation.
        bind_native_texture_vfs_name_resolution(cache.textures, c.texture_names);
        bind_static_native_camera_pool_0108ffb0(c.cameras.pool_0108ffb0);
    }

    NativeStringRawPoolContext& strings() { return context.cameras.nodes.require_raw_name_pool(); }
    void site(BodyState& state, std::uint32_t address) noexcept { state.native_site = address; }
    void release_captured_name(void* header, void* data, std::uint32_t getter_site,
        std::uint32_t return_site) {
        if (!data) return;
        const auto count = word(header)+1u; // capture before current getter
        auto& pool = strings();
        site(block.base_state_,getter_site);
        auto* actual = native_string_pool_get_or_create_00419cc0(
            pool.actual_published_01090aa8,pool.actual_manager_publication_01090aa0);
        site(block.base_state_,return_site);
        return_native_string_pool_00bd1510(actual,data,count,pool.actual_small_returns_disabled_01090aa4);
    }
    NativeCameraOwner& camera(void* actual) {
        auto* reference = dynamic_cast<NativeCameraReference*>(
            context.lifetime.nodes.attachments.find_actual_node(key(actual)));
        if (!reference || &reference->camera_owner().storage.node != actual)
            throw std::invalid_argument("shadow construction requires the canonical actual camera");
        return reference->camera_owner();
    }
    NativeViewportOwner& viewport(void* owner, std::uint32_t offset) noexcept {
        return *static_cast<NativeViewportOwner*>(pointer(word(owner, offset)));
    }

    void unwind_base() {
        auto& s = block.base_state_;
        // DEC698: consume the map edge before invoking its cleanup. Map-only
        // states5/8/11/14 are retained, but normal execution never publishes them.
        static constexpr int next[19] = {-1,0,0,0,3,0,0,6,0,0,9,0,0,12,0,0,0,0,0};
        static constexpr std::uint32_t actions[19] = {0xcb6410,0xcb6418,0xcb6420,
            0xcb642b,0xcb6433,0xcb6433,0xcb644c,0xcb6454,0xcb6454,0xcb646d,
            0xcb6475,0xcb6475,0xcb648e,0xcb6496,0xcb6496,0xcb64af,0xcb64ba,0xcb64c5,0xcb64d0};
        while (s.unwind_state >= 0) {
            const int state = s.unwind_state;
            s.unwind_state = next[state];
            site(s, actions[state]);
            if (state == 0) destroy_native_ref_counted_base_00bd30f0(pointer(word(block.base_lanes_,4)));
            else if (state == 1) destroy_native_string_header_0041dd20(at(block.base_lanes_,8),strings());
            else if (state == 2 || state >= 15) singleton_lifetime_free(pointer(word(block.base_lanes_,0x48)));
            else if (state == 3 || state == 6 || state == 9 || state == 12) {
                auto& slot = block.cameras_[(state-3)/3];
                if (!slot.camera_completed) {
                    if (slot.owner && slot.owner->phase == NativeCameraOwner::Phase::prepared) slot.owner.reset();
                    return_native_camera_slot_00b71350(pointer(word(block.base_lanes_,0x48)));
                }
                // A post-camera host binding diagnostic must retain a live
                // companion/slot. This is not a new native EH state/action.
            } else {
                const unsigned index = static_cast<unsigned>((state-4)/3);
                const auto bit = 1u << index;
                const auto flags = word(block.base_lanes_);
                if (flags & bit) {
                    put(block.base_lanes_,0,flags & ~bit);
                    destroy_native_string_header_0041dd20(at(block.base_lanes_,0x10+index*8),strings());
                }
            }
        }
    }

    void* make_viewport(std::size_t record, int state, std::uint32_t alloc_site,
        std::uint32_t ctor_site) {
        auto& s = block.base_state_;
        site(s,alloc_site);
        auto* raw = singleton_lifetime_allocate({SingletonAllocationKind::object,0x34,0x34});
        put(block.base_lanes_,0x48,key(raw));
        s.unwind_state = state;
        if (!raw) return nullptr; // original explicit null arm
        site(s,ctor_site);
        auto* result = initialize_native_viewport_owner_00b1f850(raw,context.cameras.viewport);
        context.viewports.constructed(admission.viewport_admissions_[record],*result);
        return result; // parent disarms raw free before publishing
    }

    void make_camera(void* owner, unsigned index) {
        static constexpr std::uint32_t alloc_sites[] = {0xa8e434,0xa8e4aa,0xa8e51d,0xa8e590};
        static constexpr std::uint32_t name_sites[] = {0xa8e451,0xa8e4c7,0xa8e53a,0xa8e5ad};
        static constexpr std::uint32_t ctor_sites[] = {0xa8e46a,0xa8e4dd,0xa8e550,0xa8e5c3};
        static constexpr std::uint32_t release_sites[] = {0xa8e499,0xa8e50c,0xa8e57f,0xa8e5ed};
        auto& s = block.base_state_;
        site(s,alloc_sites[index]);
        void* raw = allocate_native_camera_slot_00b71930();
        put(block.base_lanes_,0x48,key(raw));
        s.unwind_state = 3+static_cast<int>(index)*3;
        auto* name = at(block.base_lanes_,0x10+index*8);
        const auto bit = 1u << index;
        void* result = nullptr;
        if (raw) {
            site(s,name_sites[index]);
            construct_native_string_cstring_0041e870(name,
                context.camera_names_00d5b5bc_00d5b5a8_00d5b594_00d5b580[index],strings());
            if (index) put(block.base_lanes_,0,word(block.base_lanes_) | bit);
            ++s.unwind_state;
            if (!index) put(block.base_lanes_,0,1); // A8E462, after state4
            auto& slot = block.cameras_[index];
            slot.owner.emplace(raw,0x45c,context.cameras,std::move(admission.scene_admissions_[index]));
            site(s,ctor_sites[index]);
            result = construct_native_camera_00b71a80(*slot.owner,name,context.node_constants,
                std::move(admission.viewport_admissions_[index+1]));
            slot.camera_completed = true;
            slot.reference.emplace(*slot.owner,NativeCameraCompanionDisposal{&slot.retirement,
                Block::record_camera_retirement},std::move(admission.lifetime_admissions_[index]));
        }
        const auto flags = word(block.base_lanes_);
        put(owner,0x20+index*4,key(result));
        s.unwind_state = 0; // before any normal temporary return
        if (flags & bit) {
            auto* data = pointer(word(name,4)); // before the normal flags clear
            // Normal fourth cleanup deliberately leaves bit8 set (A8E5DA).
            if (index != 3) put(block.base_lanes_,0,flags & ~bit);
            release_captured_name(name,data,release_sites[index],release_sites[index]+7);
        }
    }

    void* base(void* owner, void* light);
    void* derived(void* owner, void* light);
    void* factory(void* light);
};
void* NativeDirectionalShadowConstructionExecution::base(void* owner, void* light) {
    auto& s = block.base_state_;
    s.phase = BodyPhase::running;
    put(block.base_lanes_,0x48,key(light)); // original public argument
    put(block.base_lanes_,0,0);           // saved conditional-cleanup flags
    site(s,0xa8e302);
    put(owner,0,0x00ceb130);
    put(block.base_lanes_,4,key(owner));
    put(owner,4,1);
    put(owner,0x34,0);
    put(owner,0,0x00d5b574);
    put(owner,0x30,0);
    for (const auto offset : {0x4f0u,0x4f4u,0x4f8u,0x4fcu,0x500u,
        0x10u,0x14u,0x18u,0x1cu,0x20u,0x24u,0x28u,0x2cu,0x504u}) put(owner,offset,0);
    put(owner,0x388,word(context.target_00f8bbf0,4)); // two separate global reads
    put(owner,0x38c,word(context.target_00f8bbf0,8));
    auto* const incoming_light = pointer(word(block.base_lanes_,0x48));
    put(owner,0x0c,key(incoming_light));
    put(owner,0x34,word(incoming_light,0xa4));
    auto* const captured_renderer = const_cast<void*>(context.renderer_00f8d394); // A8E38B
    s.unwind_state = 0; // A8E397, before white-header initialization/resize
    try {
        auto* white = at(block.base_lanes_,8);
        put(white,0,0); put(white,4,0);
        site(s,0xa8e3a3);
        resize_native_string_header_0041dd40(white,strings(),9,true);
        if (auto* data = pointer(word(white,4))) {
            const auto count = word(white)+1u;
            site(s,0xa8e3be);
            if (count) std::memmove(data,context.white_name_00ce77a4,count); // BF7680 admits overlap
        }
        // Current profile/slot after string construction, captured receiver.
        site(s,0xa8e3c6);
        const auto profile = word(captured_renderer);
        if (profile != 0x00d5f0a8)
            throw std::invalid_argument("shadow constructor requires current D5F0A8 renderer");
        const auto slot64 = context.renderer_table_00d5f0a8[0x64/4];
        s.unwind_state = 1; // A8E3D3, immediately before the loader call
        site(s,0xa8e3d8);
        if (slot64 != 0x00b319b0)
            throw std::invalid_argument("unsupported current shadow texture-loader slot64");
        auto* texture = load_native_renderer_texture_00b319b0(captured_renderer,white,0,
            context.texture_cache,&*block.cache_acquisition_);
        put(owner,0x384,key(texture)); // no additional retain
        auto* white_data = pointer(word(white,4)); // A8E3E0, before state disarm
        s.unwind_state = 0; // A8E3E6, before normal white return
        release_captured_name(white,white_data,0xa8e3f7,0xa8e3fe);

        void* view = make_viewport(0,2,0xa8e405,0xa8e41c);
        s.unwind_state = 0;
        put(owner,0x504,key(view));
        for (unsigned i=0;i<4;++i) make_camera(owner,i);

        static constexpr std::uint32_t mode_sites[] = {0xa8e5fe,0xa8e608,0xa8e612,0xa8e61c};
        static constexpr std::uint32_t flags_sites[] = {0xa8e626,0xa8e630,0xa8e63a,0xa8e644};
        static constexpr std::uint32_t color_sites[] = {0xa8e650,0xa8e65c,0xa8e668,0xa8e674};
        for (unsigned i=0;i<4;++i) {
            site(s,mode_sites[i]);
            auto& current = camera(pointer(word(owner,0x20+4*i)));
            set_camera_render_mode_00b6fdf0(current.frame,current.storage.camera.render_mask_19c,2);
        }
        for (unsigned i=0;i<4;++i) {
            site(s,flags_sites[i]);
            const std::uint32_t flags = 2;
            set_native_camera_clear_flags_00b6fe10(pointer(word(owner,0x20+4*i)),&flags);
        }
        for (unsigned i=0;i<4;++i) {
            site(s,color_sites[i]);
            const std::uint32_t color = 0xffffffffu;
            set_native_camera_clear_color_00b6fe50(pointer(word(owner,0x20+4*i)),nullptr,color);
        }
        static constexpr std::uint32_t alloc_sites[] = {0xa8e67b,0xa8e6a4,0xa8e6cd,0xa8e6f6};
        static constexpr std::uint32_t ctor_sites[] = {0xa8e692,0xa8e6bb,0xa8e6e4,0xa8e70d};
        for (unsigned i=0;i<3;++i) {
            view = make_viewport(5+i,15+static_cast<int>(i),alloc_sites[i],ctor_sites[i]);
            s.unwind_state = 0;
            put(owner,0x10+i*4,key(view));
        }
        view = make_viewport(8,18,alloc_sites[3],ctor_sites[3]);
        auto* first_view = static_cast<NativeViewportOwner*>(pointer(word(owner,0x10))); // A8E716
        void* first_camera = pointer(word(owner,0x20)); // A8E71A
        s.unwind_state = 0;
        put(owner,0x1c,key(view));
        site(s,0xa8e724);
        set_native_camera_viewport_00b71990(camera(first_camera),first_view);
        static constexpr std::uint32_t replace_sites[] = {0xa8e730,0xa8e73c,0xa8e748};
        for (unsigned i=1;i<4;++i) {
            auto* incoming = static_cast<NativeViewportOwner*>(pointer(word(owner,0x10+i*4)));
            auto* actual_camera = pointer(word(owner,0x20+i*4));
            site(s,replace_sites[i-1]);
            set_native_camera_viewport_00b71990(camera(actual_camera),incoming);
        }
        static constexpr std::uint32_t min_sites[] = {0xa8e756,0xa8e764,0xa8e772,0xa8e780};
        static constexpr std::uint32_t max_sites[] = {0xa8e78e,0xa8e79c,0xa8e7aa,0xa8e7b8};
        for (unsigned i=0;i<4;++i) {
            const auto bits = zero_depth(); // one native FLDZ/FSTP32 per reached call
            site(s,min_sites[i]);
            set_native_viewport_min_depth_00b1f750(viewport(owner,0x10+i*4),bits);
        }
        for (unsigned i=0;i<4;++i) {
            const auto bits = one_depth(); // one native FLD1/FSTP32 per reached call
            site(s,max_sites[i]);
            set_native_viewport_max_depth_00b1f760(viewport(owner,0x10+i*4),bits);
        }
        auto bits = zero_depth();
        site(s,0xa8e7c9);
        set_native_viewport_min_depth_00b1f750(viewport(owner,0x504),bits);
        bits = one_depth();
        site(s,0xa8e7da);
        set_native_viewport_max_depth_00b1f760(viewport(owner,0x504),bits);
        const auto width = word(owner,0x388), height = word(owner,0x38c);
        auto* current_view = static_cast<NativeViewportOwner*>(pointer(word(owner,0x504)));
        put(block.base_lanes_,0x30,width); put(block.base_lanes_,0x34,height);
        site(s,0xa8e7fe);
        set_native_viewport_dimensions_00b1f940(*current_view,at(block.base_lanes_,0x30));
        site(s,0xa8e803);
        finish_base_fields(owner,context.node_constants.one_00d7a24c,
            context.cameras.constants.scalar_00ce77fc);
        s.phase = BodyPhase::complete;
        return owner;
    } catch (...) {
        s.phase = BodyPhase::failed;
        try { unwind_base(); } catch (...) { std::terminate(); }
        throw;
    }
}
void* NativeDirectionalShadowConstructionExecution::derived(void* owner, void* light) {
    auto& s = block.derived_state_;
    s.phase = BodyPhase::running;
    put(block.derived_lanes_,0x5c,key(light));
    put(block.derived_lanes_,0,key(owner));
    try {
        site(s,0xa8fa57);
        base(owner,pointer(word(block.derived_lanes_,0x5c)));
        s.unwind_state = 0; // A8FA66, only after full base success
        put(owner,0,0x00d5b5d8);
        static constexpr std::uint32_t origin_sites[] = {0xa8fa78,0xa8fad6,0xa8fb34,0xa8fb9a};
        static constexpr std::uint32_t dimensions_sites[] = {0xa8fa9d,0xa8fafb,0xa8fb59,0xa8fbbf};
        static constexpr std::uint32_t matrix_sites[] = {0xa8faad,0xa8fb0b,0xa8fb69,0xa8fbcf};
        static constexpr std::uint32_t copy_sites[] = {0xa8fab9,0xa8fb17,0xa8fb75,0xa8fbdb};
        for (unsigned i=0;i<4;++i) {
            // These are unsigned DWORD SHR1 operations, not float conversion.
            if (i==0) { put(block.derived_lanes_,4,0); put(block.derived_lanes_,8,0); }
            if (i==1) { put(block.derived_lanes_,4,word(owner,0x388)>>1); put(block.derived_lanes_,8,0); }
            if (i==2) { put(block.derived_lanes_,8,word(owner,0x38c)>>1); put(block.derived_lanes_,4,0); }
            if (i==3) {
                const auto height = word(owner,0x38c), width = word(owner,0x388);
                put(block.derived_lanes_,8,height>>1); put(block.derived_lanes_,4,width>>1);
            }
            site(s,origin_sites[i]);
            set_native_viewport_origin_00b1f920(viewport(owner,0x10+i*4),at(block.derived_lanes_,4));
            std::uint32_t width, height;
            if (i==3) { width=word(owner,0x388); height=word(owner,0x38c); }
            else { height=word(owner,0x38c); width=word(owner,0x388); }
            if (i<2) { put(block.derived_lanes_,8,height>>1); put(block.derived_lanes_,4,width>>1); }
            else { put(block.derived_lanes_,4,width>>1); put(block.derived_lanes_,8,height>>1); }
            site(s,dimensions_sites[i]);
            set_native_viewport_dimensions_00b1f940(viewport(owner,0x10+i*4),at(block.derived_lanes_,4));
            site(s,matrix_sites[i]);
            void* matrix = build_native_shadow_viewport_matrix_00a8aaa0(owner,nullptr,
                at(block.derived_lanes_,0x0c),pointer(word(owner,0x10+i*4)));
            site(s,copy_sites[i]);
            copy_native_camera_matrix_004134f0(at(owner,0x244+i*0x40),nullptr,matrix);
        }
        site(s,0xa8fbe6);
        auto* captured_color = static_cast<NativeSurfaceOwnerStorage*>(
            native_shadow_target_field14_00a8fda0(context.target_00f8bbf0));
        std::uint32_t field = 0x4f0;
        put(block.derived_lanes_,0x5c,4); // public light word reused as loop count
        do {
            site(s,0xa8fc02);
            void* raw = singleton_lifetime_allocate({SingletonAllocationKind::object,0x40,0x40});
            put(block.derived_lanes_,4,key(raw)); // aliases earlier width word
            s.unwind_state = 1;
            NativeFrameTargetOwnerStorage* result = nullptr;
            if (raw) {
                site(s,0xa8fc19);
                result = construct_native_frame_target_owner_00b1fbb0(raw);
            }
            s.unwind_state = 0; // raw free disarmed before publication/use
            put(owner,field,key(result));
            site(s,0xa8fc2c);
            set_native_frame_target_color_00b1fab0(*result,0,captured_color,context.lifetime.frame_target);
            site(s,0xa8fc37);
            auto* current_depth = static_cast<NativeSurfaceOwnerStorage*>(
                native_shadow_target_field1c_00a8fdc0(context.target_00f8bbf0));
            auto* current_target = static_cast<NativeFrameTargetOwnerStorage*>(pointer(word(owner,field)));
            site(s,0xa8fc3f);
            set_native_frame_target_depth_00b1fb00(*current_target,current_depth,context.lifetime.frame_target);
            field += 4;
            put(block.derived_lanes_,0x5c,word(block.derived_lanes_,0x5c)-1u);
        } while (word(block.derived_lanes_,0x5c));

        site(s,0xa8fc50);
        void* raw = singleton_lifetime_allocate({SingletonAllocationKind::object,0x40,0x40});
        put(block.derived_lanes_,0x5c,key(raw)); // same public lane now fifth raw target
        s.unwind_state = 2;
        NativeFrameTargetOwnerStorage* result = nullptr;
        if (raw) {
            site(s,0xa8fc67);
            result = construct_native_frame_target_owner_00b1fbb0(raw);
        }
        s.unwind_state = 0;
        put(owner,0x500,key(result));
        site(s,0xa8fc7e);
        set_native_frame_target_color_00b1fab0(*result,0,captured_color,context.lifetime.frame_target);
        site(s,0xa8fc89);
        auto* current_depth = static_cast<NativeSurfaceOwnerStorage*>(
            native_shadow_target_field1c_00a8fdc0(context.target_00f8bbf0));
        auto* current_target = static_cast<NativeFrameTargetOwnerStorage*>(pointer(word(owner,0x500)));
        site(s,0xa8fc95);
        set_native_frame_target_depth_00b1fb00(*current_target,current_depth,context.lifetime.frame_target);
        s.phase = BodyPhase::complete;
        return owner;
    } catch (...) {
        s.phase = BodyPhase::failed;
        try {
            if (s.unwind_state == 1 || s.unwind_state == 2) {
                const auto lane = s.unwind_state == 1 ? 4u : 0x5cu;
                site(s,s.unwind_state == 1 ? 0xcb6558u : 0xcb6563u);
                s.unwind_state = 0;
                singleton_lifetime_free(pointer(word(block.derived_lanes_,lane)));
            }
            if (s.unwind_state == 0) {
                s.unwind_state = -1;
                site(s,0xcb6550);
                destroy_native_directional_shadow_owner_00a8dec0(
                    pointer(word(block.derived_lanes_)),context.lifetime);
            }
        } catch (...) { std::terminate(); }
        throw;
    }
}

void* NativeDirectionalShadowConstructionExecution::factory(void* light) {
    auto& s = block.factory_state_;
    s.phase = BodyPhase::running;
    try {
        site(s,0xa8fd4e);
        void* raw = singleton_lifetime_allocate({SingletonAllocationKind::object,0x508,0x508});
        put(block.factory_raw_lane_,0,key(raw));
        s.unwind_state = 0; // A8FD5C
        void* result = nullptr;
        if (raw) { site(s,0xa8fd69); result=derived(raw,light); }
        s.phase = BodyPhase::complete;
        return result;
    } catch (...) {
        s.phase = BodyPhase::failed;
        if (s.unwind_state == 0) {
            s.unwind_state = -1; // consume ownership; raw preimage is not a live pointer API
            site(s,0xcb6580);
            singleton_lifetime_free(pointer(word(block.factory_raw_lane_)));
        }
        throw;
    }
}

void* construct_native_directional_shadow_base_00a8e2e0(void* owner, void* light,
    NativeDirectionalShadowConstructionContext& context,
    NativeDirectionalShadowConstructionBlock::Admission&& admission) {
    NativeDirectionalShadowConstructionExecution::validate(owner,false,light,context,admission);
    auto attempt = std::move(admission);
    attempt.begin_execution();
    NativeDirectionalShadowConstructionExecution execution{attempt,context,*attempt.block_};
    return execution.base(owner,light);
}
void* construct_native_directional_shadow_owner_00a8fa30(void* owner, void* light,
    NativeDirectionalShadowConstructionContext& context,
    NativeDirectionalShadowConstructionBlock::Admission&& admission) {
    NativeDirectionalShadowConstructionExecution::validate(owner,false,light,context,admission);
    auto attempt = std::move(admission);
    attempt.begin_execution();
    NativeDirectionalShadowConstructionExecution execution{attempt,context,*attempt.block_};
    return execution.derived(owner,light);
}
void* allocate_native_directional_shadow_owner_00a8fd30(void* light,
    NativeDirectionalShadowConstructionContext& context,
    NativeDirectionalShadowConstructionBlock::Admission&& admission) {
    NativeDirectionalShadowConstructionExecution::validate(nullptr,true,light,context,admission);
    auto attempt = std::move(admission);
    attempt.begin_execution();
    NativeDirectionalShadowConstructionExecution execution{attempt,context,*attempt.block_};
    return execution.factory(light);
}
} // namespace bsp
