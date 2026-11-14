p='Game/Source/JamCore/Public/JamCarrierMarks.h'
s=open(p,encoding='utf-8').read()
old="		double LitNearM = 7.0;         // lit while the opponent is this far off or more ..."
new="		double LitNearM = 7.5;         // lit while the opponent is this far off or more ... Lane A201 (evolution 12, the late hand): 7 -> 7.5 m: the bar goes out when a\n		                               // press would begin too late (law 3 at pro: 7.5 m 69 %, 7 m 38 %); of the presses a 12- to 24-frame hand makes while it is lit, 90.5 % or more pass"
assert old in s; s=s.replace(old,new)
open(p,'w',encoding='utf-8').write(s)
p='Game/Source/JamCore/Private/Tests/JamCarryTests.cpp'
s=open(p,encoding='utf-8').read()
old='''		TestTrue(TEXT("the defender 7.5 m ahead: still lit"), At(7.5, 0.0, -5.0).bLit);'''
new='''		TestTrue(TEXT("the defender 7.75 m ahead: still lit"), At(7.75, 0.0, -5.0).bLit);
		TestFalse(TEXT("A201 (evolution 12): the defender 7.25 m ahead: gone (the bar goes out at 7.5 m: a press begun later loses the feet)"), At(7.25, 0.0, -5.0).bShow);'''
assert old in s; s=s.replace(old,new)
open(p,'w',encoding='utf-8').write(s)
print('ok')
