#pragma once

#include "bsp/xlive_ipc.hpp"
#include "bsp/xlive_pipe_protocol.hpp"
#include <bitset>

namespace bsp {

// Borrow the actual initialized globals. F8B8F8 is a raw 72-byte region,
// not an encoded-value object with a skipped lock pointer.
struct XLivePipeFrameGlobals {
    CRITICAL_SECTION& acquisition_lock_f8b778;
    XLivePipeEncodedWideValue& value_f8b858;
    XLivePipeEncodedWideValue& value_f8b9a0;
    XLivePipeEncodedValue& value_f8bba0;
    XLivePipeEncodedValue& key_f8badc;
    const std::array<std::uint8_t, 72>& raw_f8b8f8;
};

// Backing bytes are not defined merely because C++ initialized their storage.
// A failed CryptGenRandom can reach the native consumer when last error is
// signed nonnegative; that branch needs all eight real preimage/output bytes.
struct XLivePipeRandomOutput {
    std::array<std::uint8_t, 8> bytes{};
    std::bitset<8> defined{};
};
struct XLivePipeFramePreimageHost {
    virtual ~XLivePipeFramePreimageHost() = default;
    virtual std::array<std::uint8_t, 72> temporary_wide_preimage() = 0;
    virtual XLivePipeRandomOutput random_output_preimage() = 0;
};
struct XLivePipeFrameSystemHost {
    virtual ~XLivePipeFrameSystemHost() = default;
    virtual std::array<std::uint8_t, 72> temporary_wide_preimage() = 0;
    virtual XLivePipeRandomOutput random_output_preimage() = 0;
    virtual XLivePipeValueLock* create_value_lock() = 0;
    virtual void set_last_error(DWORD) = 0;
    virtual DWORD get_last_error() = 0;
    virtual bool random_bytes(HCRYPTPROV, XLivePipeRandomOutput&) = 0;
};
class Win32XLivePipeFrameSystemHost final : public XLivePipeFrameSystemHost {
public:
    explicit Win32XLivePipeFrameSystemHost(XLivePipeFramePreimageHost& p) : preimages_(p) {}
    std::array<std::uint8_t, 72> temporary_wide_preimage() override;
    XLivePipeRandomOutput random_output_preimage() override;
    XLivePipeValueLock* create_value_lock() override;
    void set_last_error(DWORD) override;
    DWORD get_last_error() override;
    bool random_bytes(HCRYPTPROV, XLivePipeRandomOutput&) override;
private:
    XLivePipeFramePreimageHost& preimages_;
};

// Native generic stdcall RET24 bodies, with an explicit immutable-table view.
// Signed lengths, displacement and overlapping buffers retain native meaning;
// callers must supply valid bounded buffers (framing uses 40/72, displacement0).
void transform_xlive_pipe_seed_bits_00a5fa70(const std::uint8_t* seed,
    const std::uint8_t* encoded, std::uint8_t* output, const std::uint8_t* rows1,
    const std::uint8_t* rows2, std::int32_t displacement, std::int32_t left_size,
    std::int32_t right_size, std::int32_t output_size, const XLivePipeProtocolTables&);
void transform_xlive_pipe_packed_bits_00a5fd15(const std::uint8_t* left,
    const std::uint8_t* right, std::uint8_t* output, const std::uint8_t* rows1,
    const std::uint8_t* rows2, std::int32_t displacement, std::int32_t left_size,
    std::int32_t right_size, std::int32_t output_size, const XLivePipeProtocolTables&);

// The two predicates invert a packed signed-result test. Equality is unproven.
bool compare_xlive_pipe_value_00a5f25c(XLivePipeEncodedValue&, XLivePipeEncodedValue&,
    XLivePipeFrameGlobals&, const XLivePipeProtocolTables&);
bool compare_xlive_pipe_wide_value_00a5f2a4(XLivePipeEncodedWideValue&,
    XLivePipeEncodedWideValue&, XLivePipeFrameGlobals&, const XLivePipeProtocolTables&);

// Native thiscall ECX=prefix, (payload bytes,output*), RET8. Null protocol ->
// E_FAIL; bytes > 3B8h -> 80070008; otherwise output=bytes+48h. Output unchanged
// on failure. The argument is a payload capacity, not a channel identifier.
std::int32_t send_xlive_pipe_capacity_00a5f204(XLivePipeNativeState&,
    std::uint32_t, std::uint32_t*);
std::int32_t receive_xlive_pipe_capacity_00a5f230(XLivePipeNativeState&,
    std::uint32_t, std::uint32_t*);
std::int32_t send_xlive_pipe_capacity_00a5df96(XLivePipeTransport*,
    std::uint32_t, std::uint32_t*);
std::int32_t receive_xlive_pipe_capacity_00a5dfce(XLivePipeTransport*,
    std::uint32_t, std::uint32_t*);

// Native thiscall RET10h. Protocol captured before callback. Encode NEVER
// writes outer capacity; callback gets frame+72 and a local payload capacity.
// Decode invokes its callback before header validation/protocol mutation.
std::int32_t encode_xlive_pipe_frame_00a5f6c0(XLivePipeNativeState&, void*,
    std::uint32_t*, XLiveIpcEncodeCallback, void*, XLivePipeFrameGlobals&,
    const XLivePipeProtocolTables&, XLivePipeFrameSystemHost&);
std::int32_t decode_xlive_pipe_frame_00a5f7c5(XLivePipeNativeState&, const void*,
    std::uint32_t, XLiveIpcDecodeCallback, void*, XLivePipeFrameGlobals&,
    const XLivePipeProtocolTables&, XLivePipeFrameSystemHost&);
// Public stdcall RET14h wrappers: null/-1 owner first, then mode must be0,
// then reject null callback with nonnull context. No buffer/size-pointer guard.
std::int32_t encode_xlive_pipe_frame_00a5e055(XLivePipeTransport*, void*,
    std::uint32_t*, XLiveIpcEncodeCallback, void*, XLivePipeFrameGlobals&,
    const XLivePipeProtocolTables&, XLivePipeFrameSystemHost&);
std::int32_t decode_xlive_pipe_frame_00a5e09e(XLivePipeTransport*, const void*,
    std::uint32_t, XLiveIpcDecodeCallback, void*, XLivePipeFrameGlobals&,
    const XLivePipeProtocolTables&, XLivePipeFrameSystemHost&);

// Partial concrete host: transport open/close and actual I/O remain required
// abstract methods from XLiveIpcPipeHost. All bindings outlive operations.
class ReconstructedXLivePipeFramingHost : public XLiveIpcPipeHost {
public:
    ReconstructedXLivePipeFramingHost(XLivePipeFrameGlobals& g,
        const XLivePipeProtocolTables& t, XLivePipeFrameSystemHost& s)
        : globals_(g), tables_(t), system_(s) {}
    std::int32_t send_capacity_00a5df96(void*, std::uint32_t, std::uint32_t&) override;
    std::int32_t receive_capacity_00a5dfce(void*, std::uint32_t, std::uint32_t&) override;
    std::int32_t encode_00a5e055(void*, void*, std::uint32_t&,
        XLiveIpcEncodeCallback, void*) override;
    std::int32_t decode_00a5e09e(void*, const void*, std::uint32_t,
        XLiveIpcDecodeCallback, void*) override;
private:
    XLivePipeFrameGlobals& globals_;
    const XLivePipeProtocolTables& tables_;
    XLivePipeFrameSystemHost& system_;
};
} // namespace bsp
