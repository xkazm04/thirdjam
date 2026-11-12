summary: two stills, which is closer to the bar and why, element by element, with a verdict a | b | tie and a confidence
role: pro
stills: 2
distance: true

## Prompt
Task: A/B PAIR. Two stills of the same frame of the game with two different looks. Decide which is closer to the bar described below (and to the reference image if attached).
{{bar}}
The camera is about {{distance}} m from the nearest figures.
For each element (sky, ice, pools, figures, construction, outline) say which still is closer (a, b or tie) and why in one sentence of pixel evidence. Then the verdict for the whole picture, a confidence 0-1, the single biggest reason, and one thing the losing still does better that the winner should keep.
Stills:
{{stills}}

## Shape
```json
{
  "elements": {
    "sky": { "closer": "enum a|b|tie", "why": "string" },
    "ice": { "closer": "enum a|b|tie", "why": "string" },
    "pools": { "closer": "enum a|b|tie", "why": "string" },
    "figures": { "closer": "enum a|b|tie", "why": "string" },
    "construction": { "closer": "enum a|b|tie", "why": "string" },
    "outline": { "closer": "enum a|b|tie", "why": "string" }
  },
  "verdict": "enum a|b|tie",
  "confidence": "number 0-1",
  "reason": "string",
  "loser_keeps": "string"
}
```
