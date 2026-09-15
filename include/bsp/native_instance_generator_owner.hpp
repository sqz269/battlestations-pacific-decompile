#pragma once
#include "bsp/gui_text_native_renderer.hpp"
#include "bsp/gui_text_native_layout.hpp"
#include <memory>

namespace bsp {
struct NativeInstanceGeneratorProfiles {
    const volatile std::uint32_t* base_00d62190;
    const volatile std::uint32_t* generic_00d61bfc;
    const volatile std::uint32_t* building_00d61c1c;
    const volatile std::uint32_t* binding_00d619f8;
};
struct NativeInstanceGeneratorContext {
    GuiTextNativeRendererServices& graphics;
    GuiTextNativeLayoutServices& layouts;
    NativeInstanceGeneratorProfiles profiles;
    volatile std::uint32_t& binding_serial_0108fd30;
    const char* generic_declaration_00d61c08; // actual readable18-byte constant
    const char* building_declaration_00d61c28; // actual readable42-byte constant
    // Both concrete services MUST share the SAME current renderer publication,
    // declaration/string pools, hardware tree and canonical geometry registry.
    // They outlive all resulting generator, declaration and layout references.
};
struct NativeInstanceGeneratorAcquired {
    enum class Phase { empty, selection, allocation, name, declaration,
        stream_descriptor, layout, constructor_complete, generator_registration,
        binding_allocation, binding_registration, assignment, generator_release,
        section_publication, binding_release, complete };
    Phase phase{Phase::empty};
    std::uint32_t native_site{};
    bool attachment_started{}, derived_started{}, base_started{};
    bool base_complete{}, derived_complete{}, raw_generator_freed{};
    NativeString temporary_name;
    bool temporary_name_armed{}, temporary_name_returned{};
    GuiNativeDeclarationAcquired declaration;
    void* captured_declaration{}; // audit identity; no extra retain
    void* captured_layout{};
    void* raw_generator{};
    void* generator{}; // completed creator, until consumed by native release
    void* binding{};
    RenderCommandReference* generator_companion{};
    RenderCommandReference* binding_companion{};
    bool generator_registered{}, binding_registered{};
    // No destructor cleanup. Interrupted cache/layout/metadata continuations
    // and native effects remain in their existing services and this frame.
};

// Host companions only; bind into graphics.streams.geometry's SAME canonical
// registry and borrow raw+04. No duplicate owner count or private resolver.
// Registration failure retains the completed creator and any unbound companion.
class NativeInstanceGeneratorOwners final {
public:
    explicit NativeInstanceGeneratorOwners(NativeInstanceGeneratorContext&);
    ~NativeInstanceGeneratorOwners();
    NativeInstanceGeneratorOwners(const NativeInstanceGeneratorOwners&) = delete;
    NativeInstanceGeneratorOwners& operator=(const NativeInstanceGeneratorOwners&) = delete;
    void register_generator(NativeInstanceGeneratorAcquired&);
    void register_binding(NativeInstanceGeneratorAcquired&);
    NativeInstanceGeneratorContext& context() noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Complete B55B20[189], ECX raw1Ch object; stack section/ignored/name; RET0C.
// Actual B317E0 declaration lookup, first section stream's B48CE0 descriptor,
// actual B2F710 layout with section-first/instance-second key. +18 starts null.
// Constructor EH releases ONLY embedded name and base, not acquired resources.
void* construct_native_instance_generator_declarations_00b55b20(void* actual,
    void* actual_section, void* ignored, const void* actual_name,
    NativeInstanceGeneratorContext&, NativeInstanceGeneratorAcquired&);
// Full 182-byte derived bodies. ECX raw; stack section/ignored; EAX raw; RET8.
// Temporary name length17/41; copy current length+1, base construction, return
// temporary then publish D61BFC/D61C1C. Preserve distinct EH states0/1/2.
void* construct_native_generic_instance_generator_00b44fd0(void*, void*, void*,
    NativeInstanceGeneratorContext&, NativeInstanceGeneratorAcquired&);
void* construct_native_building_instance_generator_00b450d0(void*, void*, void*,
    NativeInstanceGeneratorContext&, NativeInstanceGeneratorAcquired&);
// B55BE0[205]: publish D62190; release then clear +18,+10,+14; release name+8;
// stamp CEB130 on success or unwind. Never retry failed resource cleanup.
void destroy_native_instance_generator_00b55be0(void*, NativeInstanceGeneratorContext&);
// Complete scalar deleting bodies; bit0 frees raw allocation AFTER destruction;
// all return original address bits, including after free. ECX raw; RET4.
void* delete_native_instance_generator_00b55cb0(void*, std::uint32_t, NativeInstanceGeneratorContext&);
void* delete_native_generic_instance_generator_00b450a0(void*, std::uint32_t, NativeInstanceGeneratorContext&);
void* delete_native_building_instance_generator_00b451a0(void*, std::uint32_t, NativeInstanceGeneratorContext&);
// B41710[112] releases then clears +C; only base is protected by its EH state.
void destroy_native_instance_generator_binding_00b41710(void*, NativeInstanceGeneratorContext&);
void* delete_native_instance_generator_binding_00b417c0(void*, std::uint32_t, NativeInstanceGeneratorContext&);
// B41780[59], ECX binding; stack generator; RET4. Identity skip; publish,
// retain new, release captured old through current canonical terminal.
void set_native_instance_generator_binding_00b41780(void*, void*, NativeRenderActualOwners&);

// Full B451D0[339]: ECX effect; stacked section/ignored; RET8. Re-read effect+C4
// for the generic comparison. Empty/unrecognized leaves current binding alone.
// Selected 1Ch constructor has raw-free-only outer unwind. After construction,
// no creator rollback: allocate10h binding, set generator, consume generator,
// publish through existing B417E0, consume binding. Metadata adds no retain.
void attach_native_material_instance_generator_00b451d0(void* actual_effect,
    void* actual_section, void* ignored, NativeInstanceGeneratorOwners&,
    NativeInstanceGeneratorAcquired&);
// B85610[30]: ECX section; stack ignored; RET4. If section+20 material and
// material+7C effect are nonnull, call the full attachment above.
void finalize_native_mesh_section_generator_00b85610(void* actual_section,
    void* ignored, NativeInstanceGeneratorOwners&, NativeInstanceGeneratorAcquired&);

// Original bytes, ABI and FH3 evidence: docs/NATIVE_INSTANCE_GENERATOR_CK.md.
// Explicit-context Win32 source interfaces; no binary ABI/SEH/gameplay claim.
} // namespace bsp
