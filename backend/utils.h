#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_FREELANCERS 100
#define MAX_PROJECTS 100
#define MAX_SKILLS 15
#define MAX_SKILL_LENGTH 50
#define MAX_NAME_LENGTH 100

// Structure to store freelancer information
typedef struct {
    int id;
    char name[MAX_NAME_LENGTH];
    char skills[MAX_SKILLS][MAX_SKILL_LENGTH];
    int num_skills;
    int experience;
    bool availability[7]; // 7 days of the week
} Freelancer;

// Structure to store project information
typedef struct {
    int id;
    char name[MAX_NAME_LENGTH];
    char required_skills[MAX_SKILLS][MAX_SKILL_LENGTH];
    int num_required_skills;
    int min_experience;
    int deadline_days;
} Project;

// Structure to store assignment information
typedef struct {
    int freelancer_id;
    int project_id;
    int score;
} Assignment;

// Graph structures
typedef struct GraphNode {
    int id;
    int weight;
    struct GraphNode* next;
} GraphNode;

typedef struct {
    GraphNode** adjacency_list;
    int num_nodes;
    int* node_types;  // 0 for freelancer, 1 for project
} BipartiteGraph;

// Function declarations for file reading and data processing
void read_freelancers(const char* filename, Freelancer* freelancers, int* num_freelancers);
void read_projects(const char* filename, Project* projects, int* num_projects);
void read_availability(const char* filename, Freelancer* freelancers, int num_freelancers);
int calculate_skill_mismatch(const Freelancer* freelancer, const Project* project);
int calculate_experience_mismatch(const Freelancer* freelancer, const Project* project);
int calculate_availability_mismatch(const Freelancer* freelancer, const Project* project);
void generate_cost_matrix(Freelancer* freelancers, int num_freelancers, 
                         Project* projects, int num_projects, 
                         int cost_matrix[MAX_FREELANCERS][MAX_PROJECTS]);

// Graph operations
BipartiteGraph* create_graph(int num_freelancers, int num_projects);
void add_edge(BipartiteGraph* graph, int freelancer_id, int project_id, int weight);
void free_graph(BipartiteGraph* graph);

// Matching functions
void match_freelancers_to_projects(const Freelancer* freelancers, int num_freelancers,
                                 const Project* projects, int num_projects,
                                 Assignment* assignments);
int calculate_compatibility(const Freelancer* freelancer, const Project* project);

// Function to format matches as JSON
char* format_matches_json(const Freelancer* freelancers, int num_freelancers,
                         const Project* projects, int num_projects,
                         const Assignment* assignments);

#endif // UTILS_H 