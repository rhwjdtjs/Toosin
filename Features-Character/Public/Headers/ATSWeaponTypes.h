#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ETSWeaponType : uint8
{
    Unarmed UMETA(DisplayName = "Unarmed"), // 맨손
    OneHanded UMETA(DisplayName = "One-Handed Weapon"), // 한손 무기
    OneHandedShield UMETA(DisplayName = "One-Handed + Shield"), // 한손 무기 + 방패
    TwoHandedSword UMETA(DisplayName = "Two-Handed Sword"), // 양손 무기 (대검)
    TwoHandedAxe UMETA(DisplayName = "Two-Handed Axe"), // 양손 도끼
    Polearm UMETA(DisplayName = "Polearm") // 폴암 (창)
};

UENUM(BlueprintType)
enum class ETSCharacterState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Moving UMETA(DisplayName = "Moving"),
    Attacking UMETA(DisplayName = "Attacking"),
    Blocking UMETA(DisplayName = "Blocking"),
    Dodging UMETA(DisplayName = "Dodging"),
    Stunned UMETA(DisplayName = "Stunned"),
    Dead UMETA(DisplayName = "Dead")
};