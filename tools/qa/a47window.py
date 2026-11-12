import unreal
# Lane A47: a floating PIE window at 1920x1080 so the UMG lays out as a player sees it (tools/skills/camera-third-person.md).
s = unreal.get_default_object(unreal.LevelEditorPlaySettings)
s.set_editor_property('new_window_width', 1920)
s.set_editor_property('new_window_height', 1080)
'window 1920x1080'
