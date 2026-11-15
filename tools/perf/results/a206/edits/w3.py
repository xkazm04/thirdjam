# lane A206 wave 3: the source edits (run once from the worktree root)
def edit(p, pairs):
    s = open(p, encoding='utf-8', newline='').read()
    for o, n in pairs:
        assert s.count(o) == 1, (p, o[:90])
        s = s.replace(o, n)
    open(p, 'w', encoding='utf-8', newline='').write(s)

H = 'Game/Source/JamCore/AI/JamAICore.h'
edit(H, [
("""		double Dodge9Share = 0.0;
""", """		double Dodge9Share = 0.0;
		// Lane A206 wave 3 (R-0068, the eye's lists: EYE 1, EYE 8, EYE 2, EYE 3, item 12):
		// DropNoHit9Frames: > 0 = for this many frames after a faceoff's drop the bot plays the puck only: no body check, no cross-check (rookie 90; pro and
		// all_star 45). The losing centre charged and felled the human 40 frames after every drop.
		int DropNoHit9Frames = 0;
		// bPassCare9: a carrier passes for where the pass puts the puck: no pass to a mate nearer than 5 m, none in its first 20 frames with the puck, none
		// straight back to the mate it came from inside 60 frames while he is no nearer the net, none whose line runs through its own crease (3 m of the
		// own net); unless an opponent is on it (within 2 m). EYE 8: 41 of 81 passes re-passed within 15 frames, 18 of 58 straight back.
		bool bPassCare9 = false;
		// bDefendTeam9: the human's side defends without him (the human is not its presser unless he is goal-side within 6 m of the carrier); a defending bot
		// gives way to a team-mate inside 3 m after 20 frames unless both play the puck; and an opposing carrier inside 35 m of the net is never left alone
		// for more than 30 frames (nobody within 3 m): the nearest bot goes at him; the man in the crease steps onto a shot's line (EYE 2, EYE 3).
		bool bDefendTeam9 = false;
"""),
("""		if (!bOn) { H.OppPace9 = 0.0; H.AimError9Deg = 0.0; H.React9Frames = 0; H.ShotCharge9Frames = 0; H.Poke9Frames = 0; H.ShootExtra9M = 0.0; H.Dodge9Share = 0.0; }""",
 """		H.bPassCare9 = bOn && kEvo9PassCare;
		H.bDefendTeam9 = bOn && kEvo9DefendTeam;
		if (!bOn) { H.OppPace9 = 0.0; H.AimError9Deg = 0.0; H.React9Frames = 0; H.ShotCharge9Frames = 0; H.Poke9Frames = 0; H.ShootExtra9M = 0.0; H.Dodge9Share = 0.0; H.DropNoHit9Frames = 0; }"""),
("""	constexpr bool kEvo9DriveFirst = true; // wave 2
""", """	constexpr bool kEvo9DriveFirst = true; // wave 2
	constexpr bool kEvo9PassCare = true;   // wave 3
	constexpr bool kEvo9DefendTeam = true; // wave 3
"""),
("""		bool bWasFaceoff = false;
""", """		bool bWasFaceoff = false;
		int64_t DropFrame9 = -100000; bool bWasDrawOpen9 = false;   // lane A206 wave 3: the frame of the last faceoff drop (DropNoHit9Frames)
		int LastPassBy9 = -1; int ReceivedFrom9[kMaxSkaters] = {}; int64_t ReceivedFrame9[kMaxSkaters] = {};   // lane A206 wave 3 (bPassCare9)
		int AloneRun9[2] = {};   // lane A206 wave 3 (bDefendTeam9): frames an opposing carrier inside 35 m of this team's net has had nobody of it within 3 m
"""),
("""		FVec2 DodgeHit(const FJamAIWorld& W, int Id, const FVec2& Dir) const;   // lane A206 (evolution 9, wave 2)
""", """		FVec2 DodgeHit(const FJamAIWorld& W, int Id, const FVec2& Dir) const;   // lane A206 (evolution 9, wave 2)
		void PassCare(const FJamAIWorld& W, int Id, FJamAIInput& In) const;   // lane A206 (evolution 9, wave 3)
		bool NoCarrierAlone(const FJamAIWorld& W, int Id) const;   // lane A206 (evolution 9, wave 3)
"""),
])

C = 'Game/Source/JamCore/AI/JamAICore.cpp'
edit(C, [
("""		for (int i = 0; i < kMaxSkaters; ++i) { DownNear[i] = 0; DownNearFrame[i] = -10; bDownLeaving[i] = false; ClumpRun[i] = 0; ClumpFrame[i] = -10; bClumpSplit[i] = false; }   // lane A202
""", """		for (int i = 0; i < kMaxSkaters; ++i) { DownNear[i] = 0; DownNearFrame[i] = -10; bDownLeaving[i] = false; ClumpRun[i] = 0; ClumpFrame[i] = -10; bClumpSplit[i] = false; }   // lane A202
		DropFrame9 = -100000; bWasDrawOpen9 = false; LastPassBy9 = -1; AloneRun9[0] = AloneRun9[1] = 0;   // lane A206 wave 3
		for (int i = 0; i < kMaxSkaters; ++i) { ReceivedFrom9[i] = -1; ReceivedFrame9[i] = -100000; }
"""),
("""		AssignJobs(W);
		TeamIntent = FJamAIIntent();""", """		// Lane A206 wave 3: bookkeeping only (nothing steers here): the drop's frame, who passed the puck to whom, how long an opposing carrier near each net
		// has been alone
		if (W.bDrawOpen && !bWasDrawOpen9) { DropFrame9 = W.Frame; }
		bWasDrawOpen9 = W.bDrawOpen;
		for (int i = 0; i < W.NumSkaters; ++i)
		{
			if (CarryFrames[i] == 1)
			{
				const bool bMate = LastPassBy9 >= 0 && LastPassBy9 < W.NumSkaters && LastPassBy9 != i && W.Skaters[LastPassBy9].Team == W.Skaters[i].Team;
				ReceivedFrom9[i] = bMate ? LastPassBy9 : -1; ReceivedFrame9[i] = W.Frame;
			}
		}
		if (W.PuckPassBy >= 0) { LastPassBy9 = W.PuckPassBy; }
		else if (W.CarrierOrHolder() >= 0 && CarryFrames[W.CarrierOrHolder()] > 1) { LastPassBy9 = -1; }
		for (int Tm = 0; Tm < 2; ++Tm)
		{
			const int Cc = W.Carrier;
			bool bAlone = false;
			if (!W.bFrozen && Cc >= 0 && Cc < W.NumSkaters && W.Skaters[Cc].Team != Tm && W.Skaters[Cc].Pos.DistTo(W.OwnGoal(Tm)) < 35.0)
			{
				bAlone = true;
				for (int o = 0; o < W.NumSkaters; ++o) { if (W.Skaters[o].Team == Tm && !W.Skaters[o].bDown && W.Skaters[o].Pos.DistTo(W.Skaters[Cc].Pos) < 3.0) { bAlone = false; break; } }
			}
			AloneRun9[Tm] = bAlone ? AloneRun9[Tm] + 1 : 0;
		}
		AssignJobs(W);
		TeamIntent = FJamAIIntent();"""),
("""			if (SkillOf(W, i).Hk.LongPassM > 0.0) { LongShotToPass(W, i, In); }   // lane A202 (evolution 8): no shot from far with a mate open nearer
""", """			if (SkillOf(W, i).Hk.LongPassM > 0.0) { LongShotToPass(W, i, In); }   // lane A202 (evolution 8): no shot from far with a mate open nearer
			if (SkillOf(W, i).Hk.bPassCare9) { PassCare(W, i, In); }   // lane A206 (wave 3): a pass for where it puts the puck
			if (SkillOf(W, i).Hk.DropNoHit9Frames > 0 && W.Frame - DropFrame9 < SkillOf(W, i).Hk.DropNoHit9Frames) { In.bCheck = false; In.bLift = false; In.bCrossHold = false; }   // lane A206 (wave 3): the puck after a drop, not the man
"""),
("""	void FJamBrain::LongShotToPass(const FJamAIWorld& W, int Id, FJamAIInput& In) const
""", """	// Lane A206 (track Q2, evolution 9, wave 3; BRAIN-9.md): PassCare (bPassCare9). The pass of this frame (an edge or a saucer's hold) by the carrier is not
	// made when it goes to a mate nearer than 5 m, or in the carrier's first 20 frames with the puck, or straight back to the mate it came from inside 60
	// frames while he is no nearer the net than the carrier, or along a line that runs within 3 m of its own net; unless an opposing skater on its feet is
	// within 2 m (the pass out of pressure stays). The receiver is the sim's (CarrierBestPass / HolderBestPass). The carrier carries on.
	void FJamBrain::PassCare(const FJamAIWorld& W, int Id, FJamAIInput& In) const
	{
		if (!(In.bPass || In.bPassHeld) || W.CarrierOrHolder() != Id) { return; }
		const FJamAISkater& S = W.Skaters[Id];
		for (int o = 0; o < W.NumSkaters; ++o) { if (W.Skaters[o].Team != S.Team && !W.Skaters[o].bDown && W.Skaters[o].Pos.DistTo(S.Pos) < 2.0) { return; } }
		const int To = W.Carrier == Id ? W.CarrierBestPass : W.HolderBestPass;
		bool bVeto = CarryFrames[Id] < 20;
		if (To >= 0 && To < W.NumSkaters)
		{
			const FVec2 G = W.OppGoal(S.Team), Own = W.OwnGoal(S.Team), Rp = W.Skaters[To].Pos;
			if (Rp.DistTo(S.Pos) < 5.0) { bVeto = true; }
			if (To == ReceivedFrom9[Id] && W.Frame - ReceivedFrame9[Id] < 60 && Rp.DistTo(G) >= S.Pos.DistTo(G)) { bVeto = true; }
			const FVec2 L = Rp - S.Pos;
			const double Ll = L.Length();
			if (Ll > 1e-3)
			{
				const double Along = Clamp((Own - S.Pos).Dot(L) / Ll, 0.0, Ll);
				if ((S.Pos + L * (Along / Ll)).DistTo(Own) < 3.0) { bVeto = true; }   // item 12: never through the own crease
			}
		}
		if (bVeto) { In.bPass = false; In.bPassHeld = false; }
	}
	// Lane A206 (wave 3): NoCarrierAlone (bDefendTeam9). An opposing carrier inside 35 m of this bot's net has had nobody of its team within 3 m for more
	// than 30 frames: the bot of its team nearest him (the human aside) goes at him (Attack: the chase, the stick, the body as its level plays them).
	bool FJamBrain::NoCarrierAlone(const FJamAIWorld& W, int Id) const
	{
		const FJamAISkater& S = W.Skaters[Id];
		if (S.Team < 0 || S.Team > 1 || AloneRun9[S.Team] <= 30 || W.Carrier < 0 || W.Carrier >= W.NumSkaters) { return false; }
		const FVec2 Cp = W.Skaters[W.Carrier].Pos;
		int Best = -1; double Bd = 1e18;
		for (int o = 0; o < W.NumSkaters; ++o)
		{
			if (W.Skaters[o].Team != S.Team || W.IsHumanSeat(o) || W.Skaters[o].bDown) { continue; }
			const double D = W.Skaters[o].Pos.DistTo(Cp);
			if (D < Bd) { Bd = D; Best = o; }
		}
		return Best == Id;
	}
	void FJamBrain::LongShotToPass(const FJamAIWorld& W, int Id, FJamAIInput& In) const
"""),
# presser substitution in AssignJobs
("""				if (Kp >= 0) { Give(Take(Kp), EJob::Press); }""", """				// Lane A206 wave 3 (bDefendTeam9): the human is not his side's presser unless he is on the carrier (goal-side within 6 m): the nearest bot goes
				if (H.bDefendTeam9 && Kp >= 0 && W.IsHumanSeat(Fw[Kp]) && bTheirs
					&& !(W.Skaters[Fw[Kp]].Pos.DistTo(Cp) < 6.0 && (W.Skaters[Fw[Kp]].Pos - Cp).Dot(OwnGoal - Cp) > 0.0))
				{
					int Bk = -1; double Bd = 1e18;
					for (int k = 0; k < NF; ++k)
					{
						if (W.IsHumanSeat(Fw[k])) { continue; }
						const double D = W.Skaters[Fw[k]].Pos.DistTo(Cp) + ((W.Skaters[Fw[k]].Pos - Cp).Dot(OwnGoal - Cp) > 0.0 ? 0.0 : 6.0);
						if (D < Bd) { Bd = D; Bk = k; }
					}
					if (Bk >= 0) { Kp = Bk; }
				}
				if (Kp >= 0) { Give(Take(Kp), EJob::Press); }"""),
# clump thresholds while defending
("""		if (Mate < 0 || bIKeep || Md > (bClumpSplit[Id] ? 3.0 : 1.5)) { ClumpRun[Id] = 0; bClumpSplit[Id] = false; return Dir; }
		if (!bClumpSplit[Id] && ++ClumpRun[Id] < SkillOf(W, Id).Hk.ClumpFrames) { return Dir; }""", """		// lane A206 wave 3 (bDefendTeam9): defending (an opposing carrier), a bot gives way inside 3 m after 20 frames unless both play the puck
		const bool bDef9 = SkillOf(W, Id).Hk.bDefendTeam9 && Ch >= 0 && Ch < W.NumSkaters && W.Skaters[Ch].Team != S.Team
			&& !(Mate >= 0 && W.PuckPos.DistTo(S.Pos) < 3.0 && W.PuckPos.DistTo(W.Skaters[Mate].Pos) < 3.0);
		const double NearM = bDef9 ? 3.0 : 1.5, FarM = bDef9 ? 4.5 : 3.0;
		const int RunF = bDef9 ? 20 : SkillOf(W, Id).Hk.ClumpFrames;
		if (Mate < 0 || bIKeep || Md > (bClumpSplit[Id] ? FarM : NearM)) { ClumpRun[Id] = 0; bClumpSplit[Id] = false; return Dir; }
		if (!bClumpSplit[Id] && ++ClumpRun[Id] < RunF) { return Dir; }"""),
])
P = 'Game/Source/JamCore/AI/DifficultyProfile.cpp'
edit(P, [
("""			if (std::strcmp(Key, "hk_dodge9_share") == 0)""", """			if (std::strcmp(Key, "hk_drop_no_hit9_frames") == 0) { K.Hk.DropNoHit9Frames = static_cast<int>(V); return true; }
			if (std::strcmp(Key, "hk_pass_care9") == 0) { K.Hk.bPassCare9 = V != 0.0; return true; }
			if (std::strcmp(Key, "hk_defend_team9") == 0) { K.Hk.bDefendTeam9 = V != 0.0; return true; }
			if (std::strcmp(Key, "hk_dodge9_share") == 0)"""),
("""			P.Opp.Hk.Dodge9Share = 0.3;""", """			P.Opp.Hk.DropNoHit9Frames = 45;   // lane A206 (wave 3, EYE 1): pro plays the puck, not the man, for 45 frames after a drop
			P.Opp.Hk.Dodge9Share = 0.3;"""),
("""			P.Opp.Hk.Dodge9Share = 0.5;""", """			P.Opp.Hk.DropNoHit9Frames = 45;   // lane A206 (wave 3, EYE 1)
			P.Opp.Hk.Dodge9Share = 0.5;"""),
("""			Opp.Hk.OppPace9 = 0.9;
""", """			Opp.Hk.OppPace9 = 0.9;
			Opp.Hk.DropNoHit9Frames = 90;   // lane A206 (wave 3, EYE 1): rookie never hits in the 90 frames after a drop
"""),
])
T = 'Game/Source/JamCore/Tests/HockeyBrain9Tests.cpp'
edit(T, [("""&& H.Dodge9Share == 0.0;""", """&& H.Dodge9Share == 0.0 && H.DropNoHit9Frames == 0 && !H.bPassCare9 && !H.bDefendTeam9;""")])
print('ok')
edit(C, [
("""		const bool bPressKeeps = Hk.Hk.bPressInHunt""", """		// Lane A206 wave 3 (bDefendTeam9): an opposing carrier near our net alone for over 30 frames: the nearest bot goes at him
		if (Hk.Hk.bDefendTeam9 && !bYield && W.CarrierOrHolder() != Id && NoCarrierAlone(W, Id))
		{
			Used[Id] = ERole::Chaser;
			Out = Attack(W, Id, Me, Pp, Dirg, D, bShoot, bAimed, bDeke, bPass);
			return Out + Separate(Me, W, Id, Params.SeparateRadius) * 1.2;
		}
		const bool bPressKeeps = Hk.Hk.bPressInHunt"""),
("""				GoalieSideNow[Id] = std::fabs(Cy) < 1.0 ? 0 : static_cast<int>(Sign(Cy)) * (S.Team == 0 ? 1 : -1);
""", """				GoalieSideNow[Id] = std::fabs(Cy) < 1.0 ? 0 : static_cast<int>(Sign(Cy)) * (S.Team == 0 ? 1 : -1);
				// Lane A206 wave 3 (bDefendTeam9): the man in the crease plays a shot: onto its line (his body blocks), not standing on his post
				const double Pv9 = W.PuckVel.Length();
				if (SkillOf(W, Id).Hk.bDefendTeam9 && Pv9 > 6.0 && W.PuckVel.Dot(OwnGoal - Pp) > 0.0)
				{
					const FVec2 Dv = W.PuckVel * (1.0 / Pv9);
					const double Tg = std::fabs(Dv.X) > 1e-6 ? (OwnGoal.X - Pp.X) / Dv.X : -1.0;
					if (Tg > 0.0 && std::fabs(Pp.Y + Dv.Y * Tg) < W.GoalHalfWidth + 1.0)
					{
						double Along = (Me - Pp).Dot(Dv);
						Along = Along < 0.0 ? 0.0 : (Along > Tg ? Tg : Along);
						Out = Arrive(Me, Pp + Dv * Along - S.Vel * 0.25, 1.0);
					}
				}
"""),
])
print('ok2')
