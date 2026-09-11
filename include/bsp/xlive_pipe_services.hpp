#pragma once

#include "bsp/xlive_pipe_framing.hpp"
#include "bsp/xlive_pipe_io.hpp"

namespace bsp {

// Complete concrete XLiveIpcPipeHost: recovered framing + protocol lifecycle +
// transport and I/O. No endpoint or worker starts during construction. The
// application retains every global, verified table and system host for the
// full lifetime of open pipes and IPC worker callbacks.
class ReconstructedXLivePipeServices final : public ReconstructedXLivePipeFramingHost {
public:
    ReconstructedXLivePipeServices(XLivePipeProtocolGlobals&, XLivePipeFrameGlobals&,
        const XLivePipeProtocolTables&, XLivePipeProtocolSystemHost&,
        XLivePipeFrameSystemHost&, XLivePipeSystemHost&, XLivePipeIoHost&);
    std::int32_t open_00a5de34(std::uint32_t mode, HANDLE stop_event, void*& pipe) override;
    void close_00a5de68(void*) override;
    std::int32_t send_00a5df20(void*, const void*, std::uint32_t bytes) override;
    std::int32_t wait_send_00a5df5e(void*, std::uint32_t& bytes, std::uint32_t timeout) override;
    std::int32_t receive_00a5deaa(void*, void*, std::uint32_t capacity) override;
    std::int32_t wait_receive_00a5dee8(void*, std::uint32_t& bytes, std::uint32_t timeout) override;
private:
    ReconstructedXLivePipeProtocolHost protocol_;
    XLivePipeSystemHost& transport_;
    XLivePipeIoHost& io_;
};

} // namespace bsp
