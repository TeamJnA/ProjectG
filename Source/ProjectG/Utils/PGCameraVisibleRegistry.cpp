// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/PGCameraVisibleRegistry.h"
#include "Gimmick/CameraVisibleGimmick/PGCameraVisibleProp.h"


void UPGCameraVisibleRegistry::RegisterProp(APGCameraVisibleProp* Prop)
{
	if (!Prop)
	{
		return;
	}

	Props.AddUnique(Prop);
	Prop->SetCameraModeVisible(bCameraModeVisible);
}

void UPGCameraVisibleRegistry::UnregisterProp(APGCameraVisibleProp* Prop)
{
	Props.RemoveAllSwap([Prop](const TWeakObjectPtr<APGCameraVisibleProp>& Weak)
	{
		return Weak.Get() == Prop;
	});
}

void UPGCameraVisibleRegistry::SetCameraModeVisible(bool bVisible)
{
	if (bCameraModeVisible == bVisible)
	{
		return;
	}

	bCameraModeVisible = bVisible;

	for (int32 i = Props.Num() - 1; i >= 0; --i)
	{
		if (APGCameraVisibleProp* Prop = Props[i].Get())
		{
			Prop->SetCameraModeVisible(bVisible);
		}
		else
		{
			Props.RemoveAtSwap(i);
		}
	}
}
