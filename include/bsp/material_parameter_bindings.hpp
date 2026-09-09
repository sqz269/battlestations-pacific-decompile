#pragma once
#include "bsp/material_constants.hpp"
#include "bsp/shader_reflection.hpp"
#include <array>
#include <memory>
#include <optional>
#include <string_view>

namespace bsp {
inline constexpr std::size_t material_parameter_selector_count = 14;
inline constexpr std::size_t material_parameter_capacity = 32;

struct MaterialParameterSelector {
    ShaderConstantBindings vertex;
    ShaderConstantBindings pixel;
};
using MaterialParameterSelectors = std::array<std::optional<MaterialParameterSelector>,
    material_parameter_selector_count>;

// Only names and register arrays are owned. Source memory is BORROWED and must
// remain readable/at a stable address until replaced, cleared or destroyed.
// Mutating its words between serialized pack calls supplies live values.
struct MaterialParameterSource {
    const void* words{};
    std::size_t available_words{};
    std::uint32_t word_count{}; // Native +Ch; not a float4 count.
    bool matrix{};
};
struct MaterialParameterRecord {
    std::string name;
    MaterialParameterSource source;
    std::array<std::int32_t, material_parameter_selector_count> vertex_registers;
    std::array<std::int32_t, material_parameter_selector_count> pixel_registers;
    MaterialParameterRecord() { vertex_registers.fill(-1); pixel_registers.fill(-1); }
};
enum class MaterialParameterRegistrationStatus { bound, no_match, unsupported };
struct MaterialParameterRegistration {
    MaterialParameterRegistrationStatus status{MaterialParameterRegistrationStatus::unsupported};
    // Stable across table growth/moves; invalidated by set_shader/destruction.
    // A no_match may return the unchanged existing record, as native EAX does.
    const MaterialParameterRecord* parameter{};
};

// New owning table projection of00b17e10/00b44d60; not a native material ABI.
// Calls are serialized. The retained token must own the actual shader/effect
// dependencies; this component performs no shader compilation or device calls.
class MaterialParameterBindings {
public:
    MaterialParameterBindings();
    ~MaterialParameterBindings();
    MaterialParameterBindings(MaterialParameterBindings&&) noexcept;
    MaterialParameterBindings& operator=(MaterialParameterBindings&&) noexcept;
    MaterialParameterBindings(const MaterialParameterBindings&) = delete;
    MaterialParameterBindings& operator=(const MaterialParameterBindings&) = delete;

    // Copies compiled metadata, retains identity, and ALWAYS clears records,
    // including for the same shader identity. Null identity disables binding.
    // Native effect+B4 dirty signaling belongs to its external effect owner.
    // Metadata guards fail atomically; allocation exceptions propagate.
    bool set_shader_00b19210_fragment(std::shared_ptr<const void> shader,
        const MaterialParameterSelectors& selectors, std::string& error);
    // Explicit host metadata refresh without shader assignment/record clearing.
    // Existing registrations keep their old selector indices until re-registered.
    bool replace_selectors(const MaterialParameterSelectors&, std::string& error);

    // First equal-length, case-insensitive match in both the existing table and
    // each compiled stage list. A selector is written only if either stage has
    // a nonnegative match; an absent other stage receives -1. Other selectors stay as
    // they were. No match leaves the existing record/source entirely unchanged.
    // Source guards apply only when a compiled match would use the registration.
    // Nonmatrices require word_count readable words; matrices require16.
    MaterialParameterRegistration register_words_00b17e10_00b44d60(
        std::string_view name, MaterialParameterSource source, std::string& error);

    // Snapshot selected borrowed words and delegate to the existing pack helper.
    // Source ranges must not overlap either output vector. Output sizes are
    // capacities; no clearing/resizing. Failures leave both outputs unchanged.
    // Matrix VS shape.row_count comes from reflected RegisterCount, not Rows.
    // Selector/output/source guards use existing pack status values.
    MaterialConstantPackStatus pack_00b423c5(std::size_t selector,
        std::vector<float>& vertex_words, std::vector<float>& pixel_words) const;

    const void* shader_identity() const noexcept;
    std::size_t size() const noexcept;
    const MaterialParameterRecord* record(std::size_t index) const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
