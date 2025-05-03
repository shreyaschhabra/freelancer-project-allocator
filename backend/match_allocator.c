#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "match_allocator.h"

#define INF INT_MAX

// Helper function to find the minimum value in a row
static int find_min_in_row(const int* row, int n) {
    int min = INF;
    for (int i = 0; i < n; i++) {
        if (row[i] < min) {
            min = row[i];
        }
    }
    return min;
}

// Helper function to find the minimum value in a column
static int find_min_in_col(const int** cost_matrix, int col, int n) {
    int min = INF;
    for (int i = 0; i < n; i++) {
        if (cost_matrix[i][col] < min) {
            min = cost_matrix[i][col];
        }
    }
    return min;
}

// Helper function to find a zero in the matrix
static int find_zero(const int** cost_matrix, int n, int* row, int* col,
                    const int* row_cover, const int* col_cover) {
    for (int i = 0; i < n; i++) {
        if (!row_cover[i]) {
            for (int j = 0; j < n; j++) {
                if (!col_cover[j] && cost_matrix[i][j] == 0) {
                    *row = i;
                    *col = j;
                    return 1;
                }
            }
        }
    }
    return 0;
}

// Helper function to find the minimum uncovered value
static int find_min_uncovered(const int** cost_matrix, int n,
                            const int* row_cover, const int* col_cover) {
    int min = INF;
    for (int i = 0; i < n; i++) {
        if (!row_cover[i]) {
            for (int j = 0; j < n; j++) {
                if (!col_cover[j] && cost_matrix[i][j] < min) {
                    min = cost_matrix[i][j];
                }
            }
        }
    }
    return min;
}

// Helper function to cover zeros with minimum lines
static void cover_zeros(const int** cost_matrix, int n, int* row_assignment, int* col_assignment) {
    int row_cover[MAX_FREELANCERS] = {0};
    int col_cover[MAX_PROJECTS] = {0};

    // Initialize assignments to -1
    for (int i = 0; i < n; i++) {
        row_assignment[i] = -1;
        col_assignment[i] = -1;
    }

    // Find independent zeros
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (cost_matrix[i][j] == 0 && !row_cover[i] && !col_cover[j]) {
                row_assignment[i] = j;
                col_assignment[j] = i;
                row_cover[i] = 1;
                col_cover[j] = 1;
                break;
            }
        }
    }
}

// Graph operations
BipartiteGraph* create_graph(int num_freelancers, int num_projects) {
    BipartiteGraph* graph = (BipartiteGraph*)malloc(sizeof(BipartiteGraph));
    graph->num_nodes = num_freelancers + num_projects;
    graph->adjacency_list = (GraphNode**)calloc(graph->num_nodes, sizeof(GraphNode*));
    graph->node_types = (int*)calloc(graph->num_nodes, sizeof(int));
    
    // Initialize node types
    for (int i = 0; i < num_freelancers; i++) {
        graph->node_types[i] = 0;  // Freelancer
    }
    for (int i = num_freelancers; i < graph->num_nodes; i++) {
        graph->node_types[i] = 1;  // Project
    }
    
    return graph;
}

void add_edge(BipartiteGraph* graph, int freelancer_id, int project_id, int weight) {
    GraphNode* new_node = (GraphNode*)malloc(sizeof(GraphNode));
    new_node->id = project_id;
    new_node->weight = weight;
    new_node->next = graph->adjacency_list[freelancer_id];
    graph->adjacency_list[freelancer_id] = new_node;
    
    // Add reverse edge for bipartite graph
    new_node = (GraphNode*)malloc(sizeof(GraphNode));
    new_node->id = freelancer_id;
    new_node->weight = weight;
    new_node->next = graph->adjacency_list[project_id];
    graph->adjacency_list[project_id] = new_node;
}

void free_graph(BipartiteGraph* graph) {
    for (int i = 0; i < graph->num_nodes; i++) {
        GraphNode* current = graph->adjacency_list[i];
        while (current != NULL) {
            GraphNode* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(graph->adjacency_list);
    free(graph->node_types);
    free(graph);
}

// Modified Hungarian Algorithm using graph structure
void hungarian_algorithm(const BipartiteGraph* graph, int* assignments) {
    int num_freelancers = 0;
    for (int i = 0; i < graph->num_nodes; i++) {
        if (graph->node_types[i] == 0) num_freelancers++;
    }
    
    // Create cost matrix from graph
    int** cost_matrix = (int**)malloc(num_freelancers * sizeof(int*));
    if (!cost_matrix) {
        return;  // Handle allocation failure
    }
    
    for (int i = 0; i < num_freelancers; i++) {
        cost_matrix[i] = (int*)malloc(num_freelancers * sizeof(int));
        if (!cost_matrix[i]) {
            // Clean up previously allocated memory
            for (int j = 0; j < i; j++) {
                free(cost_matrix[j]);
            }
            free(cost_matrix);
            return;  // Handle allocation failure
        }
        for (int j = 0; j < num_freelancers; j++) {
            cost_matrix[i][j] = INF;  // Initialize with infinity
        }
    }
    
    // Fill cost matrix from graph edges
    for (int i = 0; i < num_freelancers; i++) {
        GraphNode* current = graph->adjacency_list[i];
        while (current != NULL) {
            if (graph->node_types[current->id] == 1) {  // If it's a project
                // Convert score to cost (higher score = lower cost)
                cost_matrix[i][current->id - num_freelancers] = 100 - current->weight;
            }
            current = current->next;
        }
    }
    
    // Initialize assignments
    for (int i = 0; i < num_freelancers; i++) {
        assignments[i] = -1;
    }
    
    // Step 1: Subtract row minimums
    for (int i = 0; i < num_freelancers; i++) {
        int min = find_min_in_row(cost_matrix[i], num_freelancers);
        if (min != INF) {
            for (int j = 0; j < num_freelancers; j++) {
                if (cost_matrix[i][j] != INF) {
                    cost_matrix[i][j] -= min;
                }
            }
        }
    }
    
    // Step 2: Subtract column minimums
    for (int j = 0; j < num_freelancers; j++) {
        int min = find_min_in_col((const int**)cost_matrix, j, num_freelancers);
        if (min != INF) {
            for (int i = 0; i < num_freelancers; i++) {
                if (cost_matrix[i][j] != INF) {
                    cost_matrix[i][j] -= min;
                }
            }
        }
    }
    
    // Step 3: Cover zeros with minimum lines
    int row_assignment[MAX_FREELANCERS];
    int col_assignment[MAX_PROJECTS];
    cover_zeros((const int**)cost_matrix, num_freelancers, row_assignment, col_assignment);
    
    // Copy assignments
    for (int i = 0; i < num_freelancers; i++) {
        assignments[i] = row_assignment[i];
    }
    
    // Free cost matrix
    for (int i = 0; i < num_freelancers; i++) {
        free(cost_matrix[i]);
    }
    free(cost_matrix);
}

// Modified matching function to use graph structure
void match_freelancers_to_projects(const Freelancer* freelancers, int num_freelancers,
                                 const Project* projects, int num_projects,
                                 Assignment* assignments) {
    // Create bipartite graph
    BipartiteGraph* graph = create_graph(num_freelancers, num_projects);
    
    // Add edges based on compatibility
    for (int i = 0; i < num_freelancers; i++) {
        for (int j = 0; j < num_projects; j++) {
            int compatibility_score = calculate_compatibility(&freelancers[i], &projects[j]);
            if (compatibility_score > 0) {
                add_edge(graph, i, j + num_freelancers, compatibility_score);
            }
        }
    }
    
    // Perform matching using Hungarian Algorithm
    int* temp_assignments = (int*)calloc(num_freelancers, sizeof(int));
    hungarian_algorithm(graph, temp_assignments);
    
    // Convert assignments to the required format
    int assignment_count = 0;
    for (int i = 0; i < num_freelancers; i++) {
        if (temp_assignments[i] != -1) {
            assignments[assignment_count].freelancer_id = freelancers[i].id;
            assignments[assignment_count].project_id = projects[temp_assignments[i]].id;
            
            // Find the edge weight for this assignment
            GraphNode* current = graph->adjacency_list[i];
            while (current != NULL) {
                if (current->id == temp_assignments[i] + num_freelancers) {
                    assignments[assignment_count].score = current->weight;
                    break;
                }
                current = current->next;
            }
            
            assignment_count++;
        }
    }
    
    free(temp_assignments);
    free_graph(graph);
}

// Helper function to calculate compatibility score
int calculate_compatibility(const Freelancer* freelancer, const Project* project) {
    int matched_skills = 0;
    int experience_match = 0;
    
    // Calculate skill match percentage
    for (int i = 0; i < project->num_required_skills; i++) {
        for (int j = 0; j < freelancer->num_skills; j++) {
            if (strcmp(project->required_skills[i], freelancer->skills[j]) == 0) {
                matched_skills++;
                break;
            }
        }
    }
    
    // Convert matched skills to percentage
    int skill_match = project->num_required_skills > 0 ?
        (matched_skills * 100) / project->num_required_skills : 0;
    
    // Calculate experience match percentage
    if (freelancer->experience >= project->min_experience) {
        experience_match = 100;
    } else if (project->min_experience > 0) {
        experience_match = (freelancer->experience * 100) / project->min_experience;
        // Cap at 100%
        if (experience_match > 100) experience_match = 100;
    }
    
    // Weighted combination (70% skills, 30% experience)
    int total_score = (skill_match * 70 + experience_match * 30) / 100;
    
    // Only return score if there's at least one skill match
    return matched_skills > 0 ? total_score : 0;
} 