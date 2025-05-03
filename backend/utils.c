#include "utils.h"

// Helper function to split a string by delimiter
static void split_string(const char* str, char delimiter, char result[][MAX_SKILL_LENGTH], int* count) {
    char temp[MAX_SKILL_LENGTH];
    int i = 0, j = 0, k = 0;
    
    while (str[i] != '\0') {
        if (str[i] == delimiter) {
            temp[j] = '\0';
            strcpy(result[k], temp);
            k++;
            j = 0;
        } else {
            temp[j] = str[i];
            j++;
        }
        i++;
    }
    
    if (j > 0) {
        temp[j] = '\0';
        strcpy(result[k], temp);
        k++;
    }
    
    *count = k;
}

void read_freelancers(const char* filename, Freelancer* freelancers, int* num_freelancers) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening freelancers file: %s\n", filename);
        return;
    }

    char line[512];
    *num_freelancers = 0;
    
    // Skip header
    fgets(line, sizeof(line), file);
    
    while (fgets(line, sizeof(line), file) && *num_freelancers < MAX_FREELANCERS) {
        Freelancer* f = &freelancers[*num_freelancers];
        char skills_str[256];
        
        // Remove newline if present
        line[strcspn(line, "\n")] = 0;
        
        // Parse CSV line
        char* token = strtok(line, ",");
        if (!token) continue;
        f->id = atoi(token);
        
        token = strtok(NULL, ",");
        if (!token) continue;
        strncpy(f->name, token, MAX_NAME_LENGTH - 1);
        
        token = strtok(NULL, ",");
        if (!token) continue;
        strncpy(skills_str, token, sizeof(skills_str) - 1);
        
        token = strtok(NULL, ",");
        if (!token) continue;
        f->experience = atoi(token);
        
        // Split skills
        split_string(skills_str, ' ', f->skills, &f->num_skills);
        
        // Initialize availability to false
        for (int i = 0; i < 7; i++) {
            f->availability[i] = false;
        }
        
        (*num_freelancers)++;
    }
    
    fclose(file);
    printf("Loaded %d freelancers\n", *num_freelancers);
}

void read_projects(const char* filename, Project* projects, int* num_projects) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening projects file\n");
        return;
    }

    char line[256];
    *num_projects = 0;
    
    // Skip header
    fgets(line, sizeof(line), file);
    
    while (fgets(line, sizeof(line), file) && *num_projects < MAX_PROJECTS) {
        Project* p = &projects[*num_projects];
        char skills_str[256];
        
        sscanf(line, "%d,%[^,],%[^,],%d,%d", 
               &p->id, p->name, skills_str, &p->min_experience, &p->deadline_days);
        
        split_string(skills_str, ' ', p->required_skills, &p->num_required_skills);
        
        (*num_projects)++;
    }
    
    fclose(file);
}

void read_availability(const char* filename, Freelancer* freelancers, int num_freelancers) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening availability file: %s\n", filename);
        return;
    }

    char line[256];
    int count = 0;
    
    // Skip header
    fgets(line, sizeof(line), file);
    
    while (fgets(line, sizeof(line), file)) {
        int freelancer_id, project_id, available;
        
        // Remove newline if present
        line[strcspn(line, "\n")] = 0;
        
        // Parse CSV line
        if (sscanf(line, "%d,%d,%d", &freelancer_id, &project_id, &available) == 3) {
            // Find the freelancer
            for (int i = 0; i < num_freelancers; i++) {
                if (freelancers[i].id == freelancer_id) {
                    // Convert project_id to day index (0-6)
                    int day_index = (project_id - 101) % 7;
                    if (day_index >= 0 && day_index < 7) {
                        freelancers[i].availability[day_index] = (available == 1);
                        count++;
                    }
                    break;
                }
            }
        }
    }
    
    fclose(file);
}

int calculate_skill_mismatch(const Freelancer* freelancer, const Project* project) {
    int matched_skills = 0;
    
    for (int i = 0; i < project->num_required_skills; i++) {
        for (int j = 0; j < freelancer->num_skills; j++) {
            if (strcmp(project->required_skills[i], freelancer->skills[j]) == 0) {
                matched_skills++;
                break;
            }
        }
    }
    
    return project->num_required_skills - matched_skills;
}

int calculate_experience_mismatch(const Freelancer* freelancer, const Project* project) {
    return project->min_experience > freelancer->experience ? 
           project->min_experience - freelancer->experience : 0;
}

int calculate_availability_mismatch(const Freelancer* freelancer, const Project* project) {
    int day_index = (project->id - 101) % 7;
    if (day_index >= 0 && day_index < 7) {
        return freelancer->availability[day_index] ? 0 : 1;
    }
    return 1;
}

void generate_cost_matrix(Freelancer* freelancers, int num_freelancers, 
                         Project* projects, int num_projects, 
                         int cost_matrix[MAX_FREELANCERS][MAX_PROJECTS]) {
    for (int i = 0; i < num_freelancers; i++) {
        for (int j = 0; j < num_projects; j++) {
            int skill_mismatch = calculate_skill_mismatch(&freelancers[i], &projects[j]);
            int exp_mismatch = calculate_experience_mismatch(&freelancers[i], &projects[j]);
            int avail_mismatch = calculate_availability_mismatch(&freelancers[i], &projects[j]);
            
            // Weight the different factors
            cost_matrix[i][j] = (skill_mismatch * 3) + (exp_mismatch * 2) + (avail_mismatch * 4);
        }
    }
}

char* format_matches_json(const Freelancer* freelancers, int num_freelancers,
                         const Project* projects, int num_projects,
                         const Assignment* assignments) {
    // Calculate buffer size needed
    int buffer_size = 1024 * 1024; // 1MB initial buffer
    char* json = (char*)malloc(buffer_size);
    int pos = 0;
    int assigned_count = 0;
    
    // Start JSON object
    pos += snprintf(json + pos, buffer_size - pos, 
                   "{\"total_freelancers\":%d,\"total_projects\":%d,\"matches\":[",
                   num_freelancers, num_projects);
    
    // Add each freelancer with their match (or null if no match)
    for (int i = 0; i < num_freelancers; i++) {
        if (i > 0) {
            pos += snprintf(json + pos, buffer_size - pos, ",");
        }
        
        pos += snprintf(json + pos, buffer_size - pos,
                       "{\"freelancer\":{\"id\":%d,\"name\":\"%s\",\"experience\":%d,\"skills\":[",
                       freelancers[i].id, freelancers[i].name, freelancers[i].experience);
        
        // Add skills
        for (int j = 0; j < freelancers[i].num_skills; j++) {
            if (j > 0) {
                pos += snprintf(json + pos, buffer_size - pos, ",");
            }
            pos += snprintf(json + pos, buffer_size - pos, "\"%s\"",
                           freelancers[i].skills[j]);
        }
        
        pos += snprintf(json + pos, buffer_size - pos, "]},");
        
        // Find if this freelancer has an assignment
        int assigned = 0;
        for (int j = 0; j < num_freelancers; j++) {
            if (assignments[j].freelancer_id == freelancers[i].id) {
                assigned = 1;
                assigned_count++;
                
                // Find the matching project
                for (int k = 0; k < num_projects; k++) {
                    if (projects[k].id == assignments[j].project_id) {
                        pos += snprintf(json + pos, buffer_size - pos,
                                     "\"project\":{\"id\":%d,\"name\":\"%s\",\"required_skills\":[",
                                     projects[k].id, projects[k].name);
                        
                        // Add required skills
                        for (int l = 0; l < projects[k].num_required_skills; l++) {
                            if (l > 0) {
                                pos += snprintf(json + pos, buffer_size - pos, ",");
                            }
                            pos += snprintf(json + pos, buffer_size - pos, "\"%s\"",
                                         projects[k].required_skills[l]);
                        }
                        
                        pos += snprintf(json + pos, buffer_size - pos,
                                     "],\"min_experience\":%d,\"deadline_days\":%d},\"score\":%d}",
                                     projects[k].min_experience,
                                     projects[k].deadline_days,
                                     assignments[j].score);
                        break;
                    }
                }
                break;
            }
        }
        
        if (!assigned) {
            pos += snprintf(json + pos, buffer_size - pos, "\"project\":null,\"score\":0}");
        }
    }

    // Add unmatched projects
    for (int i = 0; i < num_projects; i++) {
        int matched = 0;
        for (int j = 0; j < num_freelancers; j++) {
            if (assignments[j].project_id == projects[i].id) {
                matched = 1;
                break;
            }
        }
        
        if (!matched) {
            pos += snprintf(json + pos, buffer_size - pos, ",");
            pos += snprintf(json + pos, buffer_size - pos,
                         "{\"freelancer\":null,\"project\":{\"id\":%d,\"name\":\"%s\",\"required_skills\":[",
                         projects[i].id, projects[i].name);
            
            // Add required skills
            for (int j = 0; j < projects[i].num_required_skills; j++) {
                if (j > 0) {
                    pos += snprintf(json + pos, buffer_size - pos, ",");
                }
                pos += snprintf(json + pos, buffer_size - pos, "\"%s\"",
                             projects[i].required_skills[j]);
            }
            
            pos += snprintf(json + pos, buffer_size - pos,
                         "],\"min_experience\":%d,\"deadline_days\":%d},\"score\":0}",
                         projects[i].min_experience,
                         projects[i].deadline_days);
        }
    }
    
    // Add statistics and close JSON object
    pos += snprintf(json + pos, buffer_size - pos,
                   "],\"statistics\":{\"assigned_count\":%d,\"assigned_percentage\":%.1f,"
                   "\"unassigned_count\":%d,\"unassigned_percentage\":%.1f}}",
                   assigned_count,
                   (float)assigned_count / num_freelancers * 100,
                   num_freelancers - assigned_count,
                   (float)(num_freelancers - assigned_count) / num_freelancers * 100);
    
    return json;
}
