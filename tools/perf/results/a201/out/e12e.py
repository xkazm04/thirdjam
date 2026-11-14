p='Game/Source/JamCore/Private/Tests/JamCarrierSkillTests.cpp'
s=open(p,encoding='utf-8').read()
old='''			TestTrue(FString::Printf(TEXT("A201: pro, a %d-frame hand pressing while the bar is lit passes 90 %% or more (%.1f %% of %d)"), React, 100.0 * Pass, Invited), Pass >= 0.90);'''
new='''			TestTrue(FString::Printf(TEXT("A201: pro, a %d-frame hand pressing while the bar is lit passes 90 %% or more (%.1f %% of %d; none invited = no late press made)"), React, 100.0 * Pass, Invited), Invited == 0 || Pass >= 0.90);
			if (React <= 18) TestTrue(FString::Printf(TEXT("A201: ... and the bar still invites a %d-frame hand (%d of %d)"), React, Invited, Wl.N), Invited >= Wl.N * 3 / 4);'''
assert old in s; s=s.replace(old,new)
open(p,'w',encoding='utf-8').write(s)
print('ok')
