# C-0000: name of the contract

id: C-0000
version: 1
status: draft
owner: Director
parties: A, B
opened: H0

Rules for every contract are in `contracts/README.md` (including the absent-value convention). This file only fills in the specifics.

## Purpose

One paragraph: what the two sides agree on and why they must agree before building in parallel.

## Inputs, producers and first consumers

Every new input names who produces it and who consumes it first.

| Input | Producer (device, agent or script) | First consumer | Notes |
|---|---|---|---|
| `example_key` | A, rules runtime | B, look builder | replace this row |

## Types and fields

One block per type. Required fields are marked; absent optional fields are omitted keys.

### `ExampleType`

| Field | Type | Unit or range | Required | Meaning |
|---|---|---|---|---|
| `example_field` | string | enum `ExampleEnum` | yes | replace this row |

## Enum strings

### `ExampleEnum`

`first_value`, `second_value`

Unknown values are errors.

## Asset names and prefixes

Prefixes: `SM_` `M_` `MI_` `T_` `SFX_` `NS_` `BP_`. Name every asset another agent must find.

| Asset name | Kind | Folder | Used by |
|---|---|---|---|
| `SM_TestAsset` | mesh | `Game/Plugins/JamAssets/Content/Props/` | replace this row |

## Folders

| Folder | Written by | Read by |
|---|---|---|
| `Game/Plugins/JamAssets/Content/Props/` | B | A |

## Data asset names

| Data asset | Class | Referenced from cartridge key |
|---|---|---|
| `DA_Example` | `UExampleData` | `example_key` |

## Acceptance

The check that both sides agree: a validator command, an automation test, a file that must exist.

- [ ] ...

## Version history

- v1 (H0): first version.

## Open questions

Questions go to the Director; builders do not resolve them by editing this file.
