// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <unordered_map>
#include <vector>
#include "Components/SphereComponent.h"
#include "ProceduralMeshComponent.h"


#include "BaseIcosphere.generated.h"

struct Triangle
{
    int vert[3];
};

namespace icosahedron
{

    const float X = .525731112119133606f;
    const float Z = .850650808352039932f;
    const float N = 0.f;

    static const FVector vertices[] =
    {
        {-X,N,Z}, {X,N,Z}, {-X,N,-Z}, {X,N,-Z},
        {N,Z,X}, {N,Z,-X}, {N,-Z,X}, {N,-Z,-X},
        {Z,X,N}, {-Z,X, N}, {Z,-X,N}, {-Z,-X, N}
    };

    static const Triangle triangles[] =
    {
        {0,4,1},{0,9,4},{9,5,4},{4,5,8},{4,8,1},
        {8,10,1},{8,3,10},{5,3,8},{5,2,3},{2,7,3},
        {7,10,3},{7,6,10},{7,11,6},{11,0,6},{0,1,6},
        {6,1,10},{9,0,11},{9,11,2},{9,2,5},{7,2,11}
    };

    //const float t = (1.0 + sqrt(5.0)) / 2.0;

    //static const FVector vertices[] =
    //{
    //    {-1,t,0},{1,t,0},{-1,-t,0},{1,-t,0},
    //    {0,-1,t},{0,1,t},{0,-1,-t},{0,1,-t},
    //    {t,0,-1},{t,0,1},{-t,0,-1},{-t,0,1}
    //};

    //static const Triangle triangles[] =
    //{
    //    {0,11,5}, {0,5,1},{0,1,7},{0, 7, 10},{0, 10, 11},
    //    {1, 5, 9}, {5, 11, 4},{11, 10, 2},{10, 7, 6},{7, 1, 8},
    //    {3, 9, 4}, {3, 4, 2},{3, 2, 6},{3, 6, 8},{3, 8, 9},
    //    {4, 9, 5}, {2, 4, 11},{6, 2, 10},{8, 6, 7},{9, 8, 1},
    //};


}

struct uint32_pair_hash
{
    size_t operator()(const std::pair<uint32, uint32>& x) const
    {
        union
        {
            struct
            {
                uint32 A;
                uint32 B;
            };
            size_t C;
        }xx;
        xx.A = x.first;
        xx.B = x.second;
        return xx.C;
    }
};


template<typename K, typename V>
using umap = std::unordered_map<K, V, uint32_pair_hash>;

class UProceduralMeshComponent;

UCLASS()
class SPACE_PROJECT_API ABaseIcosphere : public AActor
{
	GENERATED_BODY()

private:
	TArray<FVector>  m_vertices;
    TArray<Triangle> m_triangles;
    TArray<int> m_triangles_i;
    TArray<FVector> m_normals;
    TArray<FVector2D> m_uvmapping;
    TArray<FColor> dummy_color;
    TArray<FProcMeshTangent> dummy_tangents;


    uint32 m_radius;
	umap<std::pair<uint32, uint32>, uint32> lookup; //We keep this empty except while running

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "my category", meta = (ClampMin = "0", ClampMax = "6", UIMin = "0", UIMax = "6", RangeMin="0", RangeMax="6"))
    uint8 target_subdivisions;

    UPROPERTY(Category = "my category", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UProceduralMeshComponent* MeshComponent;

    UProceduralMeshComponent* MeshComponent_UV;

    UPROPERTY(Category = "my category", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USphereComponent* CollisionComponent;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "my category")
    UMaterialInstanceDynamic* MyMaterial;


	// Sets default values for this actor's properties
	ABaseIcosphere();

protected:
	// Called when the game starts or when spawned
	// virtual void BeginPlay() override;
	uint32 vertex_for_edge(uint32 vert_index_1, uint32 vert_index_2);
	void subdivide();
	void mapuv();
    void mapNormal();
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("PostEditChangeProperty"));
        
        Super::PostEditChangeProperty(PropertyChangedEvent);
        FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
        if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseIcosphere, MyMaterial))
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("GET_MEMBER_NAME_CHECKED"));
            // Material属性已更改
            UProceduralMeshComponent* tMeshComponent = MeshComponent;
            if (tMeshComponent != nullptr)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("tMeshComponent"));
                tMeshComponent->SetMaterial(0, MyMaterial);
            }
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseIcosphere, target_subdivisions))
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("target_subdivisions"));
            // Material属性已更改
            UProceduralMeshComponent* tMeshComponent = MeshComponent;
            if (tMeshComponent != nullptr)
            {
                make_icosphere(target_subdivisions);
                MeshComponent->ClearMeshSection(0);
                MeshComponent->CreateMeshSection(0, m_vertices, m_triangles_i, m_normals, m_uvmapping, dummy_color, dummy_tangents, true);
            }
        }
    }

public:	
	// Called every frame
	// virtual void Tick(float DeltaTime) override;
	// ~ABaseIcosphere();
	void make_icosphere(uint8 subdivisions);
	// normalizing should be redundant. todo: delete
	void normalize();

	const TArray<FVector>& get_vertices() const { return m_vertices; }
	const TArray<Triangle>& get_triangles() const { return m_triangles; }
	const TArray<FVector2D>& get_uvmapping() const { return m_uvmapping; }
	const FVector* get_vertices_raw() const { return m_vertices.GetData(); }
	const int* get_triangles_raw() const { return (int*)m_triangles.GetData(); }
	uint32 get_vert_count() const { return m_vertices.Num(); }
	uint32 get_tri_count() const { return m_triangles.Num(); }
	uint32 get_index_count() const { return 3 * m_triangles.Num(); }

};
