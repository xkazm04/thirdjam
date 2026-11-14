p='Game/Source/JamCore/Public/JamCarrierMarks.h'
s=open(p,encoding='utf-8').read()
old='''		int32 AfterDropFrames = 0;'''
new='''		// Lane A201 (track F2 evolution 12, MOVEMENT-12 section 7): the ring shows only where what it says is true. 0 = off (today's ring).
		double OppTowardMps = 0.0;     // the opponent himself skates at the human (not only at the puck) at this pace or more
		double OthersSec = 0.0;        // another opponent (not the one the ring is for) reaches the puck within this long after the human: no ring (0 = off)
		int32 AfterDropFrames = 0;'''
assert old in s; s=s.replace(old,new)
old='''			if (Closing < K.ClosingMps) continue;
			if (!R.bOn || To < R.OppSec)'''
new='''			if (Closing < K.ClosingMps) continue;
			if (K.OppTowardMps > 0.0 && D > 1e-6 && -FVector2D::DotProduct(O.Vel, Between / D) < K.OppTowardMps) continue;   // lane A201
			if (!R.bOn || To < R.OppSec)'''
assert old in s; s=s.replace(old,new)
old='''		return R;
	}

	// Lane A178 (track F2, evolution 8'''
new='''		if (R.bOn && K.OthersSec > 0.0)   // lane A201: a second opponent at the puck within OthersSec after the human (the ring is about one man)
		{
			for (const FJamSkaterState& O : Sk)
			{
				if (O.Team == H.Team || !O.CanAct() || O.Index == R.Opp) continue;
				double OT = 0.0;
				const double To = SecondsTo(O, P, PV, OT);
				if (To < Th + K.OthersSec) { R.bOn = false; break; }
			}
		}
		return R;
	}
	// Lane A201: the ring's numbers by name ("early=;late=;closing=;toward=;hsec=;opptoward=;others="), the one parse the trace and the scripted player share.
	inline void ApplyContestRuleSpec(FContestRule& K, const FString& Spec)
	{
		TArray<FString> Parts;
		Spec.ParseIntoArray(Parts, TEXT(";"));
		for (const FString& Part : Parts)
		{
			FString Key, Vs;
			if (!Part.Split(TEXT("="), &Key, &Vs)) continue;
			const double V = FCString::Atod(*Vs);
			if (Key == TEXT("early")) K.OppEarlySec = V; else if (Key == TEXT("late")) K.OppLateSec = V; else if (Key == TEXT("closing")) K.ClosingMps = V;
			else if (Key == TEXT("toward")) K.TowardMps = V; else if (Key == TEXT("hsec")) K.HumanSecMax = V;
			else if (Key == TEXT("opptoward")) K.OppTowardMps = V; else if (Key == TEXT("others")) K.OthersSec = V;
		}
	}
	inline FContestRule ContestRuleFromEnv()
	{
		FContestRule K;
		ApplyContestRuleSpec(K, FPlatformMisc::GetEnvironmentVariable(TEXT("JAM_MARK_RULE")));
		return K;
	}

	// Lane A178 (track F2, evolution 8'''
assert old in s; s=s.replace(old,new,1)
open(p,'w',encoding='utf-8').write(s)

p='Game/Source/JamCore/AI/JamPlaytester.cpp'
s=open(p,encoding='utf-8').read()
old='''			const char* ContestEnv = std::getenv("JAM_PLAYTESTER_CONTEST");   // lane A166: brake = the player obeys the contested-puck mark (a measurement's arm); absent = as shipped
			Tester.bContestBrake = ContestEnv && std::strcmp(ContestEnv, "brake") == 0;'''
new='''			// Lane A166: brake = the player obeys the contested-puck mark (a measurement's arm); absent = as shipped. Lane A201: brake:N obeys only the N-th
			// showing (the paired instrument); read through FPlatformMisc so CarryTrace's arms reach it.
			const FString ContestStr = FPlatformMisc::GetEnvironmentVariable(TEXT("JAM_PLAYTESTER_CONTEST"));
			Tester.bContestBrake = ContestStr.StartsWith(TEXT("brake"));
			Tester.ContestOnlyNth = ContestStr.StartsWith(TEXT("brake:")) ? FCString::Atoi(*ContestStr.Mid(6)) : 0;'''
assert old in s; s=s.replace(old,new)
old='''			if (Pi >= 0 && Tester.bContestBrake) { const UJamSim* MarkSim = Sim.GetSim(); Tester.bContestedNow = MarkSim && JamCarrierMarks::Contested(*MarkSim, Pi).bOn; }   // lane A166'''
new='''			if (Pi >= 0 && Tester.bContestBrake)   // lane A166; lane A201: the ring as SHOWN (StepShow, the HUD's hold), the rule by JAM_MARK_RULE (the one parse)
			{
				const UJamSim* MarkSim = Sim.GetSim();
				const bool bPlayNow = MarkSim && MarkSim->GetPhase() == EJamSimPhase::Playing && !MarkSim->IsDrawOpen();
				if (!bPlayNow) ConMarkShow = JamCarrierMarks::FMarkShow();
				const JamCarrierMarks::FContested Cn = bPlayNow ? JamCarrierMarks::Contested(*MarkSim, Pi, ConMarkRule) : JamCarrierMarks::FContested();
				const bool bShownNow = bPlayNow && JamCarrierMarks::StepShow(ConMarkShow, Cn.bOn, Cn.Opp, MarkSim->GetFrame());
				if (bShownNow && ConMarkShow.Since != ConShownSince) { ConShownSince = ConMarkShow.Since; ++ConShowings; }
				Tester.bContestedNow = bShownNow && (Tester.ContestOnlyNth <= 0 || ConShowings == Tester.ContestOnlyNth);
			}'''
assert old in s; s=s.replace(old,new)
old='''		JamCarrierMarks::FStopRule StopMarkRule = JamCarrierMarks::StopRuleFromEnv();   // lane A201: read once a match'''
new='''		JamCarrierMarks::FStopRule StopMarkRule = JamCarrierMarks::StopRuleFromEnv();   // lane A201: read once a match
		const JamCarrierMarks::FContestRule ConMarkRule = JamCarrierMarks::ContestRuleFromEnv();
		JamCarrierMarks::FMarkShow ConMarkShow;
		int64 ConShownSince = -1;
		int ConShowings = 0;'''
assert old in s; s=s.replace(old,new)
open(p,'w',encoding='utf-8').write(s)

p='Game/Source/JamCore/AI/JamPlaytester.h'
s=open(p,encoding='utf-8').read()
old='''		bool bContestedNow = false;'''
new='''		bool bContestedNow = false;
		int ContestOnlyNth = 0;          // lane A201: JAM_PLAYTESTER_CONTEST=brake:N obeys only the N-th showing of the ring (0 = every showing)'''
assert old in s; s=s.replace(old,new)
open(p,'w',encoding='utf-8').write(s)

p='Game/Source/JamCore/Public/JamCarryDrill.h'
s=open(p,encoding='utf-8').read()
old='''	int32 LitAt = -1;           // the frame the line first lit (-1: not yet)'''
new='''	int32 LitAt = -1;           // the frame the line first lit (-1: not yet)
	// Lane A201 (evolution 12, the late hand): true = the hand presses only if the line is still lit on the frame it presses; a press the line no longer
	// invites is not made and the carry ends Setup at once ("not invited": the share counts the stops begun while it was lit). LightRule = the line's rule.
	bool bOnlyWhileLit = false;
	JamCarrierMarks::FStopRule LightRule;'''
assert old in s; s=s.replace(old,new)
old='''				if (LitAt < 0 && JamCarrierMarks::StopMark(Sim, Me).bLit) LitAt = Frames;
				if (LitAt >= 0 && Frames - LitAt >= LightReactFrames) { bBegin = true; LightGapM = Gap; }'''
new='''				if (LitAt < 0 && JamCarrierMarks::StopMark(Sim, Me, LightRule).bLit) LitAt = Frames;
				if (LitAt >= 0 && Frames - LitAt >= LightReactFrames)
				{
					if (bOnlyWhileLit && !JamCarrierMarks::StopMark(Sim, Me, LightRule).bLit) { End = Setup; return In; }   // lane A201: not invited
					bBegin = true; LightGapM = Gap;
				}'''
assert old in s; s=s.replace(old,new)
old='''		return FString::Printf(TEXT("STOP WHEN HE IS %s STICK LENGTHS AWAY. CUT. GO. STILL ON YOUR SHOULDER? CUT BACK"), N);'''
new='''		// Lane A201 (evolution 12): the line now lights where the stop pays; the caption says to read it. One line at 1280.
		(void)N;
		return FString(TEXT("LINE IN HIS COLOUR: STOP. CUT. GO."));'''
assert old in s; s=s.replace(old,new)
open(p,'w',encoding='utf-8').write(s)
print('ok')
