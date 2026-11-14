p='Game/Source/JamCore/Public/JamCarrierMarks.h'
s=open(p,encoding='utf-8').read()
old="		double OthersSec = 0.0;        // another opponent (not the one the ring is for) reaches the puck within this long after the human: no ring (0 = off)"
new="		double OthersSec = 0.3;        // another opponent (not the one the ring is for) reaches the puck within this long after the human: no ring (0 = off, the ring of\n		                               // lanes A166 to A194). Shipped 0.3 s: obeying every ring costs a pro 0.7 points (6 before) and the ring shows 12 times a match (21)"
assert old in s; s=s.replace(old,new)
open(p,'w',encoding='utf-8').write(s)
p='Game/Source/JamCore/Private/Tests/JamCarryTests.cpp'
s=open(p,encoding='utf-8').read()
old='''		Sim->SetSkater(Opp, FVector2D(6.0, 0.0), FVector2D(8.0, 0.0), 0.0);
		TestFalse(TEXT("the opponent skates away: not contested"), JamCarrierMarks::Contested(*Sim, Me).bOn);'''
new='''		{
			// Lane A201 (evolution 12): the ring is about one man. A second opponent at the puck within OthersSec after the human: no ring (the scene of
			// MOVEMENT-12 section 9 where holding off costs the match); with the gate off (0) the ring of before.
			int32 Opp2 = -1;
			for (const FJamSkaterState& K : Sim->GetSkaters()) if (K.Team != Sim->GetSkaters()[Me].Team && K.Index != Opp) { Opp2 = K.Index; break; }
			if (Opp2 >= 0)
			{
				const FVector2D Park2 = Sim->GetSkaters()[Opp2].Pos;
				Sim->SetSkater(Opp2, FVector2D(0.0, 5.5), FVector2D(0.0, -8.0), -UE_DOUBLE_PI / 2.0);
				JamCarrierMarks::FContestRule Before; Before.OthersSec = 0.0;
				TestFalse(TEXT("A201: a second opponent at the puck with them: no ring (OthersSec 0.3 s)"), JamCarrierMarks::Contested(*Sim, Me).bOn);
				TestTrue(TEXT("A201: ... with the gate off (OthersSec 0): the ring of before"), JamCarrierMarks::Contested(*Sim, Me, Before).bOn);
				Sim->SetSkater(Opp2, Park2, FVector2D::ZeroVector, 0.0);
				TestTrue(TEXT("A201: ... and alone with the one man again: the ring"), JamCarrierMarks::Contested(*Sim, Me).bOn);
			}
			JamCarrierMarks::FContestRule Spec;
			JamCarrierMarks::ApplyContestRuleSpec(Spec, TEXT("others=0.4;opptoward=3;hsec=0.8"));
			TestTrue(TEXT("A201: the ring's parse (the one the trace and the scripted player share)"), Spec.OthersSec == 0.4 && Spec.OppTowardMps == 3.0 && Spec.HumanSecMax == 0.8);
			TestTrue(TEXT("A201: the shipped ring: the second opponent's gate 0.3 s, the man's own approach off"), JamCarrierMarks::FContestRule().OthersSec == 0.3 && JamCarrierMarks::FContestRule().OppTowardMps == 0.0);
		}
		Sim->SetSkater(Opp, FVector2D(6.0, 0.0), FVector2D(8.0, 0.0), 0.0);
		TestFalse(TEXT("the opponent skates away: not contested"), JamCarrierMarks::Contested(*Sim, Me).bOn);'''
assert old in s; s=s.replace(old,new)
open(p,'w',encoding='utf-8').write(s)
print('ok')
