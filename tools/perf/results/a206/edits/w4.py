# lane A206 wave 4: the source edits (run once from the worktree root)
def edit(p, pairs):
    s = open(p, encoding='utf-8', newline='').read()
    for o, n in pairs:
        assert s.count(o) == 1, (p, o[:90])
        s = s.replace(o, n)
    open(p, 'w', encoding='utf-8', newline='').write(s)

C = 'Game/Source/JamCore/AI/JamAICore.cpp'
H = 'Game/Source/JamCore/AI/JamAICore.h'
P = 'Game/Source/JamCore/AI/DifficultyProfile.cpp'

# 1. NoCarrierAlone never takes the man in the crease (the first back of a five) or the presser's cover man off his post
edit(C, [
("""		for (int o = 0; o < W.NumSkaters; ++o)
		{
			if (W.Skaters[o].Team != S.Team || W.IsHumanSeat(o) || W.Skaters[o].bDown) { continue; }
			const double D = W.Skaters[o].Pos.DistTo(Cp);
			if (D < Bd) { Bd = D; Best = o; }
		}
		return Best == Id;""",
"""		for (int o = 0; o < W.NumSkaters; ++o)
		{
			if (W.Skaters[o].Team != S.Team || W.IsHumanSeat(o) || W.Skaters[o].bDown) { continue; }
			// wave 4: the crease man (the first back of a five) keeps the crease: he is never the one sent
			if (W.Skaters[o].Role == ERole::Back && TeamSize(W, S.Team) >= 5 && FiveLocal::RoleOrdinal(W, o) == 0) { continue; }
			const double D = W.Skaters[o].Pos.DistTo(Cp);
			if (D < Bd) { Bd = D; Best = o; }
		}
		return Best == Id;"""),
])
print('ok')

# 2. the crease keeper (both sides): the first back of a five keeps the crease while the other side has the puck within 45 m of his net
edit(H, [
("""		bool bDefendTeam9 = false;
""", """		bool bDefendTeam9 = false;
		// bCreaseKeeper9 (wave 4, EYE 3): the first back of a five takes the crease (1.5 m out, on the line net -> puck) whenever the other side carries the
		// puck within 45 m of his net or a shot comes at it, and steps onto a shot's line (his body blocks): both backs stood 16 m out at the far shots.
		bool bCreaseKeeper9 = false;
"""),
("""		H.bDefendTeam9 = bOn && kEvo9DefendTeam && !bMate;""", """		H.bDefendTeam9 = bOn && kEvo9DefendTeam && !bMate;
		H.bCreaseKeeper9 = bOn && kEvo9CreaseKeeper;"""),
("""	constexpr bool kEvo9DefendTeam = true; // wave 3
""", """	constexpr bool kEvo9DefendTeam = true; // wave 3
	constexpr bool kEvo9CreaseKeeper = true; // wave 4
"""),
("""		bool NoCarrierAlone(const FJamAIWorld& W, int Id) const;   // lane A206 (evolution 9, wave 3)
""", """		bool NoCarrierAlone(const FJamAIWorld& W, int Id) const;   // lane A206 (evolution 9, wave 3)
		bool CreaseKeeper(const FJamAIWorld& W, int Id, FVec2& Out) const;   // lane A206 (evolution 9, wave 4)
"""),
])
edit(C, [
("""		// Lane A206 wave 3 (bDefendTeam9): an opposing carrier near our net alone for over 30 frames: the nearest bot goes at him""",
 """		// Lane A206 wave 4 (bCreaseKeeper9): the first back keeps the crease while the other side has the puck near our net
		if (Hk.Hk.bCreaseKeeper9 && D > 2.5 && CreaseKeeper(W, Id, Out)) { Used[Id] = ERole::Back; bGoalieNow[Id] = true; return Out; }
		// Lane A206 wave 3 (bDefendTeam9): an opposing carrier near our net alone for over 30 frames: the nearest bot goes at him"""),
("""	void FJamBrain::LongShotToPass(const FJamAIWorld& W, int Id, FJamAIInput& In) const
""", """	// Lane A206 (wave 4, EYE 3): CreaseKeeper (bCreaseKeeper9). The first back of a five, while the other side carries the puck within 45 m of his net
	// (or a puck flies at it faster than 6 m/s), stands 1.5 m out on the line net -> puck, and on a shot steps onto the shot's line.
	bool FJamBrain::CreaseKeeper(const FJamAIWorld& W, int Id, FVec2& Out) const
	{
		const FJamAISkater& S = W.Skaters[Id];
		if (S.Role != ERole::Back || TeamSize(W, S.Team) < 5 || FiveLocal::RoleOrdinal(W, Id) != 0) { return false; }
		const FVec2 OwnGoal = W.OwnGoal(S.Team), Pp = W.PuckPos;
		const int Ch = W.CarrierOrHolder();
		const double Pv = W.PuckVel.Length();
		const bool bShot = Pv > 6.0 && W.PuckVel.Dot(OwnGoal - Pp) > 0.0 && Ch < 0;
		const bool bTheirs = Ch >= 0 && Ch < W.NumSkaters && W.Skaters[Ch].Team != S.Team;
		if (!(bTheirs || bShot) || Pp.DistTo(OwnGoal) > 45.0) { return false; }
		FVec2 T = OwnGoal + (Pp - OwnGoal).Normalized() * 1.5;
		if (std::fabs(T.X) > W.GoalX) { T.X = OwnGoal.X; }
		if (bShot)
		{
			const FVec2 Dv = W.PuckVel * (1.0 / Pv);
			const double Tg = std::fabs(Dv.X) > 1e-6 ? (OwnGoal.X - Pp.X) / Dv.X : -1.0;
			if (Tg > 0.0 && std::fabs(Pp.Y + Dv.Y * Tg) < W.GoalHalfWidth + 1.0)
			{
				double Along = (S.Pos - Pp).Dot(Dv);
				Along = Along < 0.0 ? 0.0 : (Along > Tg ? Tg : Along);
				T = Pp + Dv * Along;
			}
		}
		Out = Arrive(S.Pos, T - S.Vel * 0.25, 1.0);
		return true;
	}
	void FJamBrain::LongShotToPass(const FJamAIWorld& W, int Id, FJamAIInput& In) const
"""),
])
edit(P, [("""			if (std::strcmp(Key, "hk_defend_team9") == 0)""", """			if (std::strcmp(Key, "hk_crease_keeper9") == 0) { K.Hk.bCreaseKeeper9 = V != 0.0; return true; }
			if (std::strcmp(Key, "hk_defend_team9") == 0)""")])
edit('Game/Source/JamCore/Tests/HockeyBrain9Tests.cpp', [("""&& !H.bPassCare9 && !H.bDefendTeam9;""", """&& !H.bPassCare9 && !H.bDefendTeam9 && !H.bCreaseKeeper9;""")])
print('ok2')
