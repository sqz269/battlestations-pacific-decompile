#include "bsp/native_compiled_shader_reflection.hpp"
#include <d3dx9shader.h>
#include <array>
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Record = NativeCompiledShaderConstantStorage;
template<class T> T current(const void* raw, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const std::byte*>(raw) + offset);
}
template<class T> void write(void* raw, std::size_t offset, T value) noexcept {
    *reinterpret_cast<volatile T*>(static_cast<std::byte*>(raw) + offset) = value;
}
void require(bool condition, const char* reason) {
    if (!condition) throw std::logic_error(reason);
}
Record* registry_row(void* registry, std::uint32_t ordinal) noexcept {
    return reinterpret_cast<Record*>(current<std::uintptr_t>(registry, 4) + ordinal * 0x20u);
}
void construct_name(NativeString& name, const char* text, NativeStringStorage& strings) {
    ::new(&name) NativeString;
    const auto length = static_cast<std::uint32_t>(std::strlen(text));
    resize_native_string_header_0041dd40(&name, strings, length, true);
    if (name.data()) std::memcpy(name.data(), text, name.length() + 1u);
}
std::int32_t signed_word(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}
} // namespace

NativeD3dx9ShaderConstantTableImport::NativeD3dx9ShaderConstantTableImport(HMODULE module) {
    require(module != nullptr, "shader reflection requires the actual loaded d3dx9_40 module");
    const FARPROC address = GetProcAddress(module, "D3DXGetShaderConstantTable");
    require(address != nullptr, "actual D3DXGetShaderConstantTable import is unavailable");
    static_assert(sizeof(address) == sizeof(function_));
    std::memcpy(&function_, &address, sizeof(function_));
}
HRESULT NativeD3dx9ShaderConstantTableImport::acquire(const std::uint32_t* bytecode,
    ID3DXConstantTable** output) const {
    return function_(reinterpret_cast<const DWORD*>(bytecode), output);
}
Record* find_native_shader_constant_00b5b960(void* registry, const void* name) {
    require(registry && name, "native shader lookup requires the actual registry and actual name header");
    for (std::uint32_t index = 0; index < current<std::uint32_t>(registry, 8); ++index) {
        auto* const row = registry_row(registry, index);
        const auto length = current<std::uint32_t>(name);
        const auto candidate_length = current<std::uint32_t>(&row->name_length_14);
        if (length != candidate_length) continue;
        if (!length || _stricmp(current<const char*>(name, 4),
                current<const char*>(&row->name_data_18)) == 0)
            return registry_row(registry, index); // Reload current+04 after compare.
    }
    return nullptr;
}
std::uint32_t native_shader_constant_semantic_00b5b830(const Record& record) noexcept {
    return current<std::uint32_t>(&record.word_1c);
}
Record* initialize_native_shader_constant_00b5bc60(void* raw, std::uint32_t index,
    std::uint32_t count, const void* name, std::uint32_t semantic,
    std::uint32_t rows, std::uint32_t columns, std::uint32_t elements,
    NativeStringStorage& strings) {
    auto* const record = ::new(raw) Record;
    record->name_length_14 = 0;
    record->name_data_18 = nullptr;
    record->words_00[0] = index;
    record->words_00[1] = count;
    if (&record->name_length_14 != name) {
        resize_native_string_header_0041dd40(&record->name_length_14, strings,
            current<std::uint32_t>(name), true);
        if (current<std::uint32_t>(name) != 0)
            std::memcpy(record->name_data_18, current<const void*>(name, 4), record->name_length_14);
    }
    record->word_1c = semantic;
    record->words_00[4] = elements;
    record->words_00[2] = rows;
    record->words_00[3] = columns;
    return record;
}

struct NativeCompiledShaderMaterialAppendOperation::Impl {
    Phase phase{Phase::idle};
    NativeCompiledShaderStorage* owner{};
    const void* source_name{};
    NativeStringStorage* strings{};
    std::array<std::uint32_t, 5> arguments{};
    alignas(Record) std::array<std::byte, 0x20> temporary;
    bool temporary_started{};
    NativeCompiledShaderArrayOperation array;
    Impl() {} // The native temporary bytes have no invented preimage.
};
NativeCompiledShaderMaterialAppendOperation::NativeCompiledShaderMaterialAppendOperation()
    : impl_(std::make_unique<Impl>()) {}
NativeCompiledShaderMaterialAppendOperation::~NativeCompiledShaderMaterialAppendOperation() {
    if (impl_->phase == Phase::running || impl_->phase == Phase::failed) std::terminate();
}
NativeCompiledShaderMaterialAppendOperation::Phase NativeCompiledShaderMaterialAppendOperation::phase() const noexcept {
    return impl_->phase;
}
const NativeCompiledShaderArrayOperation& NativeCompiledShaderMaterialAppendOperation::array_operation() const noexcept {
    return impl_->array;
}
const Record* NativeCompiledShaderMaterialAppendOperation::temporary_record() const noexcept {
    return impl_->temporary_started ? reinterpret_cast<const Record*>(impl_->temporary.data()) : nullptr;
}
void append_native_shader_material_constant_00b3a750(NativeCompiledShaderStorage& owner,
    std::uint32_t index, std::uint32_t count, const void* name, std::uint32_t rows,
    std::uint32_t columns, std::uint32_t elements, NativeStringStorage& strings,
    NativeCompiledShaderMaterialAppendOperation& operation) {
    auto& frame = *operation.impl_;
    require(frame.phase == NativeCompiledShaderMaterialAppendOperation::Phase::idle,
        "native material constant append is one-shot");
    frame.owner = &owner;
    frame.source_name = name;
    frame.strings = &strings;
    frame.arguments = {index, count, rows, columns, elements};
    frame.phase = NativeCompiledShaderMaterialAppendOperation::Phase::running;
    try {
        frame.temporary_started = true;
        auto* const temporary = initialize_native_shader_constant_00b5bc60(frame.temporary.data(),
            index, count, name, 0x37, rows, columns, elements, strings);
        append_native_compiled_shader_constant_00b3a660(owner.constants_78, *temporary, strings, frame.array);
        destroy_native_string_header_0041dd20(&temporary->name_length_14, strings);
        frame.temporary_started = false;
        frame.phase = NativeCompiledShaderMaterialAppendOperation::Phase::complete;
    } catch (...) { frame.phase = NativeCompiledShaderMaterialAppendOperation::Phase::failed; throw; }
}

struct NativeCompiledShaderReflectionOperation::Impl {
    Phase phase{Phase::idle};
    NativeCompiledShaderStorage* owner{};
    const std::uint32_t* bytecode{};
    NativeCompiledShaderReflectionContext* context{};
    ID3DXConstantTable* table{};
    D3DXCONSTANTTABLE_DESC description;
    std::array<D3DXCONSTANT_DESC, 256> descriptors;
    UINT descriptor_count{};
    D3DXHANDLE current_handle{};
    std::uint32_t ordinal{};
    std::int32_t highest_start{-1};
    std::uint32_t highest_count{0xffffffff};
    std::uint32_t sampler_mask{};
    NativeString lookup_name;
    NativeString material_name;
    bool lookup_started{};
    bool material_started{};
    std::unique_ptr<NativeCompiledShaderMaterialAppendOperation> append;
    Impl() {} // D3DX owns output writes. Descriptor storage is not zero-filled.
};
static_assert(sizeof(D3DXCONSTANT_DESC) == 0x30);
static_assert(sizeof(D3DXCONSTANTTABLE_DESC) == 0x0c);
static_assert(offsetof(D3DXCONSTANT_DESC, Type) == 0x14);
static_assert(offsetof(D3DXCONSTANT_DESC, Rows) == 0x18);
static_assert(offsetof(D3DXCONSTANT_DESC, Elements) == 0x20);
NativeCompiledShaderReflectionOperation::NativeCompiledShaderReflectionOperation()
    : impl_(std::make_unique<Impl>()) {}
NativeCompiledShaderReflectionOperation::~NativeCompiledShaderReflectionOperation() {
    if (impl_->phase == Phase::running || impl_->phase == Phase::failed) std::terminate();
}
NativeCompiledShaderReflectionOperation::Phase NativeCompiledShaderReflectionOperation::phase() const noexcept {
    return impl_->phase;
}
std::uint32_t NativeCompiledShaderReflectionOperation::processed_constants() const noexcept {
    return impl_->ordinal;
}
void* NativeCompiledShaderReflectionOperation::constant_table_identity() const noexcept { return impl_->table; }
const NativeCompiledShaderMaterialAppendOperation* NativeCompiledShaderReflectionOperation::current_append() const noexcept {
    return impl_->append.get();
}
void reflect_native_compiled_shader_00b3aea0(const std::uint32_t* bytecode,
    NativeCompiledShaderStorage& owner, NativeCompiledShaderReflectionContext& context,
    NativeCompiledShaderReflectionOperation& operation) {
    auto& frame = *operation.impl_;
    require(frame.phase == NativeCompiledShaderReflectionOperation::Phase::idle,
        "native shader reflection operation is one-shot");
    require(bytecode && owner.vtable_00 == 0x00d61810,
        "native reflection requires real bytecode and the existing actual D61810 owner");
    frame.owner = &owner;
    frame.bytecode = bytecode;
    frame.context = &context;
    frame.phase = NativeCompiledShaderReflectionOperation::Phase::running;
    try {
        const auto acquired = context.d3dx.acquire(bytecode, &frame.table);
        require(SUCCEEDED(acquired) && frame.table,
            "D3DX did not produce a usable actual constant table; retained reflection cannot continue");
        const auto described = frame.table->GetDesc(&frame.description);
        require(SUCCEEDED(described), "D3DX GetDesc output is outside the populated native domain");
        for (; frame.ordinal < current<UINT>(&frame.description.Constants); ++frame.ordinal) {
            frame.current_handle = frame.table->GetConstant(nullptr, frame.ordinal);
            frame.descriptor_count = 256;
            const auto result = frame.table->GetConstantDesc(frame.current_handle,
                frame.descriptors.data(), &frame.descriptor_count);
            require(SUCCEEDED(result) && frame.descriptor_count && frame.descriptor_count <= 256,
                "D3DX did not populate a readable first constant descriptor");
            auto& descriptor = frame.descriptors[0];
            if (current<D3DXPARAMETER_TYPE>(&descriptor.Type) == D3DXPT_FLOAT) {
                const char* const name = current<const char*>(&descriptor.Name);
                require(name != nullptr, "D3DX FLOAT descriptor has no readable name");
                frame.lookup_started = true;
                construct_name(frame.lookup_name, name, context.strings);
                auto* const definition = find_native_shader_constant_00b5b960(
                    context.actual_registry_0108fe94, &frame.lookup_name);
                destroy_native_string_header_0041dd20(&frame.lookup_name, context.strings);
                frame.lookup_started = false;
                bool accepted = true;
                if (definition) {
                    const auto first_id = native_shader_constant_semantic_00b5b830(*definition);
                    require(first_id < 0x36, "system semantic is outside the actual54-byte binding arrays");
                    if (current<std::uint8_t>(&owner, 8u + first_id) != 0xff) accepted = false;
                    else {
                        const auto index = static_cast<std::uint8_t>(current<UINT>(&descriptor.RegisterIndex));
                        const auto count = static_cast<std::uint8_t>(current<UINT>(&descriptor.RegisterCount));
                        const auto second_id = native_shader_constant_semantic_00b5b830(*definition);
                        require(second_id < 0x36, "system semantic changed beyond the actual binding arrays");
                        write(&owner, 0x3eu + second_id, count);
                        write(&owner, 8u + second_id, index);
                    }
                } else {
                    frame.append = std::make_unique<NativeCompiledShaderMaterialAppendOperation>();
                    const char* const current_name = current<const char*>(&descriptor.Name);
                    require(current_name != nullptr, "D3DX material descriptor lost its readable name");
                    frame.material_started = true;
                    construct_name(frame.material_name, current_name, context.strings);
                    const auto elements = current<UINT>(&descriptor.Elements);
                    const auto columns = current<UINT>(&descriptor.Columns);
                    const auto rows = current<UINT>(&descriptor.Rows);
                    const auto count = current<UINT>(&descriptor.RegisterCount);
                    const auto index = current<UINT>(&descriptor.RegisterIndex);
                    append_native_shader_material_constant_00b3a750(owner, index, count,
                        &frame.material_name, rows, columns, elements, context.strings, *frame.append);
                    destroy_native_string_header_0041dd20(&frame.material_name, context.strings);
                    frame.material_started = false;
                }
                const auto index = signed_word(current<UINT>(&descriptor.RegisterIndex));
                if (accepted && frame.highest_start < index) {
                    frame.highest_start = index;
                    frame.highest_count = current<UINT>(&descriptor.RegisterCount);
                }
            }
            if (current<D3DXREGISTER_SET>(&descriptor.RegisterSet) == D3DXRS_SAMPLER)
                frame.sampler_mask |= 1u << (current<UINT>(&descriptor.RegisterIndex) & 31u);
        }
        const auto end_register = frame.highest_start < 0 ? std::uint8_t{0} : static_cast<std::uint8_t>(
            static_cast<std::uint32_t>(frame.highest_start) + frame.highest_count);
        write(&owner, 0x74, end_register);
        write(&owner, 0x84, frame.sampler_mask);
        frame.table->Release();
        frame.table = nullptr;
        frame.phase = NativeCompiledShaderReflectionOperation::Phase::complete;
    } catch (...) { frame.phase = NativeCompiledShaderReflectionOperation::Phase::failed; throw; }
}
} // namespace bsp
