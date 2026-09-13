#pragma once
#include "bsp/native_shader_sampler_owner.hpp"
#include "bsp/native_material_pass_states.hpp"

namespace bsp {
// Actual shaderfx/shaders.bin rows produced by B35340: native name00/04,
// separately allocated blob08 and byte count0C. No constructor/owner is added.
struct NativeShaderBinaryCacheRecordStorage {
    NativeString name_00;
    void* bytes_08;
    std::uint32_t byte_count_0c;
};
// B3A600 allocates10h; B38A70 opens the stream and B35340 populates the rows.
// Cursor00 is initialized by the loader only when source-mode gate is zero.
struct NativeShaderBinaryCacheStorage {
    std::uint32_t cursor_00, count_04;
    void* stream_08;
    NativeShaderBinaryCacheRecordStorage* records_0c;
};
static_assert(sizeof(NativeShaderBinaryCacheRecordStorage)==0x10);
static_assert(offsetof(NativeShaderBinaryCacheRecordStorage,bytes_08)==8);
static_assert(offsetof(NativeShaderBinaryCacheRecordStorage,byte_count_0c)==0xc);
static_assert(sizeof(NativeShaderBinaryCacheStorage)==0x10);
static_assert(offsetof(NativeShaderBinaryCacheStorage,records_0c)==0xc);

// Host-only retained call state; no fields added to native owners. Callers keep
// borrowed headers/rows/pass/state owners alive and exclude their retirement
// while running/failed. No native unwind, rollback or admission guard is added.
// CRT comparison and the existing render-state setter remain explicit boundaries.
struct NativeShaderCompilerLookupOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t function{}, native_site{}, index{}, completed_rows{};
    std::uint32_t query_length{}, row_length{}, current_cursor{}, sampled_state{}, sampled_value{};
    int comparison{};
    const NativeShaderDescriptorStorage* descriptor{};
    NativeShaderBinaryCacheStorage* cache{};
    const NativeString* query{};
    const volatile std::uint8_t* source_mode_0108d6f1{};
    void* pass{};
    void* data{};
    const void* row{};
    const char* query_data{};
    const char* row_data{};
    NativeShaderCompilerLookupOperation()=default;
    ~NativeShaderCompilerLookupOperation();
    NativeShaderCompilerLookupOperation(const NativeShaderCompilerLookupOperation&)=delete;
    NativeShaderCompilerLookupOperation& operator=(const NativeShaderCompilerLookupOperation&)=delete;
    // Only after caller resolves any retained state-setter failure. Frees nothing.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// B347E0 ECX descriptor, stack actual8h query, EAX ordinal/FFFFFFFF, RET4.
// Length equality then CRT _stricmp(row,query); zero lengths need no data.
// Overall C4 ordinal, no stage/type/source filtering; current count/data reload.
std::uint32_t find_native_sampler_ordinal_00b347e0(const NativeShaderDescriptorStorage&,
    const NativeString*, NativeShaderCompilerLookupOperation&);
// B34890 ECX actual10h cache, stack actual8h query, EAX actual16-byte row/null,
// RET4. Actual gate is read once before dereferencing cache/query, so nonzero
// permits inaccessible cache/query pointers and returns null without mutation.
// Separate loop counter starts at cursor; each row uses CURRENT cursor/data.
// Match increments CURRENT cursor, then returns CURRENT base+(cursor-1)*16.
NativeShaderBinaryCacheRecordStorage* find_native_shader_binary_cache_record_00b34890(
    NativeShaderBinaryCacheStorage*, const NativeString*, const volatile std::uint8_t&,
    NativeShaderCompilerLookupOperation&);
// B34920 caller ECX builder is unused; stacked actual pass then descriptor,
// RET8. Descriptor B8/BC pairs forward to existing B5EC40 using ECX pass and
// stack state/value (RET8), reloading descriptor count/data after each setter.
// These explicit C++ interfaces are not original ABI replacements.
void apply_native_descriptor_render_states_00b34920(void* actual_pass,
    const NativeShaderDescriptorStorage&, NativeShaderCompilerLookupOperation&);
} // namespace bsp
