#pragma once

#include "bsp/xlive_pipe_transport.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <wincrypt.h>

namespace bsp {

// Native slots are deleting destructor, enter, leave. New C++ interface, not ABI.
struct XLivePipeValueLock {
    virtual XLivePipeValueLock* destroy(std::uint32_t flags) = 0;
    virtual void enter() = 0;
    virtual void leave() = 0;
protected:
    ~XLivePipeValueLock() = default;
};
class Win32XLivePipeValueLock final : public XLivePipeValueLock {
public:
    Win32XLivePipeValueLock(); // A5FA27: initialize actual critical section
    ~Win32XLivePipeValueLock(); // A5F91E: delete section
    XLivePipeValueLock* destroy(std::uint32_t flags) override; // A5FA3E
    void enter() override; // A5F939
    void leave() override; // A5F944
    CRITICAL_SECTION& native_section() noexcept { return section_04; }
private:
    CRITICAL_SECTION section_04;
};
XLivePipeValueLock* create_xlive_pipe_value_lock_00a5fa5a();

struct XLivePipeEncodedValue {
    XLivePipeValueLock* lock;
    std::array<std::uint8_t, 40> bytes;
};
struct XLivePipeEncodedWideValue {
    XLivePipeValueLock* lock;
    std::array<std::uint8_t, 72> bytes;
};
struct XLivePipeProtocolNativeState {
    HCRYPTPROV provider_00;
    XLivePipeEncodedValue seeded_04;
    XLivePipeEncodedWideValue pending_30;
    XLivePipeEncodedValue key_7c;
};

// The source is the actual new, default-initialized allocation before any lock
// construction or payload writes. Observe its machine representation through
// the explicit capture boundary; do not read indeterminate members in C++.
// Recording hosts may supply explicit fixture bytes. Production must not invent
// zero padding: constructor writes only lock pointers; pending bytes survive.
using XLivePipeProtocolAllocationPreimage = std::array<std::uint8_t, 0xa8>;
struct XLivePipeProtocolPreimageHost {
    virtual ~XLivePipeProtocolPreimageHost() = default;
    virtual XLivePipeProtocolAllocationPreimage context_allocation_preimage(
        const XLivePipeProtocolNativeState& actual_allocation) = 0;
};

// Borrow one canonical already-initialized global section and both global
// encoded values. Static initialization at CD6E62/CD6E98 is separate work.
struct XLivePipeProtocolGlobals {
    CRITICAL_SECTION& acquisition_lock_f8b778;
    XLivePipeEncodedValue& seed_f8b8cc;
    XLivePipeEncodedValue& key_f8badc;
};
void enter_xlive_pipe_global_00a5f94f(CRITICAL_SECTION&);
void leave_xlive_pipe_global_00a5f95b(CRITICAL_SECTION&);

// Checked immutable original-image table view. Exact 200980 bytes from
// VA D25D9C / RVA 925D9C through exclusive VA D56EB0 (includes framing rows).
// SHA-256 f3456122844489dc39ceee393fc4ecfaddf789449bddf89b434b2cd55fe5c77c.
// This borrowed view must outlive all operations; the caller retains the bytes.
class XLivePipeProtocolTables {
public:
    static constexpr std::uint32_t first_va = 0x00d25d9c;
    static constexpr std::size_t size = 200980;
    XLivePipeProtocolTables(const std::uint8_t* bytes, std::size_t length);
    const std::uint8_t* bytes(std::uint32_t va, std::size_t count) const;
    std::uint32_t word(std::uint32_t base, std::uint32_t index) const;
private:
    const std::uint8_t* data_;
};

// Generic native RET24 transform bodies. Signed minimum of three lengths;
// rows contain little-endian signed16 indices. Valid bounded native buffers
// are required. In-place aliases and the first-byte/displacement case survive.
void transform_xlive_pipe_seed_00a5fbc3(const std::uint8_t* left,
    const std::uint8_t* seed, std::uint8_t* output, const std::uint8_t* rows1,
    const std::uint8_t* rows2, std::int32_t displacement, std::int32_t left_size,
    std::int32_t right_size, std::int32_t output_size, const XLivePipeProtocolTables&);
void transform_xlive_pipe_pair_00a5fe80(const std::uint8_t* left,
    const std::uint8_t* right, std::uint8_t* output, const std::uint8_t* rows1,
    const std::uint8_t* rows2, std::int32_t displacement, std::int32_t left_size,
    std::int32_t right_size, std::int32_t output_size, const XLivePipeProtocolTables&);
void copy_xlive_pipe_seed_constant_00a5ebc3(std::uint8_t*, const XLivePipeProtocolTables&);
void copy_xlive_pipe_key_constant_00a5ed17(std::uint8_t*, const XLivePipeProtocolTables&);
void mix_xlive_pipe_seed_00a60123(const XLivePipeEncodedValue&, std::uint32_t,
    std::uint8_t*, const XLivePipeProtocolTables&);
void combine_xlive_pipe_seed_00a6014e(const std::uint8_t*,
    const XLivePipeEncodedValue&, XLivePipeEncodedValue&, const XLivePipeProtocolTables&);
void mix_xlive_pipe_key_00a60259(const XLivePipeEncodedValue&,
    std::uint8_t*, const XLivePipeProtocolTables&);
void combine_xlive_pipe_key_00a60284(const std::uint8_t*,
    const XLivePipeEncodedValue&, XLivePipeEncodedValue&, const XLivePipeProtocolTables&);
void initialize_xlive_pipe_seed_00a5ed9f(XLivePipeEncodedValue& source,
    std::uint32_t seed, XLivePipeEncodedValue& output, CRITICAL_SECTION& global,
    const XLivePipeProtocolTables&);
void initialize_xlive_pipe_key_00a5eecb(XLivePipeEncodedValue& source,
    XLivePipeEncodedValue& output, CRITICAL_SECTION& global, const XLivePipeProtocolTables&);

struct XLivePipeProtocolSystemHost {
    virtual ~XLivePipeProtocolSystemHost() = default;
    virtual XLivePipeProtocolNativeState* allocate_context() = 0;
    virtual void free_context(XLivePipeProtocolNativeState*) noexcept = 0;
    virtual XLivePipeValueLock* create_value_lock() = 0;
    virtual void set_last_error(DWORD) = 0;
    virtual DWORD get_last_error() = 0;
    virtual bool acquire_context(HCRYPTPROV&) = 0;
    virtual bool random_seed(HCRYPTPROV, std::uint32_t&) = 0;
    virtual void release_context(HCRYPTPROV) = 0;
};
class Win32XLivePipeProtocolSystemHost : public XLivePipeProtocolSystemHost {
public:
    explicit Win32XLivePipeProtocolSystemHost(XLivePipeProtocolPreimageHost& preimages)
        : preimages_(preimages) {}
    XLivePipeProtocolNativeState* allocate_context() override;
    void free_context(XLivePipeProtocolNativeState*) noexcept override;
    XLivePipeValueLock* create_value_lock() override;
    void set_last_error(DWORD) override;
    DWORD get_last_error() override;
    bool acquire_context(HCRYPTPROV&) override;
    bool random_seed(HCRYPTPROV, std::uint32_t&) override;
    void release_context(HCRYPTPROV) override;
private:
    XLivePipeProtocolPreimageHost& preimages_;
};

XLivePipeProtocolNativeState* construct_xlive_pipe_protocol_00a5f336(
    XLivePipeProtocolNativeState&, XLivePipeProtocolSystemHost&);
void destroy_xlive_pipe_protocol_members_00a5f1d8(XLivePipeProtocolNativeState&);
void initialize_xlive_pipe_protocol_00a5f416(void*& slot, XLivePipeProtocolGlobals&,
    const XLivePipeProtocolTables&, XLivePipeProtocolSystemHost&);
void destroy_xlive_pipe_protocol_00a5f371(void* protocol, XLivePipeProtocolSystemHost&);

// Concrete recovered protocol lifecycle, not a success placeholder. Required
// bindings remain explicit: original tables, live globals, allocator preimages.
class ReconstructedXLivePipeProtocolHost final : public XLivePipeProtocolHost {
public:
    ReconstructedXLivePipeProtocolHost(XLivePipeProtocolGlobals& globals,
        const XLivePipeProtocolTables& tables, XLivePipeProtocolSystemHost& system)
        : globals_(globals), tables_(tables), system_(system) {}
    void initialize_protocol_00a5f416(void*& slot) override;
    void destroy_protocol_00a5f371(void* protocol) override;
private:
    XLivePipeProtocolGlobals& globals_;
    const XLivePipeProtocolTables& tables_;
    XLivePipeProtocolSystemHost& system_;
};
} // namespace bsp
