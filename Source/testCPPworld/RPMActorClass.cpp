// Fill out your copyright notice in the Description page of Project Settings.


#include "RPMActorClass.h"

// Sets default values
ARPMActorClass::ARPMActorClass()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARPMActorClass::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARPMActorClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

