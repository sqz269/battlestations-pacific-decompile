#pragma once
#include "bsp/gui_native_geometry.hpp"
#include "bsp/native_mesh_section_layout_storage.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_hardware_layout_factory.hpp"
#include "bsp/native_logical_vertex_owner.hpp"
#include "bsp/native_vertex_declaration_loading.hpp"
#include <memory>

namespace bsp {
class GuiTextNativeLayoutReference;
struct GuiTextNativeLayoutDisposal {
    void* context;
    void (*retire)(void*, GuiTextNativeLayoutReference&) noexcept;
};
// One host companion borrowing the actual hardware owner's +04. No new native
// cache/count. Current D62AF4/BD30E0/B60770 dispatch returns the SAME pool slot
// before retiring the canonical binding. Contexts outlive every reference.
class GuiTextNativeLayoutReference final : public RenderCommandReference {
public:
    GuiTextNativeLayoutReference(void*, NativeHardwareLayoutOwnerContext&,
        const volatile std::uint32_t* profile_00d62af4, GuiTextNativeLayoutDisposal);
    ~GuiTextNativeLayoutReference() override;
    void* storage() const noexcept { return storage_; }
    bool matches_context(const NativeHardwareLayoutOwnerContext&,
        const volatile std::uint32_t*) const noexcept;
    void release_zero_references() noexcept override;
    bool matches_raw_context(const NativeHardwareLayoutOwnerContext&,
        const volatile std::uint32_t*) const;
    // Real B60770; this is an ordinary raw scalar, not the noexcept terminal.
    // Retire metadata only after actual normal destruction/pool-return path.
    void* delete_raw_scalar_00b60770(std::uint32_t actual_flags);

private:
    void* storage_;
    NativeHardwareLayoutOwnerContext& context_;
    const volatile std::uint32_t* profile_;
    GuiTextNativeLayoutDisposal disposal_;
    enum class Phase { bound, destroying, retired, failed };
    Phase phase_{Phase::bound};
};
enum class GuiTextNativeLayoutPhase { empty, factory, registration, transferred };
struct GuiTextNativeLayoutAcquired {
    void* creator{};
    GuiTextNativeLayoutReference* companion{};
    bool registered{};
    bool reused{};
    GuiTextNativeLayoutPhase phase{GuiTextNativeLayoutPhase::empty};
};
// Concrete B48CE0 stream+24 and B2F710 renderer+40 service for B865A0.
// Numeric tables identify recovered bodies; no replacement callable vtable.
// Uses the geometry domain's SAME registry and the existing actual tree/pools.
// Acquired bookkeeping survives insertion/metadata failures without rollback;
// an interrupted operation is never replayed. Successful return transfers the
// factory's single reference directly to the caller's section+50 publication.
class GuiTextNativeRawLayoutInvocation;
class GuiTextNativeLayoutServices final : public NativeMeshSectionLayoutServices {
public:
    GuiTextNativeLayoutServices(GuiNativeGeometryOwners&,
        NativeLogicalVertexOwnerContext&, NativeVertexDeclarationLoadingContext&,
        NativeHardwareLayoutConstructContext&,
        const volatile std::uint32_t* profile_00d62af4);
    ~GuiTextNativeLayoutServices() override;
    void* current_stream_descriptor_virtual24(void*) override;
    void* current_renderer_layout_virtual40(const NativeMeshSectionLayoutKey&) override;
    const GuiTextNativeLayoutAcquired& acquired() const noexcept;
    // Successful transfer can leave canonical companions owned by this service.
    // Retire their actual native references before draining borrowed providers.
    bool has_live_companions() const noexcept;
private:
    friend class GuiTextNativeRawLayoutInvocation;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
// Concrete raw mutable-key layout provider, one caller-retained invocation.
// Owns ONLY Source observations, no native owner/count or cleanup. Construct
// after the service and before parent Native entry; retain through unknown
// escape and same-domain retirement. The expected key is an address borrow,
// not an early descriptor/count snapshot. Imports remain live home references.
//
// Admitted genuine Source-created families are D61D6C/B48CE0/B4BF10 logical
// streams, D62AF4/B60770 hardware layouts and D5F0A8/B2F710 renderer. Caller
// proves actual C++ birth/generation/custody, not profile/count alone. Normal
// current_table only maps these SAME immutable real views; no allocator,
// callback, native effect or reentry. OTHER profiles/targets remain outside
// this provider's qualified domain and fail Source admission; none is invoked
// as a numeric host codeword or silently substituted by a known body.
class GuiTextNativeRawLayoutInvocation final : public NativeMeshSectionLayoutStorageBindings,
    public NativeRefCountedDeleteCalls, public NativeMeshSectionLayoutStorageFactoryCalls {
public:
    GuiTextNativeRawLayoutInvocation(GuiTextNativeLayoutServices&,
        const void* actual_retained_key_home,
        NativeMeshSectionLayoutReferenceOperation const volatile& actual_increment_00ce221c,
        NativeMeshSectionLayoutReferenceOperation const volatile& actual_decrement_00ce2220);
    GuiTextNativeRawLayoutInvocation(const GuiTextNativeRawLayoutInvocation&) = delete;
    GuiTextNativeRawLayoutInvocation& operator=(const GuiTextNativeRawLayoutInvocation&) = delete;
    GuiTextNativeRawLayoutInvocation(GuiTextNativeRawLayoutInvocation&&) = delete;
    GuiTextNativeRawLayoutInvocation& operator=(GuiTextNativeRawLayoutInvocation&&) = delete;
    const NativeMeshSectionLayoutStorageContext& context() const { return context_; }
    const GuiTextNativeLayoutAcquired& acquired() const { return acquisition_; }
    const NativeHardwareLayoutFactoryObservation& factory_observation() const { return observation_; }
    bool source_failed() const { return failed_ || observation_.source_failed(); }
    bool source_factory_complete() const { return factory_complete_; }
    const volatile std::uint32_t* current_table(void*, std::uint32_t) override;
    void other_reference_current00(void*, std::uint32_t, std::uint32_t) override;
    void* other_stream_descriptor_current24(void*, std::uint32_t, std::uint32_t) override;
    void* other_renderer_layout_current40(void*, std::uint32_t, std::uint32_t,
        const void*, void**) override;
    void delete_vslot04(void*, std::uint32_t captured_profile, std::uint32_t actual_flags) override;
    void* create_layout_00b2f710(void*, std::uint32_t, const void*,
        NativeHardwareLayoutConstructContext&, void**) override;
private:
    void begin_effect();
    GuiTextNativeLayoutServices& service_;
    const void* const actual_key_home_;
    NativeMeshSectionLayoutStorageContext context_;
    GuiTextNativeLayoutAcquired acquisition_;
    NativeHardwareLayoutFactoryObservation observation_;
    bool active_{}, failed_{}, factory_started_{}, factory_complete_{};
    std::exception_ptr failure_;
};

} // namespace bsp
