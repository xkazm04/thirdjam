summary: a gameplay still with HUD: what is legible at 1080p and 720p, what collides, what is missing for a first-time player
role: flash
stills: 1-6

## Prompt
Task: HUD. Each still is a gameplay frame with the game's HUD (score plates, timer, a far-net marker, a radar mini-rink bottom-right, a controls strip, speech bubbles, a faceoff countdown or an announcement, as the frame has them). The resolution of each still is given with it; a 720p still is the same HUD at the lower resolution a laptop player sees.
For each still: list each HUD element you can see with its position, whether it is legible (would a player read it in a glance), and what it says if text; list collisions (HUD covering the puck, a figure, the far net, another HUD element; HUD elements too close to a screen edge); what is MISSING for a first-time player (what they need to know in the first ten seconds and cannot see); and a legibility score 1-5. Give the smallest text you can read and its approximate pixel height.
Stills:
{{stills}}

## Shape
```json
{
  "stills": [
    {
      "label": "string",
      "elements": [ { "name": "string", "position": "string", "legible": "bool", "text": "string?" }, "0-12" ],
      "collisions": "string[] 0-6",
      "missing": "string[] 0-5",
      "smallest_text_px": "int 0-200",
      "legibility": "int 1-5"
    },
    "1-6"
  ]
}
```
