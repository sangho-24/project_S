#include "Character/ItemShop.h"
#include "Components/BoxComponent.h"
#include "Character/CharBase.h"
#include "Data/InventoryComponent.h"
#include "Data/ItemTemplate.h"
#include "Widget/ShopWidget.h"
#include "Gas/ArenaAttributeSet.h"
#include "AbilitySystemComponent.h"

AItemShop::AItemShop()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // 상점 주인 메시
    ShopOwnerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShopOwnerMesh"));
    RootComponent = ShopOwnerMesh;

    // 트리거 박스
    ShopArea = CreateDefaultSubobject<UBoxComponent>(TEXT("ShopArea"));
    ShopArea->SetupAttachment(RootComponent);
    ShopArea->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));
    ShopArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ShopArea->SetCollisionResponseToAllChannels(ECR_Ignore);
    ShopArea->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    ShopArea->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void AItemShop::BeginPlay()
{
	Super::BeginPlay();

    ShopArea->OnComponentBeginOverlap.AddDynamic(this, &AItemShop::OnTriggerBeginOverlap);
    ShopArea->OnComponentEndOverlap.AddDynamic(this, &AItemShop::OnTriggerEndOverlap);
}

void AItemShop::BuyItem(int32 ItemIndex, ACharBase* Buyer)
{
    if (!Buyer || !ItemTemplates.IsValidIndex(ItemIndex))
    {
        return;
    }

    UItemTemplate* ItemTemplate = ItemTemplates[ItemIndex];
    if (!ItemTemplate)
    {
        return;
    }

    UArenaAttributeSet* AttributeSet = Buyer->GetAttributeSet();
    if (!AttributeSet)
    {
        return;
    }

    int32 PlayerGold = static_cast<int32>(AttributeSet->GetGold());
    int32 ItemPrice = ItemTemplate->BuyPrice;
    if (PlayerGold < ItemPrice)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuyItem 실패: 골드 부족 (보유: %d, 필요: %d)"), PlayerGold, ItemPrice);
        return;
    }

    UInventoryComponent* InventoryComponent = Buyer->FindComponentByClass<UInventoryComponent>();
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuyItem 실패: 인벤토리 없음"));
        return;
    }

    // 인벤토리에 아이템 추가
    if (InventoryComponent->AddItem(ItemTemplate, 1))
    {
        ModifyGold(Buyer, -ItemPrice);
        UE_LOG(LogTemp, Log, TEXT("BuyItem 성공: %s 구매 (-%d G)"), *ItemTemplate->ItemName.ToString(), ItemPrice);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BuyItem 실패: 인벤토리 가득 찼음"));
    }
}

void AItemShop::SellItem(int32 SlotIndex, ACharBase* Seller)
{
    if (!Seller)
    {
        return;
    }

    UInventoryComponent* InventoryComponent = Seller->FindComponentByClass<UInventoryComponent>();
    if (!InventoryComponent)
    {
        return;
    }

    FInventoryItem Item = InventoryComponent->GetItemAtSlot(SlotIndex);
    if (!Item.IsValid() || !Item.ItemTemplate)
    {
        return;
    }

    int32 SellPrice = Item.ItemTemplate->SellPrice;

    if (InventoryComponent->RemoveItem(SlotIndex, 1))
    {
        ModifyGold(Seller, SellPrice);
        UE_LOG(LogTemp, Log, TEXT("SellItem 성공: %s (+%d G)"), *Item.ItemTemplate->ItemName.ToString(), SellPrice);
    }
}

void AItemShop::ModifyGold(ACharBase* Target, int32 Amount)
{
    if (!Target || !GoldModifyEffectClass)
    {
        return;
    }

    UAbilitySystemComponent* ASC = Target->GetAbilitySystemComponent();
    if (!ASC)
    {
        return;
    }

    FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
    ContextHandle.AddSourceObject(this);

    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GoldModifyEffectClass, 1.0f, ContextHandle);
    if (SpecHandle.IsValid())
    {
        SpecHandle.Data.Get()->SetSetByCallerMagnitude(
            FGameplayTag::RequestGameplayTag(FName("Data.Gold")),
            static_cast<float>(Amount)
        );
        ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}

void AItemShop::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Log, TEXT("OnTriggerBeginOverlap 호출됨"));
    ACharBase* Character = Cast<ACharBase>(OtherActor);
    if (!Character || !Character->IsLocallyControlled())
    {
        return;
    }

    // 상점 위젯 생성 및 표시
    if (ShopWidgetClass && !CurrentShopWidget)
    {
        if (APlayerController* PC = Character->GetController<APlayerController>())
        {
            UShopWidget* ShopWidget = CreateWidget<UShopWidget>(PC, ShopWidgetClass);
            if (ShopWidget)
            {
                // 상점 초기화
                ShopWidget->InitializeShop(this, Character);
                ShopWidget->AddToViewport(10);
                CurrentShopWidget = ShopWidget;
                UE_LOG(LogTemp, Log, TEXT("상점 열림"));
                Character->SetInShop(true, this);
            }
        }
    }
}

void AItemShop::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ACharBase* Character = Cast<ACharBase>(OtherActor);
    if (!Character || !Character->IsLocallyControlled())
    {
        return;
    }

    // 상점 위젯 닫기
    if (CurrentShopWidget)
    {
        CurrentShopWidget->RemoveFromParent();
        CurrentShopWidget = nullptr;
        UE_LOG(LogTemp, Log, TEXT("상점 닫힘"));
	    Character->SetInShop(false, nullptr); //이러면 되나 널ptr?
    }
}