# SKF style table: one skeleton (same bone names, hierarchy and sockets), three looks. Shared by build_skater.py and build_clips.py.
# Metres, Blender axes: figure faces +X, left = +Y, up = +Z, feet on z = 0.
# Joint heights (ankle .. head_top) place the bones; the look builders place every mesh part relative to these joints.
# Clip adaptation (build_clips.py): the s4 pose tables were authored for p2_cel's proportions. Each style rescales their positions:
#   kx  leg scale: skate targets x and y, skate lift above the ice, pelvis drop when lying ((hip - ankle) / 0.72)
#   kz  crouch scale: pelvis height offsets of the standing poses (p4: deeper than kx, its long legs read too upright otherwise)
#   kr  reach scale: stick heel / grip / left-hand targets (height / 1.727)
#   stance  extra metres added to |y| of both skates (fat legs need a wider stance)
#   lie_add metres added to the pelvis height in lying poses (pz < -0.5: thicker bodies lie higher)
#   aim_*   the aim holds and the Poke tip are solved to the game's reach (R-0018), whatever the proportions
STYLES = {
    "p2_cel": dict(
        ankle=0.13, knee=0.49, hip=0.85, hip_y=0.115,
        pelvis=0.88, sp1=0.98, sp2=1.08, sp3=1.17, neck=1.30, head=1.37, head_top=1.62, top=1.73,
        sh_z=1.27, sh_y=0.215, upper=0.29, lower=0.26, arm_drop=50.0, grip_off=0.065,
        stick_len=1.40, stick_lie=50.0, stick_rest_tilt=62.0, stick_blade=0.34,
        head_scale=1.10, foot_fwd=0.16, blade_x0=-0.115, blade_x1=0.195,
        num_z0=0.98, num_z1=1.28,
        kx=1.0, kz=1.0, kr=1.0, stance=0.0, lie_add=0.0,
        lunge=1.0,
    ),
    "p3_clay": dict(
        ankle=0.14, knee=0.47, hip=0.80, hip_y=0.13,
        pelvis=0.83, sp1=0.93, sp2=1.02, sp3=1.11, neck=1.24, head=1.30, head_top=1.55, top=1.66,
        sh_z=1.20, sh_y=0.235, upper=0.26, lower=0.23, arm_drop=50.0, grip_off=0.07,
        stick_len=1.40, stick_lie=50.0, stick_rest_tilt=62.0, stick_blade=0.34,
        head_scale=1.0, foot_fwd=0.16, blade_x0=-0.115, blade_x1=0.20,
        num_z0=0.90, num_z1=1.22,
        kx=0.66 / 0.72, kz=0.66 / 0.72, kr=1.66 / 1.727, stance=0.035, lie_add=0.07,
        lunge=1.15,
    ),
    "p4_broadcast": dict(
        ankle=0.12, knee=0.52, hip=0.93, hip_y=0.105,
        pelvis=0.96, sp1=1.07, sp2=1.18, sp3=1.29, neck=1.50, head=1.56, head_top=1.80, top=1.81,
        sh_z=1.44, sh_y=0.20, upper=0.31, lower=0.27, arm_drop=50.0, grip_off=0.075,
        stick_len=1.46, stick_lie=50.0, stick_rest_tilt=62.0, stick_blade=0.33,
        head_scale=1.0, foot_fwd=0.17, blade_x0=-0.12, blade_x1=0.19,
        num_z0=1.05, num_z1=1.42,
        kx=0.81 / 0.72, kz=1.30, kr=1.81 / 1.727, stance=0.0, lie_add=0.0,
        lunge=0.9,
    ),
}

# sRGB flat colours; same nine slot names for every style
COLOURS = {
    "p2_cel": {
        "Jersey": (0.10, 0.26, 0.78), "Trim": (0.95, 0.95, 0.93), "Helmet": (0.93, 0.93, 0.90), "Gloves": (0.10, 0.11, 0.15),
        "Skin": (0.92, 0.70, 0.55), "Pads": (0.12, 0.13, 0.18), "Skate": (0.08, 0.08, 0.09), "Blade": (0.78, 0.82, 0.86),
        "Stick": (0.72, 0.52, 0.26)},
    "p3_clay": {
        "Jersey": (0.16, 0.30, 0.74), "Trim": (0.95, 0.91, 0.82), "Helmet": (0.20, 0.32, 0.76), "Gloves": (0.17, 0.20, 0.36),
        "Skin": (0.90, 0.66, 0.50), "Pads": (0.22, 0.20, 0.26), "Skate": (0.24, 0.17, 0.14), "Blade": (0.93, 0.90, 0.83),
        "Stick": (0.30, 0.22, 0.18)},
    "p4_broadcast": {
        "Jersey": (0.08, 0.22, 0.72), "Trim": (0.96, 0.96, 0.96), "Helmet": (0.07, 0.17, 0.58), "Gloves": (0.07, 0.15, 0.48),
        "Skin": (0.86, 0.64, 0.50), "Pads": (0.05, 0.07, 0.17), "Skate": (0.06, 0.06, 0.07), "Blade": (0.70, 0.76, 0.82),
        "Stick": (0.13, 0.13, 0.14)},
}

# slots that take the team colour in the renders (and should take TeamColour in Unreal)
TEAM_SLOTS = {"p2_cel": ("Jersey",), "p3_clay": ("Jersey", "Helmet"), "p4_broadcast": ("Jersey", "Helmet", "Gloves", "Pads")}

# the game's stick reach (R-0018: game.stick reach 1.6 m); aim holds at 1.0 m, far reach and the poke tip at 1.6 m
REACH_NEAR = 1.0
REACH_FAR = 1.6
