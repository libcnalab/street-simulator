# Known issues found by cna-engine

Defects found by cna-engine's agent while comparing this game with the engine
and moving its renderer onto it (2026-10-20/21). They are not fixed here; each
entry has a suggested fix. Their source record is `upstream-issues.md` in
[libcna/cna-engine](https://github.com/libcna/cna-engine) (UP numbers below).
Checked against `develop` at `2e40f0c` on 2026-10-22.

## Mesh memory is overstated by the vertex struct's vtable (UP-008)

`street/src/Render/GpuMesh.cpp` computes `gpuBytes_ = vertices × sizeof(Geometry::Vertex) + indices × 4`
(reported as `CityScene::BuildStats::meshBytes` and the benchmark's `meshBytes`).
`Geometry::Vertex` is CNA's `VertexPositionNormalTangentTexture`, which
implements `IVertexType` and carries a vtable: `sizeof` is 56 while the vertex
buffer's declared stride, what the GPU stores, is 48. On the default city the
game reports 79,966,804 bytes for the meshes it uploads itself; the same buffers
by their declared stride are 69,668,412 bytes, 8 bytes for each of the 1,287,299
vertices (the index bytes agree). Rendering is unaffected.

Suggested fix: use `getVertexDeclarationStatic().getVertexStrideProperty()`
instead of `sizeof`; the skinned mesh's accounting likely follows the same
pattern.

## A layout test discards a `[[nodiscard]]` result (UP-010)

`tests/LayoutTests.cpp` (around line 202) calls `collector.builder(&b);` and
`collector.builder(&c);` only for their side effect (registering a material),
but `GeometryCollector::builder()` is `[[nodiscard]]`, so GCC warns
(`-Wunused-result`). The default build passes; with `-DCNA_STREET_WERROR=ON` the
test stops compiling.

Suggested fix: `(void)collector.builder(&b);`.

## The asset pipeline needs manual help to reach the full look (UP-013)

The game runs without these assets; they decide how close it looks to the
reference screenshots.

1. `scripts/fetch-assets.sh` fetched 346 of 351 files: `GlassVaseFlowers.glb`
   and the four `mounted_fluorescent_lights_*_1k.jpg` maps fail their recorded
   SHA-256 because the upstream files changed. Update the checksums (or pin the
   URLs) in `assets/external/manifest.json`.
2. `scripts/blender-tree-lod.py` cannot export `tree_small_02` with Blender 4.3:
   it deletes every UV layer but the first, while the `tree_small_02_branches`
   material still reads `UV_map_01` through an Attribute node, and the glTF
   exporter fails with `KeyError: ... key "UV_map_01" not found`. Pointing that
   node at `UVMap` first (and removing the scene's other objects) works; the
   island and jacaranda trees and `blender-vehicles.py` work unchanged.
3. `scripts/blender-people.py` fails on its first run (`No module named
   'bl_ext.user_default.mpfb'`): it unpacks MPFB into the extension repository
   after Blender has scanned it. A second run, with `BLENDER_USER_EXTENSIONS`
   as an absolute path, builds all eight people.
4. The sound set (`scripts/prepare-audio.py`) needs the NOX Sound Essentials
   packs. The manifest's source,
   `https://www.asoundeffect.com/sound-library/essentials-series/`, now returns
   "Page not found"; the same CC0 pack is published on itch.io as
   `Essentials_Series_NOX_SOUND.zip`
   (https://nox-sound-design.itch.io/essentials-series-sfx-nox-sound) and gives
   all 22 derived sounds. Point the manifest's `source` and `acquisitionNote`
   there.
5. `docs/screenshots/` no longer matches what `develop` draws with the full
   content set (other paving, other parked-car placement), so
   `scripts/check-screenshots.sh` compares against older pictures.

## `--capture` runs out of memory while importing the models (UP-017)

On 2026-10-21 `cna-street --capture DIR --width 640 --height 360 --no-overlay`
(Release, CNA's software renderer, headless platform; `develop` at `3611348`,
built against the live `libcna/cna` checkout at `926db4ac9`) grew to 23 GB
resident while importing the Poly Haven models, before writing any image, and
the OOM killer ended it on a 30 GB machine without swap. The same capture of all
18 viewpoints ran on 2026-10-20 (commit `b137025`), and the models are older
than that. What changed since is the CNA checkout and the commits after
`b137025` (`513dd93`, derived materials in an `AssetRegistry`, among them);
which of them grows the import is not known yet.

Suggested next step: bisect between `b137025` and `develop` with the same CNA,
then the CNA revision with the same game revision, watching the resident size
during the model import.
