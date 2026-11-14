p='Game/Source/JamCore/Private/Tests/JamCarryTests.cpp'
s=open(p,encoding='utf-8').read()
# ring scene struct
old='''		TArray<FScene> Scenes;
		int64 LastRawLit = -1000;'''
new='''		TArray<FScene> Scenes;
		int64 LastRawLit = -1000;
		// Lane A201 (evolution 12): every showing of the contested ring (as shown: StepShow), the scene at its first frame and what followed in 90 frames.
		// Out: 0 he got the puck and stayed up 60 frames, 1 he got it and was down inside 60 frames of it, 2 the ring's opponent got it, 3 another opponent got it,
		// 4 a mate got it, 5 nobody carried it in 90 frames. bObey: he reverse-braked inside the first 20 frames of the showing (the obeying player's stop).
		struct FRing
		{
			int64 Start = 0; int32 Opp = -1, Out = -1, Got = -1; int64 GotAt = -1; bool bObey = false, bDown = false;
			double HSec = 0, OSec = 0, Closing = 0, Speed = 0, OppToward = 0, OtherSec = 99, MateSec = 99, NetM = 0;
			int32 Gf0 = 0, Ga0 = 0, Gf = -1, Ga = -1;
		};
		TArray<FRing> Rings;'''
assert old in s; s=s.replace(old,new)
# capture: after bConShown computed
old='''		if (bConShown) { if (T.ConShownSince < 0) { T.ConShownSince = Now; ++T.ConEpisodes; } ++T.ConFrames; }'''
new='''		if (bConShown && T.ConShownSince < 0 && Con.bOn)   // lane A201: the ring's scene at its first shown frame
		{
			FTrace::FRing Rg;
			Rg.Start = Now; Rg.Opp = Con.Opp; Rg.HSec = Con.HumanSec; Rg.OSec = Con.OppSec; Rg.Closing = Con.ClosingMps; Rg.Speed = H.Vel.Size();
			if (Sk.IsValidIndex(Con.Opp)) { const FVector2D Bt = (H.Pos - Sk[Con.Opp].Pos).GetSafeNormal(); Rg.OppToward = FVector2D::DotProduct(Sk[Con.Opp].Vel, Bt); }
			for (const FJamSkaterState& O : Sk)
			{
				if (O.Index == T.Me || !O.CanAct()) continue;
				double Tw = 0.0;
				const double Sec = JamCarrierMarks::SecondsTo(O, Puck.Pos, Puck.Vel, Tw);
				if (O.Team != H.Team && O.Index != Con.Opp) Rg.OtherSec = FMath::Min(Rg.OtherSec, Sec);
				if (O.Team == H.Team) Rg.MateSec = FMath::Min(Rg.MateSec, Sec);
			}
			Rg.NetM = FVector2D::Distance(H.Pos, FVector2D(Sim->AttackGoalX(H.Team), 0.0));
			Rg.Gf0 = Sim->GetScore(H.Team); Rg.Ga0 = Sim->GetScore(1 - H.Team);
			T.Rings.Add(Rg);
		}
		for (int32 Ri = FMath::Max(0, T.Rings.Num() - 8); Ri < T.Rings.Num(); ++Ri)
		{
			FTrace::FRing& Rg = T.Rings[Ri];
			const int64 Age = Now - Rg.Start;
			if (Age <= 20 && H.bReverseBraking) Rg.bObey = true;
			if (Age <= 150 && H.IsDown()) Rg.bDown = true;
			if (Rg.Got < 0 && Age <= 90 && Ci >= 0) { Rg.Got = Ci; Rg.GotAt = Now; }
			if (Rg.Out < 0 && Rg.Got >= 0)
			{
				if (Rg.Got == T.Me) { if (H.IsDown()) Rg.Out = 1; else if (Now - Rg.GotAt >= 60) Rg.Out = 0; }
				else Rg.Out = Rg.Got == Rg.Opp ? 2 : (Sk[Rg.Got].Team == H.Team ? 4 : 3);
			}
			if (Rg.Out < 0 && Rg.Got < 0 && Age > 90) Rg.Out = 5;
			if (Age == 600 || (Rg.Gf < 0 && Sim->GetPhase() == EJamSimPhase::Over)) { Rg.Gf = Sim->GetScore(H.Team) - Rg.Gf0; Rg.Ga = Sim->GetScore(1 - H.Team) - Rg.Ga0; }
		}
		if (bConShown) { if (T.ConShownSince < 0) { T.ConShownSince = Now; ++T.ConEpisodes; } ++T.ConFrames; }'''
assert old in s; s=s.replace(old,new)
# output rows + summary per match
old='''		for (FTrace::FScene& Sc : T->Scenes)   // lane A201: one row a raw lighting'''
new='''		for (FTrace::FRing& Rg : T->Rings)   // lane A201 (evolution 12): one row a showing of the ring
		{
			if (Rg.Gf < 0) { Rg.Gf = S.GoalsFor - Rg.Gf0; Rg.Ga = S.GoalsAgainst - Rg.Ga0; }
			AddInfo(FString::Printf(TEXT("RINGSCENE arm=%s level=%s seed=%d f=%lld hsec=%.3f osec=%.3f cl=%.2f v=%.2f opptoward=%.2f othersec=%.3f matesec=%.3f net=%.2f obey=%d out=%d down=%d gf=%d ga=%d res=%d"),
				*ArmLabel, *O.Level, Seed, static_cast<long long>(Rg.Start), Rg.HSec, Rg.OSec, Rg.Closing, Rg.Speed, Rg.OppToward, FMath::Min(Rg.OtherSec, 99.0), FMath::Min(Rg.MateSec, 99.0), Rg.NetM,
				Rg.bObey ? 1 : 0, Rg.Out, Rg.bDown ? 1 : 0, Rg.Gf, Rg.Ga, S.GoalsFor > S.GoalsAgainst ? 1 : (S.GoalsFor < S.GoalsAgainst ? -1 : 0)));
		}
		{
			int32 DownsAll = 0, DownsLoose = 0;
			for (const FDown& Dn : T->Downs) { ++DownsAll; if (Dn.Class == NoPuck || Dn.Class == CarryYoung) ++DownsLoose; }
			AddInfo(FString::Printf(TEXT("RINGMATCH arm=%s level=%s seed=%d rings=%d downs=%d downs_loose=%d"), *ArmLabel, *O.Level, Seed, T->Rings.Num(), DownsAll, DownsLoose));
		}
		for (FTrace::FScene& Sc : T->Scenes)   // lane A201: one row a raw lighting'''
assert old in s; s=s.replace(old,new)
# arms 5th field
old='''				while (P.Num() < 4) P.Add(FString());'''
new='''				while (P.Num() < 5) P.Add(FString());
				FPlatformMisc::SetEnvironmentVar(TEXT("JAM_PLAYTESTER_CONTEST"), *P[4]);   // lane A201 (evolution 12): the 5th part: the ring's obedience (brake | brake:N | empty)'''
assert old in s; s=s.replace(old,new)
s=s.replace('''"label@stopmark@rule[@hand]|..." plays every arm''','''"label@stopmark@rule[@hand[@contest]]|..." plays every arm''')
# ReadMarkRule: con keys by the shared parse
old='''			if (K == TEXT("early")) T.ConRule.OppEarlySec = V; else if (K == TEXT("late")) T.ConRule.OppLateSec = V; else if (K == TEXT("closing")) T.ConRule.ClosingMps = V;
			else if (K == TEXT("toward")) T.ConRule.TowardMps = V; else if (K == TEXT("hsec")) T.ConRule.HumanSecMax = V;
			else if (K == TEXT("minshow"))'''
new='''			if (K == TEXT("minshow"))'''
assert old in s; s=s.replace(old,new)
old='''		JamCarrierMarks::ApplyStopRuleSpec(T.StopRule, Env);   // lane A201: the stop line's parts by the one parse the scripted player reads too'''
new='''		JamCarrierMarks::ApplyStopRuleSpec(T.StopRule, Env);   // lane A201: the stop line's parts by the one parse the scripted player reads too
		JamCarrierMarks::ApplyContestRuleSpec(T.ConRule, Env);   // ... and the ring's (evolution 12)'''
assert old in s; s=s.replace(old,new)
open(p,'w',encoding='utf-8').write(s)

p='Game/Source/JamCore/Private/Tests/JamCarrierSkillTests.cpp'
s=open(p,encoding='utf-8').read()
old='''		for (const int32 React : { 0, 6, 9, 12, 15, 18, 21, 24 })   // lane A201: the stop begun on the stop line's light, after the hand's lag
		{
			FScript L;
			L.LightReactFrames = React;
			AddInfo(FString::Printf(TEXT("LAW3T %s | %s | stop on the light + %d frames: %s"), *Level, *Tune, React, *CarryHow(*C, L, 200, *Level, T).Line()));
		}'''
new='''		for (const int32 React : { 0, 6, 9, 12, 15, 18, 21, 24 })   // lane A201: the stop begun on the stop line's light, after the hand's lag
		{
			FScript L;
			L.LightReactFrames = React;
			AddInfo(FString::Printf(TEXT("LAW3T %s | %s | stop on the light + %d frames: %s"), *Level, *Tune, React, *CarryHow(*C, L, 200, *Level, T).Line()));
		}
		// Lane A201 (evolution 12, the late hand): the press made only while the bar is still lit (JAM_MARK_RULE moves the line's rule: near=, ...).
		for (const int32 React : { 12, 15, 18, 21, 24, 27, 30 })
		{
			FScript L;
			L.LightReactFrames = React; L.bOnlyWhileLit = true; L.LightRule = JamCarrierMarks::StopRuleFromEnv();
			const FShare W = Share(*C, L, 200, *Level, T);
			const int32 Invited = W.N - W.Ends[FJamCarryDrill::Setup];
			AddInfo(FString::Printf(TEXT("LAW3T %s | %s | late hand + %d frames, only while lit: invited %d of %d, past %d of those (%.1f %%)"), *Level, *Tune, React, Invited, W.N, W.Ends[FJamCarryDrill::Past], Invited > 0 ? 100.0 * W.Ends[FJamCarryDrill::Past] / Invited : 0.0));
		}'''
assert old in s; s=s.replace(old,new)
old='''	TestTrue(TEXT("the drill's line says the distance"), FJamCarryDrill::CoachLine(1, 8.0, 1.6).Contains(TEXT("FIVE STICK LENGTHS")));'''
new='''	// Lane A201 (evolution 12): the drill's words are what the line teaches now: stop when it turns his colour. One line at 1280 (40 characters or fewer).
	TestTrue(TEXT("A201: the drill's line says to stop on the line's colour"), FJamCarryDrill::CoachLine(1, 8.5, 1.6).Contains(TEXT("LINE IN HIS COLOUR")) && FJamCarryDrill::CoachLine(1, 8.5, 1.6).Len() <= 40);'''
assert old in s; s=s.replace(old,new)
# gate: the late hand
old='''		TestTrue(FString::Printf(TEXT("pro: the stop begun on the light passes 90 %% or more (%.1f %%)"), 100.0 * W.PastShare()), W.PastShare() >= 0.90);
	}'''
new='''		TestTrue(FString::Printf(TEXT("pro: the stop begun on the light passes 90 %% or more (%.1f %%)"), 100.0 * W.PastShare()), W.PastShare() >= 0.90);
		// Lane A201 (evolution 12, the late hand): a press made while the bar is still lit passes 90 % or more for hands of 12 to 24 frames.
		for (const int32 React : { 12, 18, 24 })
		{
			FScript Lh;
			Lh.LightReactFrames = React; Lh.bOnlyWhileLit = true;
			const FShare Wl = Share(*C, Lh, 200, TEXT("pro"));
			const int32 Invited = Wl.N - Wl.Ends[FJamCarryDrill::Setup];
			const double Pass = Invited > 0 ? static_cast<double>(Wl.Ends[FJamCarryDrill::Past]) / Invited : 0.0;
			AddInfo(FString::Printf(TEXT("LATE pro, a press %d frames after the light, made only while lit: invited %d of %d, past %.1f %% of those"), React, Invited, Wl.N, 100.0 * Pass));
			TestTrue(FString::Printf(TEXT("A201: pro, a %d-frame hand pressing while the bar is lit passes 90 %% or more (%.1f %% of %d)"), React, 100.0 * Pass, Invited), Pass >= 0.90);
		}
	}'''
assert old in s; s=s.replace(old,new)
open(p,'w',encoding='utf-8').write(s)
print('ok')
