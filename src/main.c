#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>

#define NUM_NODES 10
#define INF INT_MAX
#define NODE_RADIUS 20

typedef struct Edge {
    int dest;
    int cost;
    struct Edge* next;
}
Edge;

typedef struct Graph {
    Edge* adjList[NUM_NODES];
}
Graph;

typedef struct {
    int x, y;
}
NodePosition;

typedef struct {
    int path[NUM_NODES];
    int cost;
    int length;
}
Path;

typedef struct {
    int visitedOrder[NUM_NODES];
    int visitedCount;
    int parentNode[NUM_NODES];
}
VisitedNodes;

NodePosition nodePos[NUM_NODES] = {
  {
    200,
    100
  },
  {
    100,
    200
  },
  {
    300,
    200
  },
  {
    400,
    100
  },
  {
    100,
    300
  },
  {
    300,
    300
  },
  {
    500,
    200
  },
  {
    50,
    400
  },
  {
    500,
    400
  },
  {
    600,
    300
  }
};

int currentStep = 0;
int algorithmChoice = 0;
int startNode = 0;
int targetNode = 9;
bool pathFound = false;

Graph* createGraph() {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    for (int i = 0; i < NUM_NODES; i++) {
        graph->adjList[i] = NULL;
    }
    return graph;
}

void addEdge(Graph* graph, int src, int dest, int cost) {
    Edge* newEdge = (Edge*)malloc(sizeof(Edge));
    newEdge->dest = dest;
    newEdge->cost = cost;
    newEdge->next = graph->adjList[src];
    graph->adjList[src] = newEdge;
}

Path bfs(Graph* graph, int start, int goal, VisitedNodes* visited) {
    Path path = {
      .cost = 0,
      .length = 0
    };
    bool visitedArray[NUM_NODES] = {
      false
    };
    int queue[NUM_NODES], parent[NUM_NODES];
    int front = 0, rear = 0;
    memset(parent, -1, sizeof(parent));

    queue[rear++] = start;
    visitedArray[start] = true;
    visited->visitedOrder[visited->visitedCount++] = start;
    visited->parentNode[start] = -1;

    while (front < rear) {
        int current = queue[front++];
        if (current == goal) break;

        for (Edge* edge = graph->adjList[current]; edge != NULL; edge = edge->next) {
            if (!visitedArray[edge->dest]) {
                queue[rear++] = edge->dest;
                visitedArray[edge->dest] = true;
                parent[edge->dest] = current;
                visited->visitedOrder[visited->visitedCount++] = edge->dest;
                visited->parentNode[edge->dest] = current;
            }
        }
    }

    if (visitedArray[goal]) {
        int node = goal;
        while (node != -1) {
            path.path[path.length++] = node;
            if (parent[node] != -1) {
                for (Edge* edge = graph->adjList[parent[node]]; edge != NULL; edge = edge->next) {
                    if (edge->dest == node) {
                        path.cost += edge->cost;
                        break;
                    }
                }
            }
            node = parent[node];
        }
        pathFound = true;
    }
    else {
        pathFound = false;
    }
    return path;
}

void dfsHelper(Graph* graph, int current, int goal, bool* found, bool visited[], int parent[], VisitedNodes* visitedNodes) {
    visited[current] = true;
    visitedNodes->visitedOrder[visitedNodes->visitedCount++] = current;

    if (current == goal) {
        *found = true;
        return;
    }

    for (Edge* edge = graph->adjList[current]; edge != NULL; edge = edge->next) {
        if (!visited[edge->dest]) {
            parent[edge->dest] = current;
            visitedNodes->parentNode[edge->dest] = current;
            dfsHelper(graph, edge->dest, goal, found, visited, parent, visitedNodes);
            if (*found) return;
        }
    }
}

Path dfs(Graph* graph, int start, int goal, VisitedNodes* visited) {
    Path path = {
      .cost = 0,
      .length = 0
    };
    bool found = false, visitedArray[NUM_NODES] = {
      false
    };
    int parent[NUM_NODES];
    memset(parent, -1, sizeof(parent));

    visited->parentNode[start] = -1;
    dfsHelper(graph, start, goal, &found, visitedArray, parent, visited);

    if (found) {
        int node = goal;
        while (node != -1) {
            path.path[path.length++] = node;
            if (parent[node] != -1) {
                for (Edge* edge = graph->adjList[parent[node]]; edge != NULL; edge = edge->next) {
                    if (edge->dest == node) {
                        path.cost += edge->cost;
                        break;
                    }
                }
            }
            node = parent[node];
        }
        pathFound = true;
    }
    else {
        pathFound = false;
    }
    return path;
}

typedef struct PQNode {
    int node;
    int cost;
}
PQNode;
typedef struct PriorityQueue {
    PQNode elements[NUM_NODES];
    int size;
}
PriorityQueue;

void pqPush(PriorityQueue* pq, int node, int cost) {
    int i = pq->size++;
    while (i > 0 && pq->elements[(i - 1) / 2].cost > cost) {
        pq->elements[i] = pq->elements[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    pq->elements[i].node = node;
    pq->elements[i].cost = cost;
}

PQNode pqPop(PriorityQueue* pq) {
    PQNode minNode = pq->elements[0];
    pq->elements[0] = pq->elements[--pq->size];
    int i = 0;
    while (2 * i + 1 < pq->size) {
        int j = 2 * i + 1;
        if (j + 1 < pq->size && pq->elements[j + 1].cost < pq->elements[j].cost) {
            j++;
        }
        if (pq->elements[i].cost <= pq->elements[j].cost) break;
        PQNode temp = pq->elements[i];
        pq->elements[i] = pq->elements[j];
        pq->elements[j] = temp;
        i = j;
    }
    return minNode;
}

bool pqIsEmpty(PriorityQueue* pq) {
    return pq->size == 0;
}

Path ucs(Graph* graph, int start, int goal, VisitedNodes* visited) {
    Path path = {
      .cost = 0,
      .length = 0
    };
    bool visitedArray[NUM_NODES] = {
      false
    };
    int cost[NUM_NODES];
    int parent[NUM_NODES];

    for (int i = 0; i < NUM_NODES; i++) cost[i] = INF;
    memset(parent, -1, sizeof(parent));
    visited->visitedCount = 0;

    PriorityQueue pq = {
      .size = 0
    };
    pqPush(&pq, start, 0);
    cost[start] = 0;

    do {
        PQNode currentNode = pqPop(&pq);
        int current = currentNode.node;
        int currentCost = currentNode.cost;

        if (visitedArray[current]) continue;
        visitedArray[current] = true;
        visited->visitedOrder[visited->visitedCount++] = current;

        if (current == goal) {
            pathFound = true;
            break;
        }

        for (Edge* edge = graph->adjList[current]; edge != NULL; edge = edge->next) {
            int neighbor = edge->dest;
            int newCost = currentCost + edge->cost;

            if (!visitedArray[neighbor] && newCost < cost[neighbor]) {
                cost[neighbor] = newCost;
                parent[neighbor] = current;
                visited->parentNode[neighbor] = current;
                pqPush(&pq, neighbor, newCost);
            }
        }
    } while (!pqIsEmpty(&pq));

    if (pathFound) {
        int node = goal;
        while (node != -1) {
            path.path[path.length++] = node;
            node = parent[node];
        }
        path.cost = cost[goal];

        for (int i = 0; i < path.length / 2; i++) {
            int temp = path.path[i];
            path.path[i] = path.path[path.length - i - 1];
            path.path[path.length - i - 1] = temp;
        }
    }
    return path;
}

void displayVisitedNodesAndPath(VisitedNodes* visited, Path* path, int explorationCost) {
    DrawText("Visited Nodes:", 10, 600, 20, BLACK);
    for (int i = 0; i <= currentStep && i < visited->visitedCount; i++) {
        DrawText(TextFormat("%c ", 'A' + visited->visitedOrder[i]), 10 + i * 20, 630, 20, BLACK);
    }

    if (pathFound && visited->visitedOrder[currentStep] == targetNode) {
        DrawText("Final Path:", 10, 660, 20, BLACK);
        for (int i = 0; i < path->length; i++) {
            DrawText(TextFormat("%c ", 'A' + path->path[i]), 10 + i * 20, 690, 20, DARKGREEN);
        }
        DrawText(TextFormat("Total Cost: %d", path->cost), 10, 720, 20, DARKGREEN);
    }

    DrawText(TextFormat("Exploration Cost: %d", explorationCost), 950, 760, 20, RED);
}

int calculateExplorationCost(Graph* graph, VisitedNodes* visited, int currentStep) {
    int totalCost = 0;

    for (int i = 1; i <= currentStep; i++) {
        int toNode = visited->visitedOrder[i];
        int fromNode = visited->parentNode[toNode];

        for (Edge* edge = graph->adjList[fromNode]; edge != NULL; edge = edge->next) {
            if (edge->dest == toNode) {
                totalCost += edge->cost;
                break;
            }
        }
    }
    return totalCost;
}

void visualizeAlgorithmStep(VisitedNodes* visited) {
    for (int i = 0; i <= currentStep && i < visited->visitedCount; i++) {
        int node = visited->visitedOrder[i];
        DrawCircle(nodePos[node].x, nodePos[node].y, NODE_RADIUS, GREEN);
    }

    for (int i = 0; i < NUM_NODES; i++) {
        DrawText(TextFormat("%c", 'A' + i), nodePos[i].x - 10, nodePos[i].y - 10, 20, BLACK);
    }
}

void drawGraph(Graph* graph) {
    for (int i = 0; i < NUM_NODES; i++) {
        for (Edge* edge = graph->adjList[i]; edge != NULL; edge = edge->next) {
            if (i < edge->dest) {
                DrawLine(nodePos[i].x, nodePos[i].y, nodePos[edge->dest].x, nodePos[edge->dest].y, LIGHTGRAY);
                int midX = (nodePos[i].x + nodePos[edge->dest].x) / 2;
                int midY = (nodePos[i].y + nodePos[edge->dest].y) / 2;
                DrawText(TextFormat("%d", edge->cost), midX, midY, 20, DARKGRAY);
            }
        }
    }

    for (int i = 0; i < NUM_NODES; i++) {
        Color nodeColor = SKYBLUE;
        if (i == startNode) nodeColor = YELLOW;
        else if (i == targetNode) nodeColor = RED;
        DrawCircle(nodePos[i].x, nodePos[i].y, NODE_RADIUS, nodeColor);
        DrawText(TextFormat("%c", 'A' + i), nodePos[i].x - 10, nodePos[i].y - 10, 20, BLACK);
    }
}

bool Button(Rectangle rect,
    const char* text) {
    DrawRectangleRec(rect, LIGHTGRAY);
    DrawText(text, rect.x + 10, rect.y + 10, 20, BLACK);
    return (CheckCollisionPointRec(GetMousePosition(), rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
}

int main() {
    InitWindow(1200, 800, "Homework1");
    SetTargetFPS(60);

    Graph* graph = createGraph();
    addEdge(graph, 0, 1, 4);
    addEdge(graph, 1, 0, 4);
    addEdge(graph, 0, 2, 7);
    addEdge(graph, 2, 0, 7);
    addEdge(graph, 1, 2, 2);
    addEdge(graph, 2, 1, 2);
    addEdge(graph, 1, 4, 6);
    addEdge(graph, 4, 1, 6);
    addEdge(graph, 2, 3, 3);
    addEdge(graph, 3, 2, 3);
    addEdge(graph, 2, 5, 8);
    addEdge(graph, 5, 2, 8);
    addEdge(graph, 3, 6, 9);
    addEdge(graph, 6, 3, 9);
    addEdge(graph, 4, 5, 5);
    addEdge(graph, 5, 4, 5);
    addEdge(graph, 4, 7, 3);
    addEdge(graph, 7, 4, 3);
    addEdge(graph, 5, 6, 1);
    addEdge(graph, 6, 5, 1);
    addEdge(graph, 5, 8, 7);
    addEdge(graph, 8, 5, 7);
    addEdge(graph, 6, 9, 6);
    addEdge(graph, 9, 6, 6);
    addEdge(graph, 8, 9, 4);
    addEdge(graph, 9, 8, 4);
    addEdge(graph, 7, 8, 8);
    addEdge(graph, 8, 7, 8);

    VisitedNodes visited = {
      .visitedCount = 0
    };
    Path currentPath = {
      .length = 0
    };

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        drawGraph(graph);
        visualizeAlgorithmStep(&visited);

        int explorationCost = calculateExplorationCost(graph, &visited, currentStep);
        displayVisitedNodesAndPath(&visited, &currentPath, explorationCost);

        if (Button((Rectangle) {
            850,
                500,
                100,
                40
        }, "Next Step") && currentStep < visited.visitedCount - 1) {
            currentStep++;
        }
        if (Button((Rectangle) {
            700,
                500,
                100,
                40
        }, "Prev Step") && currentStep > 0) {
            currentStep--;
        }
        if (Button((Rectangle) {
            550,
                500,
                100,
                40
        }, "Restart")) {
            currentStep = 0;
            visited.visitedCount = 0;
            pathFound = false;
            currentPath.length = 0;
        }
        if (Button((Rectangle) {
            100,
                500,
                100,
                40
        }, "BFS")) {
            algorithmChoice = 0;
            visited.visitedCount = 0;
            pathFound = false;
            currentPath = bfs(graph, startNode, targetNode, &visited);
            currentStep = 0;
        }
        if (Button((Rectangle) {
            250,
                500,
                100,
                40
        }, "DFS")) {
            algorithmChoice = 1;
            visited.visitedCount = 0;
            pathFound = false;
            currentPath = dfs(graph, startNode, targetNode, &visited);
            currentStep = 0;
        }
        if (Button((Rectangle) {
            400,
                500,
                100,
                40
        }, "UCS")) {
            algorithmChoice = 2;
            visited.visitedCount = 0;
            pathFound = false;
            currentPath = ucs(graph, startNode, targetNode, &visited);
            currentStep = 0;
        }

        EndDrawing();
    }

    CloseWindow();
    free(graph);
    return 0;
}