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
            switch (data.type) {
            case NONE:
                return "[ " + data.pos.ToString() + ", NONE]";
            case ENTRY:
                return "[" + data.pos.ToString() + ", ENTRY]";
            case EXIT:
                return "[" + data.pos.ToString() + ", EXIT]";
            }

            return data.pos.ToString();
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
    
private:
    Vertex* HeadNode;
    Vertex* TailNode;
    int size;

public:
    Polygon() : HeadNode(nullptr), TailNode(nullptr), size(0) {};
	~Polygon();

    void printPolygon() const;

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

    bool pointInsidePolygon(FVector2D const& point) const;

    int Num() const;

    // Returns pointer to added Vertex
    Vertex* Insert(VertexData x, Vertex* nodeBeforex);

    Vertex* Add(VertexData X);

    void Empty();

    bool IsEmpty() const; 

    PolygonIterator begin() {
        return PolygonIterator(HeadNode,0,HeadNode);
    }

    PolygonIterator end() {
        return PolygonIterator(TailNode,1,HeadNode);
    }

    friend class UWall_Cutter_Test;
    friend class TestAdd;
    friend class TestInsert;
    friend class TestUniqueData;
    friend class TestEmpty;
       
};
