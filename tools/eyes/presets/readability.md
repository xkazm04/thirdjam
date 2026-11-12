summary: at the stated distance, can a viewer tell puck, carrier, team colours, far net and zone strips apart in under a second
role: flash
stills: 1-6
distance: true

## Prompt
Task: READABILITY at a viewing distance of {{distance}} m (the camera is about that far from the nearest figures). Judge each still on its own.
For each still and each of the five elements below, say whether a player glancing at the screen can find it in under a second, score it 1-5 (1 = cannot be found, 3 = found with effort, 5 = instant) and give one sentence of evidence from the pixels (where it is, what colour, how many pixels wide, what hides it). If an element is not visible at all, score 1 and say "not visible".
Elements: the puck (small near-black disc, maybe with a glowing disc or trail under it); the puck carrier (which figure has the puck); team colours (can the two teams be told apart; name the two colours you see); the far net (a red frame at the far end); the zone strips (the blue and red lines across the ice).
Then: the first thing the eye lands on in the still (one phrase), and what is visual noise (things that draw the eye without informing play).
Stills:
{{stills}}

## Shape
```json
{
  "stills": [
    {
      "label": "string",
      "elements": {
        "puck": { "score": "int 1-5", "evidence": "string" },
        "carrier": { "score": "int 1-5", "evidence": "string" },
        "team_colours": { "score": "int 1-5", "evidence": "string" },
        "far_net": { "score": "int 1-5", "evidence": "string" },
        "zone_strips": { "score": "int 1-5", "evidence": "string" }
      },
      "first_look": "string",
      "noise": "string[] 0-5",
      "overall": "int 1-5"
    },
    "1-6"
  ]
}
```
