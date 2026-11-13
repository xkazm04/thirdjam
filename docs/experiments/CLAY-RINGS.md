# CLAY-RINGS: zones as opaque ring meshes on `p3_clay` (lane A69, B-to-A-0034 point 3 / 0035 point 4a)

## What changed

On `p3_clay` every zone that already has a B strip (`MI_Zone_<kind>` loads) also owns a thin ring mesh on its outline; the decal quad is hidden there. On every
other tier the ring is hidden and the decal stays. Presentation only: no sim state, no hash.

## Geometry (`RinkActor.cpp`, inside the zone loop of `Build`)

- One `UProceduralMeshComponent` per zone, `ARinkActor::ZoneRings`, parallel to `ZoneQuads` (same index). No collision, no shadow, stencil 0.
- Circle: 64 segments, a top face plus an outer and an inner wall (each with its own normal), centred on the outline. Rect: four boxes (the corners overlap, no gap).
- Height 6 cm as briefed. **Width 30 cm, not the briefed 12 cm** (`RingHalfW` 15 cm): at 10 m the 12 cm ring was 2 px and the eyes tool called it a debug wire
  (`docs/experiments/eyes/` of the first pass); 30 cm reads as a ring at the game camera and at 10 m.
- UVs are the quad's: 0..1 over the zone's bounding box. B's `M_Zone` also reads the zone's size from the component scale (the quad is a 1 m plane scaled to
  (w, d, 1) m). A ring built in world size with scale 1 drew nothing at all (the first build); the ring is therefore built in the quad's local frame (centred,
  X and Y divided by the size in metres) and given the quad's scale and location. This is the one non-obvious rule of the binding.

## Material binding

A dynamic instance of B's `MI_Zone_<kind>` with `Circle` (1 / 0) and `Strength`, exactly the quad's. A kind without B's instance gets no ring and keeps A's decal
(`ZoneRings` and `ZoneQuads` stay parallel because both are created in the same branch). The material is B's translucent strip with its clay glow path; it
shows as a rim, not as a solid. That is why SSR gives a faint reflection only: see the ask below.

## Show and hide, no leftovers

`ARinkActor::SetZoneRingsForTier(Tier)`: rings visible and quads hidden on `p3_clay`, the opposite elsewhere; the glass pane and the glass rail follow
(`SetVisibility(false)` on clay, B-to-A-0035 point 1 optional part; their draws were all clipped by B's materials). `Build` calls it with `game.look.tier`,
`ULookTierSwitcher::PushGlass` calls it with the new tier, `SetConstructionVisible` keeps the rail hidden on clay. Rings are built with the rink and toggled, never
spawned per switch, so a switch cannot leave an actor behind. Measured (`a69run.mjs`, PIE, six zones): after three `jam.livecut` night/clay pairs the rink actor
had **19 procedural components and 8 static meshes in every state**, 6 rings, `shown` false on night and true on clay (log line `JAMRINK zone rings ...`).

## Stills (`docs/experiments/img/a69/`, six-zone clay cut, PIE 1920 x 1080, frame 80)

- `a69-clay-game-camera.png`: the game camera (6 m, 3.5 m high). `a69-clay-10m.png`: camera 10 m behind, 5 m high.
- Eyes (`docs/experiments/eyes/a69-*`, final 30 cm stills): both bug-hunt runs see the rings (a red rect, a yellow ring, a cyan ring) but call them "debug or tutorial volume
  outlines", i.e. visible and crisp, not yet a clay glow; readability gives zone strips 2 to 3 of 5 and blames the vertical specular streaks and the HUD banner. The "seam at 4 o'clock"
  is on the grey faceoff-circle marking, not on a ring. HUD overlaps (minimap on the key bar, banner over the marker) are not this lane's.
- Honest reading: the rings are crisp coloured outlines on the ice with a faint streak below them; they do **not** yet glow or reflect like the concept's clay rings.

## Cost

PIE editor viewport, 1920 x 1080 floating, RTX 4090 with other lanes loading the machine, `GetFrameTimings` 20 samples x 3 interleaved rounds, six zones, clay:
rings on (rings, no glass, no rail, no decals) vs rings off (decals, glass and rail drawn: clay as it was). GPU ms mean: 12 cm ring **7.87 -> 8.03 (+0.16)**;
30 cm ring (shipped) **6.44 -> 6.85 (+0.41)**, taken under heavier machine load, so call it **+0.2 to +0.4 ms**. Raw: `img/a69/a69-cost.json`.
The same run's game thread was 15 to 45 ms because of the load: the frame figure is not usable; a packaged `tools/perf/match.ps1` run is the clean measurement.
The temporary under-ice floor (`StaticMeshActor_0`, 220 x 140 m, top at -20 cm after `LowerFloor`, under B's room floor at -4 cm) cast shadow into the four shadowed
spots: shadow off **7.38 -> 7.04 ms (-0.34 ms)** in PIE (3 rounds x 20 samples), nothing visible changes (nothing lies under it), so `LowerFloor` now sets
`SetCastShadow(false)` on it on every tier.

## Ask to B

1. Not a seam problem: the procedural ring shows no seams, so `SM_ZoneRing` is **not needed**.
2. What the ring still lacks is the concept's glow and a solid reflection. Options on B's side: an opaque variant of the zone material (a `MI_Zone_<kind>_Ring`, or a
   `Solid` scalar on `M_Zone` read through the dynamic instance: A would set it to 1 on the ring, opaque and emissive, so SSR can reflect it); a ring wider than 30 cm
   only if B wants to widen the rim in the material first. Tell A the parameter name and A67-style lane sets it in one line (`RD->SetScalarParameterValue`).
