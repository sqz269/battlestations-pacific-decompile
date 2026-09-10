#include "bsp/material_parameter_bindings.hpp"
#include <cstring>
#include <limits>
#include <utility>

namespace bsp {
namespace {
bool valid_name(std::string_view name) noexcept {
    return name.size() <= static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)())
        && name.find('\0') == std::string_view::npos;
}
bool same_name(const std::string& first, const std::string& second) noexcept {
    return first.size() == second.size() && _stricmp(first.c_str(), second.c_str()) == 0;
}
std::int32_t signed_word(std::uint32_t value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
bool valid_selectors(const MaterialParameterSelectors& selectors, std::string& error) {
    constexpr auto maximum = static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)()) / 0x20;
    for (const auto& selector : selectors) if (selector) {
        for (const auto* stage : {&selector->vertex, &selector->pixel}) {
            if (stage->material_constants.size() > maximum) {
                error = "Material constant metadata exceeds the native signed record span.";
                return false;
            }
            for (const auto& constant : stage->material_constants) if (!valid_name(constant.name)) {
                error = "Material constant names require coherent strings without embedded NUL.";
                return false;
            }
        }
    }
    return true;
}
std::int32_t first_register(const ShaderConstantBindings& stage, const std::string& name) noexcept {
    for (const auto& constant : stage.material_constants)
        if (same_name(constant.name, name)) return signed_word(constant.register_index);
    return -1;
}
bool valid_source(const MaterialParameterSource& source) noexcept {
    constexpr auto maximum_words = (std::numeric_limits<std::uint32_t>::max)() / sizeof(std::uint32_t);
    if (source.word_count > maximum_words) return false;
    const std::size_t required = source.matrix ? 16 : source.word_count;
    return required <= source.available_words && (required == 0 || source.words);
}
bool overlaps(const void* source, std::size_t words, const std::vector<float>& output) noexcept {
    if (words == 0 || output.empty()) return false;
    const auto start = reinterpret_cast<std::uintptr_t>(source);
    const auto target = reinterpret_cast<std::uintptr_t>(output.data());
    const auto bytes = words * sizeof(std::uint32_t);
    const auto output_bytes = output.size() * sizeof(float);
    // Subtraction avoids end-address overflow on the native32-bit target.
    return start <= target ? target - start < bytes : start - target < output_bytes;
}
}

struct MaterialParameterBindings::Impl {
    std::shared_ptr<const void> shader;
    MaterialParameterSelectors selectors;
    std::shared_ptr<const MaterialParameterSelectors> shared_selectors;
    std::vector<std::unique_ptr<MaterialParameterRecord>> parameters;
    const MaterialParameterSelectors& metadata() const noexcept {
        return shared_selectors ? *shared_selectors : selectors;
    }
};

MaterialParameterBindings::MaterialParameterBindings() : impl_(std::make_unique<Impl>()) {}
MaterialParameterBindings::~MaterialParameterBindings() = default;
MaterialParameterBindings::MaterialParameterBindings(MaterialParameterBindings&&) noexcept = default;
MaterialParameterBindings& MaterialParameterBindings::operator=(MaterialParameterBindings&&) noexcept = default;

bool MaterialParameterBindings::set_shader_00b19210_fragment(std::shared_ptr<const void> shader,
    const MaterialParameterSelectors& selectors, std::string& error) {
    error.clear();
    if (!impl_) { error = "Material parameter table was moved from."; return false; }
    if (!valid_selectors(selectors, error)) return false;
    auto copied = selectors;
    impl_->parameters.clear(); // Also required when shader.get() did not change.
    impl_->selectors = std::move(copied);
    impl_->shared_selectors.reset();
    impl_->shader = std::move(shader);
    return true;
}
bool MaterialParameterBindings::set_shared_shader_00b19210_fragment(
    std::shared_ptr<const void> shader,
    std::shared_ptr<const MaterialParameterSelectors> selectors, std::string& error) {
    error.clear();
    if (!impl_ || !selectors) { error = "Shared material metadata is missing or table was moved from."; return false; }
    if (!valid_selectors(*selectors, error)) return false;
    impl_->parameters.clear();
    impl_->shared_selectors = std::move(selectors);
    impl_->shader = std::move(shader);
    return true;
}
bool MaterialParameterBindings::clone_shader_binding_empty_00b18b60_fragment(
    const MaterialParameterBindings& source, std::string& error) {
    if (!source.impl_) { error = "Source material parameter binding was moved from."; return false; }
    if (source.impl_->shared_selectors)
        return set_shared_shader_00b19210_fragment(source.impl_->shader,
            source.impl_->shared_selectors, error);
    return set_shader_00b19210_fragment(source.impl_->shader, source.impl_->selectors, error);
}
bool MaterialParameterBindings::replace_selectors(const MaterialParameterSelectors& selectors,
    std::string& error) {
    error.clear();
    if (!impl_) { error = "Material parameter table was moved from."; return false; }
    if (!valid_selectors(selectors, error)) return false;
    auto copied = selectors;
    impl_->selectors = std::move(copied);
    impl_->shared_selectors.reset();
    return true;
}

MaterialParameterRegistration MaterialParameterBindings::register_words_00b17e10_00b44d60(
    std::string_view name, MaterialParameterSource source, std::string& error) {
    error.clear();
    if (!impl_) { error = "Material parameter table was moved from."; return {}; }
    // Native00b17e10 checks shader null before reading the name or source.
    if (!impl_->shader) return {MaterialParameterRegistrationStatus::no_match, nullptr};
    if (!valid_name(name)) {
        error = "Material parameter names require coherent strings without embedded NUL.";
        return {};
    }
    std::string owned_name(name);
    MaterialParameterRecord* existing = nullptr;
    for (const auto& parameter : impl_->parameters) if (same_name(parameter->name, owned_name)) {
        existing = parameter.get();
        break;
    }
    auto next = existing ? *existing : MaterialParameterRecord{};
    bool found = false;
    const auto& metadata = impl_->metadata();
    for (std::size_t i = 0; i < metadata.size(); ++i) if (metadata[i]) {
        const auto& selector = *metadata[i];
        const auto vertex = first_register(selector.vertex, owned_name);
        const auto pixel = first_register(selector.pixel, owned_name);
        if (vertex < 0 && pixel < 0) continue; // Preserve both old selector words.
        found = true;
        next.vertex_registers[i] = vertex;
        next.pixel_registers[i] = pixel;
    }
    if (!found) return {MaterialParameterRegistrationStatus::no_match, existing};
    if (!valid_source(source)) {
        error = "Material parameter source is null, too short, or its byte count would overflow.";
        return {};
    }
    if (!existing && impl_->parameters.size() == material_parameter_capacity) {
        error = "Material parameter table exceeds its native32-record capacity.";
        return {};
    }
    next.name = std::move(owned_name);
    next.source = source;
    if (existing) {
        *existing = std::move(next);
        return {MaterialParameterRegistrationStatus::bound, existing};
    }
    auto created = std::make_unique<MaterialParameterRecord>(std::move(next));
    const auto* published = created.get();
    impl_->parameters.push_back(std::move(created));
    return {MaterialParameterRegistrationStatus::bound, published};
}

MaterialConstantPackStatus MaterialParameterBindings::pack_00b423c5(std::size_t selector,
    std::vector<float>& vertex_words, std::vector<float>& pixel_words) const {
    if (selector >= material_parameter_selector_count)
        return MaterialConstantPackStatus::selector_out_of_range;
    if (&vertex_words == &pixel_words) return MaterialConstantPackStatus::shared_output_buffer;
    if (!impl_) return MaterialConstantPackStatus::selector_out_of_range;
    std::vector<VertexConstantShape> shapes;
    if (impl_->metadata()[selector]) {
        const auto& metadata = impl_->metadata()[selector]->vertex.material_constants;
        shapes.reserve(metadata.size());
        for (const auto& constant : metadata)
            // Native00b5bc60 stores reflected RegisterCount at record+4h.
            shapes.push_back({signed_word(constant.register_index), signed_word(constant.register_count)});
    }
    std::vector<MaterialConstantParameter> parameters;
    parameters.reserve(impl_->parameters.size());
    for (const auto& record : impl_->parameters) {
        MaterialConstantParameter parameter;
        parameter.matrix = record->source.matrix;
        parameter.vertex_registers.assign(record->vertex_registers.begin(), record->vertex_registers.end());
        parameter.pixel_registers.assign(record->pixel_registers.begin(), record->pixel_registers.end());
        const auto vertex = record->vertex_registers[selector];
        const auto pixel = record->pixel_registers[selector];
        bool read_source = vertex >= 0 || pixel >= 0;
        if (parameter.matrix && pixel < 0) {
            std::int32_t rows = 0;
            for (const auto& shape : shapes) if (shape.start_register == vertex) rows = shape.row_count;
            read_source = vertex >= 0 && rows >= 2 && rows <= 4;
        }
        if (read_source) {
            const std::size_t words = parameter.matrix ? 16 : record->source.word_count;
            if (!valid_source(record->source)) return MaterialConstantPackStatus::source_too_short;
            // The existing status has no source-alias variant. Reject through
            // its shared-buffer status rather than normalize native overlap.
            if (overlaps(record->source.words, words, vertex_words) || overlaps(record->source.words, words, pixel_words))
                return MaterialConstantPackStatus::shared_output_buffer;
            parameter.source_words.resize(words);
            if (words) std::memcpy(parameter.source_words.data(), record->source.words, words * sizeof(std::uint32_t));
        }
        parameters.push_back(std::move(parameter));
    }
    return pack_material_parameter_constants_00b423c5(parameters, selector, shapes, vertex_words, pixel_words);
}

const void* MaterialParameterBindings::shader_identity() const noexcept { return impl_ ? impl_->shader.get() : nullptr; }
std::size_t MaterialParameterBindings::size() const noexcept { return impl_ ? impl_->parameters.size() : 0; }
const MaterialParameterRecord* MaterialParameterBindings::record(std::size_t index) const noexcept {
    return impl_ && index < impl_->parameters.size() ? impl_->parameters[index].get() : nullptr;
}
}
