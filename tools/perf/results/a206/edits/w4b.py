def edit(p, pairs):
    s = open(p, encoding='utf-8', newline='').read()
    for o, n in pairs:
        assert s.count(o) == 1, (p, o[:90]); s = s.replace(o, n)
    open(p, 'w', encoding='utf-8', newline='').write(s)
H='Game/Source/JamCore/AI/JamAICore.h'; C='Game/Source/JamCore/AI/JamAICore.cpp'; P='Game/Source/JamCore/AI/DifficultyProfile.cpp'
edit(H, [
("""		bool bScrumStay9 = false;
""", """		bool bScrumStay9 = false;
		// bNetNeverEmpty9 (wave 4, the Director 03:00: "a net is never empty"): the crease keeper keeps the crease whenever the puck is outside the end his
		// side attacks (the neutral zone or his own end), not only when the other side has it.
		bool bNetNeverEmpty9 = false;
		// EarlyLongShot9Frames: > 0 = the opponents take no shot from beyond 15 m in this many frames from the match's first drop (rookie 600: the first goal
		// against a newcomer fell at frame 360 in 7 matches of 7, a shot from the scrum).
		int EarlyLongShot9Frames = 0;
"""),
("""		H.bCreaseKeeper9 = bOn && kEvo9CreaseKeeper;""", """		H.bCreaseKeeper9 = bOn && kEvo9CreaseKeeper;
		H.bNetNeverEmpty9 = bOn && kEvo9CreaseKeeper;"""),
("""		if (!bOn) { H.bScrumStay9 = false;""", """		if (!bOn) { H.EarlyLongShot9Frames = 0; H.bScrumStay9 = false;"""),
("""		int64_t DropFrame9 = -100000; bool bWasDrawOpen9 = false; FVec2 DropPos9;""", """		int64_t DropFrame9 = -100000; bool bWasDrawOpen9 = false; FVec2 DropPos9; int64_t FirstDrop9 = -1;"""),
])
edit(C, [
("""		if (W.bDrawOpen && !bWasDrawOpen9) { DropFrame9 = W.Frame; DropPos9 = W.PuckPos; }""", """		if (W.bDrawOpen && !bWasDrawOpen9) { DropFrame9 = W.Frame; DropPos9 = W.PuckPos; if (FirstDrop9 < 0 || W.Frame < FirstDrop9) { FirstDrop9 = W.Frame; } }"""),
("""		DropFrame9 = -100000; bWasDrawOpen9 = false; DropPos9 = FVec2();""", """		DropFrame9 = -100000; bWasDrawOpen9 = false; DropPos9 = FVec2(); FirstDrop9 = -1;"""),
("""		const bool bScrum = SkillOf(W, Id).Hk.bScrumStay9 && W.Frame - DropFrame9 < 180 && Pp.DistTo(DropPos9) < 8.0;
		if (!(bTheirs || bShot || bScrum) || Pp.DistTo(OwnGoal) > 45.0) { return false; }""", """		const bool bScrum = SkillOf(W, Id).Hk.bScrumStay9 && W.Frame - DropFrame9 < 180 && Pp.DistTo(DropPos9) < 8.0;
		// wave 4 (bNetNeverEmpty9): the puck outside the end his side attacks (its third of the rink) and not his own carry
		const double Up = Sign(-OwnGoal.X);
		const bool bNotAttacking = SkillOf(W, Id).Hk.bNetNeverEmpty9 && Pp.X * Up < W.HalfLength / 3.0 && Ch != Id;
		if (!(bTheirs || bShot || bScrum || bNotAttacking)) { return false; }
		if (!bNotAttacking && Pp.DistTo(OwnGoal) > 45.0) { return false; }"""),
("""			if (SkillOf(W, i).Hk.bPassCare9) { PassCare(W, i, In); }   // lane A206 (wave 3): a pass for where it puts the puck
""", """			if (SkillOf(W, i).Hk.bPassCare9) { PassCare(W, i, In); }   // lane A206 (wave 3): a pass for where it puts the puck
			if (SkillOf(W, i).Hk.EarlyLongShot9Frames > 0 && FirstDrop9 >= 0 && W.Frame - FirstDrop9 < SkillOf(W, i).Hk.EarlyLongShot9Frames
				&& W.PuckPos.DistTo(W.OppGoal(W.Skaters[i].Team)) > 15.0) { In.bShoot = false; In.bShootHeld = false; }   // lane A206 (wave 4): no long shot in a rookie match's first 10 s
"""),
])
edit(P, [
("""			Opp.Hk.bScrumStay9 = true;""", """			Opp.Hk.EarlyLongShot9Frames = 600;   // lane A206 (wave 4): rookie's opponents shoot from beyond 15 m only after the match's first 10 s
			Opp.Hk.bScrumStay9 = true;"""),
("""			if (std::strcmp(Key, "hk_scrum_stay9") == 0)""", """			if (std::strcmp(Key, "hk_net_never_empty9") == 0) { K.Hk.bNetNeverEmpty9 = V != 0.0; return true; }
			if (std::strcmp(Key, "hk_early_long_shot9_frames") == 0) { K.Hk.EarlyLongShot9Frames = static_cast<int>(V); return true; }
			if (std::strcmp(Key, "hk_scrum_stay9") == 0)"""),
])
edit('Game/Source/JamCore/Tests/HockeyBrain9Tests.cpp', [("""&& !H.bScrumStay9;""", """&& !H.bScrumStay9 && !H.bNetNeverEmpty9 && H.EarlyLongShot9Frames == 0;""")])
print('ok')
