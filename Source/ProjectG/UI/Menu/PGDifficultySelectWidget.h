// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Type/PGDifficultyTypes.h"
#include "Type/PGSessionTypes.h"
#include "PGDifficultySelectWidget.generated.h"

class UButton;
class UEditableText;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHostSessionConfirmedDelegate, const FPGHostSessionOptions&, Options);

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGDifficultySelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnHostSessionConfirmedDelegate OnHostSessionConfirmed;
	void SetReturnFocusWidget(UUserWidget* InWidget) { ReturnFocusWidget = InWidget; }

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> NormalButton;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> HardButton;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Public;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_InviteOnly;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UEditableText> ET_SessionName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostGameButton;

	/** 싱글플레이 전용 팝업 여부 (WBP 기본값으로 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "Session")
	bool bSinglePlayMode = false;

	bool bIsDifNormal;
	bool bIsInviteOnly;

	UFUNCTION()
	void OnNormalClicked();

	UFUNCTION()
	void OnHardClicked();

	UFUNCTION()
	void OnPublicClicked();

	UFUNCTION()
	void OnInviteOnlyClicked();

	UFUNCTION()
	void OnCancelClicked();

	UFUNCTION()
	void OnHostGameClicked();

	UFUNCTION()
	void OnSessionNameChanged(const FText& NewText);

	FString GetDefaultSessionName() const;
	FString BuildSessionDisplayName() const;

private:
	TWeakObjectPtr<UUserWidget> ReturnFocusWidget;
};
