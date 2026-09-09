# Official SDK headers only; runtime remains the locally installed D3DX9_40.
include(FetchContent)
FetchContent_Declare(dxsdk_d3dx
  URL https://api.nuget.org/v3-flatcontainer/microsoft.dxsdk.d3dx/9.29.952.8/microsoft.dxsdk.d3dx.9.29.952.8.nupkg
  URL_HASH SHA256=ead0906ae8a26c18a7525da7490127a2110f7c58f18293738283e30e97c6ea4b
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(dxsdk_d3dx)
add_library(bsp_d3dx_headers INTERFACE)
target_include_directories(bsp_d3dx_headers SYSTEM INTERFACE "${dxsdk_d3dx_SOURCE_DIR}/build/native/include")
