#include "bsp/native_compiled_shader_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <limits>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Record = NativeCompiledShaderConstantStorage;
using Array = NativeCompiledShaderConstants;
using Operation = NativeCompiledShaderArrayOperation;
void require_array(const Array& array) {
    if (array.count_04 < 0 || array.capacity_08 < array.count_04
        || array.capacity_08 > (std::numeric_limits<std::int32_t>::max)() / 0x20
        || (array.capacity_08 && !array.data_00))
        throw std::logic_error("compiled shader constant array is outside the readable native domain");
}
Record* row(Record* data, std::int32_t index) noexcept {
    return reinterpret_cast<Record*>(reinterpret_cast<std::uintptr_t>(data)
        + static_cast<std::uint32_t>(index) * 0x20u);
}
void begin(Operation& operation, Array& array, std::int32_t requested) {
    if (operation.phase != Operation::Phase::idle)
        throw std::logic_error("compiled shader array operation is one-shot");
    require_array(array);
    operation.array = &array;
    operation.requested = requested;
    operation.phase = Operation::Phase::running;
}
void reserve(Array& array, std::int32_t requested, NativeStringStorage& strings,
    Operation& operation) {
    if (requested < 0x10) requested = 0x10;
    if (array.capacity_08 >= requested) return;
    // The native SHL32 wraps. This host interface accepts only a representable,
    // readable allocation; it does not reinterpret overflow as valid storage.
    if (requested > (std::numeric_limits<std::int32_t>::max)() / 0x20)
        throw std::length_error("compiled shader constant allocation exceeds the native valid domain");
    operation.step = Operation::Step::allocation;
    operation.allocated_capacity = requested;
    const auto bytes = static_cast<std::uint32_t>(requested) * 0x20u;
    auto* const replacement = static_cast<Record*>(singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes}));
    operation.unpublished_data = replacement;
    operation.step = Operation::Step::copying;
    for (std::int32_t index = 0; index < array.count_04; ++index) {
        if (index >= requested)
            throw std::logic_error("compiled shader callback exceeded the acquired array extent");
        operation.current_record = row(replacement, index);
        if (operation.current_record)
            copy_native_compiled_shader_constant_00b38310(operation.current_record,
                *row(array.data_00, index), strings);
        operation.copied_rows = index + 1;
        operation.current_record = nullptr;
    }
    operation.step = Operation::Step::releasing_old;
    for (std::int32_t index = 0; index < array.count_04; ++index) {
        destroy_native_string_header_0041dd20(&row(array.data_00, index)->name_length_14, strings);
        operation.released_rows = index + 1;
    }
    singleton_lifetime_free(array.data_00);
    operation.step = Operation::Step::publication;
    array.data_00 = replacement;
    array.capacity_08 = requested;
    operation.unpublished_data = nullptr;
}
void shrink(Array& array, std::int32_t requested, NativeStringStorage& strings,
    Operation& operation) noexcept {
    operation.step = Operation::Step::shrinking;
    while (requested < array.count_04) {
        --array.count_04;
        destroy_native_string_header_0041dd20(
            &row(array.data_00, array.count_04)->name_length_14, strings);
        ++operation.released_rows;
    }
}
} // namespace

NativeCompiledShaderArrayOperation::~NativeCompiledShaderArrayOperation() {
    if (phase == Phase::running || phase == Phase::failed) std::terminate();
}
void NativeCompiledShaderArrayOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase != Phase::failed || unpublished_data || current_record) std::terminate();
    phase = Phase::diagnostic_retired;
}
NativeCompiledShaderStorage* initialize_native_compiled_shader_00b3b3c0_fragment(void* raw) {
    if (!raw || reinterpret_cast<std::uintptr_t>(raw) % alignof(NativeCompiledShaderStorage))
        throw std::invalid_argument("compiled shader initialization requires aligned raw88h storage");
    auto* const shader = ::new(raw) NativeCompiledShaderStorage;
    shader->vtable_00 = 0x00ceb130;
    shader->references_04.store(1, std::memory_order_relaxed);
    shader->vtable_00 = 0x00d61810;
    shader->byte_74 = 0;
    shader->constants_78.data_00 = nullptr;
    shader->constants_78.count_04 = 0;
    shader->constants_78.capacity_08 = 0;
    shader->word_84 = 0;
    std::memset(shader->initialized_ff_08.data(), 0xff, 0x36);
    return shader;
}
Record* initialize_native_compiled_shader_constant_00b5bb40(void* raw, NativeStringStorage& strings) {
    auto* const record = ::new(raw) Record;
    record->name_length_14 = 0;
    record->name_data_18 = nullptr;
    resize_native_string_header_0041dd40(&record->name_length_14, strings, 0, false);
    if (record->name_data_18)
        std::memcpy(record->name_data_18, "", record->name_length_14);
    record->word_1c = 0x37;
    return record;
}
Record* copy_native_compiled_shader_constant_00b38310(void* raw, const Record& source,
    NativeStringStorage& strings) {
    auto* const destination = ::new(raw) Record;
    for (std::size_t index = 0; index < destination->words_00.size(); ++index)
        destination->words_00[index] = source.words_00[index];
    destination->name_length_14 = 0;
    destination->name_data_18 = nullptr;
    if (&destination->name_length_14 != &source.name_length_14) {
        resize_native_string_header_0041dd40(&destination->name_length_14, strings,
            source.name_length_14, true);
        if (source.name_length_14 != 0)
            std::memcpy(destination->name_data_18, source.name_data_18, destination->name_length_14);
    }
    destination->word_1c = source.word_1c;
    return destination;
}
void reserve_native_compiled_shader_constants_00b38390(Array& array, std::int32_t requested,
    NativeStringStorage& strings, Operation& operation) {
    begin(operation, array, requested);
    try {
        reserve(array, requested, strings, operation);
        operation.phase = Operation::Phase::complete;
    } catch (...) { operation.phase = Operation::Phase::failed; throw; }
}
void resize_native_compiled_shader_constants_00b38490(Array& array, std::int32_t requested,
    NativeStringStorage& strings, Operation& operation) {
    if (requested < 0) throw std::invalid_argument("negative compiled shader constant count");
    begin(operation, array, requested);
    try {
        if (array.capacity_08 < requested) reserve(array, requested, strings, operation);
        operation.step = Operation::Step::default_rows;
        for (std::int32_t index = array.count_04; index < requested; ++index) {
            operation.current_record = row(array.data_00, index);
            if (operation.current_record)
                initialize_native_compiled_shader_constant_00b5bb40(operation.current_record, strings);
            ++operation.initialized_rows;
            operation.current_record = nullptr;
        }
        shrink(array, requested, strings, operation);
        array.count_04 = requested;
        operation.phase = Operation::Phase::complete;
    } catch (...) { operation.phase = Operation::Phase::failed; throw; }
}
void append_native_compiled_shader_constant_00b3a660(Array& array, const Record& source,
    NativeStringStorage& strings, Operation& operation) {
    begin(operation, array, 0);
    operation.requested = array.count_04 + 1;
    operation.append_source = &source;
    try {
        if (array.count_04 == array.capacity_08) reserve(array, array.capacity_08 + 8, strings, operation);
        operation.step = Operation::Step::append_row;
        operation.current_record = row(array.data_00, array.count_04);
        if (operation.current_record)
            copy_native_compiled_shader_constant_00b38310(operation.current_record, source, strings);
        ++array.count_04;
        operation.initialized_rows = 1;
        operation.current_record = nullptr;
        operation.phase = Operation::Phase::complete;
    } catch (...) { operation.phase = Operation::Phase::failed; throw; }
}
void destroy_native_compiled_shader_00b3b1e0(NativeCompiledShaderStorage& shader,
    NativeStringStorage& strings) {
    Operation operation;
    resize_native_compiled_shader_constants_00b38490(shader.constants_78, 0, strings, operation);
    singleton_lifetime_free(shader.constants_78.data_00);
    destroy_native_ref_counted_base_00bd30f0(&shader);
}
NativeCompiledShaderStorage* delete_native_compiled_shader_00b3b260(NativeCompiledShaderStorage* shader,
    NativeStringStorage& strings, std::uint32_t flags) {
    destroy_native_compiled_shader_00b3b1e0(*shader, strings);
    if (flags & 1) singleton_lifetime_free(shader);
    return shader;
}
NativeCompiledShaderReference::NativeCompiledShaderReference(NativeCompiledShaderStorage& shader,
    NativeStringStorage& strings, const volatile std::uint32_t* profile,
    NativeCompiledShaderCompanionDisposal disposal)
    : RenderCommandReference(shader.references_04), storage_(shader), strings_(strings),
      profile_(profile), disposal_(disposal) {
    if (!disposal.retire || shader.references_04.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("compiled shader companion needs a live raw owner and retirement");
    require_current_profile();
}
NativeCompiledShaderReference::~NativeCompiledShaderReference() {
    if (phase_ != Phase::retired) std::terminate();
}
void NativeCompiledShaderReference::require_current_profile() const {
    if (storage_.vtable_00 != 0x00d61810 || !profile_
        || profile_[0] != 0x00bd30e0 || profile_[1] != 0x00b3b260)
        throw std::logic_error("compiled shader companion requires the actual D61810 lifetime profile");
}
NativeCompiledShaderStorage* NativeCompiledShaderReference::scalar_delete(std::uint32_t flags) {
    if (phase_ != Phase::bound) throw std::logic_error("compiled shader lifetime is already terminal");
    require_current_profile();
    phase_ = Phase::destroying;
    auto* const raw = &storage_;
    const auto disposal = disposal_;
    delete_native_compiled_shader_00b3b260(raw, strings_, flags);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
    return raw;
}
void NativeCompiledShaderReference::delete_vslot04(void* raw, std::uint32_t profile,
    std::uint32_t flags) {
    if (raw != &storage_ || profile != 0x00d61810)
        throw std::logic_error("compiled shader terminal changed its actual identity");
    scalar_delete(flags);
}
void NativeCompiledShaderReference::release_zero_references() noexcept {
    if (storage_.references_04.load(std::memory_order_relaxed) != 0) std::terminate();
    try { invoke_native_ref_counted_delete_00bd30e0(&storage_, *this); }
    catch (...) { std::terminate(); }
}
} // namespace bsp
