#pragma once
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_string.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {
// Actual 20h reflection record. The first five words are copied without
// interpretation. B5BB40 writes only the actual string and word1C=37h.
struct NativeCompiledShaderConstantStorage {
    std::array<std::uint32_t, 5> words_00;
    std::uint32_t name_length_14;
    char* name_data_18;
    std::uint32_t word_1c;
};
struct NativeCompiledShaderConstants {
    NativeCompiledShaderConstantStorage* data_00;
    std::int32_t count_04;
    std::int32_t capacity_08;
};
// D61810 is reflection metadata, not a COM shader. The B3B3C0 producers do
// NOT initialize3E..73 or75..77. Word84 is kept uninterpreted here.
struct NativeCompiledShaderStorage {
    volatile std::uint32_t vtable_00;
    std::atomic<std::int32_t> references_04;
    std::array<std::byte, 0x36> initialized_ff_08;
    std::array<std::byte, 0x36> preserved_3e;
    std::uint8_t byte_74;
    std::array<std::byte, 3> preserved_75;
    NativeCompiledShaderConstants constants_78;
    std::uint32_t word_84;
};
static_assert(sizeof(NativeCompiledShaderConstantStorage) == 0x20);
static_assert(offsetof(NativeCompiledShaderConstantStorage, name_length_14) == 0x14);
static_assert(offsetof(NativeCompiledShaderConstantStorage, word_1c) == 0x1c);
static_assert(sizeof(NativeCompiledShaderConstants) == 0x0c);
static_assert(sizeof(NativeCompiledShaderStorage) == 0x88);
static_assert(offsetof(NativeCompiledShaderStorage, references_04) == 4);
static_assert(offsetof(NativeCompiledShaderStorage, preserved_3e) == 0x3e);
static_assert(offsetof(NativeCompiledShaderStorage, byte_74) == 0x74);
static_assert(offsetof(NativeCompiledShaderStorage, constants_78) == 0x78);
static_assert(offsetof(NativeCompiledShaderStorage, word_84) == 0x84);
static_assert(std::is_standard_layout_v<NativeCompiledShaderStorage>);
static_assert(std::is_trivially_destructible_v<NativeCompiledShaderStorage>);

// Full inlined constructor writes at B3B57C..B3B5B0 and B3B5D6..B3B60A.
// This neither allocates nor publishes the owner into a pass. No reflection or
// bytecode production is implied. The caller registers this SAME raw+04.
NativeCompiledShaderStorage* initialize_native_compiled_shader_00b3b3c0_fragment(void*);
NativeCompiledShaderConstantStorage* initialize_native_compiled_shader_constant_00b5bb40(
    void*, NativeStringStorage&);
NativeCompiledShaderConstantStorage* copy_native_compiled_shader_constant_00b38310(
    void*, const NativeCompiledShaderConstantStorage&, NativeStringStorage&);
// Same full 114-byte copy schedule through the actual mutable raw pool cells.
// No local EH cleanup: an exception can leave an uncounted partial string.
NativeCompiledShaderConstantStorage* copy_native_compiled_shader_constant_00b38310(
    void*, const NativeCompiledShaderConstantStorage&, NativeStringRawPoolContext&);

// Host continuation metadata, not native layout or native SEH emulation.
// Keep the operation, array, strings domain and append source alive after a
// failure. No automatic rollback/free of acquired native rows occurs. A failed
// call is one-shot and cannot be retried or discarded as completed cleanup.
// The caller also keeps the owning shader out of terminal admission until its
// operation is complete; this array layer does not install a second registry.
struct NativeCompiledShaderArrayOperation {
    enum class Phase { idle, running, complete, failed, diagnostic_retired };
    enum class Step { none, allocation, copying, releasing_old, publication,
        default_rows, shrinking, append_row };
    Phase phase{Phase::idle};
    Step step{Step::none};
    NativeCompiledShaderConstants* array{};
    const NativeCompiledShaderConstantStorage* append_source{};
    NativeCompiledShaderConstantStorage* unpublished_data{};
    NativeCompiledShaderConstantStorage* current_record{};
    std::int32_t requested{};
    std::int32_t allocated_capacity{};
    std::int32_t copied_rows{};
    std::int32_t initialized_rows{};
    std::int32_t released_rows{};
    NativeCompiledShaderArrayOperation() = default;
    ~NativeCompiledShaderArrayOperation();
    NativeCompiledShaderArrayOperation(const NativeCompiledShaderArrayOperation&) = delete;
    NativeCompiledShaderArrayOperation& operator=(const NativeCompiledShaderArrayOperation&) = delete;
    // Diagnostic harness only: the caller has explicitly cleaned every native
    // acquisition and cleared unpublished_data/current_record. This records
    // abandonment, never native completion or a production cleanup algorithm.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Original ECX=array, signed stack count, RET4. Reserve minimum16, copies live
// rows, releases old names forward, frees old buffer, THEN publishes data/cap.
void reserve_native_compiled_shader_constants_00b38390(NativeCompiledShaderConstants&,
    std::int32_t, NativeStringStorage&, NativeCompiledShaderArrayOperation&);
// Original ECX=array, signed stack count, RET4. Shrink decrements count BEFORE
// each name release; growth publishes count only after all default rows.
void resize_native_compiled_shader_constants_00b38490(NativeCompiledShaderConstants&,
    std::int32_t, NativeStringStorage&, NativeCompiledShaderArrayOperation&);
// Original ECX=array, stack source record, RET4. At count==capacity reserve
// max(capacity+8,16), copy the row, then increment the CURRENT count.
void append_native_compiled_shader_constant_00b3a660(NativeCompiledShaderConstants&,
    const NativeCompiledShaderConstantStorage&, NativeStringStorage&,
    NativeCompiledShaderArrayOperation&);

// ECX=actual88h owner, RET. Shrink78 to0, free CURRENT array buffer, then
// BD30F0 stampsCEB130. Count04, stale pointer/capacity and word84 are untouched.
void destroy_native_compiled_shader_00b3b1e0(NativeCompiledShaderStorage&, NativeStringStorage&);
// ECX=owner, stack flags, EAX=original pointer, RET4. Always destroy; free raw
// heap allocation iff flags&1. No count decrement or count gating.
NativeCompiledShaderStorage* delete_native_compiled_shader_00b3b260(
    NativeCompiledShaderStorage*, NativeStringStorage&, std::uint32_t flags);

class NativeCompiledShaderReference;
struct NativeCompiledShaderCompanionDisposal {
    void* context;
    void (*retire)(void*, NativeCompiledShaderReference&) noexcept;
};
// Bind/find/unbind in the caller's SAME NativeRenderActualOwners registry.
// Borrows raw+04 without retaining/initializing it; lookup stays valid through
// native deletion/free and is retired afterward. Profile points to the actual
// D61810 words; this is not a newly invented callable native vtable.
class NativeCompiledShaderReference final : public RenderCommandReference,
    private NativeRefCountedDeleteCalls {
public:
    NativeCompiledShaderReference(NativeCompiledShaderStorage&, NativeStringStorage&,
        const volatile std::uint32_t* actual_profile, NativeCompiledShaderCompanionDisposal);
    ~NativeCompiledShaderReference() override;
    NativeCompiledShaderStorage& storage() noexcept { return storage_; }
    void release_zero_references() noexcept override;
    NativeCompiledShaderStorage* scalar_delete(std::uint32_t flags);
private:
    enum class Phase { bound, destroying, retired };
    NativeCompiledShaderStorage& storage_;
    NativeStringStorage& strings_;
    const volatile std::uint32_t* profile_;
    NativeCompiledShaderCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    void require_current_profile() const;
    void delete_vslot04(void*, std::uint32_t, std::uint32_t) override;
};
} // namespace bsp
