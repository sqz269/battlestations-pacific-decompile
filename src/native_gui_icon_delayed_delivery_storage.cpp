#include "bsp/native_render_observed_zero.hpp"
namespace bsp {
class NativeGuiIconDelayedDeliveryStorageObservedZeroIssuer final {
public:
    static void dispatch(NativeRenderActualOwners& owners,void* actual,NativeRenderObservedZeroFrame& frame) {
        NativeRenderObservedZeroAccess::dispatch(owners,actual,frame);
    }
};
}
#include "bsp/native_gui_icon_delayed_delivery_storage.hpp"
#include "bsp/native_gui_icon_delayed_loads.hpp"
#include "bsp/native_gui_icon_texture_apply_storage.hpp"
#include "bsp/native_gui_icon_storage.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/native_filestore_factory.hpp"
#include "bsp/native_filestore_provider_lifetime.hpp"
#include "bsp/native_filestore_remove.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_crt_stricmp.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include <stdexcept>
namespace bsp {
namespace {
using Word=std::uint32_t;
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(const void* p,Word offset=0) noexcept {
    return *static_cast<const volatile Word*>(pointer(bits(p)+offset));
}
void write(void* p,Word offset,Word value) noexcept {
    *static_cast<volatile Word*>(pointer(bits(p)+offset))=value;
}
Word live(const Word& h) noexcept { return static_cast<const volatile Word&>(h); }
void put(Word& h,Word v) noexcept { static_cast<volatile Word&>(h)=v; }
void require(bool value,const char* message) { if(!value) throw std::logic_error(message); }
const volatile Word* table(const game::GameNativeReadOnlyData& data,Word profile,Word bytes) {
    const auto* p=static_cast<const volatile Word*>(data.data_at(profile,bytes));
    require(bits(p)==profile,"Icon delivery requires genuine CURRENT read-only table alias");return p;
}
}
struct NativeGuiIconDelayedDeliveryStorageFrame::Storage {
    struct Match {
        Match() {} // Ordinary Native argument/footer homes remain unwritten.
        Match* next{};
        NativeTextureCacheAcquired load;
        NativeGuiIconTextureApplyStorageFrame apply;
        Word renderer_homes[2],value_homes[3],preserve_home,footer_homes[3];
        void* renderer{};Word renderer_profile{},renderer_target{};
        void* loaded{};void* owner{};void* last{};void* destination{};
        void* footer_header{};void* footer_data{};Word footer_bytes{};
        bool load_entered{},load_returned{},copy_entered{},apply_entered{},apply_returned{};
        bool swap_entered{},swap_copy_entered{},owner_stored{},footer_captured{};
        bool pool_entered{},pool_returned{},return_entered{},return_returned{},count_decremented{};
    };
    Storage() {} // Native HOME10/14/18 not value-zeroed.
    Word index_home,texture_home,offset_home;
    Match* matches{};
    NativeRenderObservedZeroFrame cpp_zero;
    ~Storage() { while(matches){auto* p=matches;matches=p->next;delete p;} }
};
NativeGuiIconDelayedDeliveryStorageFrame::NativeGuiIconDelayedDeliveryStorageFrame():storage_(new Storage) {}
NativeGuiIconDelayedDeliveryStorageFrame::~NativeGuiIconDelayedDeliveryStorageFrame()=default;

void deliver_native_gui_icon_delayed_storage_00ab51b0(void* actual,volatile Word (&homes)[2],
    NativeGuiIconDelayedDeliveryStorageFrame& frame,NativeGuiIconDelayedDeliveryStorageContext& c) {
    using Phase=NativeGuiIconDelayedDeliveryStorageFrame::Phase;
    using Storage=NativeGuiIconDelayedDeliveryStorageFrame::Storage;
    auto& a=frame.observation_;auto& f=*frame.storage_;
    require(a.phase==Phase::fresh,"Icon delivery requires a fresh retained frame");
    require(c.cache.strings.uses_raw_pool_cells(c.apply.icon.strings),
        "Icon delivery requires the SAME actual string/cache cells");
    a.phase=Phase::entered;a.actual_singleton=actual;a.argument_homes=&homes[0];
    Word delta=0;void* texture_register=nullptr;
    try {
        a.active_native_site=0x00ab51bd;
        const Word initial_count=read(actual,8); // BEFORE the explicit Native stores.
        put(f.texture_home,0);put(f.index_home,0);
        if(static_cast<std::int32_t>(initial_count)>0) {
            void* const second=pointer(homes[1]);a.second_name=second;
            put(f.offset_home,0);
            for(;;) {
                a.active_native_site=0x00ab51d6;
                void* const row=pointer(read(actual,4)+delta);
                const Word row_length=read(row);
                const Word second_length=read(second);
                bool matched=false;
                if(row_length==second_length) {
                    Word result;
                    if(row_length==0) result=second_length==0?0u:0xffffffffu;
                    else if(second_length==0) result=1;
                    else {
                        const Word second_data=read(second,4); // Before row DATA.
                        const Word row_data=read(row,4);
                        a.active_native_site=0x00ab5205;
                        result=static_cast<Word>(compare_native_crt_strings_00bf7fbf(
                            static_cast<const char*>(pointer(row_data)),static_cast<const char*>(pointer(second_data)),
                            c.apply.locale_changed_0109de1c,c.apply.stricmp_callees));
                    }
                    matched=result==0;
                }
                if(matched) {
                    auto* node=new Storage::Match;node->next=f.matches;f.matches=node;auto& call=*node;
                    if(!texture_register) {
                        a.active_native_site=0x00ab521e;
                        call.renderer=const_cast<void*>(c.cache.textures.current_renderer_00f8d394);
                        call.renderer_profile=read(call.renderer);
                        const auto* captured=table(c.apply.data,call.renderer_profile,0x68);
                        call.renderer_target=captured[0x64/4];
                        put(call.renderer_homes[1],bits(texture_register));put(call.renderer_homes[0],bits(second));
                        require(call.renderer_profile==0x00d5f0a8 && call.renderer_target==0x00b319b0,
                            "Icon delivery requires genuine captured renderer64/B319B0");
                        a.active_native_site=0x00ab522b;call.load_entered=true;
                        call.loaded=load_native_renderer_texture_00b319b0(call.renderer,pointer(live(call.renderer_homes[0])),
                            live(call.renderer_homes[1]),c.cache,&call.load);
                        call.load_returned=true;put(f.texture_home,bits(call.loaded));texture_register=call.loaded;
                    }
                    put(call.value_homes[2],bits(texture_register)); // PUSH texture before temp name construction.
                    const bool self=bits(call.value_homes)==bits(second); // CMP before MOV-zero stores.
                    put(call.value_homes[0],0);put(call.value_homes[1],0);
                    if(!self) {
                        const Word length=read(second);put(call.preserve_home,1);
                        a.active_native_site=0x00ab5255;call.copy_entered=true;
                        resize_native_string_header_0041dd40_exact(call.value_homes,c.apply.icon.strings,
                            length,&call.preserve_home,c.feature_word_0109eea4);
                        if(read(second)!=0) {
                            const Word bytes=live(call.value_homes[0]);
                            const Word source=read(second,4),destination=live(call.value_homes[1]);
                            a.active_native_site=0x00ab526a;
                            copy_native_crt_bytes_00bf7680(pointer(destination),pointer(source),bytes,c.feature_word_0109eea4);
                        }
                    }
                    // CURRENT queue base/opaque owner only AFTER the copy callbacks.
                    call.owner=pointer(read(pointer(read(actual,4)+delta),8));
                    a.active_native_site=0x00ab527b;call.apply_entered=true;
                    apply_native_gui_icon_texture_storage_00ab3840(call.owner,call.value_homes,call.apply,c.apply);
                    call.apply_returned=true;
                    if(live(f.index_home)!=0xffffffffu) {
                        const Word count=read(actual,8);
                        if(live(f.index_home)!=count-1u) {
                            call.swap_entered=true;
                            const Word base=read(actual,4);
                            call.last=pointer(base+count*12u-12u);call.destination=pointer(base+delta);
                            if(call.destination!=call.last) {
                                const Word length=read(call.last);put(call.preserve_home,1);
                                a.active_native_site=0x00ab52aa;call.swap_copy_entered=true;
                                resize_native_string_header_0041dd40_exact(call.destination,c.apply.icon.strings,
                                    length,&call.preserve_home,c.feature_word_0109eea4);
                                if(read(call.last)!=0) {
                                    const Word bytes=read(call.destination),source=read(call.last,4),destination=read(call.destination,4);
                                    a.active_native_site=0x00ab52bf;
                                    copy_native_crt_bytes_00bf7680(pointer(destination),pointer(source),bytes,c.feature_word_0109eea4);
                                }
                            }
                            const Word owner=read(call.last,8);write(call.destination,8,owner);call.owner_stored=true;
                            delta=live(f.offset_home); // ONLY swap branch restores this mutable HOME.
                        }
                        const Word current_count=read(actual,8);
                        const Word current_base=read(actual,4);
                        call.footer_header=pointer(current_base+current_count*12u-12u);
                        call.footer_data=pointer(read(call.footer_header,4));call.footer_captured=true;
                        if(call.footer_data) {
                            void* const captured_data=call.footer_data;
                            const Word bytes=read(call.footer_header)+1u;
                            put(call.footer_homes[2],1);put(call.footer_homes[1],bytes);put(call.footer_homes[0],bits(call.footer_data));
                            call.footer_bytes=bytes;
                            a.active_native_site=0x00ab52ee;call.pool_entered=true;
                            auto* const pool=native_string_pool_get_or_create_00419cc0(
                                c.apply.icon.strings.actual_published_01090aa8,c.apply.icon.strings.actual_manager_publication_01090aa0);
                            call.pool_returned=true;a.active_native_site=0x00ab52f5;call.return_entered=true;
                            return_native_string_pool_00bd1510(pool,captured_data,bytes,
                                c.apply.icon.strings.actual_small_returns_disabled_01090aa4);
                            call.return_returned=true;
                        }
                        write(actual,8,read(actual,8)-1u);call.count_decremented=true; // CURRENT count, no header zeroing.
                    }
                    texture_register=pointer(live(f.texture_home));
                } else {
                    put(f.index_home,live(f.index_home)+1u);delta+=12u;put(f.offset_home,delta);
                }
                const Word current_index=live(f.index_home);
                const Word current_count=read(actual,8);
                if(static_cast<std::int32_t>(current_index)>=static_cast<std::int32_t>(current_count)) break;
            }
            if(texture_register) {
                auto* const count_home=reinterpret_cast<volatile long*>(pointer(bits(texture_register)+4u));
                a.active_native_site=0x00ab5317;a.decrement_entered=true;
                const auto decrement=c.apply.decrement_00ce2220;
                a.decrement_result=decrement(count_home);a.decrement_returned=true;
                if(a.decrement_result==0) {
                    a.zero_profile=read(texture_register);a.zero_target=table(c.apply.data,a.zero_profile,4)[0];
                    require(a.zero_target==0x00bd30e0,"Icon delivery requires genuine CURRENT texture terminal0");
                    a.active_native_site=0x00ab5327;a.zero_entered=true;
                    NativeGuiIconDelayedDeliveryStorageObservedZeroIssuer::dispatch(c.apply.icon.actual_owners,texture_register,f.cpp_zero);
                    a.zero_returned=true; // No further texture payload read.
                }
            }
        }
        require(c.prepare_file_call!=nullptr,"Icon delivery requires the genuine cold Runtime footer supplier");
        a.file_call=&c.prepare_file_call(c.file_provider);
        auto& file=*a.file_call;
        require(file.phase_==NativeGuiIconDelayedDeliveryFileInvocation::Phase::fresh,"Icon FileStore footer cannot replay");
        file.validate_source_entry();file.phase_=NativeGuiIconDelayedDeliveryFileInvocation::Phase::entered;
        a.active_native_site=file.site_=0x00ab5329;
        a.factory=get_native_filestore_factory_004fc150(file.factory_);
        put(file.remove_home_,homes[0]); // AFTER factory getter, BEFORE zero-argument BE80B0.
        a.active_native_site=file.site_=0x00ab5335;
        a.provider=get_native_file_store_provider_00be80b0(a.factory,file.provider_);
        a.active_native_site=file.site_=0x00ab533c;
        remove_native_file_store_file_00be7130(a.provider,pointer(live(file.remove_home_)),file.strings_,file.streams_,file.invalid_);
        file.phase_=NativeGuiIconDelayedDeliveryFileInvocation::Phase::normal_returned;
        a.phase=Phase::normal_returned;
    } catch(...) {
        a.escaped_native_site=a.active_native_site;a.failure=std::current_exception();a.phase=Phase::unknown_source_escape;
        if(a.file_call && a.file_call->phase_==NativeGuiIconDelayedDeliveryFileInvocation::Phase::entered) {
            a.file_call->failure_=a.failure;a.file_call->phase_=NativeGuiIconDelayedDeliveryFileInvocation::Phase::unknown_source_escape;
        }
        throw; // Keep remaining actual string/texture/queue/stream debt and all backing.
    }
}
struct NativeGuiIconDelayedCompletionStorageFrame::Storage {
    Storage() {} // Native outgoing argument homes remain unwritten.
    Word outgoing[2];NativeGuiIconPreparedGetterCall* getter{};
    NativeGuiIconDelayedDeliveryStorageFrame delivery;
};
NativeGuiIconDelayedCompletionStorageFrame::NativeGuiIconDelayedCompletionStorageFrame():storage_(new Storage) {}
NativeGuiIconDelayedCompletionStorageFrame::~NativeGuiIconDelayedCompletionStorageFrame()=default;
void complete_native_gui_icon_file_loaded_storage_00ab58d0(volatile Word (&homes)[2],
    NativeGuiIconDelayedCompletionStorageFrame& frame,NativeGuiIconDelayedCompletionStorageContext& c) {
    using Phase=NativeGuiIconDelayedCompletionStorageFrame::Phase;
    auto& a=frame.observation_;auto& f=*frame.storage_;
    require(a.phase==Phase::fresh,"Icon completion requires a distinct fresh retained frame");
    require(&c.delayed==&c.delivery.apply.icon.delayed,"Icon completion requires the SAME actual delayed service");
    a.phase=Phase::entered;a.callback_homes=&homes[0];
    try {
        f.getter=&c.delayed.prepare_raw_getter_call(); // Cold fresh service-owned record, no Native call.
        const Word second=homes[1];const Word first=homes[0];
        put(f.outgoing[1],second);put(f.outgoing[0],first); // BOTH before actual getter.
        a.active_native_site=0x00ab58da;a.getter_entered=true;
        a.actual_singleton=f.getter->call_getter();a.getter_returned=true;
        a.active_native_site=0x00ab58e1;a.delivery_entered=true;
        deliver_native_gui_icon_delayed_storage_00ab51b0(a.actual_singleton,f.outgoing,f.delivery,c.delivery);
        a.delivery_returned=true;a.phase=Phase::normal_returned;
    } catch(...) {
        a.escaped_native_site=a.active_native_site;a.failure=std::current_exception();a.phase=Phase::unknown_source_escape;
        throw;
    }
}
} // namespace bsp
