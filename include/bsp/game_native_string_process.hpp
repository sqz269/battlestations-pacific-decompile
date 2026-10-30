#pragma once
#include "bsp/native_string_pool_storage.hpp"

namespace bsp { struct NativeParticleModelManagerStorage; }

namespace bsp::game {

// Original loader-zero publications outlive the application and every CRT
// callback, including a late 00419CC0 that recreates the manager/string pool.
// This owns cells and source bindings only; getters create the native owners.
class GameNativeStringProcess final {
public:
    GameNativeStringProcess(const GameNativeStringProcess&) = delete;
    GameNativeStringProcess& operator=(const GameNativeStringProcess&) = delete;
    void* volatile& manager_01090aa0() noexcept { return manager_; }
    // Actual pending-registry publication shares this retained process lifetime.
    // Accessing the cell does not construct or register a registry owner.
    void* volatile& pending_registry_00f878cc() noexcept;
    // Permanent allocation-statistics publication; cell access creates no owner.
    void* volatile& allocation_stats_0109cefc() noexcept;
    // Permanent typed F8C274 authority; this accessor creates no owner.
    NativeParticleModelManagerStorage* volatile& particle_manager_00f8c274() noexcept;
    // Source admission policy, not Native storage. Claim once while quiescent;
    // never reset, including after drain or host replacement. No owner is made.
    bool claim_particle_manager_domain() noexcept;
    // Sole loader-zero Application publication. The genuine BEA810/BEA8B0
    // providers publish/clear it; borrowing this cell starts no owner.
    void* volatile& application_00e1ae90() noexcept;
    // Source admission only: one Application graph for this process, even
    // after normal retirement. Never reset or use as a native reference count.
    bool claim_application_domain() noexcept;
    // Sole typed Source authority for loader-zero Original F8AEFC, retained
    // with the SAME manager process through CRT/atexit. This is not a literal
    // fixed-address native mapping. Borrow the cell; do not copy/reset it.
    // Access creates no factory, registration, module or sound state.
    void* volatile& movie_factory_00f8aefc() noexcept;
    // Sole permanent Source locale publication; borrowing creates no manager.
    // The actual constructor/base lifetime owns its values, never a semantic view.
    void* volatile& locale_manager_00f8bc4c() noexcept;
    // One quiescent Source owner graph; never reset after failure or drain.
    bool claim_locale_manager_domain() noexcept;
    NativeStringPoolStorage* volatile& pool_01090aa8() noexcept { return pool_; }
    volatile std::uint32_t& returns_disabled_01090aa4() noexcept { return disabled_; }
    NativeStringRawPoolContext& raw_context() noexcept { return raw_; }
    ActualNativeStringPoolStorage& strings() noexcept { return strings_; }
    // Distinct permanent Source authorities for the two Original initial-NUL
    // roles. Only const pointers escape; Original's writable .data is not
    // claimed universally immutable. Shader E17654 borrowers use this SAME byte.
    const char* property_empty_00e177e4() const noexcept;
    const char* query_empty_00e17654() const noexcept;
    // Distinct stable-zero ordinary clone-key role for Original F89450.
    // This does not assert universal immutability of Original writable .data.
    const char* clone_empty_00f89450() const noexcept;
    // Distinct permanent initial-NUL role for Original library key E18560.
    // Borrow the SAME retained process; do not alias another fallback cell.
    const char* library_empty_00e18560() const noexcept;
    // DISTINCT stable initial-NUL enum role E186ED. The retained process owns
    // this byte; it does not construct/publish an enum table or symbol owner.
    // Original writable DATA is not claimed universally immutable.
    const char* enum_empty_00e186ed() const noexcept;
private:
    friend GameNativeStringProcess& game_native_string_process();
    GameNativeStringProcess() noexcept = default;
    void* volatile manager_{};
    NativeStringPoolStorage* volatile pool_{};
    volatile std::uint32_t disabled_{};
    NativeStringRawPoolContext raw_{pool_, disabled_, manager_};
    ActualNativeStringPoolStorage strings_{pool_, disabled_, manager_};
    // Append after every established publication/context/storage field.
    const char property_empty_00e177e4_{};
    const char query_empty_00e17654_{};
    // Append after every established authority; do not reuse another raw8 owner.
    void* volatile pending_registry_00f878cc_{};
    void* volatile allocation_stats_0109cefc_{};
    NativeParticleModelManagerStorage* volatile particle_manager_00f8c274_{};
    bool particle_manager_domain_claimed_{};
    // Append after all established authorities; never alias either fallback.
    const char clone_empty_00f89450_{};
    // Append after every established field; Original writable .data is not
    // claimed universally immutable by this ordinary Source authority.
    const char library_empty_00e18560_{};
    // Append after every established field, even when this uses tail padding.
    // Never alias a property, query, clone or library fallback authority.
    const char enum_empty_00e186ed_{};
    // Append after all established process authorities. The Application graph
    // uses the SAME manager_ authority, not another private singleton manager.
    void* volatile application_00e1ae90_{};
    bool application_domain_claimed_{};
    // Append after every established authority/field. The process owns this
    // ONE writable publication; application hosts only borrow its reference.
    void* volatile movie_factory_00f8aefc_{};
    // Append after all established authorities, retaining their offsets.
    void* volatile locale_manager_00f8bc4c_{};
    bool locale_manager_domain_claimed_{};
};

// Intentionally retained through process termination. No C++ exit destructor
// may invalidate the cells/bindings before another native CRT callback.
GameNativeStringProcess& game_native_string_process();
} // namespace bsp::game
