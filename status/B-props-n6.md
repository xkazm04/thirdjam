# B lane N6: generated props, batch 2 (Hyper3D Rodin)

14 generations x 0.5 = **7.0 Rodin credits** (limit 12). Balance 415 -> 408. Twelve props kept; two retries (canister kept as retry; reel retry rejected, first reel kept).
All: tier Gen-2.5-Low, GLB, text-to-3d, `--polycount` 5000 (pickups) or 10000 (arena), 3 x 2048 maps (diffuse, normal, orm), 1 material, 1 mesh. Bounds are as delivered (Rodin rescales everything to about 1.7-1.9 m on its longest axis; x, y, z in Blender axes, z up), so every prop needs a scale factor in the engine.
Suffix A (pickups, verbatim): `stylised toy prop, painted cel style, flat solid colours, no text, no letters, no numbers, no logos`.
Suffix B (arena, verbatim, S3's): `stylised, painted cel style, flat solid colours, mid-grey and warm accent colours (not black: it must read in a dark room), no text, no letters, no numbers, no logos`.
Prompt = the text below + ", " + suffix. Folder for each: `C:\Users\kazda\jam\skater-lab\n6\<name>\base_basic_pbr.glb` (frames `fr\`, sheet `sheet_<name>.png`, log `<name>.log`, inspect `<name>.insp`).

## A. Pickups
| Pickup (folder) | Task id | Prompt (before suffix A) | Tris | Bounds m | Look | Note |
|---|---|---|---|---|---|---|
| speed: film reel (`reel`) | c334ebc7-df14-46e5-b54b-7e25883c2b48 | a chunky toy film reel lying flat, a thick round reel with six large holes, in bright yellow painted metal, | 4,716 | 1.829 x 1.842 x 0.368 | 3 | six-hole disc, reads as a reel only from above; the yellow came out pale sand, not bright; very flat |
| precision: camera lens (`lens`) | 2ee28217-51d5-4631-8f53-01cf0110d04d | a chunky toy camera lens lying on its side, thick barrel with a big glass front element, in bright cyan painted metal with a dark glass centre, | 4,836 | 1.755 x 1.834 x 1.709 | 3 | clear barrel with black glass disc; a little like a pipe; the glass is near black (fine for a lens) |
| heavy puck: film canister (`canister2`; `canister` rejected) | d1a28f78-3a82-437d-86b8-b02bd0a7615f | a chunky toy film canister, a wide flat round tin like a pizza tin with a rim and a lid, in bright red painted metal with a thin pale band around the middle, no handle, | 4,992 | 1.800 x 1.805 x 1.776 | 3 | red tin with pale band, no marks; still a tall barrel (Rodin ignores "flat"); acceptable. First try b19f3953-d921-416e-a8a2-c3ca534235d7 (4,764 tris, 1.47 x 1.47 x 1.71; `a chunky toy round film canister, a short fat flat cylinder with a lid and a handle, in bright red painted metal,`) rejected: reads as a rubbish bin |
| big puck: spotlight head (`spot`) | 4337e317-ab71-478f-b838-e9434316522f | a chunky toy small studio spotlight head lying on the ice, round lamp with a short bracket and a glowing lens, in bright orange painted metal with a pale yellow lens, | 4,672 | 1.753 x 1.721 x 1.887 | 4 | orange drum lamp, cream lens, small bracket with eye; reads as a lamp |
| reverse: megaphone (`megaphone`) | 21e1de89-19df-4e74-82df-ec8c6123fa3e | a chunky toy megaphone, a fat cone horn with a handle, in bright magenta pink painted plastic, | 4,692 | 1.032 x 1.816 x 1.444 | 5 | exactly a megaphone, magenta, best silhouette of the six |
| mystery: clapper marker (`clapper`) | 0b4839da-61c3-4f47-a812-a11577d15b7d | a chunky toy director's clapper marker, a clapperboard with a hinged clap stick, in bright green painted wood with white and dark green stripes on the stick only, blank board, | 4,952 | 1.174 x 1.762 x 1.842 | 4 | green slate, striped open stick, two dots; body looks doubled (two slabs) from some angles; stripes only, no letters |
| rejected retry: reel2 (`reel2`) | 2a0f1a14-7385-4c8d-a8d4-49f9ca1e3dd7 | a chunky toy film reel, two big round flanges with six large round holes joined by a thick hub, standing upright on its edge, in saturated bright yellow painted metal, | 4,770 | 1.656 x 1.856 x 1.865 | 3 | better reel shape (two flanges on a stand) but a **lightning-bolt-like mark on the hub box** (logo-like), still pale; not kept |

### Distinguishability at 40 px (sheet `pickups_together.png`, filed `lab-n6-pickups-together`: 128 px row and 40 px row on a dark ground)
- Easy to tell apart: megaphone (pink, horn shape), lens (cyan, black disc), clapper (green, jagged stick), canister (red with pale band).
- **Confusable:** spotlight head (orange) vs canister (red): both round warm blobs, only the cream lens patch and bracket separate them. The reel (pale sand) is low contrast and nearly flat, the weakest at distance; it can pass for a puck-sized disc.
- Fixes: re-prompt the spotlight in a colour far from red (white-hot yellow or violet) and the reel in a stronger yellow, standing upright (silhouette a wheel, not a disc); in the engine push diffuse saturation on reel and spotlight and add a ring glow in one accent colour per kind, so shape matters less.

## B. Arena pieces
| Piece (folder) | Task id | Prompt (before suffix B) | Tris | Bounds m | Look | Note |
|---|---|---|---|---|---|---|
| bench (`bench`) | 8c8c6999-4c51-4b7a-a002-5a9dd6779512 | a players' bench for an ice hockey team, a long low bench seat with a low wooden backrest, in warm brown painted wood with mid-grey metal legs, | 9,634 | 1.878 x 0.528 x 0.691 | 4 | orange-brown slats, low backrest, thin pale legs; clean, no marks |
| penalty box (`penalty`) | 244171f5-05e1-4257-8b19-6fe726e53996 | a small open penalty box booth with a bench inside, three low walls and no roof, in mid-grey painted panels with a warm orange trim, chunky toy proportions, | 9,824 | 1.890 x 1.248 x 1.521 | 3 | open three-wall booth, bench inside, grey panels, orange frame; outer face has blotchy dark stains (texture noise, not letters); thick walls |
| scoreboard housing (`scoreboard`) | f58cd4f7-ba92-4d7f-ab6e-2ed8f6968f82 | a studio light box scoreboard housing shaped like an ON AIR light box, a wide rectangular box with a completely blank flat dark-grey front face, in mid-grey painted metal with warm orange frame and two small brackets, chunky toy proportions, | 9,414 | 1.812 x 1.113 x 1.142 | 5 | blank dark face, orange frame, grey body, two side brackets; no marks at 256 px (face slightly mottled: flat-fill it in the material, device A draws the score) |
| goal light (`goallight`) | c8c954d0-08d0-49fb-afb8-340ebd7be770 | a goal light on a tall pole, a round red beacon lamp on top of a mid-grey metal pole with a small round base, the lamp in bright red glowing glass, chunky toy proportions, | 9,856 | 0.503 x 0.503 x 1.734 | 4 | red lamp on a grey pole with round base; the lamp is a disc facing one way, the back is plain grey |
| water-bottle crate (`crate`) | ce70c14e-640e-4aa5-bed2-95aa8263beb4 | a water bottle crate holding six plastic water bottles, a boxy open crate in light mid-grey painted plastic with bottles with warm orange caps, chunky toy proportions, | 9,740 | 1.874 x 1.876 x 1.654 | 4 | grey crate, blue bottles with orange caps (5 or 6 bottles, counts are unreliable); blue was unprompted but reads well |
| coach's stool (`stool`) | 88bf26e1-6637-454a-8318-2e14255c58dc | a coach's tall wooden stool with a megaphone resting on the seat, stool in warm brown painted wood, megaphone in bright orange, chunky toy proportions, | 9,556 | 0.647 x 1.029 x 1.756 | 5 | exactly as asked: brown stool with rungs, orange megaphone on a grey mount |

### Suggested size and placement (default rink 90 x 45 m; dressing stands outside the boards; scale = target / delivered size)
- **Bench:** 4.5 m long (scale about 2.4 on the 1.88 m side), about 1.3 m high with backrest. Two, one per team, outside the boards on one long side either side of the centre line, 2 m off the boards, backs to the stand, long axis along x.
- **Penalty box:** about 3.5 x 3 x 2 m (scale about 1.9). One mirrored pair on the opposite long side, centred about 6 m from the centre line, open side to the ice, 1 m off the boards.
- **Scoreboard housing:** about 6 m wide x 3.6 m tall (scale about 3.3). Hung above centre ice at 9 to 11 m, or one per end wall 5 m above the stand; blank face to the ice, device A draws the score on a plane over the face; hang it from the truss (brackets are on the sides).
- **Goal light:** about 3.5 m tall (scale about 2.0). One behind each goal, 1.5 m outside the end boards, centred on the goal, lamp to the ice; drive the lamp material emissive on goal.
- **Water crate:** about 0.5 m cube (scale about 0.27). A few beside each bench behind the boards.
- **Coach's stool:** about 1.0 m tall (scale about 0.57 on the 1.76 m side). One at each bench end, behind the boards.
- **Pickups:** about 0.6 m on the longest side (scale about 0.33); bob or spin on the ice. Textures are 3 x 2K: use base colour only at 1K, as with the S3 props.

## Credits
14 generations x 0.5 = 7.0 (`consumed` 0.5 each). Balance 415 before, 408 after. Submit ledger: `C:\Users\kazda\jam\skater-lab\n6\ledger.jsonl` (holds a job token; do not paste it).

## Ledger rows to paste (the 12 kept; one `mesh` and one `texture` row each, `ai-generated`, tool `Hyper3D Rodin`; names are suggestions, `shipped` for the lead)
```
SM_PickupReel         | mesh, texture | ai-generated | Hyper3D Rodin | note: task c334ebc7-df14-46e5-b54b-7e25883c2b48 (Gen-2.5-Low, text prompt)
SM_PickupLens         | mesh, texture | ai-generated | Hyper3D Rodin | note: task 2ee28217-51d5-4631-8f53-01cf0110d04d
SM_PickupCanister     | mesh, texture | ai-generated | Hyper3D Rodin | note: task d1a28f78-3a82-437d-86b8-b02bd0a7615f (first try b19f3953-d921-416e-a8a2-c3ca534235d7 rejected)
SM_PickupSpotHead     | mesh, texture | ai-generated | Hyper3D Rodin | note: task 4337e317-ab71-478f-b838-e9434316522f
SM_PickupMegaphone    | mesh, texture | ai-generated | Hyper3D Rodin | note: task 21e1de89-19df-4e74-82df-ec8c6123fa3e
SM_PickupClapper      | mesh, texture | ai-generated | Hyper3D Rodin | note: task 0b4839da-61c3-4f47-a812-a11577d15b7d
SM_PlayersBench       | mesh, texture | ai-generated | Hyper3D Rodin | note: task 8c8c6999-4c51-4b7a-a002-5a9dd6779512
SM_PenaltyBox         | mesh, texture | ai-generated | Hyper3D Rodin | note: task 244171f5-05e1-4257-8b19-6fe726e53996
SM_ScoreboardHousing  | mesh, texture | ai-generated | Hyper3D Rodin | note: task f58cd4f7-ba92-4d7f-ab6e-2ed8f6968f82
SM_GoalLight          | mesh, texture | ai-generated | Hyper3D Rodin | note: task c8c954d0-08d0-49fb-afb8-340ebd7be770
SM_WaterCrate         | mesh, texture | ai-generated | Hyper3D Rodin | note: task ce70c14e-640e-4aa5-bed2-95aa8263beb4
SM_CoachStool         | mesh, texture | ai-generated | Hyper3D Rodin | note: task 88bf26e1-6637-454a-8318-2e14255c58dc
```
Not recommended: reel2 (mark on the hub), first canister (bin look). The reel is the weakest kept pickup: re-prompt before shipping.

## Contact sheets filed (devlog/shots, `20261112-1411xx-B-lab-n6-*`)
`pickup-film-reel`, `pickup-camera-lens`, `pickup-film-canister`, `pickup-spotlight-head`, `pickup-megaphone`, `pickup-clapper`, `arena-bench`, `arena-penalty-box`, `arena-scoreboard`, `arena-goal-light`, `arena-water-crate`, `arena-coach-stool`, `pickups-together`. Sources in `C:\Users\kazda\jam\skater-lab\n6\` (`sheet_*.png`, `pickups_together.png`, `all_pick.png`, `all_arena.png`). Rendered with `n6\render_fit.py`, a copy of `tools/meshy/render.py` whose camera distance comes from the bounding-box diagonal (the original frames by height only and crops flat props such as the reel; `tools/` untouched). 4 views, 256 px each.

## Could not verify
- Anything in Unreal (import, scale, toon material on a PBR GLB, the three 2K maps); sizes above are suggestions from the delivered bounds.
- Text-like marks were checked by eye in 4 views at 256 px only; the penalty box's stained outer face and the scoreboard's mottled face could hold faint marks at higher resolution. Textures were not opened at 2K.
- The 40 px sheet is the first turntable view in Workbench flat light, not the game's lighting or play camera; colours will shift in the night look.
- Licence wording for shipping Rodin output is still open (see `status/B-skater-s3.md`).
