// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionDetectorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

UInteractionDetectorComponent::UInteractionDetectorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Current = nullptr;
}

void UInteractionDetectorComponent::BeginPlay()
{
	Super::BeginPlay();

	const float Interval = (UpdateRateHz > 0.f) ? (1.f / UpdateRateHz) : 0.1f;
	GetWorld()->GetTimerManager().SetTimer(Timer, this, &UInteractionDetectorComponent::TickDetect, Interval, true);
}

bool UInteractionDetectorComponent::PassesLOS(const FVector& Eye, const FVector& Target) const
{
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractLOS), /*bTraceComplex*/ false, GetOwner());

	const bool bHitSomething = GetWorld()->LineTraceSingleByChannel(
		Hit, Eye, Target, LineOfSightChannel, Params
	);

	return !bHitSomething;
}

float UInteractionDetectorComponent::ScoreCandidate(AActor* Actor, const FVector& Eye, const FVector& Fwd) const
{
	if (!Actor) return -FLT_MAX;

	// interface?
	TScriptInterface<IInteractable> Ii(Actor);
	if (!Ii) return -FLT_MAX;

	// CanInteract?
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!IInteractable::Execute_CanInteract(Actor, PawnOwner)) return -FLT_MAX;

	const FVector Target = IInteractable::Execute_GetInteractWorldLocation(Actor);
	const FVector To = (Target - Eye);
	const float Dist = To.Size();

	if (Dist > Radius) return -FLT_MAX;
	const FVector Dir = To / FMath::Max(Dist, 1.f);

	// Uhol: dot s forward
	const float Dot = FVector::DotProduct(Fwd, Dir); // -1..1 (1 = priamo pred)
	const float DistScore = 1.f - (Dist / Radius);      // 0..1
	const float AimScore = (Dot + 1.f) * 0.5f;         // map -1..1 -> 0..1

	return DistScore * 0.6f + AimScore * 0.4f;
}

void UInteractionDetectorComponent::TickDetect()
{
	//UE_LOG(LogTemp, Warning, TEXT("Detect the object"));
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!PawnOwner) return;

	//UE_LOG(LogTemp, Warning, TEXT("Detect the object 2"));
	FVector EyeLoc; FRotator EyeRot;
	PawnOwner->GetActorEyesViewPoint(EyeLoc, EyeRot);
	const FVector Fwd = EyeRot.Vector();

	TArray<FOverlapResult> Hits;
	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	ObjParams.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractOverlap), false, PawnOwner);

	GetWorld()->OverlapMultiByObjectType(
		Hits,
		EyeLoc,
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(Radius),
		Params
	);

	//UE_LOG(LogTemp, Warning, TEXT("Detect the object 3"));

	AActor* Best = nullptr;
	float BestScore = -FLT_MAX;

	for (const FOverlapResult& R : Hits)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Candidates: %d"), Hits.Num());
		AActor* A = R.GetActor();
		if (!A || A == PawnOwner) continue;

		if (A->ActorHasTag(RequiredActorTag))
		{
			//UE_LOG(LogTemp, Error, TEXT("fount at least 1 interactable with correct tag"));
			// LOS + scoring
			const FVector Target = (TScriptInterface<IInteractable>(A))
				? IInteractable::Execute_GetInteractWorldLocation(A)
				: A->GetActorLocation();

			//if (!PassesLOS(EyeLoc, Target)) continue;


			const float S = ScoreCandidate(A, EyeLoc, Fwd);
			//UE_LOG(LogTemp, Error, TEXT("Candidate %s has %f points"), *A->GetActorNameOrLabel(), S);
			if (S > BestScore)
			{
				BestScore = S;
				Best = A;
			}
		}
	}



	TScriptInterface<IInteractable> NewCurrent;
	NewCurrent.SetObject(Best);

	auto CanCallInteractIface = [](UObject* Obj) -> bool
	{
		return IsValid(Obj) && Obj->GetClass()->ImplementsInterface(UInteractable::StaticClass());
	};

	AActor* Prev = Current.Get();
	AActor* Next = Cast<AActor>(NewCurrent.GetObject());

	const bool bPrevValid = IsValid(Prev);
	const bool bNextValid = IsValid(Next);
	const bool bChanged = (!bPrevValid) || (Prev != Next);

	if (bChanged)
	{
		//UE_LOG(LogTemp, Error, TEXT("Try to interact with object 1"));
		if (bPrevValid && CanCallInteractIface(Prev))
		{
			OnFocusedChanged.Broadcast(Next, Prev);
			IInteractable::Execute_OnFocEnd(Prev, PawnOwner);
		}

		Current = bNextValid ? Next : nullptr;

		if (bNextValid && CanCallInteractIface(Next))
		{
			OnFocusedChanged.Broadcast(Next, Prev);
			//UE_LOG(LogTemp, Error, TEXT("Try to interact with object"));
			IInteractable::Execute_OnFocStart(Next, PawnOwner);
		}
		else
		{
			Current = nullptr;
		}
	}

	if (!Current.IsValid())
	{
		Current = nullptr;
	}
}

void UInteractionDetectorComponent::TryInteract(APawn* By)
{
	AActor* Target = Current.Get();

	auto CanCallInteractIface = [](UObject* Obj) -> bool
	{
		return IsValid(Obj) && Obj->GetClass()->ImplementsInterface(UInteractable::StaticClass());
	};

	if (!IsValid(Target) || !CanCallInteractIface(Target) || !IsValid(By))
		return;

	if (IInteractable::Execute_CanInteract(Target, By))
	{
		IInteractable::Execute_Interact(Target, By);
	}
}