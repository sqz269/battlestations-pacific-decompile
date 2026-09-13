#pragma once
#include "bsp/native_compiled_shader_owner.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <memory>

struct ID3DXConstantTable;
namespace bsp {
// Resolve the actual named d3dx9_40 import. No compiler/reflection callback or
// alternate provider. Caller keeps the module loaded through all retained
// operations and their actual COM table lifetimes; this binding borrows it.
class NativeD3dx9ShaderConstantTableImport final {
public:
    explicit NativeD3dx9ShaderConstantTableImport(HMODULE actual_d3dx9_40);
    NativeD3dx9ShaderConstantTableImport(const NativeD3dx9ShaderConstantTableImport&) = delete;
    NativeD3dx9ShaderConstantTableImport& operator=(const NativeD3dx9ShaderConstantTableImport&) = delete;
    HRESULT acquire(const std::uint32_t* bytecode, ID3DXConstantTable** actual_output) const;
private:
    using Function = HRESULT (WINAPI*)(const DWORD*, ID3DXConstantTable**);
    Function function_;
};

// Complete B5B960, ECX=actual registry, stack actual8h name, EAX=actual20h
// record or0, RET4. Registry+04=data,+08=unsigned count, as produced by B5BF70;
// current length/data/count reloads and first length+_stricmp match are kept.
// This does not construct the registry or convert the semantic52-record list.
NativeCompiledShaderConstantStorage* find_native_shader_constant_00b5b960(
    void* actual_registry, const void* actual_name_header);
// Complete B5B830, ECX=actual20h record, EAX=word1C, RET.
std::uint32_t native_shader_constant_semantic_00b5b830(const NativeCompiledShaderConstantStorage&) noexcept;
// Complete B5BC60 normal body, ECX=fresh20h, seven stack arguments in this
// order, EAX=same, RET1C. Actual name header is borrowed through the call.
NativeCompiledShaderConstantStorage* initialize_native_shader_constant_00b5bc60(
    void* actual_storage, std::uint32_t register_index, std::uint32_t register_count,
    const void* actual_name_header, std::uint32_t semantic_id, std::uint32_t rows,
    std::uint32_t columns, std::uint32_t elements, NativeStringStorage&);

class NativeCompiledShaderMaterialAppendOperation final {
public:
    enum class Phase { idle, running, complete, failed };
    NativeCompiledShaderMaterialAppendOperation();
    ~NativeCompiledShaderMaterialAppendOperation();
    NativeCompiledShaderMaterialAppendOperation(const NativeCompiledShaderMaterialAppendOperation&) = delete;
    NativeCompiledShaderMaterialAppendOperation& operator=(const NativeCompiledShaderMaterialAppendOperation&) = delete;
    Phase phase() const noexcept;
    const NativeCompiledShaderArrayOperation& array_operation() const noexcept;
    const NativeCompiledShaderConstantStorage* temporary_record() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void append_native_shader_material_constant_00b3a750(NativeCompiledShaderStorage&,
        std::uint32_t, std::uint32_t, const void*, std::uint32_t, std::uint32_t,
        std::uint32_t, NativeStringStorage&, NativeCompiledShaderMaterialAppendOperation&);
};
// Complete B3A750 normal body, ECX=owner, six stack arguments, RET18.
// Construct the retained actual20h temporary with semantic37h, append via
// B3A660 and its SAME retained array frame, then release temporary name.
// On host failure retain temporary/source/array acquisitions; no native unwind
// cleanup is substituted. Running/failed operation destruction terminates.
void append_native_shader_material_constant_00b3a750(NativeCompiledShaderStorage&,
    std::uint32_t register_index, std::uint32_t register_count, const void* actual_name_header,
    std::uint32_t rows, std::uint32_t columns, std::uint32_t elements,
    NativeStringStorage&, NativeCompiledShaderMaterialAppendOperation&);

struct NativeCompiledShaderReflectionContext {
    NativeStringStorage& strings; // SAME actual pool/publication/lifetime domain
    void* const volatile& actual_registry_0108fe94; // Required actual B5BF70 owner
    const NativeD3dx9ShaderConstantTableImport& d3dx;
};
class NativeCompiledShaderReflectionOperation final {
public:
    enum class Phase { idle, running, complete, failed };
    NativeCompiledShaderReflectionOperation();
    ~NativeCompiledShaderReflectionOperation();
    NativeCompiledShaderReflectionOperation(const NativeCompiledShaderReflectionOperation&) = delete;
    NativeCompiledShaderReflectionOperation& operator=(const NativeCompiledShaderReflectionOperation&) = delete;
    Phase phase() const noexcept;
    std::uint32_t processed_constants() const noexcept;
    void* constant_table_identity() const noexcept;
    const NativeCompiledShaderMaterialAppendOperation* current_append() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void reflect_native_compiled_shader_00b3aea0(const std::uint32_t*,
        NativeCompiledShaderStorage&, NativeCompiledShaderReflectionContext&,
        NativeCompiledShaderReflectionOperation&);
};
// B3AEA0 normal populated-output domain. Native ECX unused, stack bytecode
// then owner, RET8. Acquire actual D3DX table; GetDesc/+14, GetConstant/+20,
// GetConstantDesc/+18 into256 raw descriptors; consume descriptor0. FLOAT
// system slots write count3E then register08 only if current register isFF;
// unknown FLOAT records append. Publish end74, sampler mask84, THEN Release/+08.
// Native ignores HRESULTs; this interface rejects failed/missing COM outputs
// before reading uninitialized outputs. It supplies no synthetic descriptor.
// Repeated reflection preserves existing slots and appends again as native.
// Failed operations retain the actual table, names, descriptor storage and
// current append. Caller MUST retain bytecode/owner/context/module/publication
// and exclude owner terminal admission. Destructor rejects running/failed;
// no retry, automatic rollback, native SEH or successful compiler claim.
void reflect_native_compiled_shader_00b3aea0(const std::uint32_t* actual_bytecode,
    NativeCompiledShaderStorage&, NativeCompiledShaderReflectionContext&,
    NativeCompiledShaderReflectionOperation&);
} // namespace bsp
