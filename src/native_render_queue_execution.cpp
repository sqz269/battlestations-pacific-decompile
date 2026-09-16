#include "bsp/native_render_queue_execution.hpp"
#include "bsp/native_render_command_execution.hpp"
#include "bsp/native_render_command_owner.hpp"
#include "bsp/native_render_queue_access.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

namespace bsp {
namespace {
using Word=std::uint32_t;
void* at(const void* p,Word offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p)+offset);
}
__declspec(noinline) Word word(const void* p,Word offset=0) noexcept {
    return *static_cast<const volatile Word*>(at(p,offset));
}
void* pointer(const void* p,Word offset=0) noexcept {
    return reinterpret_cast<void*>(word(p,offset));
}
void put(void* p,Word offset,void* value) noexcept {
    *static_cast<void* volatile*>(at(p,offset))=value;
}
void require(bool valid,const char* message) {
    if(!valid) throw std::logic_error(message);
}
NativeRenderCommandStorage* current_command(NativeRenderCommandQueueStorage& queue,Word index) noexcept {
    return static_cast<NativeRenderCommandStorage*>(pointer(pointer(&queue,0x14),index*4u));
}
NativeRenderCommandExecutionFrame* take_frame(NativeRenderQueueExecutionFrame* frame) {
    if(!frame) return nullptr;
    require(frame->commands && frame->used<frame->capacity,
        "raw queue requires an unused prepared persistent command frame slot");
    auto* const child=frame->commands[frame->used];
    ++frame->used;
    return child;
}
void require_lifetime(NativeRenderQueueExecutionContext& c) {
    require(c.actual_lifetime && &c.actual_lifetime->owners==&c.actual_owners &&
        &c.actual_lifetime->strings.actual_published_01090aa8==&c.actual_commands.actual_strings.actual_published_01090aa8 &&
        &c.actual_lifetime->strings.actual_small_returns_disabled_01090aa4==&c.actual_commands.actual_strings.actual_small_returns_disabled_01090aa4 &&
        &c.actual_lifetime->strings.actual_manager_publication_01090aa0==&c.actual_commands.actual_strings.actual_manager_publication_01090aa0,
        "raw queue destruction must use the same actual owners and raw string-pool cells");
}
} // namespace

void execute_native_render_queue_00b1ebe0(NativeRenderCommandQueueStorage& queue,
    NativeRenderQueueExecutionContext* context,NativeRenderQueueExecutionFrame* frame) {
    Word index=0;
    if(static_cast<std::int32_t>(word(&queue,0x18))>0) {
        require(context!=nullptr,"nonempty raw queue requires its actual execution context");
        auto& c=*context;
        do {
            auto* const first=current_command(queue,index);
            auto* const incoming=static_cast<NativeRenderContextStorage*>(pointer(first,0x28));
            void* const old=pointer(&queue,0x30);
            if(old!=incoming) {
                put(&queue,0x30,incoming);
                if(incoming) (void)incoming->references_04.fetch_add(1,std::memory_order_seq_cst);
                if(old) release_native_render_actual_owner(c.actual_owners,old);
            }
            auto* const command=current_command(queue,index);
            require(word(command)==0x00d5e5e0 && c.actual_command_profile_00d5e5e0 &&
                c.actual_command_profile_00d5e5e0[0]==0x00b1d950,
                "raw queue requires current D5E5E0/B1D950 command execution");
            execute_native_render_command_00b1d950(command,c.actual_commands,take_frame(frame));
            void* const current=pointer(&queue,0x30);
            if(current) {
                release_native_render_actual_owner(c.actual_owners,current);
                put(&queue,0x30,nullptr);
            }
            if(word(&queue,0x20)==0) {
                auto* const doomed=current_command(queue,index);
                if(doomed) {
                    require_lifetime(c);
                    destroy_native_render_command_00b1ddd0(*doomed,*c.actual_lifetime);
                    singleton_lifetime_free(doomed);
                }
            }
            ++index;
        } while(static_cast<std::int32_t>(index)<static_cast<std::int32_t>(word(&queue,0x18)));
    }
    if(word(&queue,0x20)==0) resize_native_render_command_pointers_00b1cc80(queue.commands_14,0);
}
} // namespace bsp
