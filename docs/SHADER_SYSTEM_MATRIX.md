# Runtime system matrix writer

`write_system_matrix_00b404a0` reconstructs the complete101-byte helper used by
the fixed system constant uploader00b46a70. Its native ABI is ECX destination,
one source pointer on the stack, RET4. The C++ interface uses ordinary pointer
arguments and requires16 readable source floats and16 writable destination floats.

In destination order it writes `dst[4*r+c] = src[4*c+r]`. Each value passes through
an x87 FLD/FSTP pair, as in the original. The Win32 port retains those instructions
instead of replacing them with raw word copies or SSE moves. No temporary matrix
is introduced: overlapping pointers observe earlier writes, so calling with the
same pointer is not a mathematical in-place transpose. The caller's x87 control
word is retained. Instruction/data exception addresses and original calling
convention are not preserved by the C++ loop.

The original/saved bytes match across00b404a0..00b40504; the hash and ABI are in
`reports/shader_system_matrix.json`. Assembly has sixteen FLD/FSTP pairs and no
callee. This helper is used for the screen, view, inverse-view, view-projection,
inverse-view-projection and projection matrices in00b46a70. The later shadow
matrix transposes use MOVSS, and must not automatically inherit this helper's
exceptional-value behavior.

The existing material fixture checks the non-symmetric matrix containing1..16
against its explicit transpose. The generated debug shader draw now uses this
writer to prepare cViewProjMat atc15 from the diagnostic identity camera matrix.
Win32 Release build, both existing CTests and the full D3D9 probe pass, including
centerFF407FBF/outsideFF000000 with state restoration. No new test target was
added. Exceptional FP inputs, overlap and native differential behavior were not
fixture-tested. Full camera caching, prefix gathering/upload and game execution
remain unfinished; parallel findings are recorded in the constant dispatch,
lighting and camera analysis documents.

Follow-up: the generated draw now obtains a perspective matrix from the recovered
lazy camera projection slice instead of its earlier identity-only fixture.
See `CAMERA_PROJECTION_EVIDENCE.md` for the current validation boundary.
