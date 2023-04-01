// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
// 9. 비동기 로더인 FStreamalbeManager를 위해 추가해주어야 한다.
#include "Engine/StreamableManager.h"
#include "MyGameInstance.generated.h"
struct FStudentData
{
	FStudentData() {}
	FStudentData(int32 InOrder, const FString& InName)
		: Order(InOrder), Name(InName) {}

	int32 Order = -1;
	FString Name = TEXT("홍길동");

	friend FArchive& operator<<(FArchive& Ar, FStudentData& InStudentData)
	{
		Ar << InStudentData.Order << InStudentData.Name; 
		return Ar;
	}
};
/**
 * 
 */
UCLASS()
class UNREALASYNCASSETLOAD_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UMyGameInstance();
	
	virtual void Init() override;

	void SaveStudentPackage() const;
	void LoadStudentPackage() const;
	// 1. 패키지가 아니라 바로 오브젝트 경로로 애셋을 로드하는 함수
	void LoadStudentObject() const;

private:
	static const FString PackageName;
	static const FString AssetName;
	
	UPROPERTY()
	TObjectPtr<class UStudent> StudentSrc;

	// 8. 비동기 로드
	FStreamableManager StreamableManager;
	// 10. 스트림된 애셋을 관리할 수 있는 핸들.
	TSharedPtr<FStreamableHandle> Handle;
};
