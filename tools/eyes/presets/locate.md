summary: can the viewer tell where the puck and the far net are, on the screen or by an arrow or badge at the frame's edge (lane A76)
role: flash
stills: 1-6
distance: false

## Prompt
Task: LOCATE. A hockey game's screen follows one skater. The puck and the far net may be off the picture; the HUD then points at them with an arrow or a badge on the frame's edge and a distance in metres ("24 m"). Judge each still on its own.
For each still answer two questions. Where is the puck: can a viewer tell where it is, either because they see it or because an arrow, badge or distance points at it? Where is the far net (the red frame at the far end): the same question, a diamond badge with a distance or an arrow counts.
Score each 1-5 (1 = a viewer cannot tell where it is, 3 = tell with effort, 5 = instant), say in one sentence what tells them (the puck itself, an arrow at which edge, a badge, a distance, or nothing) and, for the far net badge or arrow if there is one, say whether it covers the net's frame, the crossbar or a label, or stands clear of them. Then list every pair of HUD marks or labels that touch or overlap each other (a label over another label, a chevron over a label), and say whether the MATCH card (a dark plate with MATCH n) covers the middle third of the frame's height.
Stills:
{{stills}}

## Shape
```json
{
  "stills": [
    {
      "label": "string",
      "puck": { "score": "int 1-5", "told_by": "string" },
      "far_net": { "score": "int 1-5", "told_by": "string", "badge_covers_net": "bool" },
      "overlapping_marks": "string[] 0-6",
      "match_card_in_middle_third": "bool",
      "overall": "int 1-5"
    },
    "1-6"
  ]
}
```
