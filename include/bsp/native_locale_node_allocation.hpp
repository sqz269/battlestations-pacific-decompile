#pragma once

#include <cstdint>
#include <exception>

namespace bsp::game { class GameNativeLocalePoolProcess; }

namespace bsp {

enum class NativeLocaleNodeAllocationPhase { prepared, entered, returned, failed };

// Copy-only diagnostics, never writable issuance/cleanup authority. Saved
// pointers can name unpublished, freed or partially initialized storage.
struct NativeLocaleNodeAllocationProgress {
    NativeLocaleNodeAllocationPhase phase{NativeLocaleNodeAllocationPhase::prepared};
    const game::GameNativeLocalePoolProcess* owner{};
    void* pool{};
    void* section{};
    void* slab_allocation{};
    void* constructed_slab{};
    void* table_allocation{};
    void* old_table{};
    void* selected_slab{};
    void* node{};
    std::uint32_t table_allocation_bytes{};
    bool attempted{};
    bool pool_borrow_returned{};
    bool enter_entered{}, enter_returned{}, depth_incremented{};
    bool first_free_published{};
    bool slab_allocation_entered{}, slab_allocation_returned{};
    bool block_initialization_entered{}, block_initialization_returned{};
    bool capacity_published{};
    bool table_allocation_entered{}, table_allocation_returned{};
    bool old_table_free_entered{}, old_table_free_returned{};
    bool table_published{}, slab_store_performed{}, slab_count_incremented{};
    bool free_count_decremented{};
    bool depth_decremented{}, leave_entered{}, leave_returned{};
    bool normal_returned{}, failed{};
};

class NativeLocaleNodeAllocationFrame final {
public:
    // Pure preparation; does not initialize or borrow the pool or touch bytes.
    explicit NativeLocaleNodeAllocationFrame(game::GameNativeLocalePoolProcess&) noexcept;
    ~NativeLocaleNodeAllocationFrame() = default; // NO native cleanup/unlock/free
    NativeLocaleNodeAllocationFrame(const NativeLocaleNodeAllocationFrame&) = delete;
    NativeLocaleNodeAllocationFrame& operator=(const NativeLocaleNodeAllocationFrame&) = delete;
    NativeLocaleNodeAllocationFrame(NativeLocaleNodeAllocationFrame&&) = delete;
    NativeLocaleNodeAllocationFrame& operator=(NativeLocaleNodeAllocationFrame&&) = delete;

    NativeLocaleNodeAllocationProgress progress() const noexcept { return progress_; }
    std::exception_ptr failure() const noexcept { return failure_; }
    bool has_normal_result() const noexcept;
    void* normal_result() const noexcept;

private:
    friend void* allocate_native_locale_node_00a9f0b0(
        game::GameNativeLocalePoolProcess&, NativeLocaleNodeAllocationFrame&);
    game::GameNativeLocalePoolProcess& owner_;
    NativeLocaleNodeAllocationProgress progress_{};
    std::exception_ptr failure_;
    [[noreturn]] void reject(const char*);
    void require_entered();
    void fail() noexcept;
};

// Ordinary MSVC Win32 Source ABI, not native ECX/RET/EAX/FH3 replacement.
// Exactly one attempt per prepared frame, on its SAME canonical owner. Healthy
// pool_storage is borrowed BEFORE Enter; startup health is not quiescence.
// Keep frame/native backing/provider imports disjoint and externally serialize
// pool/list/trim/lifetime use. On any escape retain ALL partial custody; there
// is no unlock/free/rollback/retry or safe-exit/trim permission. A returned
// slot is raw storage: no key/wide/next/map payload lifetime is established.
void* allocate_native_locale_node_00a9f0b0(
    game::GameNativeLocalePoolProcess&, NativeLocaleNodeAllocationFrame&);

} // namespace bsp
