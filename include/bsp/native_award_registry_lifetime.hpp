#pragma once
#include "bsp/native_game_profile_lifetime.hpp"
#include <cstdint>

namespace bsp {
// Reuses the actual pool/call-service context already retained by game profile
// lifetime. No second pool or synthetic award record domain is introduced.
struct NativeAwardRegistryLifetimeProgress {
    void* owner{};
    void* node{};
    void* record{};
    void* cursor{};
    std::uint32_t native_site{};
    std::int32_t registry_unwind{-1},node_unwind{-1},record_unwind{-1};
    std::uint32_t iterator_output[2];
};
struct NativeAwardRegistryLifetimeOperation final : NativeAwardRegistryLifetimeProgress {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    NativeGameProfileLifetimeContext* context{};
    NativeAwardRegistryLifetimeOperation()=default;
    ~NativeAwardRegistryLifetimeOperation();
    NativeAwardRegistryLifetimeOperation(const NativeAwardRegistryLifetimeOperation&)=delete;
    NativeAwardRegistryLifetimeOperation& operator=(const NativeAwardRegistryLifetimeOperation&)=delete;
    // Caller resolves retained strings/nodes/storage first. No implicit rollback.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Full72B487110 and5B489CA0 JMP. ECX{opaque0,head4,count8},RET.
// Reset current sentinel, capture next before each free, compare current head,
// finally free current sentinel and clear4. Payload integer bytes are untouched.
void destroy_native_award_index_list_00487110(void*,NativeGameProfileLifetimeContext&,NativeAwardRegistryLifetimeProgress&);
void destroy_native_award_index_list_thunk_00489ca0(void*,NativeGameProfileLifetimeContext&,NativeAwardRegistryLifetimeProgress&);
// Full256B501FA0, ECX54h record,RET. Free current buffer48,clear48/4C/50;
// destroy list30;release strings28,1C,14,C,0 in order without resetting headers.
// Called on award-map value node14; other record users share this body.
void destroy_native_award_record_00501fa0(void*,NativeGameProfileLifetimeContext&,NativeAwardRegistryLifetimeProgress&);
// Full149B6B9120,ECX tree,stack node,RET4.6Ch node,nil69; recurse right,
// capture left, destroy record14 then captured keyC/10,free node,follow left.
void erase_native_award_registry_subtree_006b9120(void* tree,void* node,NativeGameProfileLifetimeContext&,NativeAwardRegistryLifetimeProgress&);
// Only the full-range library branch of6B91F0 reached by6B93BA. Require both
// owners==tree,first==current head.left,last==current head. Partial erase stays
// unavailable. Original reference body201B has five stack arguments,RET14.
void* clear_native_award_registry_full_range_006b91f0(void* tree,void* output,
    void* first_owner,void* first,void* last_owner,void* last,
    NativeGameProfileLifetimeContext&,NativeAwardRegistryLifetimeProgress&);
// Full149B6B9380,ECX20h registry,RET. Full tree10 cleanup/currentheadfree,
// clear14/18,then current string vector4/8 cleanup/free and clear4/8/C.
// Preserve registry0/10/1C. Explicit source ABI,not native FH3/SEH cleanup.
void destroy_native_award_registry_006b9380(void*,NativeGameProfileLifetimeContext&,NativeAwardRegistryLifetimeOperation&);
} // namespace bsp
