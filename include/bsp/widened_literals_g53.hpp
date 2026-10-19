#pragma once
// cc9-gunnery53 (SHIP_AI_OPEN_ITEMS 296): the ship-AI .cpp constants whose image qword is a
// float widened to a double (low 29 mantissa bits zero) while the host wrote a different
// double. tools/const_width_sweep.py reads only `inline constexpr` header lines, so these
// were found with the same comparison over the .cpp declarations; every load site below is
// 8 bytes (m64), so the image computes with the widened value.
//
//   00CE3828  6.2831854820251465 (2*pi as a float)   00605070: FLD 00605076, FADD 00605095,
//                                                    FSUB 006050AE; host had 6.283185307179586
//   00CE3D18 -3.1415927410125732                     00605070: FLD 0060508B; host -3.14159265358979
//   00CE3D28  3.1415927410125732                     00605070: FLD 006050A0; host 3.14159265358979
//   00D049A8  1.7999999523162842 (1.8f)              the crossing clip (009D8ED6..): FDIV 009D9044;
//                                                    host had 1.8000000476837158, the NEXT float up
//   00D7A358  0.009999999776482582 (0.01f)           009EF7EF FSUB, 009EF7FF FADD; host 0.01
//
// True: the host uses the image's qwords. False: the old literals.
namespace bsp {

inline constexpr bool kShipAiWidenedLiteralsBound = false;

}  // namespace bsp
