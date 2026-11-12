summary: anything that looks wrong: floating objects, z-fighting, missing shadows, clipping, wrong scale
role: flash
stills: 1-6

## Prompt
Task: BUG HUNT. Each still is a real frame of the game. Find anything that looks WRONG as a rendering or a scene, not a matter of taste: objects floating above the ice or sunk into it, z-fighting or shimmering stripes, missing or doubled shadows, meshes clipping through boards, nets or each other, wrong scale (a puck bigger than a skate, a net taller than a figure), stretched textures, seams, visible edges of the world, lights with no source, mismatched outlines, text or HUD overlapping the play, a figure in an impossible pose.
For each finding: where (quadrant and a short description of the position), what, the severity (1 = cosmetic, 3 = a player would notice, 5 = breaks the picture), and your confidence 0-1 that it is a real bug and not an intended style. Be specific. If a still is clean say so with an empty list.
Stills:
{{stills}}

## Shape
```json
{
  "stills": [
    {
      "label": "string",
      "findings": [ { "where": "string", "what": "string", "severity": "int 1-5", "confidence": "number 0-1" }, "0-10" ],
      "clean": "bool"
    },
    "1-6"
  ]
}
```
