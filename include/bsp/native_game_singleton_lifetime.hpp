#pragma once
#include <cstdint>

namespace bsp {
// Complete normal source paths; the scalar payload bodies remain required
// bindings. Defaults use the actual raw singleton manager and Win32 section.
struct NativeGameSingletonLifetimeCalls {
    virtual ~NativeGameSingletonLifetimeCalls()=default;
    virtual void* singleton_manager_00415350(void* volatile& actual_publication);
    virtual void unregister_singleton_00bcfca0(void* manager,void* object);
    virtual void enter_singleton_section(void* actual_section);
    virtual void leave_singleton_section(void* actual_section);
    virtual void virtual_scalar(void* captured,std::uint32_t slot,std::uint32_t flags)=0;
};
struct NativeGameSingletonLifetimeContext {
    void* volatile& actual_manager_01090aa0;
    void* volatile& publication_00e18e6c;
    void* volatile& publication_00e18d80;
    void* volatile& publication_00f89b34;
    NativeGameSingletonLifetimeCalls& calls;
};
struct NativeGameSingletonLifetimeOperation final {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    NativeGameSingletonLifetimeContext* context{};
    void* volatile* publication{};
    void* captured_section{};
    bool entered_section_pending{};
    std::uint32_t native_site{};
    NativeGameSingletonLifetimeOperation()=default;
    ~NativeGameSingletonLifetimeOperation();
    NativeGameSingletonLifetimeOperation(const NativeGameSingletonLifetimeOperation&)=delete;
    NativeGameSingletonLifetimeOperation& operator=(const NativeGameSingletonLifetimeOperation&)=delete;
    // Caller must resolve the partial graph and any retained lock first.
    // This only retires diagnostics; no release, rollback or retry is performed.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// No native inputs, plain RET; source interfaces supply actual publications.
// Capture manager+10 once. Enter then increment its +18 recursion word;
// unregister and delete current publication; clear it AFTER the scalar call;
// decrement then leave the captured section. Null section is supported.
// 4C0C30/4C0CE0 capture adjusted object BEFORE the second manager getter.
// 4C0D90 loads unadjusted object AFTER it. All scalar receivers are reloaded.
void delete_native_game_singleton_004c0c30(NativeGameSingletonLifetimeContext&,NativeGameSingletonLifetimeOperation&);
void delete_native_game_singleton_004c0ce0(NativeGameSingletonLifetimeContext&,NativeGameSingletonLifetimeOperation&);
void delete_native_game_singleton_004c0d90(NativeGameSingletonLifetimeContext&,NativeGameSingletonLifetimeOperation&);
// Verified complete one-byte C3 routine. No side effect, native input or output.
void native_game_cleanup_noop_008d88f0() noexcept;
// Source failure retains partial state/lock and rejects replay. Original FH3
// guard unwinding, SEH, payload classes and drop-in binary ABI are not supplied.
} // namespace bsp
