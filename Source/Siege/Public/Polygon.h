#pragma once

#include "CoreMinimal.h"

class SIEGE_API Polygon
{
public:
    enum InterceptTypes { NONE, ENTRY, EXIT };

    struct VertexData {
        FVector2D pos;
        InterceptTypes type = NONE;

        bool static compareAproxPos(FVector2D A, FVector2D B) {
            return (std::roundf(A.X) == std::roundf(B.X) &&
                std::roundf(A.Y) == std::roundf(B.Y));
        }

        bool operator==(const VertexData& other) const {
            return (compareAproxPos(pos, other.pos) && type == other.type);
        }

        bool operator!=(const VertexData& other) const {
            return !(compareAproxPos(pos, other.pos) && type == other.type);
        }
    };

    class Vertex {

    public:
        VertexData data;
        Vertex* NextNode;
        Vertex* PrevNode;
        Vertex* intercept_link;

        FString ToString() const {

            FString out = "(" + FString::SanitizeFloat(data.pos.X, 0) + "," + FString::SanitizeFloat(data.pos.Y, 0);

            switch (data.type) {
            case NONE:
                return out + ",NONE)";
            case ENTRY:
                return out + ",ENTRY)";
            case EXIT:
                return out + ",EXIT)";
            }

            return out;
        }

        bool operator==(const Vertex& other) const {
            return data == other.data;
        }

        bool operator!=(const Vertex& other) const {
            return data != other.data;
        }
            
        Vertex(VertexData data) : data(data), NextNode(nullptr), PrevNode(nullptr), intercept_link(nullptr) {}
    };

    class PolygonIterator {
    public:
        PolygonIterator(Vertex* ptr, int loopTimes, Vertex* HeadNode) : currentVertex(ptr), loopTimes(loopTimes), HeadNode(HeadNode) {}

        PolygonIterator& operator++() {
            currentVertex = currentVertex->NextNode;
            if (currentVertex == HeadNode) {
                loopTimes += 1;
            }
            return *this;
        }

        PolygonIterator& operator++(int) {
            currentVertex = currentVertex->NextNode;
            if (currentVertex == HeadNode) {
                loopTimes += 1;
            }
            return *this;
        }


        Vertex* operator*() {
            return currentVertex;
        }

        bool operator==(const PolygonIterator& other) const {
            return currentVertex == other.currentVertex && loopTimes == other.loopTimes;
        }

        bool operator!=(const PolygonIterator& other) const {
            return !(currentVertex == other.currentVertex && loopTimes == other.loopTimes);
        }

        int getLoopCount() const {
            return loopTimes;
        }

    private:
        Vertex* currentVertex;
        int loopTimes;
        Vertex* HeadNode;
    };
    
public:
    Vertex* HeadNode;
    Vertex* TailNode;
    int size;

public:
    Polygon() : HeadNode(nullptr), TailNode(nullptr), size(0) {};
	~Polygon();

    void printPolygon(FString title = "") const;

    Polygon(FString polygonString) {
        HeadNode = nullptr;
        TailNode = nullptr;
        size = 0;

        this->initalizeWithString(polygonString);
    }

    Polygon(const Polygon& target)
    {
        HeadNode = nullptr; 
        TailNode = nullptr;
        size = 0;

        // Copy Constructor
        if (target.size == 0) return;

        Vertex* CurrentNode = target.HeadNode;

        do{
            if (CurrentNode != nullptr) {
                Add({ CurrentNode->data.pos,CurrentNode->data.type });
                CurrentNode = CurrentNode->NextNode;
            }

        } while (CurrentNode != nullptr && CurrentNode != target.HeadNode);
    }

    Polygon& operator=(const Polygon& target)
    {
        // Assignment operator 
        Empty();

        if (target.size == 0) return *this;

        Vertex* CurrentNode = target.HeadNode;

        do {
            Add({ CurrentNode->data.pos,CurrentNode->data.type });
            CurrentNode = CurrentNode->NextNode;

        } while (CurrentNode != target.HeadNode);

        return *this;
    }

    bool operator==(const Polygon& target) const
    {
        Vertex* vertexA = HeadNode;
        Vertex* vertexB = target.HeadNode;

        if (Num() != target.Num()) {
            return false;
        }

        if (vertexA == nullptr || vertexB == nullptr) {
            return (vertexA == nullptr && vertexB == nullptr);
        }

        do {

            if (vertexA->data != vertexB->data) {
                return false;
            }

            vertexA = vertexA->NextNode;
            vertexB = vertexB->NextNode;

        } while (vertexA != HeadNode);

        return true;
    }

    bool operator!=(const Polygon& target) const
    {
        return !operator==(target);
    }

    void initalizeWithString(FString polygonString);

    bool pointInsidePolygon(FVector2D const& point);

    bool triangulatePolygon(TArray<FVector2D>& out_vertices, FJsonSerializableArrayInt& out_triangles);

    bool extrudePolygon(float length, TArray<FVector>& out_vertices, FJsonSerializableArrayInt& out_triangles, TArray<FVector2D>& in_vertices, FJsonSerializableArrayInt& in_triangles);

    bool isPolygonClockwise();

    void flipPolygonVertexOrder();

    int Num() const;

    // Returns pointer to added Vertex
    Vertex* Insert(VertexData x, Vertex* nodeBeforex);

    Vertex* Add(VertexData X);

    void Remove(Vertex* x);

    void Empty();

    bool IsEmpty() const; 

    PolygonIterator begin() {
        return PolygonIterator(HeadNode,0,HeadNode);
    }

    PolygonIterator end() {
        return PolygonIterator(HeadNode,1,HeadNode);
    }
};
