#pragma once

namespace bsp {
// Complete C0DCCD..C0DCE5 (_EH4_TransferToHandler), 25 native bytes.
// MSVC Win32 register entry: ECX=actual native handler, EDX=actual frame.
// The original dispatcher JMPs here with its existing ESP; it does not push
// a return address or stack arguments. This declaration describes register
// assignment only. An ordinary C++ call does not establish the native frame,
// dispatcher stack, handler continuation or nonvolatile-register contract.
//
// Requires the accepted process-lifetime canonical DD owner and actual DJ
// NLG provider. EBP becomes frame; ESI retains handler. NLG receives EAX=handler,
// EBP=frame and the one native stacked code word1, then RET4 restores entry ESP.
// The final JMP ESI transfers with EAX/EBX/ECX/EDX/EDI zero, ESI=handler,
// EBP=frame and unchanged entry ESP. No nonvolatile register is saved.
// CF/OF/SF=0, ZF/PF=1, AF undefined from final XOR EDI,EDI; DF is unchanged.
// No frame repair, RET, validation, exception catch or callback wrapper exists.
// Actual handler continuation/stack effects remain that handler's obligation;
// no C++ noreturn attribute invents a policy for its eventual control flow.
// See docs/NATIVE_CRT_SEH4_HANDLER_TRANSFER_EF.md.
void __fastcall transfer_native_crt_seh4_handler_00c0dccd(
    void* native_handler_entry, void* native_frame);
} // namespace bsp
