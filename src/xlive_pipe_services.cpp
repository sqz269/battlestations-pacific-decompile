#include "bsp/xlive_pipe_services.hpp"

#include <stdexcept>
#include <type_traits>

namespace bsp {
namespace {
DWORD* win32_output(std::uint32_t& value) noexcept {
    static_assert(sizeof(DWORD) == sizeof(std::uint32_t));
    static_assert(alignof(DWORD) == alignof(std::uint32_t));
    // MSVC Win32 foreign-function boundary. Forward this exact caller-owned
    // DWORD storage to the actual API, including writes on failed completion.
    // A temporary DWORD plus copy-back would change observable pointer identity
    // and writes before callbacks/errors. No preclear is performed here.
    return reinterpret_cast<DWORD*>(&value);
}
}

static_assert(!std::is_abstract_v<ReconstructedXLivePipeServices>);

ReconstructedXLivePipeServices::ReconstructedXLivePipeServices(
    XLivePipeProtocolGlobals& protocol_globals, XLivePipeFrameGlobals& frame_globals,
    const XLivePipeProtocolTables& tables, XLivePipeProtocolSystemHost& protocol_system,
    XLivePipeFrameSystemHost& frame_system, XLivePipeSystemHost& transport, XLivePipeIoHost& io)
    : ReconstructedXLivePipeFramingHost(frame_globals, tables, frame_system),
      protocol_(protocol_globals, tables, protocol_system), transport_(transport), io_(io) {
    if (&protocol_globals.acquisition_lock_f8b778 != &frame_globals.acquisition_lock_f8b778 ||
        &protocol_globals.key_f8badc != &frame_globals.key_f8badc)
        throw std::invalid_argument("Pipe protocol and framing must share canonical globals");
}

std::int32_t ReconstructedXLivePipeServices::open_00a5de34(
    std::uint32_t mode, HANDLE stop_event, void*& pipe) {
    XLivePipeTransport* result;
    const auto status = open_xlive_pipe_00a5de34(mode, stop_event, &result, protocol_, transport_);
    pipe = result; // publish only on a normal return from the recovered wrapper
    return status;
}

void ReconstructedXLivePipeServices::close_00a5de68(void* pipe) {
    close_xlive_pipe_00a5de68(static_cast<XLivePipeTransport*>(pipe));
}

std::int32_t ReconstructedXLivePipeServices::send_00a5df20(
    void* pipe, const void* buffer, std::uint32_t bytes) {
    return write_xlive_pipe_00a5df20(static_cast<XLivePipeTransport*>(pipe), buffer, bytes, io_);
}

std::int32_t ReconstructedXLivePipeServices::wait_send_00a5df5e(
    void* pipe, std::uint32_t& bytes, std::uint32_t timeout) {
    return finish_xlive_pipe_write_00a5df5e(static_cast<XLivePipeTransport*>(pipe),
        win32_output(bytes), timeout, io_);
}

std::int32_t ReconstructedXLivePipeServices::receive_00a5deaa(
    void* pipe, void* buffer, std::uint32_t capacity) {
    return read_xlive_pipe_00a5deaa(static_cast<XLivePipeTransport*>(pipe), buffer, capacity, io_);
}

std::int32_t ReconstructedXLivePipeServices::wait_receive_00a5dee8(
    void* pipe, std::uint32_t& bytes, std::uint32_t timeout) {
    return finish_xlive_pipe_read_00a5dee8(static_cast<XLivePipeTransport*>(pipe),
        win32_output(bytes), timeout, io_);
}

} // namespace bsp
