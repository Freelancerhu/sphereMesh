// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseIcosphere.h"
#include <array>
#include "KismetProceduralMeshLibrary.h"





// Sets default values
ABaseIcosphere::ABaseIcosphere()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
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
    const float& y = normal.Z;
    const float& z = normal.Y;
    float normalisedX = 0;
    float normalisedZ = 1;
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
    //FVector2D UVScale(1.0/999999, 1.0/999999);
    //uv *= UVScale;
    uv.X = FMath::Fmod(uv.X, 1.0f);
    uv.Y = FMath::Fmod(uv.Y, 1.0f);
    if (uv.X < 0) uv.X += 1.0f;
    if (uv.Y < 0) uv.Y += 1.0f;
}

FVector2D FindUV(const FVector& Vertex, const FVector& Normal, FVector2D& uv)
{
    FVector2D UVScale(1,1);
    FVector2D UVOffset(0,0);
    // 计算顶点在3D空间中的位置
    float SphereRadius = 100; // 球体的半径
    //FVector Position = (Vertex + Normal).GetSafeNormal() * 0.0001;
    FVector Position = (Vertex) * 1;
    /*FVector T = FVector(0, 1, 0);
    if (FMath::Abs(Normal.Z) < 1 - KINDA_SMALL_NUMBER) {
        T = FVector(Normal.Z, 0, -Normal.X).GetSafeNormal();
    }
    FVector U = FVector::CrossProduct(T, Normal).GetSafeNormal();
    FVector Position = FVector::ZeroVector;
    Position.X = FVector::DotProduct(Vertex, T);
    Position.Y = FVector::DotProduct(Vertex, U);
    Position.Z = FVector::DotProduct(Vertex, Normal);
*/
       


    // 根据位置计算出UV坐标
    FVector2D UV;
    UV.X = FMath::Atan2(Position.Z, Position.Y) / (2.0 * PI) + 0.5f;
    UV.Y = 1.0f + FMath::Asin(Position.X) / PI;
   /* if (Position.Z < 0) {
        UV.Y = (PI / 2 - FMath::Asin(Position.Y)) / 2.0f  / PI;
    }
    else {
        UV.Y = (3 * PI / 2 + FMath::Asin(Position.Y)) / 2.0f / PI;
    }*/

    // 应用缩放和偏移
    //UV *= UVScale;
    //UV += UVOffset;
    //UV.X = FMath::Fmod(UV.X, 1.0f);
    //UV.Y = FMath::Fmod(UV.Y, 1.0f);
    //if (UV.X < 0) UV.X += 1.0f;
    //if (UV.Y < 0) UV.Y += 1.0f;
    return UV;

}

void FindUV(const TArray<FVector>& m_vertices, const FVector& Normal, TArray<Triangle>& m_triangles, TArray<FVector2D>& uv) {
    uv.Init(FVector2D::ZeroVector, m_vertices.Num() * 2);
    for (int i = 0; i < m_vertices.Num(); ++i) {
        uv[2*i].X = FMath::Atan2(m_vertices[i].X, m_vertices[i].Y) / (2.0 * PI) + 0.5f;
        uv[2*i].Y = 0.5f + FMath::Asin(m_vertices[i].Z) / PI;
    }

    TArray<int> m_triangles_i = TArray<int>((int*)m_triangles.GetData(), 3 * m_triangles.Num());
    for (int i = 0; i < m_triangles_i.Num(); i += 3) {
        int32 a = m_triangles_i[i] * 2, b = m_triangles_i[i + 1] * 2, c = m_triangles_i[i + 2] * 2;
        int32 ay = uv[a + 1].Y, by = uv[b + 1].Y, cy = uv[c + 1].Y;
        int32 ax = uv[a + 1].X, bx = uv[b + 1].X, cx = uv[c + 1].X;
        if (1) { // uv fixing code; don't ask me how I got here
            if (bx - ax >= 0.5 && ay != 1) bx -= 1;
            if (cx - bx > 0.5) cx -= 1;
            if (ax > 0.5 && ax - cx > 0.5 || ax == 1 && cy == 0) ax -= 1;
            if (bx > 0.5 && bx - ax > 0.5) bx -= 1;
            if (ay == 0 || ay == 1) ax = (bx + cx) / 2;
            if (by == 0 || by == 1) bx = (ax + cx) / 2;
            if (cy == 0 || cy == 1) cx = (ax + bx) / 2;
        }
        ax = FMath::Fmod(ax, 1.0f);
        ay = FMath::Fmod(ay, 1.0f);
        if (ax < 0) ax += 1.0f;
        if (ay < 0) ay += 1.0f;
        bx = FMath::Fmod(bx, 1.0f);
        by = FMath::Fmod(by, 1.0f);
        if (bx < 0) bx += 1.0f;
        if (by < 0) by += 1.0f;
        cx = FMath::Fmod(cx, 1.0f);
        cy = FMath::Fmod(cy, 1.0f);
        if (cx < 0) cx += 1.0f;
        if (cy < 0) cy += 1.0f;
    }

}

void ABaseIcosphere::mapuv()
{
    m_uvmapping.Empty(m_normals.Num());
    //for (int32 i = 0; i < m_normals.Num(); ++i)
    //{
    //    // FindUV(m_vertices[i], *(m_uvmapping.GetData() + i));
    //    m_uvmapping.Add(FindUV(m_vertices[i], m_normals[i], *(m_uvmapping.GetData() + i)));
    //}
    m_uvmapping.Init(FVector2D::ZeroVector, m_vertices.Num());
    FindUV(m_vertices, FVector(0,0,1), m_triangles, m_uvmapping);
    // FindUV(m_vertices, FVector(0, 0, 1), m_triangles, m_uvmapping);
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
            FVector TriangleNormal = FVector::CrossProduct(B-C, A-C).GetSafeNormal();
            m_normals[m_triangles_i[i + j]] += TriangleNormal;
        }
    }
    for (int32 i = 0; i < m_normals.Num(); i++)
    {
        m_normals[i].Normalize();
    }


}
