#pragma once
#include "bsp/native_compiled_shader_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <memory>

namespace bsp {
// Actual D62A3C owner. Entries are the SAME 20h records consumed by shader
// reflection. This owner has no reference count and no private string pool.
struct NativeSystemConstantRegistryStorage {
    std::uint32_t vtable_00;
    NativeCompiledShaderConstants constants_04;
};
static_assert(sizeof(NativeSystemConstantRegistryStorage)==0x10);
static_assert(offsetof(NativeSystemConstantRegistryStorage,constants_04)==4);

class NativeSystemConstantRegistryLifetimeBinding;
// Host continuation metadata, not original FH3 state. A failed operation is
// one-shot and must remain alive with owner, source, binding and string domain.
// No unwinding destructor frees native acquisitions or unregisters the owner.
struct NativeSystemConstantRegistryOperation {
    enum class Phase { fresh, running, complete, failed };
    enum class Step { none, base, arrays, literal_name, literal_record, append,
        release_record, release_name, destroy_array, destroy_base, free_owner };
    Phase phase{Phase::fresh};
    Step step{Step::none};
    NativeSystemConstantRegistryStorage* owner{};
    NativeSystemConstantRegistryLifetimeBinding* binding{};
    std::unique_ptr<NativeCompiledShaderArrayOperation> array;
    NativeString temporary_name;
    NativeCompiledShaderConstantStorage temporary_record;
    const NativeCompiledShaderConstantStorage* source{};
    const NativeCompiledShaderConstantStorage* current_copy_source{};
    char* captured_name_data{};
    std::uint32_t literal_index{};
    bool name_live{}, record_live{}, base_published{}, arrays_initialized{};
    NativeSystemConstantRegistryOperation() = default;
    ~NativeSystemConstantRegistryOperation();
    NativeSystemConstantRegistryOperation(const NativeSystemConstantRegistryOperation&)=delete;
    NativeSystemConstantRegistryOperation& operator=(const NativeSystemConstantRegistryOperation&)=delete;
};

// Full B5BBC0: ECX fresh20h, stack name/id/second/first/array; RET14h.
// Caller retains fresh storage and source through any borrowed allocation
// failure. Normal constructor writes +00/+04 before copying the current name,
// then +0C/+08/+10/+1C. No cleanup or transaction is added on failure.
NativeCompiledShaderConstantStorage* construct_native_system_constant_00b5bbc0(
    void* fresh, const NativeString&, std::uint32_t id, std::uint32_t second,
    std::uint32_t first, std::uint32_t array_count, NativeStringStorage&);

// Explicit owner/binding/operation host interfaces to original ECX=array
// helpers. They use owner.constants_04 directly and guard its canonical
// lifetime callback until normal completion. No shadow array or owner exists.
void reserve_native_system_constants_00b5bd10(NativeSystemConstantRegistryStorage&,
    std::int32_t request, NativeSystemConstantRegistryLifetimeBinding&,
    NativeSystemConstantRegistryOperation&);
void resize_native_system_constants_00b5be10(NativeSystemConstantRegistryStorage&,
    std::int32_t request, NativeSystemConstantRegistryLifetimeBinding&,
    NativeSystemConstantRegistryOperation&);
void append_native_system_constant_00b5bed0(NativeSystemConstantRegistryStorage&,
    const NativeCompiledShaderConstantStorage&, NativeSystemConstantRegistryLifetimeBinding&,
    NativeSystemConstantRegistryOperation&);

NativeSystemConstantRegistryStorage* construct_native_system_constant_base_00b5b9e0(
    NativeSystemConstantRegistryStorage&, NativeSystemConstantRegistryLifetimeBinding&,
    NativeSystemConstantRegistryOperation&);
void destroy_native_system_constant_base_00b5ba80(NativeSystemConstantRegistryStorage&,
    NativeSystemConstantRegistryLifetimeBinding&, NativeSystemConstantRegistryOperation&);
NativeSystemConstantRegistryStorage* delete_native_system_constant_base_00b5bb20(
    NativeSystemConstantRegistryStorage*, NativeSystemConstantRegistryLifetimeBinding&,
    std::uint32_t flags, NativeSystemConstantRegistryOperation&);
NativeSystemConstantRegistryStorage* construct_native_system_constant_registry_00b5bf70(
    void* fresh, NativeSystemConstantRegistryLifetimeBinding&, NativeSystemConstantRegistryOperation&);
void destroy_native_system_constant_registry_00b5df00(NativeSystemConstantRegistryStorage&,
    NativeSystemConstantRegistryLifetimeBinding&, NativeSystemConstantRegistryOperation&);
NativeSystemConstantRegistryStorage* delete_native_system_constant_registry_00b5df70(
    NativeSystemConstantRegistryStorage*, NativeSystemConstantRegistryLifetimeBinding&,
    std::uint32_t flags, NativeSystemConstantRegistryOperation&);

// Compose callbacks with the SAME 01090AA0 domain and live0108FE94 pointer.
// Construct binding, create domain(callbacks()), then bind(domain, strings).
// Keep these and every failed operation alive through canonical shutdown.
class NativeSystemConstantRegistryLifetimeBinding final {
public:
    NativeSystemConstantRegistryLifetimeBinding(void* volatile& published,
        SingletonLifetimeCallbacks next);
    ~NativeSystemConstantRegistryLifetimeBinding();
    NativeSystemConstantRegistryLifetimeBinding(const NativeSystemConstantRegistryLifetimeBinding&)=delete;
    NativeSystemConstantRegistryLifetimeBinding& operator=(const NativeSystemConstantRegistryLifetimeBinding&)=delete;
    SingletonLifetimeCallbacks callbacks() noexcept;
    void bind(SingletonLifetimeDomain&, NativeStringStorage&);
    void* volatile& publication() noexcept { return published_; }
    SingletonLifetimeDomain& lifetime();
    NativeStringStorage& strings();
    // Checked operation admission; no native ownership is acquired here.
    // Only one unfinished registry operation is supported by this binding,
    // including across different registry owners. Other callback profiles forward.
    void begin(NativeSystemConstantRegistryStorage&, NativeSystemConstantRegistryOperation&);
    void complete(NativeSystemConstantRegistryOperation&);
    bool terminal_allowed(const NativeSystemConstantRegistryStorage&) const noexcept;
private:
    static void destroy_registered(void*,void*,std::uint32_t) noexcept;
    static void invalid_parameter(void*);
    void* volatile& published_;
    SingletonLifetimeCallbacks next_;
    SingletonLifetimeDomain* lifetime_{};
    NativeStringStorage* strings_{};
    NativeSystemConstantRegistryOperation* guarded_{};
};
} // namespace bsp
