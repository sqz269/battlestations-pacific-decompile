#include "bsp/native_render_observed_zero.hpp"
namespace bsp {
class NativeGuiIconVisibilityStorageObservedZeroIssuer final {
public:
    static void dispatch(NativeRenderActualOwners& owners, void* actual, NativeRenderObservedZeroFrame& frame) {
        NativeRenderObservedZeroAccess::dispatch(owners, actual, frame);
    }
};
}
#include "bsp/native_gui_icon_visibility_storage.hpp"
#include "bsp/native_gui_icon_storage.hpp"
#include "bsp/native_gui_icon_delayed_loads.hpp"
#include "bsp/native_gui_icon_placeholder_storage.hpp"
#include "bsp/native_gui_icon_rebuild_storage.hpp"
#include "bsp/native_renderer_texture_find_storage.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include <stdexcept>
#include <optional>
namespace bsp {
namespace {
using Word=std::uint32_t;
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
void* at(const void* p, Word d=0) noexcept { return pointer(bits(p)+d); }
Word read(const void* p, Word d=0) noexcept { return *static_cast<const volatile Word*>(at(p,d)); }
Word live(const Word& h) noexcept { return static_cast<const volatile Word&>(h); }
void put(Word& h,Word v) noexcept { static_cast<volatile Word&>(h)=v; }
Word count(Word last,Word first) noexcept { return static_cast<Word>(static_cast<std::int32_t>(last-first)>>6); }
void require(bool v,const char* s) { if(!v) throw std::logic_error(s); }
void checked(void* actual,Word first,Word index) {
    if(first==0 || index>=count(read(actual,0xf8),first)) invoke_native_invalid_parameter_00bf6713();
}
const volatile Word* table(const game::GameNativeReadOnlyData& data,Word profile,Word size) {
    const auto* result=static_cast<const volatile Word*>(data.data_at(profile,size));
    require(bits(result)==profile,"Icon visibility requires genuine CURRENT mapped table alias");
    return result;
}
std::uint8_t visible(const volatile Word& home) noexcept {
    return *reinterpret_cast<const volatile std::uint8_t*>(&home);
}
}
struct NativeGuiIconVisibilityStorageFrame::Storage {
    struct Show {
        Show() : find_acquired(cache) {} // No native frame/home initialization.
        Show* next{};
        NativeTextureCacheAcquired cache;
        NativeRendererTextureFindStorageFrame find_frame;
        NativeRendererTextureFindStorageAcquired find_acquired;
        NativeGuiIconTextureApplyStorageFrame apply;
        NativeGuiIconDelayedRequestStorageFrame request;
        NativeGuiIconPreparedGetterCall* getter{};
        Word find_homes[2], value_homes[3], preserve_home;
        void* renderer{}; Word renderer_profile{}, renderer_target{};
        void* texture{}; void* name_source{};
        bool find_entered{},find_returned{},copy_returned{},apply_entered{},apply_returned{};
        bool getter_entered{},getter_returned{},request_entered{},request_returned{};
        NativeGuiIconDelayedLoadsStorage* singleton{};
        NativeTextureSurfaceReferenceIncrement captured_decrement{};
        long decrement_result{}; Word zero_profile{},zero_target{};
        bool decrement_entered{},decrement_returned{},zero_entered{},zero_returned{};
        NativeRenderObservedZeroFrame cpp_zero;
    };
    struct Hide {
        Hide() {} // Native placeholder/home backing remains unseeded.
        Hide* next{};
        NativeTextureCacheAcquired cache;
        NativeRenderActualOwnerReleaseObservation old_release;
        NativeGuiIconPlaceholderStorageFrame placeholder;
        std::optional<NativeGuiIconPlaceholderStorageContext> context;
        std::unique_ptr<NativeGuiIconTextureApplyRebuildInvocation> rebuild;
        void* state{}; Word profile{},target{};
        bool placeholder_entered{},placeholder_returned{},preparation_entered{},rebuild_entered{},rebuild_returned{};
    };
    Storage() {} // Ordinary Native spill/state words are not value-zeroed.
    Word loop_homes[2],native_state;
    Show* shows{};Hide* hides{};
    ~Storage() {
        while(shows){auto* p=shows;shows=p->next;delete p;}
        while(hides){auto* p=hides;hides=p->next;delete p;}
    }
};
NativeGuiIconVisibilityStorageFrame::NativeGuiIconVisibilityStorageFrame() : storage_(new Storage) {}
NativeGuiIconVisibilityStorageFrame::~NativeGuiIconVisibilityStorageFrame()=default;

void native_gui_icon_visibility_storage_00ab6120(void* actual,const volatile Word& visible_home,
    NativeGuiIconVisibilityStorageFrame& frame,NativeGuiIconVisibilityStorageContext& c) {
    using Storage=NativeGuiIconVisibilityStorageFrame::Storage;
    auto& a=frame.observation_;auto& f=*frame.storage_;
    require(a.phase==NativeGuiIconVisibilityStorageFrame::Phase::fresh,"Icon visibility frame cannot replay");
    require(&c.apply.icon.delayed==&c.delayed && &c.find.strings==&c.apply.icon.strings &&
        &c.request.strings==&c.apply.icon.strings,"Icon visibility requires the SAME existing service/string graph");
    a.phase=NativeGuiIconVisibilityStorageFrame::Phase::entered;
    a.actual_icon=actual;a.visible_home=&visible_home;
    Word index=0,delta=0;put(f.native_state,0xffffffffu);
    try {
        for(;;) {
            a.active_native_site=0x00ab6142;
            const Word first=read(actual,0xf4);
            put(f.loop_homes[0],delta);put(f.loop_homes[1],index);
            if(first==0 || index>=count(read(actual,0xf8),first)) break;
            checked(actual,first,index);
            const Word flag_base=read(actual,0xf4);
            const auto pending=*static_cast<const volatile std::uint8_t*>(pointer(flag_base+delta+0x3cu));
            if(pending!=0 && visible(visible_home)!=0) {
                auto* fresh=new Storage::Show;
                fresh->next=f.shows;f.shows=fresh;auto& call=*fresh;
                // Explicit initialized private Source ABI inputs, not defaults.
                put(call.find_frame.guard_token,c.find_private_preimages[0]);
                put(call.find_frame.guard_renderer,c.find_private_preimages[1]);
                a.active_native_site=0x00ab619f;
                call.renderer=const_cast<void*>(c.find.cache.textures.current_renderer_00f8d394);
                checked(actual,flag_base,index);
                const Word name_base=read(actual,0xf4);
                call.renderer_profile=read(call.renderer);
                const auto* renderer_table=table(c.apply.data,call.renderer_profile,0x6c);
                const Word name= name_base+delta+4u;
                put(call.find_homes[1],0);put(call.find_homes[0],name);
                call.renderer_target=renderer_table[0x68/4];
                require(call.renderer_profile==0x00d5f0a8 && call.renderer_target==0x00b31ac0,
                    "Icon visibility requires genuine captured renderer find68");
                a.active_native_site=0x00ab61d4;call.find_entered=true;
                call.texture=find_native_renderer_texture_storage_00b31ac0(call.renderer,
                    call.find_homes,c.find,call.find_frame,call.find_acquired);
                call.find_returned=true;
                const Word post_find_base=read(actual,0xf4); // BEFORE testing returned texture.
                if(call.texture) {
                    put(call.value_homes[2],bits(call.texture));
                    checked(actual,post_find_base,index);
                    call.name_source=pointer(read(actual,0xf4)+live(f.loop_homes[0])+4u);
                } else {
                    put(call.value_homes[2],bits(actual));
                    checked(actual,post_find_base,index);
                    call.name_source=pointer(read(actual,0xf4)+live(f.loop_homes[0])+4u);
                }
                put(call.value_homes[0],0);put(call.value_homes[1],0);
                if(bits(call.value_homes)!=bits(call.name_source)) {
                    const Word length=read(call.name_source);
                    put(call.preserve_home,1);
                    a.active_native_site=call.texture?0x00ab622f:0x00ab62bf;
                    resize_native_string_header_0041dd40_exact(call.value_homes,c.apply.icon.strings,
                        length,&call.preserve_home,c.find.feature_word_0109eea4);
                    if(read(call.name_source)!=0) {
                        const Word bytes=live(call.value_homes[0]);
                        const Word source=read(call.name_source,4);
                        const Word destination=live(call.value_homes[1]);
                        a.active_native_site=call.texture?0x00ab6245:0x00ab62d4;
                        copy_native_crt_bytes_00bf7680(pointer(destination),pointer(source),bytes,c.find.feature_word_0109eea4);
                    }
                }
                call.copy_returned=true;
                if(call.texture) {
                    a.active_native_site=0x00ab624f;call.apply_entered=true;
                    apply_native_gui_icon_texture_storage_00ab3840(actual,call.value_homes,call.apply,c.apply);
                    call.apply_returned=true;
                    auto* const count_home=static_cast<volatile long*>(at(call.texture,4));
                    a.active_native_site=0x00ab6258;
                    const auto decrement=c.apply.decrement_00ce2220;
                    call.captured_decrement=decrement;call.decrement_entered=true;
                    const long result=decrement(count_home);
                    call.decrement_result=result;call.decrement_returned=true;
                    if(result==0) {
                        call.zero_profile=read(call.texture);
                        call.zero_target=table(c.apply.data,call.zero_profile,4)[0];
                        require(call.zero_target==0x00bd30e0,"Icon visibility requires genuine captured texture current0");
                        a.active_native_site=0x00ab6268;call.zero_entered=true;
                        NativeGuiIconVisibilityStorageObservedZeroIssuer::dispatch(c.apply.icon.actual_owners,
                            call.texture,call.cpp_zero);
                        call.zero_returned=true; // No further texture payload access.
                    }
                } else {
                    put(f.native_state,0);
                    call.getter=&c.delayed.prepare_raw_getter_call(); // Fresh own service record; no Native call.
                    a.active_native_site=0x00ab62e0;call.getter_entered=true;
                    call.singleton=call.getter->call_getter();
                    call.getter_returned=true;put(f.native_state,0xffffffffu);
                    a.active_native_site=0x00ab62ef;call.request_entered=true;
                    request_native_gui_icon_delayed_loads_storage_00ab5f00(call.singleton,
                        call.value_homes,call.request,c.request);
                    call.request_returned=true;
                }
                index=live(f.loop_homes[1]);delta=live(f.loop_homes[0]);
            } else {
                checked(actual,flag_base,index);
                const Word hide_base=read(actual,0xf4);
                const auto delayed=*static_cast<const volatile std::uint8_t*>(pointer(hide_base+delta+0x3du));
                if(delayed!=0 && visible(visible_home)==0) {
                    auto* fresh=new Storage::Hide;fresh->next=f.hides;f.hides=fresh;auto& call=*fresh;
                    checked(actual,hide_base,index);
                    call.state=pointer(read(actual,0xf4)+delta);
                    call.context.emplace(NativeGuiIconPlaceholderStorageContext{c.apply.icon,c.find.cache,call.cache,
                        call.old_release,c.find.feature_word_0109eea4,c.transparent_literal_00d5c058});
                    a.active_native_site=0x00ab635f;call.placeholder_entered=true;
                    set_native_gui_icon_placeholder_storage_00ab3b30(call.state,call.placeholder,*call.context);
                    call.placeholder_returned=true;
                    const auto selected=*static_cast<const volatile std::int16_t*>(at(actual,0xec));
                    if(index==static_cast<Word>(static_cast<std::int32_t>(selected))) {
                        call.preparation_entered=true;
                        require(c.apply.prepare_rebuild!=nullptr,"Icon visibility requires genuine cold raw80 preparer");
                        call.rebuild=c.apply.prepare_rebuild(c.apply.rebuild_provider,actual);
                        require(call.rebuild && &call.rebuild->frame.context==&call.rebuild->context &&
                            &call.rebuild->context.owners==&c.apply.icon.actual_owners,
                            "Icon visibility raw80 must retain the SAME actual owner graph");
                        call.profile=read(actual);call.target=table(c.apply.data,call.profile,0x84)[0x80/4];
                        require(call.profile==0x00d5c4c0 && call.target==0x00ab3cb0,"Icon visibility requires genuine captured default80");
                        put(call.rebuild->state_home,index);
                        a.active_native_site=0x00ab637e;call.rebuild_entered=true;
                        rebuild_native_gui_icon_geometry_storage_00ab3cb0(actual,call.rebuild->state_home,
                            call.rebuild->frame,call.rebuild->context);
                        call.rebuild_returned=true;
                    }
                }
                // Hide preserves register index/delta across calls: no spill reload.
            }
            ++index;delta+=0x40u;
        }
        a.phase=NativeGuiIconVisibilityStorageFrame::Phase::normal_returned;
    } catch(...) {
        a.escaped_native_site=a.active_native_site;a.failure=std::current_exception();
        a.phase=NativeGuiIconVisibilityStorageFrame::Phase::unknown_source_escape;
        throw; // Retain all actual debt, no guessed Native FH3 string/texture action.
    }
}
}
