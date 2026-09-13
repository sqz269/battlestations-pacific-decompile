#pragma once
#include "bsp/gui_native_geometry.hpp"
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
private:
    void* storage_;
    NativeHardwareLayoutOwnerContext& context_;
    const volatile std::uint32_t* profile_;
    GuiTextNativeLayoutDisposal disposal_;
    enum class Phase { bound, destroying, retired };
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
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp
