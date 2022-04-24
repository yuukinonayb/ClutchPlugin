// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/ObjectMacros.h"
#include "CharacterClutchWallComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FClutchWallStartSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FClutchWallEndSignature);

class ACharacter; 
class ASplineMeshActor;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CLUTCHWALL_API UCharacterClutchWallComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterClutchWallComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	//~ Begin ActorComponent Interface 
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void RegisterComponentTickFunctions(bool bRegister) override;
	virtual void PostLoad() override;
	virtual void Deactivate() override;
	virtual void Serialize(FArchive& Ar) override;

	/** Overridden to auto-register the updated component if it starts NULL, and we can find a root component on our owner. */
	virtual void InitializeComponent() override;

	/** Overridden to update component properties that should be updated while being edited. */
	virtual void OnRegister() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	//~ End ActorComponent Interface


	UFUNCTION()
	void OnMovementModeChange(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);
	UFUNCTION()
	void OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
	UFUNCTION()
	void OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION(BlueprintCallable)
	void RequestStartClutchWall();
	UFUNCTION(BlueprintCallable)
	void RequestEndClutchWall();
	
	UFUNCTION(BlueprintCallable)
	void InputDirection(float Direction);
private:
	void StartClutchWall();
	void EndClutchWall();

	bool IsCharacterFalling() const;
	bool IsClutchable() const;
	float GetClutchSplineDistance(FVector CharacterLocaiton) const;
	bool IsClutchMoveAble();

	class USplineComponent* GetSpline() const;
protected:
	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<ACharacter> CharacterOwner;

	UPROPERTY(EditDefaultsOnly)
	uint8 bEnableAutoClutch : 1;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> SplineActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Tag")
	FName ClutchHorizontal;
	UPROPERTY(EditDefaultsOnly, Category = "Tag")
	FName ClutchVerticalLeft;
	UPROPERTY(EditDefaultsOnly, Category = "Tag")
	FName ClutchVerticalRight;

	UPROPERTY(EditDefaultsOnly, Category = "Tag")
	FName ClutchMove;

	UPROPERTY(EditDefaultsOnly)
	float ClutchHeight;

	UPROPERTY(EditDefaultsOnly)
	uint8 bOverwriteMoveSpeed : 1;

	UPROPERTY(EditDefaultsOnly, meta =(Editcondition="OverwriteMoveSpeed"))
	float ClutchMoveSpeed;

	TWeakObjectPtr<AActor> CurrentSplineMeshActor = nullptr;

	uint8	bIsClutching : 1;
	float	ClutchSplineLength = 0.f;
	FVector CharacterLocationCache;
public:
	FClutchWallStartSignature	OnClutchWallStart;
	FClutchWallEndSignature		OnClutchWallEnd;
};
