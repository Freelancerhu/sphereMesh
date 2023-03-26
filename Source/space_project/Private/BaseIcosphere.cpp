// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseIcosphere.h"
#include <array>





// Sets default values
ABaseIcosphere::ABaseIcosphere()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    target_subdivisions = 2;
    make_icosphere(target_subdivisions);
    bCollideWhenPlacing = true;
    //SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    RootComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");

    //CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("PPawn_CollisionComponent"));
    //CollisionComponent->InitSphereRadius(40.0f);
    //CollisionComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
    //CollisionComponent->CanCharacterStepUpOn = ECB_Yes;
    //CollisionComponent->SetCanEverAffectNavigation(false);
    //CollisionComponent->bDynamicObstacle = true;

    //CollisionComponent->SetupAttachment(RootComponent);

    MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("PPawn_MeshComponent"));
    //MeshComponent->RegisterComponent();
    //MeshComponent->AttachToComponent(RootComponent,FAttachmentTransformRules(EAttachmentRule::KeepRelative,true));
    if (MeshComponent) {
        MeshComponent->SetupAttachment(RootComponent);
        MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    }


    m_triangles_i = TArray<int>((int*)m_triangles.GetData(), 3 * m_triangles.Num());

    FString Mv = FString::Printf(TEXT("length m_vertices of %d"), m_vertices.Num());
   // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Mv);
    UE_LOG(LogTemp, Warning, TEXT("hhhhhh"))
    FString Mt = FString::Printf(TEXT("length m_triangles_i of %d"), m_triangles_i.Num());
    //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Mt);


    MeshComponent->CreateMeshSection(0, m_vertices, m_triangles_i, m_normals, m_uvmapping, dummy_color, dummy_tangents, true);
    //CollisionComponent->SetSphereRadius(m_radius);

    static ConstructorHelpers::FObjectFinder<UMaterialInstanceDynamic> DefaultMaterail(TEXT("/Game/StarterContent/Materials/M_ColorGrid_LowSpec"));
    if (DefaultMaterail.Succeeded())
    {
        MyMaterial = DefaultMaterail.Object;
    }

    // 绑定Material
    UProceduralMeshComponent* TempMeshComponent = MeshComponent;
    if (TempMeshComponent != nullptr)
    {
        TempMeshComponent->SetMaterial(0, MyMaterial);
    }


}

void ABaseIcosphere::make_icosphere(uint8 subdivisions)
{
    m_vertices.Reset(16);
    m_triangles.Reset(20);
    //logInfo(Geometry, "Setting up icosahedron.");
    m_vertices.Append(icosahedron::vertices, ARRAY_COUNT(icosahedron::vertices));
    m_triangles.Append(icosahedron::triangles, ARRAY_COUNT(icosahedron::triangles));
    normalize(); //just to be sure
    for (int i = 0; i < subdivisions; ++i)
    {
        // logInfo(Geometry, "Subdividing icosphere\niteration: %d\ncurrent vertex count: %d", i, m_vertices.Num());
        subdivide();
    }

    for (FVector& each : m_vertices)
    {
        each *= 500;
    }

    mapNormal();
    mapuv();
    // FString M = FString::Printf(TEXT("length of %d"), m_triangles.Num());
    //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, M);
}

void ABaseIcosphere::normalize()
{
    for (FVector& each : m_vertices)
    {
        each.Normalize();
    }
}

//todo: add log lines
void ABaseIcosphere::subdivide()
{
    //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("subdivide"));
    TArray<Triangle> swap_sphere;
    swap_sphere.Reserve(m_triangles.Num() * 3);
    for (auto&& triangle : m_triangles)
    {
        std::array<int, 3> mid;
        for (int edge = 0; edge < 3; ++edge)
        {
            mid[edge] = vertex_for_edge(triangle.vert[edge], triangle.vert[(edge + 1) % 3]);
        }
        swap_sphere.Add({ triangle.vert[0], mid[0], mid[2] });
        swap_sphere.Add({ triangle.vert[1], mid[1], mid[0] });
        swap_sphere.Add({ triangle.vert[2], mid[2], mid[1] });
        swap_sphere.Add({ mid[0], mid[1], mid[2] });
    }
    Swap(m_triangles, swap_sphere); // no new memory needed
    lookup.clear();
}

uint32 ABaseIcosphere::vertex_for_edge(uint32 vi1, uint32 vi2)
{
    uint32 a = std::min(vi1, vi2);
    uint32 b = std::max(vi1, vi2);
    std::pair<uint32, uint32> key(a, b);
    auto inserted = lookup.insert({ key, m_vertices.Num() });
    if (inserted.second)
    {
        FVector& edge0 = m_vertices[a];
        FVector& edge1 = m_vertices[b];
        auto point = edge0 + edge1;
        point.Normalize();
        m_vertices.Add(point);
    }

    return inserted.first->second;
}

void FindUV(const FVector& normal, FVector2D& uv)
{
    const float& x = normal.X;
    const float& y = normal.Y;
    const float& z = normal.Z;
    float normalisedX = 0;
    float normalisedZ = -1;
    if (((x * x) + (z * z)) > 0)
    {
        normalisedX = sqrt((x * x) / ((x * x) + (z * z)));
        if (x < 0)
        {
            normalisedX = -normalisedX;
        }
        normalisedZ = sqrt((z * z) / ((x * x) + (z * z)));
        if (z < 0)
        {
            normalisedZ = -normalisedZ;
        }
    }
    if (normalisedZ == 0)
    {
        uv.X = ((normalisedX * PI) / 2);
    }
    else
    {
        uv.X = atan(normalisedX / normalisedZ);
        if (normalisedX < 0)
        {
            uv.X = PI - uv.X;
        }
        if (normalisedZ < 0)
        {
            uv.X += PI;
        }
    }
    if (uv.X < 0)
    {
        uv.X += 2 * PI;
    }
    uv.X /= 2 * PI;
    uv.Y = (-y + 1) / 2;
    FVector2D UVScale(99, 99);
    uv *= UVScale;
}

FVector2D FindUV(const FVector& Vertex, const FVector& Normal, FVector2D& uv)
{
    FVector2D UVScale(99,99);
    FVector2D UVOffset(0,0);
    // 计算顶点在3D空间中的位置
    FVector Position = Vertex + Normal;

    // 根据位置计算出UV坐标
    FVector2D UV;
    UV.X = FMath::Atan2(Position.Y, Position.X) / (2.0 * PI) + 0.5;
    UV.Y = FMath::Asin(Position.Z) / PI + 0.5;

    // 应用缩放和偏移
    UV *= UVScale;
    UV += UVOffset;

    return UV;

}

void ABaseIcosphere::mapuv()
{
    m_uvmapping.Empty(m_normals.Num());
    //for (int32 i = 0; i < m_normals.Num(); ++i)
    //{
    //    FindUV(m_normals[i], *(m_uvmapping.GetData() + i));
    //    //m_uvmapping.Add(FindUV(m_vertices[i], m_normals[i], *(m_uvmapping.GetData() + i)));
    //}
}

void ABaseIcosphere::mapNormal() {
    m_normals.Empty(m_vertices.Num());
    m_triangles_i = TArray<int>((int*)m_triangles.GetData(), 3 * m_triangles.Num());

    TArray<FVector> temp_normal;
    m_normals.Init(FVector::ZeroVector, m_vertices.Num());
    for (int32 i = 0; i < m_triangles_i.Num(); i += 3)
    {
        for (int32 j = 0; j < 3; ++j) {
            FVector C = m_vertices[m_triangles_i[i + j]];
            FVector A = m_vertices[m_triangles_i[i + (j + 1) % 3]];
            FVector B = m_vertices[m_triangles_i[i + (j + 2) % 3]];
            FVector TriangleNormal = FVector::CrossProduct(C - A, C - B).GetSafeNormal();
            m_normals[m_triangles_i[i + j]] += TriangleNormal;
        }
    }

    for (int32 i = 0; i < m_normals.Num(); i++)
    {
        m_normals[i].Normalize();
    }


}
