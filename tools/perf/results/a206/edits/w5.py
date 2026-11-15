# lane A206 READY 5: the receiver holds his line and meets the pass
def edit(p, pairs):
    s = open(p, encoding='utf-8', newline='').read()
    for o, n in pairs:
        assert s.count(o) == 1, (p, o[:90]); s = s.replace(o, n)
    open(p, 'w', encoding='utf-8', newline='').write(s)
H='Game/Source/JamCore/AI/JamAICore.h'; C='Game/Source/JamCore/AI/JamAICore.cpp'; P='Game/Source/JamCore/AI/DifficultyProfile.cpp'
edit(H, [
("""		int PuckPassBy = -1;      // lane A206: the sim's Puck.PassBy (the passer of the pass in flight), -1 = none
""", """		int PuckPassBy = -1;      // lane A206: the sim's Puck.PassBy (the passer of the pass in flight), -1 = none
		int PuckPassTo = -1;      // lane A206 (READY 5): the sim's Puck.PassTo (the man the pass in flight is aimed at, under pass.catch_m), -1 = none
"""),
("""		bool bMarkMan9 = false;
""", """		bool bMarkMan9 = false;
		// bMeetLine9 (READY 5, R-0067 "no pass reaches the end"): the bot a pass in flight is aimed at (Puck.PassTo) holds the velocity he had at the release
		// for MeetHoldFrames (the passer's lead stays true), then skates to the point where the puck's line passes nearest him, and takes no other job until
		// the pass is caught, taken by another, or 90 frames have gone (MeetPass).
		bool bMeetLine9 = false;
"""),
("""	constexpr bool kEvo9CreaseKeeper = true; // wave 4
""", """	constexpr bool kEvo9CreaseKeeper = true; // wave 4
	constexpr bool kEvo9MeetLine = true;     // READY 5
	constexpr int kEvo9MeetHoldFrames = 12;
"""),
("""		H.bCreaseKeeper9 = bOn && kEvo9CreaseKeeper;""", """		H.bCreaseKeeper9 = bOn && kEvo9CreaseKeeper;
		H.bMeetLine9 = bOn && kEvo9MeetLine;"""),
("""		bool CreaseKeeper(const FJamAIWorld& W, int Id, FVec2& Out) const;   // lane A206 (evolution 9, wave 4)
""", """		bool CreaseKeeper(const FJamAIWorld& W, int Id, FVec2& Out) const;   // lane A206 (evolution 9, wave 4)
		bool MeetPass(const FJamAIWorld& W, int Id, FVec2& Out);   // lane A206 (evolution 9, READY 5)
		int MeetBy9[kMaxSkaters] = {}; int64_t MeetStart9[kMaxSkaters] = {}; FVec2 MeetVel9[kMaxSkaters];   // lane A206 READY 5: the pass being met
"""),
])
edit(C, [
("""			FVec2 Dir = Decide(W, i, bShoot, bAimed, bDeke, bPass);""", """			FVec2 Dir;
			if (!(SkillOf(W, i).Hk.bMeetLine9 && MeetPass(W, i, Dir))) { Dir = Decide(W, i, bShoot, bAimed, bDeke, bPass); }   // lane A206 (READY 5): the receiver meets the pass"""),
("""	void FJamBrain::LongShotToPass(const FJamAIWorld& W, int Id, FJamAIInput& In) const
""", """	// Lane A206 (track Q2, evolution 9, READY 5; BRAIN-9.md): MeetPass (bMeetLine9). A pass in flight aimed at this bot (Puck.PassTo == Id, by another skater,
	// nobody carrying): for kEvo9MeetHoldFrames from the frame he first sees it he keeps the velocity he had then (the passer led him on it); after that he
	// skates to the point where the puck's line passes nearest him (slowing inside 1 m) and plays nothing else, until the pass is caught or taken (the
	// sim clears PassTo) or 90 frames have gone.
	bool FJamBrain::MeetPass(const FJamAIWorld& W, int Id, FVec2& Out)
	{
		const FJamAISkater& S = W.Skaters[Id];
		if (W.PuckPassFrames <= 0 || W.PuckPassTo != Id || W.PuckPassBy < 0 || W.PuckPassBy == Id || W.CarrierOrHolder() >= 0 || S.bDown) { MeetBy9[Id] = -1; return false; }
		if (MeetBy9[Id] != W.PuckPassBy || W.Frame - MeetStart9[Id] > 120 || W.Frame < MeetStart9[Id]) { MeetBy9[Id] = W.PuckPassBy; MeetStart9[Id] = W.Frame; MeetVel9[Id] = S.Vel; }
		const int64_t Age = W.Frame - MeetStart9[Id];
		if (Age > 90) { return false; }
		bPlaysPuck[Id] = true;
		const double Top = FreeSpeed(SkillOf(W, Id).Hk) * Params.SpeedScale;
		if (Age < kEvo9MeetHoldFrames)
		{
			const double V = MeetVel9[Id].Length();
			Out = V > 0.5 ? MeetVel9[Id] * (1.0 / (Top > 1.0 ? Top : 1.0)) : FVec2();
			Out = Out.LimitLength(1.0);
			return true;
		}
		const FVec2 Pv = W.PuckVel;
		const double P2 = Pv.Dot(Pv);
		FVec2 Q = W.PuckPos;
		if (P2 > 1e-6)
		{
			double T = (S.Pos - W.PuckPos).Dot(Pv) / P2;
			T = T < 0.0 ? 0.0 : (T > 3.0 ? 3.0 : T);
			Q = W.PuckPos + Pv * T;
		}
		Out = Arrive(S.Pos, Q, 1.0);
		return true;
	}
	void FJamBrain::LongShotToPass(const FJamAIWorld& W, int Id, FJamAIInput& In) const
"""),
("""		DropFrame9 = -100000; bWasDrawOpen9 = false; DropPos9 = FVec2(); FirstDrop9 = -1;""", """		DropFrame9 = -100000; bWasDrawOpen9 = false; DropPos9 = FVec2(); FirstDrop9 = -1;
		for (int i = 0; i < kMaxSkaters; ++i) { MeetBy9[i] = -1; MeetStart9[i] = -100000; MeetVel9[i] = FVec2(); }   // lane A206 READY 5"""),
])
edit('Game/Source/JamCore/AI/JamSimAdapter.cpp', [("""		W.PuckPassFrames = S.GetPuck().PassFrames; W.PuckPassBy = S.GetPuck().PassBy;   // lane A206 (evolution 9, wave 2): A207's pass in flight, read by the brain
""", """		W.PuckPassFrames = S.GetPuck().PassFrames; W.PuckPassBy = S.GetPuck().PassBy;   // lane A206 (evolution 9, wave 2): A207's pass in flight, read by the brain
		W.PuckPassTo = S.GetPuck().PassTo;   // lane A206 (READY 5): the man the pass in flight is aimed at
""")])
edit(P, [("""			if (std::strcmp(Key, "hk_crease_keeper9") == 0)""", """			if (std::strcmp(Key, "hk_meet_line9") == 0) { K.Hk.bMeetLine9 = V != 0.0; return true; }
			if (std::strcmp(Key, "hk_crease_keeper9") == 0)""")])
edit('Game/Source/JamCore/Tests/HockeyBrain9Tests.cpp', [("""&& H.EarlyLongShot9Frames == 0;""", """&& H.EarlyLongShot9Frames == 0 && !H.bMeetLine9;""")])
print('ok')
