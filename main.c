#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct graphVertex Vertex;
struct graphVertex
{
    Vertex *next;
    char team[81];
    int inDegree;
    int outDegree;
    int isVisited;
    struct graphEdge *firstArc;
};

typedef struct graphEdge Edge;
struct graphEdge
{
    Vertex *destination;
    int weight;
    Edge *nextArc;
};

typedef struct graphHead graphHead;
struct graphHead
{
    int count;
    Vertex *first;
};

graphHead *readTeams(FILE *);
void readMatches(FILE *, graphHead *);
graphHead *createGraph(void);
void createVertex(graphHead *, char[]);
void createEdge(graphHead *, char[], char[], int);
void printGraph(graphHead *);
void getMostWins(graphHead *);
void getMostLosses(graphHead *);
void getMaxGoals(graphHead *);
void getMinGoals(graphHead *);
int checkwinChain(graphHead *, char[], char[]);
int checkPath(graphHead *, char[], char[]);
// EXTRA FUNCTIONS
Vertex* findTeam(graphHead *, char[]); // finds the wanted team according to team name
int DFS(Vertex *, Vertex *); // Depth First Search
void freeGraph(graphHead *); // Frees the graph

int main(int argc, char *argv[]){
    FILE *inFile;
    graphHead *graph;
    inFile = fopen("teams.txt", "r");
    if (inFile == NULL){
        printf("File could not be opened.\n");
        exit(1);
    }
    printf("Teams File has been opened successfully; the graph with no edges can be seen below:\n");
    graph = readTeams(inFile);
    printGraph(graph);
    printf("***************************************************************************************************************\n");

    fclose(inFile);
    inFile = fopen("matches.txt", "r");
    if (inFile == NULL){
        printf("File could not be opened.\n");
        exit(1);
    }
    printf("Matches File has been opened successfully; the graph with edges can be seen below:\n");
    readMatches(inFile, graph);
    printGraph(graph);
    printf("***************************************************************************************************************\n");

    getMostWins(graph);
    getMostLosses(graph);
    getMaxGoals(graph);
    getMinGoals(graph);
    printf("***************************************************************************************************************\n");
    if (argc < 3){
        printf("Two teams are not given in the command line arguments!");
    }
    else{
        char str1[81], str2[81];
        int i = 0;
        while (argv[1][i] != '\0'){
            if (argv[1][i] == '_')
                str1[i] = ' ';
            else
                str1[i] = argv[1][i];
            i++;
        }
        str1[i] = '\0';
        i = 0;
        while (argv[2][i] != '\0'){
            if (argv[2][i] == '_')
                str2[i] = ' ';
            else
                str2[i] = argv[2][i];
            i++;
        }
        str2[i] = '\0';

        int result = checkwinChain(graph, str1, str2);
        if (result)
            printf("%s have beaten a team that beat %s.\n", str1, str2);

        else
            printf("%s have NOT beaten a team that beat %s.\n", str1, str2);

        int result2 = checkPath(graph, str1, str2);
        if (result2)
            printf("There is a path from %s to %s\n", str1, str2);
        else
            printf("There is no path from %s to %s\n", str1, str2);
    }
    freeGraph(graph);
    return 0;
}

graphHead *createGraph(void)
{
    graphHead *graph;
    graph = (graphHead *)malloc(sizeof(graphHead));
    if (graph == NULL)
    {
        printf("Memory allocation is failed.");
        exit(1);
    }
    graph->count = 0;
    graph->first = NULL;
    return graph;
}

void createVertex(graphHead *graph, char teamName[])
{
    Vertex *vertex = (struct graphVertex *)malloc(sizeof(struct graphVertex));
    vertex->next = NULL;
    strcpy(vertex->team, teamName);
    vertex->inDegree = 0;
    vertex->outDegree = 0;
    vertex->firstArc = NULL;
    vertex->isVisited = 0;

    graph->count++;

    if (graph->first == NULL)
    {
        graph->first = vertex;
    }
    else
    {
        struct graphVertex *temp = graph->first;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = vertex;
    }
}

graphHead *readTeams(FILE *inFile)
{
    graphHead *graph;
    graph = createGraph();

    char line[81], teamName[81];

    while (fgets(line, sizeof(line), inFile))
    {
        if (sscanf(line, "%[^\n]\n", teamName))
        {
            teamName[strlen(teamName) - 1] = '\0'; // I'm using mac. I did this for proper string reading.
            createVertex(graph, teamName);
        }
    }
    return graph;
}

void createEdge(graphHead *graph, char winningTeam[], char losingTeam[], int goalDiff)
{
    Vertex *tempVertex = graph->first;
    Vertex *loserTeamVertex;
    Edge *newEdge;

    loserTeamVertex = findTeam(graph, losingTeam); // Finds loser team

    while (tempVertex != NULL)
    {
        if (strcasecmp(tempVertex->team, winningTeam) == 0) // Finds the winning team
        {
            newEdge = (struct graphEdge *)malloc(sizeof(struct graphEdge));
            if (tempVertex->firstArc == NULL) // First edge or not
                tempVertex->firstArc = newEdge;
            else // Adds edge to the end
            {
                Edge *tempEdge = tempVertex->firstArc;
                while (tempEdge->nextArc != NULL)
                {
                    tempEdge = tempEdge->nextArc;
                }
                tempEdge->nextArc = newEdge;
            }
            // Update the values
            newEdge->nextArc = NULL;
            newEdge->weight = goalDiff;
            newEdge->destination = loserTeamVertex;
            newEdge->destination->inDegree++;
            tempVertex->outDegree++;
        }

        tempVertex = tempVertex->next;
    }
}

void readMatches(FILE *inFile, graphHead *graph)
{
    char line[200];
    int Season_End_Year, Wk, HomeGoals, AwayGoals;
    char Date[50], Home[50], Away[50];
    char FTR;

    if (fgets(line, sizeof(line), inFile) == NULL)
    {
        perror("Error reading the first line\n");
        exit(1);
    } // Skipped the first line

    while (fgets(line, sizeof(line), inFile))
    {
        if (sscanf(line, "%d;%d;%[^;];%[^;];%d;%d;%[^;];%c\n",
                   &Season_End_Year, &Wk, Date, Home, &HomeGoals, &AwayGoals, Away,
                   &FTR) == 8)
        {
            if (FTR == 'H') // Home Wins
            {
                createEdge(graph, Home, Away, abs(HomeGoals - AwayGoals));
            }

            else if (FTR == 'A') // Away Wins
            {
                createEdge(graph, Away, Home, abs(HomeGoals - AwayGoals));
            }
        }
    }
}

void getMostWins(graphHead *graph){
    Vertex *tempVertex = graph->first;
    int max = tempVertex->outDegree;

    while(tempVertex != NULL) // Finds max number of wins
    {
        if(max <= tempVertex->outDegree)
            max = tempVertex->outDegree;
        tempVertex = tempVertex->next;
    }

    tempVertex = graph->first;

    while(tempVertex->outDegree != max) // Finds which team has the max number of wins
    {
        tempVertex = tempVertex->next;
    }

    printf("Team with the most victories:\n");
    printf("%s have won %d matches.\n", tempVertex->team, tempVertex->outDegree);
}

void getMostLosses(graphHead *graph){
    Vertex *tempVertex = graph->first;
    int max = tempVertex->inDegree;

    while(tempVertex != NULL) // Finds the max number of losses
    {
        if(max <= tempVertex->inDegree)
            max = tempVertex->inDegree;
        tempVertex = tempVertex->next;
    }

    tempVertex = graph->first;

    while(tempVertex->inDegree != max) // Finds which team has the max number of losses
    {
        tempVertex = tempVertex->next;
    }

    printf("\nTeam with the most losses:\n");
    printf("%s have lost %d matches.\n", tempVertex->team, tempVertex->inDegree);
}

void getMaxGoals(graphHead *graph){

    Vertex *tempVertex = graph->first;
    int max = 0; // Creating and initializing max goal difference
    char maxTeam[50];
    strcpy(maxTeam, tempVertex->team); // Giving an initial value to the maxTeam
    
    //In the following nested loop Im traversing the whole graph for each vertex
    while(tempVertex != NULL) // Traverse all vertices
    {
        Vertex *tempVertex2 = graph->first; // This vertex is for reaching the other vertice's edges
        int goalDiffWon = 0; // Reseting for each vertex
        int goalDiffLost = 0; // Reseting for each vertex
        while(tempVertex2 != NULL)
        {
            Edge *tempEdge = tempVertex2->firstArc;
            while(tempEdge != NULL) 
            /*
            In here Im trying to find if our team(tempVertex->team) has lost to another team.
            If team has lost to another team, I do ->  goalDiffLost += tempEdge->weight;
            */ 
            {
                if(strcasecmp(tempVertex->team, tempEdge->destination->team) == 0)
                {
                    goalDiffLost += tempEdge->weight;
                }

                tempEdge = tempEdge->nextArc;
            }

            tempVertex2 = tempVertex2->next;
        }
        Edge *tempEdge = tempVertex->firstArc;
        while(tempEdge != NULL) // In here Im adding the weights of defeated teams by tempVertex->team
        {
            goalDiffWon += tempEdge->weight;
            tempEdge = tempEdge->nextArc;
        }

        if(max < (goalDiffWon - goalDiffLost)) // Trying to find max goal difference
        {
            // Update the values
            max = (goalDiffWon - goalDiffLost);
            strcpy(maxTeam, tempVertex->team);
        }

        tempVertex = tempVertex->next;
    }

    printf("\nTeam with the highest goal difference:\n"
           "%s have a goal difference of %d.\n", maxTeam, max);
}

// This function is the same as getMaxGoals function.
void getMinGoals(graphHead *graph){

    Vertex *tempVertex = graph->first;
    int min = 0;
    char minTeam[50];
    strcpy(minTeam, tempVertex->team);
    while(tempVertex != NULL) // goalDiffLost for each team
    {
        Vertex *tempVertex2 = graph->first;
        int goalDiffWon = 0;
        int goalDiffLost = 0;
        while(tempVertex2 != NULL)
        {
            Edge *tempEdge = tempVertex2->firstArc;
            while(tempEdge != NULL)
            {
                if(strcasecmp(tempVertex->team, tempEdge->destination->team) == 0)
                {
                    goalDiffLost += tempEdge->weight;
                }

                tempEdge = tempEdge->nextArc;
            }

            tempVertex2 = tempVertex2->next;
        }
        Edge *tempEdge = tempVertex->firstArc;
        while(tempEdge != NULL)
        {
            goalDiffWon += tempEdge->weight;
            tempEdge = tempEdge->nextArc;
        }

        if(min > (goalDiffWon - goalDiffLost)) // This is the only difference
        {
            min = (goalDiffWon - goalDiffLost);
            strcpy(minTeam, tempVertex->team);
        }
    
        tempVertex = tempVertex->next;
    }
    
    printf("\nTeam with the lowest goal difference:\n"
           "%s have a goal difference of %d.\n", minTeam, min);
}

// Function to find a vertex in the graph by its team name
Vertex* findTeam(graphHead* graph, char team[]) {
    Vertex* currentVertex = graph->first;
    while (currentVertex != NULL) {
        if (strcasecmp(currentVertex->team, team) == 0) {
            return currentVertex;
        }
        currentVertex = currentVertex->next;
    }
    return NULL;  // Vertex not found
}

int checkwinChain(graphHead *graph, char team1[], char team2[]){

    Vertex* vertex1 = findTeam(graph, team1); // Finds the team1
    Edge *tempEdge = vertex1->firstArc; // First Edge of the team1

    while(tempEdge != NULL)
    {
        if(tempEdge->destination != NULL) // I put this if statement just in case to check my graph connections
        {
            Edge *tempEdge2 = tempEdge->destination->firstArc;
            while(tempEdge2 != NULL) // Traverses destination vertex's edges
            {
                if(strcasecmp(tempEdge2->destination->team, team2) == 0) // Found
                    return 1;

                tempEdge2 = tempEdge2->nextArc;
            }
        }

        tempEdge = tempEdge->nextArc;
    }

    return 0; // Not Found
}

int DFS(Vertex* current, Vertex* target) {
    // Mark the current vertex as visited
    current->isVisited = 1;

    // Check if the current vertex is the target
    if (strcasecmp(current->team, target->team) == 0) {
        return 1;  // Path found
    }

    // Traverse outgoing edges of the current vertex
    Edge* currentEdge = current->firstArc;
    while (currentEdge != NULL) {
        if (!currentEdge->destination->isVisited) {
            // Recursively call DFS on the unvisited neighbour
            if (DFS(currentEdge->destination, target)) {
                return 1; // Path found
            }
        }
        // Move to the next outgoing edge
        currentEdge = currentEdge->nextArc;
    }

    return 0; // No path found
}

int checkPath(graphHead *graph, char team1[], char team2[]){
    // Find the vertices corresponding to team1 and team2
    Vertex* vertex1 = findTeam(graph, team1);
    Vertex* vertex2 = findTeam(graph, team2);

    // If either vertex1 or vertex2 is not in the graph, there is no path
    if (vertex1 == NULL || vertex2 == NULL) {
        return 0;
    }

    // Call the recursive DFS function to check for a path
    int result = DFS(vertex1, vertex2);

    return result;
}

void printGraph(graphHead *graph)
{
    Vertex *tempVertex = graph->first;
    while (tempVertex != NULL)
    {
        printf("%s -> ", tempVertex->team);
        if (tempVertex->firstArc != NULL)
        {
            Edge *tempEdge = tempVertex->firstArc;
            while (tempEdge != NULL)
            {
                printf("%s | %d", tempEdge->destination->team, tempEdge->weight);
                if(tempEdge->nextArc != NULL)
                    printf(" -> ");
                tempEdge = tempEdge->nextArc;
            }
        }
        printf("\n-------------------------------------------------------------------------------------------------------------------\n");
        printf("-------------------------------------------------------------------------------------------------------------------\n");
        tempVertex = tempVertex->next;
    }
}

void freeGraph(graphHead* graph) {
    if (graph == NULL) {
        return;  // Nothing to free
    }
    // Traverse through each vertex
    Vertex* currentVertex = graph->first;
    while (currentVertex != NULL) {
        // Traverse through each edge of the current vertex
        Edge* currentEdge = currentVertex->firstArc;
        while (currentEdge != NULL) {
            Edge* tempEdge = currentEdge;
            currentEdge = currentEdge->nextArc;
            free(tempEdge);  // Free the current edge
        }

        Vertex* tempVertex = currentVertex;
        currentVertex = currentVertex->next;
        free(tempVertex);  // Free the current vertex
    }
    // Free the graph structure
    free(graph);
}