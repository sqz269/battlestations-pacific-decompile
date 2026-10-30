#pragma once

#include "bsp/native_locale_node_allocation.hpp"
#include "bsp/native_string.hpp"

#include <cstdint>
#include <exception>

namespace bsp {

// Immutable identities, not a manager/header construction or ownership proof.
// Borrow the SAME live AA8/AA4/AA0 cells; do not substitute semantic storage.
struct NativeLocaleMapInsertionContext final {
    void* const actual_map4004;
    const void* const actual_key8;
    NativeStringRawPoolContext& strings;
    game::GameNativeLocalePoolProcess& pool_owner;

    NativeLocaleMapInsertionContext(void* map, const void* key,
        NativeStringRawPoolContext& raw_strings,
        game::GameNativeLocalePoolProcess& owner) noexcept
        : actual_map4004(map), actual_key8(key), strings(raw_strings), pool_owner(owner) {}
    NativeLocaleMapInsertionContext(const NativeLocaleMapInsertionContext&) = delete;
    NativeLocaleMapInsertionContext& operator=(const NativeLocaleMapInsertionContext&) = delete;
    NativeLocaleMapInsertionContext(NativeLocaleMapInsertionContext&&) = delete;
    NativeLocaleMapInsertionContext& operator=(NativeLocaleMapInsertionContext&&) = delete;
};

enum class NativeLocaleMapInsertionPhase { prepared, entered, returned, failed };

// Copied historical diagnostics only. Returned/recycled pointers are IDs, not
// permission to follow payloads or to repeat construction/cleanup/publication.
struct NativeLocaleMapInsertionProgress {
    NativeLocaleMapInsertionPhase phase{NativeLocaleMapInsertionPhase::prepared};
    const NativeLocaleMapInsertionContext* context{};
    void* map{};
    const void* key{};
    void* current_node{};
    void* allocated_slot{};
    void* actual_pool{};
    void* constructed_node{};
    void* value_header{};
    std::uint32_t bucket_index{};
    bool attempted{}, hash_completed{}, duplicate_found{}, count_incremented{};
    bool pool_borrow_entered{}, pool_borrow_returned{};
    bool allocation_entered{}, allocation_returned{};
    bool constructor_entered{}, constructor_returned{}, constructor_failed{};
    bool selected_return_entered{}, selected_return_returned{}, selected_return_failed{};
    bool next_stored{}, previous_stored{}, former_head_previous_stored{}, head_published{};
    bool normal_returned{}, failed{};
};

class NativeLocaleMapInsertionFrame final {
public:
    explicit NativeLocaleMapInsertionFrame(const NativeLocaleMapInsertionContext&) noexcept;
    ~NativeLocaleMapInsertionFrame() = default; // NO native cleanup/retry/rollback
    NativeLocaleMapInsertionFrame(const NativeLocaleMapInsertionFrame&) = delete;
    NativeLocaleMapInsertionFrame& operator=(const NativeLocaleMapInsertionFrame&) = delete;
    NativeLocaleMapInsertionFrame(NativeLocaleMapInsertionFrame&&) = delete;
    NativeLocaleMapInsertionFrame& operator=(NativeLocaleMapInsertionFrame&&) = delete;

    NativeLocaleMapInsertionProgress progress() const noexcept { return progress_; }
    NativeLocaleNodeAllocationProgress allocation_progress() const noexcept {
        return allocation_.progress();
    }
    std::exception_ptr failure() const noexcept { return failure_; }
    std::exception_ptr constructor_failure() const noexcept { return constructor_failure_; }
    std::exception_ptr selected_return_failure() const noexcept { return selected_return_failure_; }
    bool has_normal_result() const noexcept;
    void* normal_result() const noexcept;

private:
    friend void* insert_or_get_native_locale_value_00a9fc30(
        const NativeLocaleMapInsertionContext&, NativeLocaleMapInsertionFrame&);
    const NativeLocaleMapInsertionContext& context_;
    NativeLocaleNodeAllocationFrame allocation_;
    NativeLocaleMapInsertionProgress progress_{};
    std::exception_ptr failure_, constructor_failure_, selected_return_failure_;
    [[noreturn]] void reject(const char*);
    void require_entered();
    void fail(std::exception_ptr) noexcept;
    void* finish(void*);
};

// Ordinary qualified MSVC Win32 Source interface, not native ECX/RET4/EAX/FH3.
// Caller supplies genuine initialized map/key/native backing and canonical
// pool authority, disjoint stable metadata/providers and serialized lifetime.
// Duplicate returns node+8 without allocation/count changes. New insertion
// increments count BEFORE allocating, copies key and initializes empty value.
// ONLY an exception escaping the actual F460 invocation selects real736090
// on the captured allocator-returned slot/pool. No payload/header cleanup or
// count rollback; all other failure origins retain their partial graph. Keep
// every frame/provider/debt alive; startup health is not safe exit/trim proof.
void* insert_or_get_native_locale_value_00a9fc30(
    const NativeLocaleMapInsertionContext&, NativeLocaleMapInsertionFrame&);

} // namespace bsp
