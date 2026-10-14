#pragma once
// Compressed-vector magnitude gate. Math ported from libsamp
// reimpl/src/net/openmp_compressed_vector_compat.h (MIT): open.mp
// writeCompressedVEC3 emits direction floats only when magnitude > 1e-5;
// plain RakNet ReadVector tests != 0 and would consume following sync fields.
// This helper implements the gate on already-decoded floats (wire float
// decode stays behind net/raknet_adapter.h until the RakNet source is
// vendored — TODO(rea)).
#include <cmath>

namespace samp { namespace net {

inline constexpr float kVecMagnitudeEpsilon = 0.00001f;

// Scales direction by magnitude when above epsilon, else zeroes.
// Returns false on non-finite/negative magnitude (caller must drop packet).
inline bool ScaleCompressedVector(float magnitude, float cx, float cy, float cz,
                                  float out[3]) {
  if (!std::isfinite(magnitude) || magnitude < 0.0f) return false;
  if (magnitude > kVecMagnitudeEpsilon) {
    if (!std::isfinite(cx) || !std::isfinite(cy) || !std::isfinite(cz)) return false;
    out[0] = cx * magnitude; out[1] = cy * magnitude; out[2] = cz * magnitude;
  } else {
    out[0] = out[1] = out[2] = 0.0f;
  }
  return true;
}

}} // namespace
