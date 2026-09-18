#pragma once
#include <cstdint>

namespace bsp {
struct NativeGameContainerLifetimeCalls {
    virtual ~NativeGameContainerLifetimeCalls()=default;
    virtual void* allocate_00bf55be(std::uint32_t bytes);
    virtual void free_00bf65ac(void*);
    virtual void free_00bf6989(void*);
    virtual std::int32_t interlocked_increment(volatile std::int32_t*);
    virtual std::int32_t interlocked_decrement(volatile std::int32_t*);
    virtual void virtual_scalar(void* captured,std::uint32_t slot,std::uint32_t flags)=0;
    virtual void virtual_terminal(void* captured)=0;
};
// Diagnostic observations, not owning storage. No rollback or implicit cleanup.
// A failed growth can retain a replacement allocation and partially moved refs;
// callers resolve that graph before any retry. The game parent is one-shot.
struct NativeGameContainerLifetimeProgress {
    void* owner{};
    void* cursor{};
    void* replacement_allocation{};
    std::uint32_t native_site{};
    std::uint32_t index{};
    std::int32_t unwind_state{-1};
};
// Full70B4BF8E0 and70B4C2CE0. Actual {count,head,tail}, node{prev,next,...}.
// Capture current head, reload links, unlink, decrement count,free; repeat
// against CURRENT count. Payload untouched. Empty input leaves head/tail as-is.
void clear_native_game_unit_links_004bf8e0(void*,NativeGameContainerLifetimeCalls&,NativeGameContainerLifetimeProgress&);
void clear_native_game_scene_links_004c2ce0(void*,NativeGameContainerLifetimeCalls&,NativeGameContainerLifetimeProgress&);
// Full101B4BF930: same unlink/free schedule, but scalar-delete captured node+8
// payload with flags1 and clear node+8 AFTER callback, before reading links.
void destroy_native_game_scene_records_004bf930(void*,NativeGameContainerLifetimeCalls&,NativeGameContainerLifetimeProgress&);
// Full72B4C4B40 and5B4C8180 JMP. Raw {opaque,head,count} sentinel list.
// Reset head links/count; capture next before nodefree,compare CURRENT head;
// free current sentinel,clear head4. No payload destructor inferred.
void destroy_native_game_pointer_list_004c4b40(void*,NativeGameContainerLifetimeCalls&,NativeGameContainerLifetimeProgress&);
void destroy_native_game_pointer_list_thunk_004c8180(void*,NativeGameContainerLifetimeCalls&,NativeGameContainerLifetimeProgress&);
// Full86B4CF3F0: same node walk but retain sentinel; unsigned node20>=16
// frees nodeC data; then set20=15,1C=0,only byteC=0 before nodefree.
// 4C1950 allocates24h nodes. These are consumed fields,not a new STL class.
void clear_native_game_small_string_nodes_004cf3f0(void*,NativeGameContainerLifetimeCalls&,NativeGameContainerLifetimeProgress&);
// Full104B4C4A50: drain DWORD10 tozero,clearingC only on transition tozero;
// capture unsigned count8 and free nonnull pointer slots in reverse, reloading
// base4 each time; free current base then clear4/8. Preserve0 and other bytes.
void destroy_native_game_pointer_blocks_004c4a50(void*,NativeGameContainerLifetimeCalls&,NativeGameContainerLifetimeProgress&);
// Full244B4C86A0: signed requested<1 clamps1; signed capacity gate. Allocate
// wrapped requested*4 bytes,copy/ref-retain ascending CURRENT old count,then
// release/clear CURRENT old cells ascending. Free CURRENT old base,publish
// captured replacement and requested capacity. Actual header{base,count,capacity}.
void reserve_native_game_reference_slots_004c86a0(void*,std::uint32_t requested,NativeGameContainerLifetimeCalls&,NativeGameContainerLifetimeProgress&);
// Full126B4CB220: signed capacity/count comparisons; grow if needed; zero new
// slots ascending; shrink by decrementing current count before each release,
// capture cell/payload across atomic/terminal calls,clear captured cell afterward.
// Both vector entries use original ECX owner,one stack DWORD,RET4; other entries
// use ECX owner/no stack args/RET. Explicit source interfaces,not native FH3/ABI.
void resize_native_game_reference_slots_004cb220(void*,std::uint32_t requested,NativeGameContainerLifetimeCalls&,NativeGameContainerLifetimeProgress&);
} // namespace bsp
