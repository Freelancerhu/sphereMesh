// Fill out your copyright notice in the Description page of Project Settings.


#include "CelestiaBody.h"

// Sets default values
ACelestiaBody::ACelestiaBody()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACelestiaBody::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACelestiaBody::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

