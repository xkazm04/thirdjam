summary: how far the still is from painted-cel toy hockey at night; the three biggest gaps with one change each, and what already works
role: pro
stills: 1
distance: true

## Prompt
Task: DISTANCE TO THE BAR. Compare the still to the bar described below (and to the reference image if one is attached; the reference is concept art for the mood and the style, not a layout to copy).
{{bar}}
The camera in the still is about {{distance}} m from the nearest figures.
Give: a score 1-10 for how close the still is to the bar (10 = at the bar); the three biggest gaps in order of importance, each with the one change most likely to close it (a concrete, buildable change: a material parameter, a light, a mesh band, a post-process setting, a proportion), and the element it concerns (sky, ice, pools, figures, construction, outline, puck, net, palette, other); what already works and should not be touched; and one sentence a lead would say to the team.
Stills:
{{stills}}

## Shape
```json
{
  "score": "int 1-10",
  "gaps": [
    { "rank": "int 1-3", "element": "enum sky|ice|pools|figures|construction|outline|puck|net|palette|other", "gap": "string", "change": "string" },
    "3-3"
  ],
  "works": "string[] 1-5",
  "lead_sentence": "string"
}
```
