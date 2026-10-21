// SPDX-License-Identifier: MIT
#pragma once

#include "CnaEngine/Core/Noise.hpp"

#include <cstdint>

namespace CnaStreet::Noise {

/**
 * @file Noise.hpp
 * @brief The noise the city's surfaces are made of.
 *
 * Header-only and hash-based rather than table-based on purpose: with no
 * permutation table to initialise there is no shared state, so the texture
 * generators can be run on several threads without any coordination, and the
 * result depends only on the coordinates and the seed. That is what makes an
 * asphalt texture byte-identical between a debug build, a release build and a
 * different machine.
 *
 * Every function here is *tileable* on a given period. A road surface that does
 * not tile is a road surface with a visible seam every few metres.
 *
 * The arithmetic lives in cna-engine (CnaEngine::Noise, taken over from this
 * file unchanged); these are the city's names for it, so the texture
 * generators read as they always have.
 */

/// 32-bit integer hash (a variant of Wang/Jenkins mixing). Cheap and well
/// distributed, which is all a value-noise lattice needs.
[[nodiscard]] inline std::uint32_t hashInt(std::uint32_t x) { return CnaEngine::Noise::Hash(x); }

[[nodiscard]] inline std::uint32_t hash2(int x, int y, std::uint32_t seed)
{
    return CnaEngine::Noise::Hash2(x, y, seed);
}

[[nodiscard]] inline std::uint32_t hash3(int x, int y, int z, std::uint32_t seed)
{
    return CnaEngine::Noise::Hash3(x, y, z, seed);
}

/// Uniform in [0,1) from a lattice cell.
[[nodiscard]] inline float random2(int x, int y, std::uint32_t seed) { return CnaEngine::Noise::Random2(x, y, seed); }

[[nodiscard]] inline float random3(int x, int y, int z, std::uint32_t seed)
{
    return CnaEngine::Noise::Random3(x, y, z, seed);
}

/// Quintic smoothstep — C2 continuous, so a normal map derived from it has no
/// visible lattice creases the way a cubic one does.
[[nodiscard]] inline float fade(float t) { return CnaEngine::Noise::Fade(t); }

[[nodiscard]] inline float lerp(float a, float b, float t) { return CnaEngine::Noise::Lerp(a, b, t); }

/// Positive modulo, so a negative coordinate still lands in the tile.
[[nodiscard]] inline int wrap(int v, int period) { return CnaEngine::Noise::Wrap(v, period); }

/// Tileable 2-D value noise on an integer lattice with the given period.
[[nodiscard]] inline float value2(float x, float y, int period, std::uint32_t seed)
{
    return CnaEngine::Noise::Value2(x, y, period, seed);
}

/// Tileable gradient (Perlin-style) noise. Sharper features than value noise
/// and no axis-aligned bias, which matters for anything that is going to be
/// turned into a normal map.
[[nodiscard]] inline float gradient2(float x, float y, int period, std::uint32_t seed)
{
    return CnaEngine::Noise::Gradient2(x, y, period, seed);
}

/// Fractional Brownian motion over value noise. The general-purpose "surface
/// variation" function: dirt, staining, mottling, cloud cover.
[[nodiscard]] inline float fbm(float x, float y, int period, int octaves, float lacunarity,
                               float gain, std::uint32_t seed)
{
    return CnaEngine::Noise::Fbm(x, y, period, octaves, lacunarity, gain, seed);
}

/// Ridged multifractal — the sharp creases used for cracks and bark.
[[nodiscard]] inline float ridged(float x, float y, int period, int octaves, std::uint32_t seed)
{
    return CnaEngine::Noise::Ridged(x, y, period, octaves, seed);
}

/// Worley / cellular noise. Returns the distance to the nearest feature point
/// (F1) and, through @p secondOut, to the second nearest — F2-F1 is what draws
/// the joints between cobbles, paving slabs and cracked mud.
[[nodiscard]] inline float worley2(float x, float y, int period, std::uint32_t seed,
                                   float* secondOut = nullptr)
{
    return CnaEngine::Noise::Worley2(x, y, period, seed, secondOut);
}

[[nodiscard]] inline float saturate(float v) { return CnaEngine::Noise::Saturate(v); }

/**
 * @brief Hermite interpolation between two edges, in either direction.
 *
 * GLSL leaves `smoothstep` undefined when `edge0 >= edge1`; this one does not,
 * and that matters because a *descending* pair is how you write "1 where the
 * value is small". Rejecting it — returning a hard 0/1 step instead — silently
 * turns every such ramp in a texture generator into a binary mask, which is a
 * class of bug you can look straight at without seeing: the texture is still
 * plausible, just hard-edged everywhere it should be soft.
 */
[[nodiscard]] inline float smoothstep(float edge0, float edge1, float x)
{
    return CnaEngine::Noise::SmoothStep(edge0, edge1, x);
}

}  // namespace CnaStreet::Noise
