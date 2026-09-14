#pragma once
#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_game_resource_classification.hpp"
#include "bsp/native_resource_stream_reads.hpp"
#include <cstdint>

namespace bsp {
// Actual item prefix: profile+0, references+4. B868B0 stamps CEB130, sets
// references=1, then stamps D631A0; B86890 stamps D5C104 then CEB130.
// Native ECX item, RET; constructor returns the captured item in EAX.
void* construct_native_resource_item_base_00b868b0(void*) noexcept;
void destroy_native_resource_item_base_00b86890(void*) noexcept;

// Actual GroupParams item is 0Ch: D634B0/refcount/float. Construction leaves
// +8 untouched. Reader: ECX item, stacked node handle, RET4; FSTP32 directly
// to +8, then explicitly skip/detach the node's remaining payload.
void* construct_native_group_params_item_00b8e5a0(void*) noexcept;
void read_native_group_params_item_00b8e580(void*, void* handle, NativeResourceStreamReadContext&);
// Parser ECX ignored; stacked handle, EAX item, RET4. Allocation cleanup is
// disarmed before the current item slot20 call. No cleanup on reader failure.
void* parse_native_group_params_item_00b8eb50(void* handle, NativeResourceStreamReadContext&);
void* delete_native_group_params_item_00b8ebc0(void*, std::uint32_t flags);

// Actual camera item is 18h: D632DC/refcount, two floats+8/+C, name header
// +10/+14. Parser allocates/constructs, disarms allocation cleanup, then reads.
// Reader preserves original x87 stores around host _CIatan and optional
// TargetName children. Its EH states own only completed child/name temporaries.
// Native reader ECX item, stacked handle, RET4; parser ECX ignored, EAX item.
void read_native_camera_item_00b8b0b0(void*, void* handle, NativeResourceStreamReadContext&);
void* parse_native_camera_item_00b8b240(void* handle, NativeResourceStreamReadContext&);
// Destructor returns captured name data without clearing its header, then
// destroys the base (also on name-return failure). Scalar deletes test bit0
// only after destruction and return the original pointer; native RET4.
void destroy_native_camera_item_00b8aa60(void*, NativeStringRawPoolContext&);
void* delete_native_camera_item_00b8b2c0(void*, std::uint32_t flags, NativeStringRawPoolContext&);

// ECX ignored; stacked token, AL Boolean (upper EAX unspecified), RET4.
// Borrow exactly three CURRENT cells at 010902E4 / 01090288 respectively.
std::uint8_t matches_native_group_params_type_00b8e5c0(std::uint32_t, const volatile std::uint32_t*);
std::uint8_t matches_native_camera_type_00b8aa30(std::uint32_t, const volatile std::uint32_t*);

// Bind captured parser/type targets into the existing raw root/game-resource
// dispatch. Other targets forward. Numeric native tables are identities;
// borrowed type cells retain their application's actual startup publication.
class NativeCameraGroupResourceCalls final : public NativeResourceDispatchCalls, public NativeResourceItemTypeCalls {
public:
    NativeCameraGroupResourceCalls(NativeResourceDispatchCalls&, NativeResourceItemTypeCalls&,
        NativeResourceStreamReadContext&, const volatile std::uint32_t* group_tokens_010902e4,
        const volatile std::uint32_t* camera_tokens_01090288);
    void renderer_hook(std::uintptr_t, void*) override;
    void* parse_item(std::uintptr_t, void*, void*) override;
    void append_item(std::uintptr_t, void*, void*) override;
    std::uint8_t matches_type(std::uintptr_t, void*, std::uint32_t) override;
private:
    NativeResourceDispatchCalls& other_;
    NativeResourceItemTypeCalls& types_;
    NativeResourceStreamReadContext& reads_;
    const volatile std::uint32_t* group_tokens_;
    const volatile std::uint32_t* camera_tokens_;
};

// Compose actual slot-release -> BD30E0 -> CURRENT slot4 -> scalar deletion.
// Forward stream/node/other owner operations to the supplied chain. This adds
// no resource-container destructor or owner/refcount field projection.
class NativeCameraGroupResourceReferences final : public NativeAdoptedSubstreamDispatch {
public:
    NativeCameraGroupResourceReferences(NativeAdoptedSubstreamDispatch&, NativeStringRawPoolContext&);
    std::uint8_t source_is_open(std::uintptr_t, void*) override;
    std::uint32_t source_seek(std::uintptr_t, void*, std::uint32_t, std::uint32_t, std::uint32_t) override;
    void source_read(std::uintptr_t, void*, void*, std::uint32_t, std::uint32_t*) override;
    void source_write(std::uintptr_t, void*, const void*, std::uint32_t, std::uint32_t*) override;
    void source_zero_reference(std::uintptr_t, void*, std::uintptr_t) override;
private:
    NativeAdoptedSubstreamDispatch& other_;
    NativeStringRawPoolContext& strings_;
};
// Source interfaces, not binary replacements. Native FH3, hardware faults,
// private frame aliases and the BF8490 CRT implementation remain boundaries.
} // namespace bsp
