#pragma once

namespace bsp {
// Complete normal 00C3CC30..00C3F174. The original receives its work record
// in ESI and uses RET. This explicit source interface receives it in ECX and
// preserves ESI. The record is the actual 1F8h scratch object produced by
// 00C535E0/00C53010/00C51EF0; this function does not construct it.
//
// +80: three-double search direction; +98: four three-double difference
// points; +F8/+158: matching support witnesses; +1B8: current point count.
// Counts 1..4 select the native reduction paths. Other counts return without
// changing storage. Reduction retains the original ordered tests, x87 operand
// schedule, double spills, witness-copy order and degenerate arithmetic.
//
// Valid borrowed storage and initialized active points/witnesses required.
// No new GJK search loop, dispatcher, native exception ABI or game proof.
void __fastcall reduce_native_dyn_convex_simplex_00c3cc30(void* work) noexcept;
} // namespace bsp
