#pragma once

#include "CoreMinimal.h"

class SIEGE_API Polygon
{
public:
    enum InterceptTypes { NONE, ENTRY, EXIT };

    class Vertex {
    public:
        FVector2D pos;
        InterceptTypes type;
        Vertex* NextNode;
        Vertex* PrevNode;
        bool visited = false;

        // Marks where W-A (Weiler-Atherton) Algorithm goes for intercepts
        Vertex* intercept_link = nullptr;

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

        Vertex(FVector2D pos, InterceptTypes type) : pos(pos), type(type), NextNode(nullptr), PrevNode(nullptr) { }
    };

    Vertex* HeadNode;
    Vertex* TailNode;
    int size;

public:
    Polygon() : HeadNode(nullptr), TailNode(nullptr), size(0) {};
	~Polygon();

    Polygon(const Polygon& target)
    {
        // Copy Constructor
        if (target.size == 0) return;

        Vertex* CurrentNode = target.HeadNode;

        do{
      
            Add(new Vertex(CurrentNode->pos, CurrentNode->type));
            CurrentNode = CurrentNode->NextNode;

        } while (CurrentNode != target.HeadNode);
    }

    Polygon& operator=(const Polygon& target)
    {
        // Assignment operator 
        Empty();

        if (target.size == 0) return *this;

        Vertex* CurrentNode = target.HeadNode;

        do {
            Add(new Vertex(CurrentNode->pos, CurrentNode->type));
            CurrentNode = CurrentNode->NextNode;

        } while (CurrentNode != target.HeadNode);

        return *this;
    }

   
    bool pointInsidePolygon(FVector2D const& point) const;

    int Num() const;

    void Insert(Vertex* x, Vertex* nodeBeforex);

    void Add(Vertex* X);

    void Empty();

    bool IsEmpty() const; 

private:
    TArray<Vertex> vertices; 

};
