#include "bsp/native_resource_stream_reads.hpp"
#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource stream reads require MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* ptr(U v) noexcept { return reinterpret_cast<void*>(v); }
U word(const void* p,U n=0) noexcept { return *static_cast<const volatile U*>(ptr(bits(p)+n)); }
void put(void* p,U n,U v) noexcept { *static_cast<volatile U*>(ptr(bits(p)+n))=v; }
U scalar(void* stream,U* actual,NativeAdoptedSubstreamDispatch& calls) {
    U slot=bits(actual);
    const auto target=word(ptr(word(stream)),0x24);
    calls.source_read(target,stream,&slot,4,actual);
    return slot;
}
U scalar_entry(U target,void* stream,U* actual,NativeResourceStreamReadContext& context) {
    if(target==0x00be42e0) return read_native_stream_dword_slot34_00be42e0(stream,actual,context.streams);
    if(target==0x00be4300) return read_native_stream_dword_slot38_00be4300(stream,actual,context.streams);
    throw std::runtime_error("Reached native stream scalar target is not reconstructed");
}
void give_back(void* data,U bytes,NativeStringRawPoolContext& context) {
    auto* const pool=native_string_pool_get_or_create_00419cc0(context.actual_published_01090aa8,context.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool,data,bytes,context.actual_small_returns_disabled_01090aa4);
}
}
U read_native_stream_dword_slot34_00be42e0(void* stream,U* actual,NativeAdoptedSubstreamDispatch& calls) { return scalar(stream,actual,calls); }
U read_native_stream_dword_slot38_00be4300(void* stream,U* actual,NativeAdoptedSubstreamDispatch& calls) { return scalar(stream,actual,calls); }

void* read_native_stream_string_00be4620(void* stream,void* output,U* actual,NativeResourceStreamReadContext& context) {
    const auto target=word(ptr(word(stream)),0x38);
    U prefix_actual=0;
    const auto length=scalar_entry(target,stream,&prefix_actual,context);
    U temporary[2]{0,0};bool temporary_armed=true,output_complete=false;
    try {
        if(length==0) {
            if(actual) put(actual,0,prefix_actual);
            const bool identical=output==temporary;put(output,0,0);put(output,4,0);
            if(!identical) resize_native_string_header_0041dd40(output,context.strings,0,true);
            return output;
        }
        resize_native_string_header_0041dd40(temporary,context.strings,length,true);
        const auto count=word(temporary);auto* const data=ptr(word(temporary,4));
        if(count!=0) std::memset(data,0x20,count);
        U payload_actual=0;
        auto* const destination=data ? data : context.actual_empty_string_storage_0109db64;
        const auto read_target=word(ptr(word(stream)),0x24);
        context.streams.source_read(read_target,stream,destination,length,&payload_actual);
        if(actual) put(actual,0,payload_actual+prefix_actual);
        const bool identical=output==temporary;put(output,0,0);put(output,4,0);
        if(!identical) {
            resize_native_string_header_0041dd40(output,context.strings,count,true);
            if(count!=0) {
                const auto current_count=word(output);auto* const current_data=ptr(word(output,4));
                if(current_count!=0) std::memmove(current_data,data,current_count);
            }
        }
        output_complete=true;temporary_armed=false;
        if(data) give_back(data,count+1u,context.strings);
        return output;
    } catch(...) {
        // FH3 state1 ->0 destroys the current temp; state0 tests/clears the
        // completed-output flag. A second exception during unwind terminates.
        try {
            if(temporary_armed) destroy_native_string_header_0041dd20(temporary,context.strings);
            if(output_complete) { output_complete=false;destroy_native_string_header_0041dd20(output,context.strings); }
        } catch(...) { std::terminate(); }
        throw;
    }
}
U read_native_resource_dword_00bf0280(void* reader,U* budget,NativeResourceStreamReadContext& context) {
    U actual=bits(reader);auto* const stream=ptr(word(reader));
    const auto target=word(ptr(word(stream)),0x34);
    const auto result=scalar_entry(target,stream,&actual,context);
    put(budget,0,word(budget)-actual);
    return result;
}
void* read_native_resource_string_00bf0510(void* reader,void* output,U* budget,NativeResourceStreamReadContext& context) {
    auto* const stream=ptr(word(reader));const auto target=word(ptr(word(stream)),0x48);
    U actual=bits(output);
    if(target!=0x00be4620) throw std::runtime_error("Reached native stream string target is not reconstructed");
    read_native_stream_string_00be4620(stream,output,&actual,context);
    put(budget,0,word(budget)-actual);
    return output;
}
}
