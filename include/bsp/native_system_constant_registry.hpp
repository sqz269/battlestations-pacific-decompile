#pragma once
#include "bsp/native_compiled_shader_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/sound_lifetime_access.hpp"
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

// Borrow the application's actual 0108FE94, 01090AA0, 01090AA8 and 01090AA4
// cells. The string context supplies the last three; no projected manager,
// retained operation, lifetime binding, allocator callback or heap frame.
struct NativeSystemConstantRegistryRawContext {
    void* volatile& current_registry_0108fe94;
    NativeStringRawPoolContext& strings;
};

// B5BF70 raw-owner path. Caller supplies fresh, DWORD-aligned readable/writable
// 10h storage; this call keeps its own fixed 8h name/20h record scratch. The
// borrowed cells/providers remain valid through all calls and cleanup. Native
// array extents must remain valid; the normal literal sequence builds 52 rows.
// Source C++ exceptions run the recovered 106-state cleanup; this never frees
// fresh, retains a host operation or retries construction. Parent B32410 owns
// the later free. Native placement cleanup is a no-op, so unpublished arrays
// and uncounted partial strings can leak. A base-construction failure can leave
// the native publication/registration intact. See the raw-constructor report.
// Profiles are identity DWORDs; this is not the original FH3/SEH callable ABI.
NativeSystemConstantRegistryStorage* construct_native_system_constant_registry_00b5bf70(
    void* fresh, NativeSystemConstantRegistryRawContext&);
NativeSystemConstantRegistryStorage* construct_native_system_constant_base_00b5b9e0(
    NativeSystemConstantRegistryStorage&, NativeSystemConstantRegistryRawContext&);
void destroy_native_system_constant_base_00b5ba80(
    NativeSystemConstantRegistryStorage&, NativeSystemConstantRegistryRawContext&);
NativeCompiledShaderConstantStorage* construct_native_system_constant_00b5bbc0(
    void* fresh, const NativeString&, std::uint32_t id, std::uint32_t second,
    std::uint32_t first, std::uint32_t array_count, NativeStringRawPoolContext&);
void reserve_native_system_constants_00b5bd10(NativeCompiledShaderConstants&,
    std::int32_t request, NativeStringRawPoolContext&);
void append_native_system_constant_00b5bed0(NativeCompiledShaderConstants&,
    const NativeCompiledShaderConstantStorage&, NativeStringRawPoolContext&);
// B5BF50's resize-to-zero composition: valid nonnegative current array/count;
// decrement count before each current-row string release, then free current
// data. The stale data/capacity fields are intentionally preserved.
void destroy_native_system_constant_array_00b5bf50(NativeCompiledShaderConstants&,
    NativeStringRawPoolContext&);

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
    // Borrow the actual0108FE94 and actual01090AA0 cells and existing strings.
    // No projected domain or other-owner callbacks are needed for this route.
    // Canonical deletion uses the existing scalar wrappers and this binding;
    // a failed operation remains retained and prevents terminal admission.
    NativeSystemConstantRegistryLifetimeBinding(void* volatile& published,
        void* volatile& actual_manager_01090aa0, NativeStringStorage&);
    ~NativeSystemConstantRegistryLifetimeBinding();
    NativeSystemConstantRegistryLifetimeBinding(const NativeSystemConstantRegistryLifetimeBinding&)=delete;
    NativeSystemConstantRegistryLifetimeBinding& operator=(const NativeSystemConstantRegistryLifetimeBinding&)=delete;
    SingletonLifetimeCallbacks callbacks() noexcept;
    void bind(SingletonLifetimeDomain&, NativeStringStorage&);
    void* volatile& publication() noexcept { return published_; }
    SingletonLifetimeDomain& lifetime();
    SoundLifetimeAccess lifetime_access();
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
    void* volatile* actual_manager_{};
    NativeStringStorage* strings_{};
    NativeSystemConstantRegistryOperation* guarded_{};
};
} // namespace bsp
