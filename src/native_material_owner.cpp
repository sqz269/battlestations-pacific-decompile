#include "bsp/native_material_owner.hpp"
#include "bsp/native_material_lifetime.hpp"
#include "bsp/scene_renderer_cc13.hpp"
#include "bsp/wire_goal4_chain621_switches.hpp"
#include "native_material_typed_storage.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native material ownership requires MSVC Win32 pointer and atomic widths.
#endif

namespace bsp {
// CPP-private observer bound only by the authentic private owning entry.
class NativeMaterialStorageIssuer final {
public:
    explicit NativeMaterialStorageIssuer(NativeMaterialFrame& f,bool payload=false) noexcept:frame_(f),payload_(payload){}
    void prefix(NativeMaterialStorage& m) noexcept {frame_.cpp_prefix_born(m);}
    void count(NativeMaterialStorage& m) noexcept {frame_.native_count1_born(m);}
    void returned(NativeMaterialStorage& m) noexcept {frame_.native_constructor_returned(m);}
    void release(NativeRenderActualOwners& owners,void* actual,std::size_t site){frame_.release_child(owners,actual,site,payload_);}
private:
    NativeMaterialFrame& frame_;
    const bool payload_;
};
static_assert(sizeof(NativeMaterialStorage) == 0x110);
static_assert(offsetof(NativeMaterialStorage, references_04) == 4);
static_assert(offsetof(NativeMaterialStorage, source_owner_0c) == 0x0c);
static_assert(offsetof(NativeMaterialStorage, textures_10) == 0x10);
static_assert(offsetof(NativeMaterialStorage, texture_count_34) == 0x34);
static_assert(offsetof(NativeMaterialStorage, lighting_38) == 0x38);
static_assert(offsetof(NativeMaterialStorage, effect_7c) == 0x7c);
static_assert(offsetof(NativeMaterialStorage, parameters_80) == 0x80);
static_assert(offsetof(NativeMaterialStorage, parameter_count_100) == 0x100);
static_assert(offsetof(NativeMaterialStorage, word_104) == 0x104);
static_assert(offsetof(NativeMaterialStorage, word_108) == 0x108);
static_assert(offsetof(NativeMaterialStorage, lighting_enabled_10c) == 0x10c);
static_assert(offsetof(NativeMaterialStorage, retain_source_10d) == 0x10d);
namespace {
constexpr std::uint32_t base_table = 0x00ceb130;
constexpr std::uint32_t material_table = 0x00d5e520;
struct BaseCleanup {
    NativeMaterialStorage& material;
    ~BaseCleanup() { material.vtable_00 = base_table; }
};
void retain_actual(void* identity) noexcept {
    auto* actual = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(identity) + 4));
    actual->fetch_add(1, std::memory_order_seq_cst);
}
void assign_actual(void*& slot, void* incoming, NativeRenderActualOwners& owners,
    NativeMaterialStorageIssuer* issuer=nullptr,std::size_t site=0) {
    void* const old = slot;
    if (old == incoming) return;
    slot = incoming;
    if (incoming) retain_actual(incoming);
    if (old) {if(issuer)issuer->release(owners,old,site);else release_native_render_actual_owner(owners, old);}
}
void release_then_clear(void*& slot, NativeRenderActualOwners& owners,
    NativeMaterialStorageIssuer* issuer=nullptr,std::size_t site=0,
    NativeRenderActualOwnerReleaseObservation* external=nullptr) {
    void* const old = slot;
    if (!old) return;
    if(external)release_native_render_actual_owner_observed(owners,old,*external);
    else if(issuer)issuer->release(owners,old,site);else release_native_render_actual_owner(owners, old);
    slot = nullptr;
}
NativeMaterialStorage& prepare(void* slot,NativeMaterialStorageIssuer* issuer) {
    if (!slot || reinterpret_cast<std::uintptr_t>(slot) % alignof(NativeMaterialStorage))
        throw std::invalid_argument("material construction requires aligned actual110h storage");
    auto* material = detail::begin_material_storage(slot);
    if(issuer)issuer->prefix(*material);
    material->vtable_00 = base_table;
    material->references_04.store(1, std::memory_order_relaxed);
    if(issuer)issuer->count(*material);
    material->vtable_00 = material_table;
    return *material;
}
void initialize_common(NativeMaterialStorage& material) noexcept {
    material.source_owner_0c = nullptr;
    material.texture_count_34 = 0;
    for (auto& texture : material.textures_10) texture = nullptr;
    // Reuse the established17-DWORD lighting initializer, over this SAME record.
    material.lighting_38 = initialize_lighting_record_00b17840();
    material.effect_7c = nullptr;
    material.parameter_count_100 = 0;
}
std::int32_t texture_count(const NativeMaterialStorage& material) {
    const auto count = static_cast<std::int32_t>(material.texture_count_34);
    if (count < 0 || count > 9)
        throw std::logic_error("native material texture extent is outside0..9");
    return count;
}
} // namespace

namespace {
NativeMaterialStorage* initialize_body(void* slot,
    void* effect, NativeRenderActualOwners& owners,NativeMaterialStorageIssuer* issuer) {
    auto& material = prepare(slot,issuer);
    material.word_08 = 0;
    try {
        initialize_common(material);
        if constexpr (kWireGoal4Chain621Bound) {
            // 00B18900 stores FFFFFFFF at +104 after zeroing +100. Reuse
            // 00B17310's single +104 store on this same live allocation.
            // Evidence: bsp.py show 00b18900 / 00b17310.
            scene_renderer::set_material_alpha_ref_00b17310(&material, -1);
        } else {
            material.word_104 = 0xffffffff;
        }
        material.word_108 = 0xffffffff;
        material.lighting_enabled_10c = 0;
        material.retain_source_10d = 0;
        assign_actual(material.effect_7c, effect, owners,issuer,0);
        // Reload the actual publication after assignment/native terminal call.
        if (material.effect_7c)
            *(static_cast<std::uint8_t*>(material.effect_7c) + 0xb4) = 1;
        if(issuer)issuer->returned(material); // exact Native normal continuation, before outer host/effect tails
        return &material;
    } catch (...) { material.vtable_00 = base_table; throw; }
}
NativeMaterialStorage* clone_body(void* slot,
    const NativeMaterialStorage& source, NativeRenderActualOwners& owners,NativeMaterialStorageIssuer* issuer) {
    if (slot == &source) throw std::invalid_argument("material clone requires distinct fresh storage");
    auto& material = prepare(slot,issuer);
    try {
        initialize_common(material);
        material.retain_source_10d = 0;
        material.lighting_enabled_10c = 0;
        material.word_08 = source.word_08;
        material.source_owner_0c = source.source_owner_0c;
        material.retain_source_10d = source.retain_source_10d;
        if (material.retain_source_10d && material.source_owner_0c)
            retain_actual(material.source_owner_0c);
        // Native initial signed compare skips negative counts; only a positive
        // count enters this valid-slot interface. Reload the source end each time.
        for (std::int32_t index = 0; index < source.texture_count_34; ++index) {
            if (index >= 9) throw std::logic_error("native material clone texture extent exceeds9");
            void* const texture = source.textures_10[static_cast<std::size_t>(index)];
            if (static_cast<std::uint32_t>(material.texture_count_34) <= static_cast<std::uint32_t>(index))
                material.texture_count_34 = static_cast<std::int16_t>(index + 1);
            assign_actual(material.textures_10[static_cast<std::size_t>(index)], texture, owners,issuer,static_cast<std::size_t>(index)+1);
        }
        material.lighting_enabled_10c = 1;
        std::memcpy(material.lighting_38.data(), source.lighting_38.data(), sizeof(material.lighting_38));
        assign_actual(material.effect_7c, source.effect_7c, owners,issuer,0);
        if constexpr (kWireGoal4Chain621Bound) {
            // 00B18B60 copies source+104 after effect assignment. Preserve
            // all DWORD bits, including the FFFFFFFF default, across the
            // setter's signed source interface; no numeric conversion.
            std::int32_t alpha_ref;
            std::memcpy(&alpha_ref, &source.word_104, sizeof(alpha_ref));
            scene_renderer::set_material_alpha_ref_00b17310(&material, alpha_ref);
        } else {
            material.word_104 = source.word_104;
        }
        material.word_108 = source.word_108;
        if(issuer)issuer->returned(material); // exact Native normal continuation, before outer host/effect tails
        return &material;
    } catch (...) { material.vtable_00 = base_table; throw; }
}

} // anonymous shared constructor kernels

NativeMaterialStorage* initialize_native_material_00b18900(void* slot,
    void* effect,NativeRenderActualOwners& owners){return initialize_body(slot,effect,owners,nullptr);}
NativeMaterialStorage* clone_native_material_00b18b60(void* slot,
    const NativeMaterialStorage& source,NativeRenderActualOwners& owners){return clone_body(slot,source,owners,nullptr);}
NativeMaterialStorage* NativeMaterialStorageAccess::initialize(void* slot,void* effect,
    NativeRenderActualOwners& owners,NativeMaterialFrame& frame){
    try{frame.native_constructor_entered(slot);NativeMaterialStorageIssuer issuer(frame);
        return initialize_body(slot,effect,owners,&issuer);}
    catch(...){frame.source_constructor_escaped();throw;}
}
NativeMaterialStorage* NativeMaterialStorageAccess::clone(void* slot,const NativeMaterialStorage& source,
    NativeRenderActualOwners& owners,NativeMaterialFrame& frame){
    try{frame.native_constructor_entered(slot);NativeMaterialStorageIssuer issuer(frame);
        return clone_body(slot,source,owners,&issuer);}
    catch(...){frame.source_constructor_escaped();throw;}
}

namespace {
void set_parameter_owner_body(NativeMaterialStorage& material, void* actual_owner,
    std::uint8_t retain_flag, NativeRenderActualOwners& owners,
    NativeRenderActualOwnerReleaseObservation* observation) {
    if (material.retain_source_10d)
        release_then_clear(material.source_owner_0c, owners,nullptr,0,observation);
    material.source_owner_0c = actual_owner;
    material.retain_source_10d = retain_flag;
    if (retain_flag && actual_owner) retain_actual(actual_owner);
}
}
void set_native_material_parameter_owner_00b18a40(NativeMaterialStorage& material,
    void* actual_owner, std::uint8_t retain_flag, NativeRenderActualOwners& owners) {
    set_parameter_owner_body(material,actual_owner,retain_flag,owners,nullptr);
}
void NativeMaterialStorageAccess::set_parameter_owner(NativeMaterialStorage& material,
    void* actual_owner,std::uint8_t retain_flag,NativeRenderActualOwners& owners,
    NativeRenderActualOwnerReleaseObservation& observation) {
    set_parameter_owner_body(material,actual_owner,retain_flag,owners,&observation);
}

namespace {
void destroy_body(NativeMaterialStorage& material,
    NativeMaterialDestructionAccess& access,NativeMaterialStorageIssuer* issuer) {
    material.vtable_00 = material_table;
    const BaseCleanup base{material};
    release_then_clear(material.effect_7c, access.retained_owners,issuer,0);
    std::int32_t index = 0;
    while (index != texture_count(material)) {
        if (index >= 9) throw std::logic_error("material texture end moved behind its destruction cursor");
        release_then_clear(material.textures_10[static_cast<std::size_t>(index)], access.retained_owners,issuer,static_cast<std::size_t>(index)+1);
        ++index;
    }
    if (material.retain_source_10d)
        release_then_clear(material.source_owner_0c, access.retained_owners,issuer,1+std::tuple_size_v<decltype(NativeMaterialStorage::textures_10)>);
    for (index = 0; index < material.parameter_count_100; ++index) {
        if (index >= 32) throw std::logic_error("native material parameter extent exceeds32");
        void* const parameter = material.parameters_80[static_cast<std::size_t>(index)];
        if (parameter) {
            // The parameter begins with the actual8h native string header.
            destroy_native_string_header_0041dd20(parameter, access.parameter_names);
            access.parameter_slots.return_slot_00b193fa_fragment(parameter);
        }
    }
}
} // anonymous shared payload kernel
void destroy_native_material_00b192f0(NativeMaterialStorage& material,NativeMaterialDestructionAccess& access){
    destroy_body(material,access,nullptr);
}
void NativeMaterialStorageAccess::destroy(NativeMaterialStorage& material,NativeMaterialDestructionAccess& access,
    NativeMaterialFrame& frame){NativeMaterialStorageIssuer issuer(frame,true);destroy_body(material,access,&issuer);}
NativeMaterialStorage* delete_native_material_00b194b0(NativeMaterialStorage* material,
    NativeMaterialDestructionAccess& access, std::uint32_t flags) {
    destroy_native_material_00b192f0(*material, access);
    if (flags & 1) {
        material->~NativeMaterialStorage();
        access.material_slots.return_slot_00b17a80(material);
    }
    return material;
}
void* allocate_native_material_slot_00b18780(NativeMaterialSlotPool& pool) {
    return pool.allocate_00b18200();
}

NativeMaterialReference::NativeMaterialReference(NativeMaterialStorage& material,
    NativeMaterialDestructionAccess& access,const volatile std::uint32_t* table,NativeMaterialFrame& frame) noexcept
    :RenderCommandReference(material.references_04),ordinary_frame_(&frame),storage_(material),
     access_(access),vtable_(table),disposal_{nullptr,nullptr}{}
void NativeMaterialReference::validate_ordinary_target() const {
    ordinary_frame_->require_owned(storage_.vtable_00==material_table&&vtable_&&
        vtable_[0]==0x00bd30e0&&vtable_[1]==0x00b194b0,
        "ordinary Material requires its actual current deleting target");
}
NativeMaterialReference::NativeMaterialReference(NativeMaterialStorage& material,
    NativeMaterialDestructionAccess& access, const volatile std::uint32_t* table,
    NativeMaterialCompanionDisposal disposal)
    : RenderCommandReference(material.references_04), storage_(material), access_(access),
      vtable_(table), disposal_(disposal) {
    if (!disposal.retire || material.references_04.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("material reference requires live actual storage and explicit companion retirement");
    require_current_profile();
}
NativeMaterialReference::~NativeMaterialReference() {
    if(ordinary_frame_)return; // genuine Frame/owner close, never callback self-erasure
    if (phase_ != Phase::retired) std::terminate();
}
void NativeMaterialReference::require_current_profile() const noexcept {
    if (storage_.vtable_00 != material_table || !vtable_ ||
        vtable_[0] != 0x00bd30e0 || vtable_[1] != 0x00b194b0) std::terminate();
}
void NativeMaterialReference::release_zero_references() {
    if(ordinary_frame_){ordinary_frame_->ordinary_virtual_zero(*this);return;}
    release_legacy_zero_references();
}
void NativeMaterialReference::release_legacy_zero_references() noexcept {
    if (phase_ != Phase::bound) std::terminate();
    require_current_profile();
    phase_ = Phase::destroying;
    const auto disposal = disposal_;
    try { delete_native_material_00b194b0(&storage_, access_, 1); }
    catch (...) { std::terminate(); }
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}
} // namespace bsp
