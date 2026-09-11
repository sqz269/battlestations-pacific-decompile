#pragma once

#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {

// Full 00B34120. Native ECX=owner, stack=name/borrowed COM/flags, EAX=owner,
// RET 0Ch. The actual 24h-byte owner prefix stores profile/count at +00/+04,
// an eight-byte name at +08, borrowed COM at +10, flags at +1C, serial at +20.
// Bytes +14..+1B and storage after +24 are untouched. An existing name is
// abandoned by construction; the source may alias any accessible owner bytes.
// The final current-buffer copy preserves BF7680's backward-overlap handling.
// Pass the actual shared texture serial DWORD corresponding to 0108D6E8,
// including for callers of the unnamed constructors. It is incremented with
// DWORD wrap only after the current source/header copy and COM/flags stores.
// Pool A is supplied by PooledStringStorage over the shared SizedStoragePool.
// These explicit C++ interfaces do not expose the original calling convention.
void* construct_native_logical_texture_named_base_00b34120(void* actual_owner,
    const void* actual_name_header, void* borrowed_com, std::uint32_t flags,
    NativeStringStorage&, std::uint32_t& actual_shared_serial_0108d6e8);

// Full 00B34230, same native ABI. Run the complete base constructor, then set
// profile D5F228. Failure leaves the base constructor's unwound owner state.
void* construct_native_logical_texture_named_profile_00b34230(void* actual_owner,
    const void* actual_name_header, void* borrowed_com, std::uint32_t flags,
    NativeStringStorage&, std::uint32_t& actual_shared_serial_0108d6e8);

// Full 00B33F50 and the full five-byte JMP entry 00B34010; native ECX/RET.
// Install D5F1F4; return captured nonnull name data with current length+1;
// leave both name fields (including release-time changes) intact; install
// CEB130 through the complete BD30F0 base action. No COM/count/serial action.
void destroy_native_logical_texture_named_base_00b33f50(void* actual_owner,
    NativeStringStorage&);
void unwind_native_logical_texture_named_base_00b34010(void* actual_owner,
    NativeStringStorage&);

// Full four-byte 00B33E40; native ECX=owner, EAX=owner+8, RET. Address only;
// neither the owner nor its name fields are read, including for wrapped input.
void* native_logical_texture_name_address_00b33e40(void* actual_owner) noexcept;

// Shared diagnostic record cleanup remains
// destroy_native_buffer_diagnostic_record_00b3f4c0 in
// native_physical_buffer_owner.hpp; no second implementation belongs here.
// Evidence, original EH maps and limitations: docs/NATIVE_LOGICAL_TEXTURE_NAMED_BASE.md.

} // namespace bsp
