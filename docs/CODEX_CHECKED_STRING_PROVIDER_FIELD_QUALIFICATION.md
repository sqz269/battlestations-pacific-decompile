# Checked string provider raw field qualification

Existing raw range assignment48DA50 now reads its DWORD headers through fenced
byte copies. Its complete body, including source-length reloads, destination
resize, source/data-before-destination/data order and zero-copy omission, is
otherwise unchanged.

The raw-pool overload of alias construction44BCB0 now uses dedicated fenced
byte helpers. It retains the exact specialization of the existing shared
template: null destination skips source, identity is captured before the two
zero stores, identity skips resizing, then current source length, real resize,
current length guard, destination length, source data and destination data are
read before the existing memmove. No cleanup or terminator write is added.

The shared template, its typed overloads and every other alias-file body remain
byte-identical. A token comparison verifies the specialized raw schedule against
the previous raw template instantiation. Inverse substitutions recover both
complete prior files. Both corrected translation units pass strict Win32 object
compilation. Independent review and common build evidence remain in the report.

No Native/vendor body credit, new API, provider, callback, test, runtime call or
activation follows. This supplies a raw access qualification for existing
registration providers; it does not register a table or grant loaded-manager,
raw16, Original ABI/FH3/SEH or game validation.

Independent full-diff/specialization/inverse review accepted exact alias72f91f50
and range96d76995. Both strict x86 objects are014C. The combined canonical Win32
build passed existing checks2/2 with the vector fixes and joined Main6f786fd in
10.41s. No Source changed between that build and final review.
