#pragma once

#include "CoreMinimal.h"

class SIEGE_API Polygon
{
public:
	Polygon();
	~Polygon();

    enum InterceptTypes { NONE, ENTRY, EXIT };

	struct Vertex {
        FVector2D pos;
        InterceptTypes type;
        bool visited = false;

        // Marks where W-A (Weiler-Atherton) Algorithm goes for intercepts
        int intercept_index = -1;

        bool compareAproxPos(FVector2D A, FVector2D B) const {
        return (std::roundf(A.X) == std::roundf(B.X) &&
                std::roundf(A.Y) == std::roundf(B.Y));
        }

        bool equals(const Vertex& other) const {
        return (compareAproxPos(pos, other.pos) && type == other.type);
        }

        bool operator==(const Vertex& other) const {
        return (compareAproxPos(pos, other.pos) && type == other.type);
        }
    };

    TArray<Vertex> vertices;

    bool pointInsidePolygon(FVector2D const& point) const;

    Vertex* getVertex(int index);

    int Num() const;

    int Insert(Vertex x, int index);

    void Add(Vertex X);

    void Empty();

    bool IsValidIndex(int index) const;

    bool IsEmpty() const;
};
