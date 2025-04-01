#include "CameraShake_AnimNotify.h"

#include "Player/SwordslikeCharacter.h"

void UCameraShake_AnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                     const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if(const ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(MeshComp))
	{
		if(const APlayerController* Controller = Cast<APlayerController>(Character->GetController()))
		{
			Controller->PlayerCameraManager->StartCameraShake(CameraShakeClass);
		}
	}
}
