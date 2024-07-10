#pragma once

#include "CoreMinimal.h"

class SIEGE_API Polygon
{
public:
	Polygon();
	~Polygon();

    Polygon(const Polygon& target)
    {
        // Copy Constructor
        for (int i = 0; i < target.Num(); i++) {
            Polygon::Vertex* current = new Vertex(target.getVertexConst(i).pos,Polygon::NONE );
            Add(current);
        }
    }

    Polygon& operator=(const Polygon& target)
    {
        // Assignment operator 
        Empty();
        for (int i = 0; i < target.Num(); i++) {
            Polygon::Vertex* current = new Vertex(target.getVertexConst(i).pos, Polygon::NONE);
            this->Add(current);
        }
        return *this;
    }

    enum InterceptTypes { NONE, ENTRY, EXIT };

	class Vertex {
    public:
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

        Vertex(FVector2D pos, InterceptTypes type) : pos(pos), type(type) { }
    };

    bool pointInsidePolygon(FVector2D const& point) const;

    Vertex const getVertexConst(int index) const;
    Vertex* getVertex(int index);

    int Num() const;

    int Insert(Vertex* x, int index);

    void Add(Vertex* X);

    void Empty();

    bool IsValidIndex(int index) const;

    bool IsEmpty() const; 

private:
    TArray<Vertex> vertices; 

};
