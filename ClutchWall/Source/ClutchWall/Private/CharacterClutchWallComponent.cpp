// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterClutchWallComponent.h"
#include <GameFramework/Character.h>
#include <Kismet/KismetSystemLibrary.h>
#include <GameFramework/CharacterMovementComponent.h>
#include <Engine/Engine.h>
#include <Engine/EngineTypes.h>
#include <Engine/SplineMeshActor.h>
#include <Components/SplineComponent.h>
#include <Components/CapsuleComponent.h>


// Sets default values for this component's properties
UCharacterClutchWallComponent::UCharacterClutchWallComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCharacterClutchWallComponent::BeginPlay()
{
	Super::BeginPlay();

	if (CharacterOwner)
	{
		CharacterOwner->MovementModeChangedDelegate.AddDynamic(this, &UCharacterClutchWallComponent::OnMovementModeChange);
	}
	// ...
}


// Called every frame
void UCharacterClutchWallComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CharacterOwner && CurrentSplineMeshActor.IsValid())
	{
		if (bIsClutching)
		{
			
		}
		else if (bEnableAutoClutch)
		{
			FVector CharacterLocation = CharacterOwner->GetActorLocation();
			ClutchSplineLength = GetClutchSplineDistance(CharacterLocation);
			FVector SplineLocation = GetSpline()->GetLocationAtDistanceAlongSpline(ClutchSplineLength, ESplineCoordinateSpace::World);
			float Height = SplineLocation.Z-CharacterLocation.Z - CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - ClutchHeight;
			if (Height > 0.f && Height < 10.f)
			{
				StartClutchWall();
			}
			if (Height < 0.f)
			{

			}
		}
		// FVector SplinePosition = GetClutchPosition(CharacterOwner->GetActorLocation());
		// if (SplinePosition == FVector(FLT_MAX))
		// {
		//	 SplinePosition = GetClutchPosition(CharacterLocationCache);
		//	 if (SplinePosition == FVector(FLT_MAX))
		//   {
		//		EndClutchWall();
		//		return;
		//	 }
		// }
		// CharacterOwner->SetActorLocation(SplinePosition);
		// CharacterLocationCache = SplinePosition;
		// GEngine->AddOnScreenDebugMessage(0, 2.f, FColor::Green, *CharacterOwner->Internal_GetPendingMovementInputVector().ToString());
	}
	// ...
}

void UCharacterClutchWallComponent::RegisterComponentTickFunctions(bool bRegister)
{
	Super::RegisterComponentTickFunctions(bRegister);

	// ...
}

void UCharacterClutchWallComponent::PostLoad()
{
	Super::PostLoad();

	CharacterOwner = Cast<ACharacter>(GetOwner());
	// ...
}

void UCharacterClutchWallComponent::Deactivate()
{
	Super::Deactivate();

	// ...
}

void UCharacterClutchWallComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	// ...
}

void UCharacterClutchWallComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// ...
}

void UCharacterClutchWallComponent::OnRegister()
{
	Super::OnRegister();

	// ...
}

void UCharacterClutchWallComponent::OnMovementModeChange(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	if (Character)
	{
		if (UCharacterMovementComponent* MoveComp = Cast<UCharacterMovementComponent>(Character->GetMovementComponent()))
		{
			if(MoveComp->MovementMode ==EMovementMode::MOVE_Falling)
			{
				CharacterOwner->OnActorBeginOverlap.AddDynamic(this, &UCharacterClutchWallComponent::OnActorBeginOverlap);
				CharacterOwner->OnActorEndOverlap.AddDynamic(this, &UCharacterClutchWallComponent::OnActorEndOverlap);
				TArray<AActor*> SpineActors;
				CharacterOwner->GetOverlappingActors(SpineActors, SplineActorClass);
				float NearestDistance = FLT_MAX;
				for (auto& Actor : SpineActors)
				{
					float Dis =	CharacterOwner->GetDistanceTo(Actor);
					if (Dis < NearestDistance)
					{
						NearestDistance = Dis;
						CurrentSplineMeshActor = Actor;
					}
				}
			}
			else if(PrevMovementMode==EMovementMode::MOVE_Falling)
			{
				CharacterOwner->OnActorBeginOverlap.RemoveDynamic(this, &UCharacterClutchWallComponent::OnActorBeginOverlap);
				CharacterOwner->OnActorEndOverlap.RemoveDynamic(this, &UCharacterClutchWallComponent::OnActorEndOverlap);
				CurrentSplineMeshActor = false;
				EndClutchWall();
			}
		}
		// ...
	}
}

void UCharacterClutchWallComponent::OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!CharacterOwner || !OtherActor->IsA(SplineActorClass)) return;
	if (!CurrentSplineMeshActor.IsValid() || CharacterOwner->GetDistanceTo(CurrentSplineMeshActor.Get()) > CharacterOwner->GetDistanceTo(OtherActor))
	{
		CurrentSplineMeshActor = OtherActor;
	}
}

void UCharacterClutchWallComponent::OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{	
// 	if(!CurrentSplineMeshActor.IsValid()) return;
// 	if (CurrentSplineMeshActor->GetUniqueID() == OtherActor->GetUniqueID())
// 	{
// 		CurrentSplineMeshActor = nullptr;
// 	}
}

void UCharacterClutchWallComponent::RequestStartClutchWall()
{
	if (bEnableAutoClutch) return;
	if (bIsClutching) return;
	StartClutchWall();
}

void UCharacterClutchWallComponent::RequestEndClutchWall()
{
	if (!bIsClutching) return;
	EndClutchWall();
}

void UCharacterClutchWallComponent::InputDirection(float Direction)
{
	if (!bIsClutching) return;
	if (!IsClutchMoveAble()) return;
	if (USplineComponent* SplineComponent = GetSpline())
	{
		float Speed = bOverwriteMoveSpeed ? ClutchMoveSpeed : CharacterOwner->GetMovementComponent()->GetMaxSpeed();
		ClutchSplineLength = FMath::Clamp(ClutchSplineLength + Direction * Speed, 0.f, SplineComponent->GetSplineLength());

		FVector SplineLocation = GetSpline()->GetLocationAtDistanceAlongSpline(ClutchSplineLength, ESplineCoordinateSpace::World);
		SplineLocation.Z -= (CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 30.f);
		CharacterOwner->SetActorLocation(SplineLocation);
	}
}

#if WITH_EDITOR
void UCharacterClutchWallComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// ...
}
#endif


void UCharacterClutchWallComponent::StartClutchWall()
{
	if (!IsClutchable()) return;
	bIsClutching = true;

	if (CharacterOwner)
	{
		CharacterOwner->GetMovementComponent()->Deactivate();
		CharacterLocationCache = CharacterOwner->GetActorLocation();
		ClutchSplineLength = GetClutchSplineDistance(CharacterLocationCache);
		FVector SplineLocation = GetSpline()->GetLocationAtDistanceAlongSpline(ClutchSplineLength, ESplineCoordinateSpace::World);
		SplineLocation.Z -= (CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 30.f);
		CharacterOwner->SetActorLocation(SplineLocation);
	}
	OnClutchWallStart.Broadcast();
}

void UCharacterClutchWallComponent::EndClutchWall()
{
	bIsClutching = false;

	if (CharacterOwner)
	{
		CharacterOwner->GetMovementComponent()->Activate();
	}
	OnClutchWallEnd.Broadcast();
}

bool UCharacterClutchWallComponent::IsCharacterFalling() const
{
	if(CharacterOwner) return CharacterOwner->GetMovementComponent()->IsFalling();
	return false;
}

bool UCharacterClutchWallComponent::IsClutchable() const
{
	return IsCharacterFalling() && (CharacterOwner && CurrentSplineMeshActor.IsValid());
}

float UCharacterClutchWallComponent::GetClutchSplineDistance(FVector CharacterLocaiton) const
{
	if (USplineComponent* SplineComponent = GetSpline())
	{
		float	InputKey = SplineComponent->FindInputKeyClosestToWorldLocation(CharacterLocaiton);
		return SplineComponent->GetDistanceAlongSplineAtSplineInputKey(InputKey);
	}
	return -1.f;
}

bool UCharacterClutchWallComponent::IsClutchMoveAble()
{
	if (USplineComponent* SplineComponent = GetSpline())
	{
		FVector Direction =	SplineComponent->GetDirectionAtDistanceAlongSpline(ClutchSplineLength, ESplineCoordinateSpace::World);
		float Tangent =	FMath::Abs(Direction.Z/Direction.X);
		// Horizontal // Vertical
		if (Tangent < 0.2 || Tangent > 0.8)
		{
			return CurrentSplineMeshActor->Tags.Contains(ClutchMove);
		}
	}
	return false;
}

USplineComponent* UCharacterClutchWallComponent::GetSpline() const
{
	if (CurrentSplineMeshActor.IsValid())
	{
		if (USplineComponent* SplineComponent = Cast<USplineComponent>(CurrentSplineMeshActor->GetComponentByClass(USplineComponent::StaticClass())))
		{
			return SplineComponent;
		}
	}
	return nullptr;
}

