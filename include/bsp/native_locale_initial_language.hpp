#pragma once

namespace bsp {
struct NativeStringRawPoolContext;
class NativeLocaleInitialLanguageFrame;

// Only the genuine AA09D0 equal/changed-before-tables domain. The caller
// proves the actual ctor-returned raw4040 receiver and initialized E1768C
// pool, SAME raw string cells, completed borrowed raw8/NUL backing, legal
// resize aliases, and serialized disjoint metadata/provider lifetimes.
// Construction binds addresses only; it reads no native fields or cells.
class NativeLocaleInitialLanguageContext final {
public:
    NativeLocaleInitialLanguageContext(void* actual_manager,
        const void* borrowed_completed_raw8, NativeStringRawPoolContext&,
        void* actual_initialized_locale_pool_00e1768c) noexcept;
    NativeLocaleInitialLanguageContext(const NativeLocaleInitialLanguageContext&) = delete;
    NativeLocaleInitialLanguageContext& operator=(const NativeLocaleInitialLanguageContext&) = delete;
    NativeLocaleInitialLanguageContext(NativeLocaleInitialLanguageContext&&) = delete;
    NativeLocaleInitialLanguageContext& operator=(NativeLocaleInitialLanguageContext&&) = delete;
private:
    friend class NativeLocaleInitialLanguageFrame;
    friend void set_native_locale_initial_language_before_tables_00aa09d0(
        const NativeLocaleInitialLanguageContext&, NativeLocaleInitialLanguageFrame&);
    void* const manager_;
    const void* const input_;
    NativeStringRawPoolContext& strings_;
    void* const pool_;
};

struct NativeLocaleInitialLanguageProgress {
    bool attempted{}, equality_entered{}, equality_returned{}, equal{};
    bool changed{}, self_skip{}, resize_entered{}, resize_returned{};
    bool copy_entered{}, copy_returned{}, map_clear_entered{}, map_clear_returned{};
    bool names_begin_read{}, names_end_read{}, begin_null{}, count_zero{};
    bool normal_returned{}, failed{};
};

// Nonmoving retained history. Status and destruction read own metadata only,
// including after input/manager bytes end. No native cleanup or retry occurs.
class NativeLocaleInitialLanguageFrame final {
public:
    explicit NativeLocaleInitialLanguageFrame(const NativeLocaleInitialLanguageContext&) noexcept;
    ~NativeLocaleInitialLanguageFrame() noexcept = default;
    NativeLocaleInitialLanguageFrame(const NativeLocaleInitialLanguageFrame&) = delete;
    NativeLocaleInitialLanguageFrame& operator=(const NativeLocaleInitialLanguageFrame&) = delete;
    NativeLocaleInitialLanguageFrame(NativeLocaleInitialLanguageFrame&&) = delete;
    NativeLocaleInitialLanguageFrame& operator=(NativeLocaleInitialLanguageFrame&&) = delete;
    NativeLocaleInitialLanguageProgress progress() const noexcept { return progress_; }
    bool has_normal_result() const noexcept {
        return phase_ == Phase::returned && normal_result_;
    }
private:
    friend void set_native_locale_initial_language_before_tables_00aa09d0(
        const NativeLocaleInitialLanguageContext&, NativeLocaleInitialLanguageFrame&);
    enum class Phase { fresh, active, returned, failed };
    const NativeLocaleInitialLanguageContext& context_;
    Phase phase_{Phase::fresh};
    bool normal_result_{};
    NativeLocaleInitialLanguageProgress progress_{};
    void require_active();
    void complete();
    void fail() noexcept;
    [[noreturn]] void reject(const char*);
};

// Ordinary Source interface, not the Original register/stack/FH3 ABI. Borrow
// input; NEVER consume/destroy/return it. Unexpected nonempty names reject
// after actual changed-language/map effects and retain the partial graph.
// No GUI/nonempty continuation, full AA09D0/reload, loading or startup grant.
void set_native_locale_initial_language_before_tables_00aa09d0(
    const NativeLocaleInitialLanguageContext&, NativeLocaleInitialLanguageFrame&);
} // namespace bsp
