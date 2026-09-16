#include "bsp/native_particle_type_texture_raw.hpp"
#include "bsp/native_particle_texture_names_raw.hpp"
#include "bsp/native_particle_type_property.hpp"
#include "bsp/native_particle_type_lifetime.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle texture loading requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
using Signed=std::int32_t;
void* at(void* p,Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p)+offset);
}
template<class T> T read(const void* p,Word offset=0) noexcept {
    T result;std::memcpy(&result,static_cast<const unsigned char*>(p)+offset,sizeof result);return result;
}
template<class T> void put(void* p,Word offset,T value) noexcept {
    std::memcpy(at(p,offset),&value,sizeof value);
}
void* initial_name(NativeParticleTypeTextureRawAcquired& a) noexcept {return a.native_locals_14_5b+8;}
void* frame_name(NativeParticleTypeTextureRawAcquired& a) noexcept {return a.native_locals_14_5b;}
void return_captured_name(void* name,NativeStringRawPoolContext& strings) {
    void* const data=read<void*>(name,4);
    if(!data)return;
    const Word bytes=read<Word>(name)+1u;
    auto* const pool=native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool,data,bytes,strings.actual_small_returns_disabled_01090aa4);
}
void unwind_name(NativeParticleTypeTextureRawAcquired& a,NativeStringRawPoolContext& strings) noexcept {
    void* const name=a.unwind_state==0?initial_name(a):frame_name(a);
    a.unwind_state=-1;
    destroy_native_string_header_0041dd20(name,strings);
}
}

bool load_native_particle_type_texture_00b01350(void* definition,const char* filename,
    NativeParticleTypeTextureRawContext& c,NativeParticleTypeTextureRawAcquired& a) {
    using Phase=NativeParticleTypeTextureRawAcquired::Phase;
    if(a.phase!=Phase::fresh||a.cache.phase!=NativeTextureCacheAcquired::Phase::not_started||a.cache.wrapper_started)
        throw std::logic_error("Particle texture loading cannot replay an acquired operation");
    a.phase=Phase::running;
    if(!filename||std::strlen(filename)==0){a.phase=Phase::complete;return false;}
    auto& strings=c.names.strings;
    void* const descriptor=at(definition,0x68);
    void* const first=a.native_locals_14_5b+0x10;
    void* const later=a.native_locals_14_5b+0x2c;
    put(first,0x18,c.first_record_stack_word18);
    put(later,0x18,c.later_record_stack_word18);
    try {
        a.native_site=0x00b013a6;
        clear_native_particle_type_records_00b00f30(descriptor);
        a.native_site=0x00b013b2;
        void* const item=find_native_particle_atlas_item_00aefb20(
            c.names.actual_atlas_manager_00f8c26c,filename,strings,c.names.null_pattern_00e17bf0);
        if(item) {
            a.native_site=0x00b0154e;
            construct_native_particle_uv_record_00b00880(first,at(item,0x14),at(item,0x1c),c.half_import);
            a.native_site=0x00b01556;
            append_native_particle_type_record_00b00ee0(descriptor,first);
        } else {
            a.native_site=0x00b013c4;
            construct_native_string_header_0041e870(initial_name(a),strings,filename);
            if(!c.cache||!c.owners)
                throw std::logic_error("Particle texture atlas miss requires the actual cache/owner domain");
            void* const renderer=const_cast<void*>(c.cache->textures.current_renderer_00f8d394);
            a.native_site=0x00b013d1;
            if(read<Word>(renderer)!=0x00d5f0a8u||!c.renderer_profile_00d5f0a8||
                c.renderer_profile_00d5f0a8[0x64/4]!=0x00b319b0u)
                throw std::logic_error("Particle texture loading requires current renderer slot64 B319B0");
            a.unwind_state=0;
            a.native_site=0x00b013e3;
            a.texture=load_native_renderer_texture_00b319b0(renderer,initial_name(a),0,*c.cache,&a.cache);
            a.captured_texture=a.texture;
            a.unwind_state=-1;
            a.native_site=0x00b01409;
            return_captured_name(initial_name(a),strings);
            put(first,0,Word{0});put(first,4,Word{0});
            const Word one=*c.one_00d7a24c;
            put(first,8,one);put(first,12,one);
            a.native_site=0x00b0143d;
            c.half_import.convert(static_cast<std::uint16_t*>(at(first,0x10)),static_cast<const float*>(first),4);
            a.native_site=0x00b01449;
            append_native_particle_type_record_00b00ee0(descriptor,first);
            void* const texture=a.texture;
            a.texture=nullptr;
            a.cache.caller_acquired=false;
            a.native_site=0x00b01452;
            release_native_render_actual_owner(*c.owners,texture);
        }
        a.native_site=0x00b01466;
        const Signed count=count_native_particle_texture_frames_00af3a20(filename,c.names);
        if(count>1) {
            a.native_site=0x00b01479;
            reserve_native_particle_type_records_00b00c20(descriptor,count);
            for(Signed index=1;index<count;++index) {
                a.native_site=0x00b01497;
                construct_native_particle_texture_frame_name_00af3b50(frame_name(a),filename,index,c.names);
                a.unwind_state=1;
                const char* const data=read<const char*>(frame_name(a),4);
                a.native_site=0x00b014b8;
                void* const next=find_native_particle_atlas_item_00aefb20(
                    c.names.actual_atlas_manager_00f8c26c,data?data:c.empty_texture_name_00f8d37c,
                    strings,c.names.null_pattern_00e17bf0);
                if(next) {
                    // Four MOVSS stores retain bit patterns, unlike the first
                    // atlas record's x87 B00880 producer (including sNaNs).
                    for(Word offset=0;offset<16;offset+=4)put(later,offset,read<Word>(next,0x14+offset));
                    a.native_site=0x00b014fd;
                    c.half_import.convert(static_cast<std::uint16_t*>(at(later,0x10)),static_cast<const float*>(later),4);
                    a.native_site=0x00b01509;
                    append_native_particle_type_record_00b00ee0(descriptor,later);
                }
                a.unwind_state=-1;
                a.native_site=next?0x00b01530:0x00b01582;
                return_captured_name(frame_name(a),strings);
                if(!next)break;
            }
        }
        auto value=static_cast<Signed>(read<Word>(definition,0x6c)-1u);
        const auto upper=read<Signed>(definition,0x54);
        if(upper<value)value=upper;
        put(definition,0x54,value);
        const auto lower=read<Signed>(definition,0x50);
        if(lower<=value)value=lower;
        put(definition,0x50,value);
    } catch(...) {
        a.phase=Phase::failed;
        if(a.unwind_state>=0)unwind_name(a,strings);
        throw;
    }
    a.phase=Phase::complete;
    return true;
}
} // namespace bsp
