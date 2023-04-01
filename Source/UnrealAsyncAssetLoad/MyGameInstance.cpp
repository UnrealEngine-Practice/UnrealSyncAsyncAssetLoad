// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Student.h"
#include "JsonObjectConverter.h"
#include "UObject/SavePackage.h"

const FString UMyGameInstance::PackageName = TEXT("/Game/Student");
const FString UMyGameInstance::AssetName = TEXT("TopStudent");

UMyGameInstance::UMyGameInstance()
{
	// 3.이번에는 생성자에서 애셋을 로드한다. 게임이 시작되기 전에 메모리에 올라가는 것을 의미한다.
	const FString TopSoftObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName);
	// 4. 이 떄는 LoadObject를 쓰는 것이 아니라 언리얼에서 제공하는 Contstructor helper를 사용한다.
	static ConstructorHelpers::FObjectFinder<UStudent> UASSET_TopStudent(*TopSoftObjectPath);
	// 5. Constructor Helper에 의해 로드되었으면 Succeeded()를 이용해 성공 실패 여부를 알 수 있다.
	if (UASSET_TopStudent.Succeeded())
	{
		// 7. 에디터 실행 후 플레이버튼을 누르면 이 로그가 2번 찍히는 것을 확인 할 수 있다.
		//    에디터가 켜질 때 한 번, 게임이 시작될 때 한 번 실행된다.
		//    그리고 이 경우에는 애셋이 반드시 있다고 가정하기 때문에, 애셋이 존재하지 않는 경우에는
		//    실행되지 않고 강력한 경고와 에러 메시지를 띄운다.
		UASSET_TopStudent.Object->PrintInfo(TEXT("Constructor"));
	}
}

void UMyGameInstance::Init()
{
	Super::Init();
	FStudentData RawDataSrc(11, TEXT("abc"));

	const FString SavedDir = FPaths::Combine(FPlatformMisc::ProjectDir(), TEXT("Saved"));
	UE_LOG(LogTemp, Log, TEXT("저장할 파일위치 : %s"), *SavedDir);

	{
		const FString RawDataFilename(TEXT("RawData.bin"));
		FString RawDataAbsolutePath = FPaths::Combine(*SavedDir, *RawDataFilename);
		UE_LOG(LogTemp, Log, TEXT("저장할 파일 절대경로 : %s"), *RawDataAbsolutePath);

		FPaths::MakeStandardFilename(RawDataAbsolutePath);
		UE_LOG(LogTemp, Log, TEXT("저장할 파일 절대경로 : %s"), *RawDataAbsolutePath);
		
		FArchive* RawFileWriterAr = IFileManager::Get().CreateFileWriter(*RawDataAbsolutePath);
		if (RawFileWriterAr != nullptr)
		{
			*RawFileWriterAr << RawDataSrc;
			RawFileWriterAr->Close();
			delete RawFileWriterAr;
			RawFileWriterAr = nullptr;

		}
		FStudentData RawDataDest;
		FArchive* RawFileReaderAr = IFileManager::Get().CreateFileReader(*RawDataAbsolutePath);
		if (RawFileReaderAr != nullptr)
		{
			*RawFileReaderAr << RawDataDest;
			RawFileReaderAr->Close();
			delete RawFileReaderAr;
			RawFileReaderAr = nullptr;

			UE_LOG(LogTemp, Log, TEXT("[RawData] 이름 %s 순번 %d"), *RawDataDest.Name, RawDataDest.Order);
		}
	}

	StudentSrc = NewObject<UStudent>();
	StudentSrc->SetName(TEXT("김동호"));
	StudentSrc->SetOrder(27);
	{
		const FString ObjectDataFileName(TEXT("ObjectData.bin"));
		FString ObjcetDataAbsolutePath = FPaths::Combine(*SavedDir, *ObjectDataFileName);
		FPaths::MakeStandardFilename(ObjcetDataAbsolutePath);

		TArray<uint8> BufferArray;
		FMemoryWriter MemoryWriterAr(BufferArray);
		StudentSrc->Serialize(MemoryWriterAr);

		{
			TUniquePtr<FArchive> FileWriterAr = TUniquePtr<FArchive>(IFileManager::Get().CreateFileWriter(*ObjcetDataAbsolutePath));
			if (FileWriterAr != nullptr)
			{
				*FileWriterAr << BufferArray;
				FileWriterAr->Close();
			}
		}
		
		TArray<uint8> BufferArrayFromFile;
		{
			TUniquePtr<FArchive> FileReaderAr = TUniquePtr<FArchive>(IFileManager::Get().CreateFileReader(*ObjcetDataAbsolutePath));
			if (FileReaderAr != nullptr)
			{
				*FileReaderAr << BufferArrayFromFile;
				FileReaderAr->Close();
			}
		}

		FMemoryReader  MemoryReaderAr(BufferArrayFromFile);
		UStudent* StudentDest = NewObject<UStudent>();
		StudentDest->Serialize(MemoryReaderAr);

		StudentDest->PrintInfo(TEXT("ObjectData"));
	}

	{
		FString JsonDataFileName(TEXT("StudenJsonData.txt"));
		FString JsonDataAbsolutePath = FPaths::Combine(*SavedDir, *JsonDataFileName);
		FPaths::MakeStandardFilename(JsonDataAbsolutePath);

		TSharedRef<FJsonObject> JsonObjectSrc = MakeShared<FJsonObject>();
		FJsonObjectConverter::UStructToJsonObject(StudentSrc->GetClass(), StudentSrc, JsonObjectSrc);

		FString JsonOutString;
		TSharedRef<TJsonWriter<TCHAR>> JsonWriterAr = TJsonWriterFactory<TCHAR>::Create(&JsonOutString);
		if (FJsonSerializer::Serialize(JsonObjectSrc, JsonWriterAr))
		{
			FFileHelper::SaveStringToFile(JsonOutString, *JsonDataAbsolutePath);
		}

		FString JsonInString;
		FFileHelper::LoadFileToString(JsonInString, *JsonDataAbsolutePath);
		
		TSharedRef<TJsonReader<TCHAR>> JsonReaderAr = TJsonReaderFactory<TCHAR>::Create(JsonInString);
		TSharedPtr<FJsonObject> JsonObjectDest;
		if (FJsonSerializer::Deserialize(JsonReaderAr, JsonObjectDest))
		{
			UStudent* JsonStudentDest = NewObject<UStudent>();
			if (FJsonObjectConverter::JsonObjectToUStruct(JsonObjectDest.ToSharedRef(), JsonStudentDest->GetClass(), JsonStudentDest))
			{
				JsonStudentDest->PrintInfo(TEXT("JsonData"));
			}
		}
	}

	SaveStudentPackage();

	// LoadStudentPackage();
	
	// 6. 주석처리하고 생성자에서 불러오는지 확인
	// LoadStudentObject();
}

void UMyGameInstance::SaveStudentPackage() const
{
	UPackage* StudentPackage = ::LoadPackage(nullptr, *PackageName, LOAD_None);
	if (StudentPackage)
	{
		StudentPackage->FullyLoad();
	}
	
	StudentPackage = CreatePackage(*PackageName);
	constexpr EObjectFlags ObjectFlag = RF_Public | RF_Standalone;
	UStudent* TopStudent = NewObject<UStudent>(StudentPackage, UStudent::StaticClass(), *AssetName, ObjectFlag);
	TopStudent->SetName(TEXT("김동호"));
	TopStudent->SetOrder(100);
	
	constexpr int NumOfSubObjects = 10;
	for (int32 i = 0 ; i < NumOfSubObjects ; i++)
	{
		FString SubObjectName = FString::Printf(TEXT("Student#%2d"), i+1);
		UStudent* SubStudent = NewObject<UStudent>(TopStudent, UStudent::StaticClass(), *SubObjectName, ObjectFlag);
		SubStudent->SetName( FString::Printf(TEXT("학생#%2d"), i+1));
		SubStudent->SetOrder(i+1);
	}

	const FString PackageFileName =FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = ObjectFlag;

	if (UPackage::SavePackage(StudentPackage, nullptr, *PackageFileName, SaveArgs))
	{
		UE_LOG(LogTemp, Log, TEXT("패키지가 성공적으로 저장되었습니다."));
	}
}

void UMyGameInstance::LoadStudentPackage() const
{
	UPackage* StudentPackage = ::LoadPackage(nullptr, *PackageName, LOAD_None);
	if (nullptr == StudentPackage)
	{
		UE_LOG(LogTemp, Warning, TEXT("패키지를 불러올 수 없습니다."));
		return;
	}
	StudentPackage->FullyLoad();
	UStudent* TopStudent = FindObject<UStudent>(StudentPackage, *AssetName);
	TopStudent->PrintInfo(TEXT("UE Object 찾음!"));
}

void UMyGameInstance::LoadStudentObject() const
{
	//2. 애셋의 오브젝트경로를 만들어준다.
	//   오브젝트 경로는 패키지이름.애셋이름으로 구성된다. 
	const FString TopSoftObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName);
	UStudent* TopStudent = LoadObject<UStudent>(nullptr, *TopSoftObjectPath);
	if (nullptr == TopStudent)
	{
		UE_LOG(LogTemp, Log, TEXT("오브젝트 경로에 의한 로드 실패."));
		return;
	}
	TopStudent->PrintInfo(TEXT("LoadObject Asset"));
	
}
