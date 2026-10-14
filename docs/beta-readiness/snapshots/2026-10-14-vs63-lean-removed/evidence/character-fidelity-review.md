# Playable character fidelity review — October 14, 2026

Verdict: **the current playable character art does not meet the approved mockups.** This is a source/render/reference review and pipeline inspection, not a claim of finished models or a packaged visual acceptance.

The user requests all characters to follow their mockups. Use the approved individual turnarounds as the design targets, keeping each species distinct. Do not reuse the sentry enemy surface kit across the player roster. Cosmetics keep the same gameplay statistics.

| Character | Reference and identity to preserve | Current gap | Required completion |
| --- | --- | --- | --- |
| Veteran | Approved Veteran four-angle sheet: grounded gritty armor, scarf, practical equipment and established helmet/head | Game loads the early modular static parts, with simplified body/helmet and flat materials; silhouette and equipment detail fall short | Author coherent body/armor, cloth/equipment, PBR surfaces, deformation and first-person hands; inspect against four angles and moving gameplay |
| Veyran | Approved Veyran v2 turnaround: slender species silhouette, crest/head, distinct blue/violet organic armor and hands | Rounded modular placeholders lose the head/crest, body continuity and species-specific surface language | Build the species form directly from its approved sheet, then rig/bake and inspect sprint/crouch/carry/weapon poses |
| Defender | Approved Defender v4 dark-grey/forest-green armor; preserve v2 basalt/reptilian face under a removable helmet | Game still loads the original modular parts. Newer v2 Blender draft is not wired into Unreal; face likeness, UV/bakes, LODs and deformation are unfinished | Refine face/armor to approved target, preserve maskless identity, create runtime mesh/LODs/materials, integrate and validate both helmet states in actual co-op |

The old three-character Blender scene has 47 mesh objects and 36,096 scene triangles, no armature, and no file textures. Runtime attaches those static pieces to the existing animated skeleton; absence of an armature in this scene does not mean the game has no animation. It does mean this is not a coherent authored/skinned production roster.

The newer Defender scene has three mesh objects and 234,952 scene triangles (including scene geometry), one 88-bone armature and two file image references. Raw file-image count does not establish authored PBR coverage. The existing README correctly identifies it as a draft: approximately 229,966 body triangles plus 4,984 helmet triangles, incomplete facial likeness and no runtime budget/LOD acceptance. Do not activate it merely because it is more detailed than the placeholder.

Unreal BuildRidgefireAppearance still loads SM_IS_Veteran_*, SM_IS_Veyran_* and SM_IS_Defender_* under Variant_Shooter/Meshes/Players; it does not use the newer SK_IS_Defender_Body_v2 export. The visible lobby scene/portraits are concept artwork and cannot establish these mesh qualities.

Acceptance requires matching front/back/side silhouette and proportions, correct face/helmet identity, clean PBR materials and UVs, deformation without gaps/clipping, suitable LODs and measured runtime cost, consistent first-person hands, and actual Unreal/co-op inspection. Record each pass separately. No arbitrary percentage of likeness or “perfect” approval is assigned.

Free local Blender 5.2.2 background inspection completed without modifying artwork. The live Blender MCP bridge was unavailable; that did not prevent read-only source inspection. No generator credit was spent. Detailed audit receipts are retained with the current task evidence. Character creation/refinement and final likeness approval remain open.
