summary: a figure sheet: does it read as a hockey player and as a toy at full size and at 128 px; proportions to change, in numbers
role: pro
stills: 1
thumbnail: true

## Prompt
Task: FIGURE SHEET. The still is a contact sheet of a skater figure (rows may be variants, columns poses). A downscaled copy of the whole sheet follows it: that is how the figure reads at the game's usual distance (a figure is 40-120 px tall in play).
Judge: does it read as a HOCKEY PLAYER (score 1-5; are the stick, blade, hands, skates and helmet each visible and in the right place), does it read as a TOY (score 1-5; chunky proportions, round forms, simple colours) rather than an athlete or a mannequin, at full size and at the downscaled size separately. If the sheet has several rows, name the row that reads best and why.
Proportions: estimate the figure's head height as a fraction of total height, shoulder width as a fraction of height, stick length relative to height; then list the changes to make, each in numbers (for example "head 0.30 of height -> 0.36").
Then: the three worst things in the sheet (what breaks the read) and what works.
Stills:
{{stills}}

## Shape
```json
{
  "hockey_player": { "full": "int 1-5", "small": "int 1-5", "parts": { "stick": "bool", "blade": "bool", "hands": "bool", "skates": "bool", "helmet": "bool" }, "evidence": "string" },
  "toy": { "full": "int 1-5", "small": "int 1-5", "evidence": "string" },
  "best_row": "string",
  "proportions": { "head_of_height": "number 0-1", "shoulder_of_height": "number 0-1", "stick_of_height": "number 0-3" },
  "changes": "string[] 1-6",
  "worst": "string[] 1-3",
  "works": "string[] 1-4"
}
```
