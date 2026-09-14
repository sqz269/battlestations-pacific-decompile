; Metadata only: register the real C++ nested-handler PROC, without another body.
; The exact external spelling is checked against the current C++ COFF symbol.
; This object is supplied explicitly to each supported image link.
.386
.model flat

EXTRN ?handle_native_crt_seh4_nested_unwind_00c0dc54@bsp@@YA?AW4_EXCEPTION_DISPOSITION@@PAU_EXCEPTION_RECORD@@PAXPAU_CONTEXT@@1@Z:PROC
.safeseh ?handle_native_crt_seh4_nested_unwind_00c0dc54@bsp@@YA?AW4_EXCEPTION_DISPOSITION@@PAU_EXCEPTION_RECORD@@PAXPAU_CONTEXT@@1@Z

END
