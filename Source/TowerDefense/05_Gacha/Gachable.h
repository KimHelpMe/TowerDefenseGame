
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Gachable.generated.h"

class ATowerSlot;



UINTERFACE(MinimalAPI)
class UGachable : public UInterface
{
	GENERATED_BODY()
};


class TOWERDEFENSE_API IGachable
{
	GENERATED_BODY()


public:

	virtual FString GetItemName() = 0;

	virtual FString GetItemGrade() = 0;

	virtual UTexture2D* GetItemIcon() = 0;

	virtual void OnSelected(ATowerSlot* Slot) = 0;

};
