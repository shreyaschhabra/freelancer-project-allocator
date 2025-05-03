#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "utils.h"
#include "match_allocator.h"
#include "bloom_filter_utils.h"

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to print a separator line
void print_separator(int length) {
    for (int i = 0; i < length; i++) {
        printf("=");
    }
    printf("\n");
}

// Function to print a header
void print_header(const char* text) {
    printf("\n%s\n", text);
    print_separator(strlen(text));
    printf("\n");
}

// Global Bloom filter for skills
static GlobalBloom global_bloom = {0};

// Function to handle HTTP requests
void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE * 2];
    
    // Read the request
    read(client_socket, buffer, BUFFER_SIZE);
    
    // Parse the request method and path
    char method[10], path[100];
    sscanf(buffer, "%s %s", method, path);
    
    // Handle CORS preflight request
    if (strcmp(method, "OPTIONS") == 0) {
        const char* cors_headers = 
            "HTTP/1.1 204 No Content\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Access-Control-Max-Age: 86400\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n";
        write(client_socket, cors_headers, strlen(cors_headers));
        close(client_socket);
        return;
    }
    
    // Handle GET request for /matches
    if (strcmp(method, "GET") == 0 && strcmp(path, "/matches") == 0) {
        Freelancer freelancers[MAX_FREELANCERS];
        Project projects[MAX_PROJECTS];
        int num_freelancers = 0;
        int num_projects = 0;
        Assignment assignments[MAX_FREELANCERS];
        
        // Read data from CSV files
        read_freelancers("../data/freelancers.csv", freelancers, &num_freelancers);
        read_projects("../data/projects.csv", projects, &num_projects);
        read_availability("../data/availability.csv", freelancers, num_freelancers);
        // Populate Bloom filter with all freelancer skills
        populate_bloom_with_freelancer_skills(&global_bloom, freelancers, num_freelancers);
        
        // Perform matching
        match_freelancers_to_projects(freelancers, num_freelancers,
                                    projects, num_projects,
                                    assignments);
        
        // Format the response as JSON
        char* json_response = format_matches_json(freelancers, num_freelancers,
                                                projects, num_projects,
                                                assignments);
        
        // Send the response headers
        char headers[BUFFER_SIZE];
        snprintf(headers, sizeof(headers),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Content-Length: %zu\r\n"
                "Connection: close\r\n\r\n",
                strlen(json_response));
        
        write(client_socket, headers, strlen(headers));
        
        // Send the JSON body
        write(client_socket, json_response, strlen(json_response));
        
        free(json_response);
    } else if (strcmp(method, "GET") == 0 && strncmp(path, "/freelancers_with_skill", 21) == 0) {
        // Parse skill from query string
        char* skill_param = strstr(path, "?skill=");
        char skill[100] = "";
        if (skill_param) {
            strncpy(skill, skill_param + 7, sizeof(skill) - 1);
            skill[sizeof(skill) - 1] = '\0';
        }
        Freelancer freelancers[MAX_FREELANCERS];
        int num_freelancers = 0;
        read_freelancers("../data/freelancers.csv", freelancers, &num_freelancers);
        // Build JSON array of freelancers with the skill
        char json_response[BUFFER_SIZE * 8];
        int pos = 0;
        pos += snprintf(json_response + pos, sizeof(json_response) - pos, "[");
        int first = 1;
        for (int i = 0; i < num_freelancers; i++) {
            int found = 0;
            for (int j = 0; j < freelancers[i].num_skills; j++) {
                if (strcmp(freelancers[i].skills[j], skill) == 0) {
                    found = 1;
                    break;
                }
            }
            if (found) {
                if (!first) pos += snprintf(json_response + pos, sizeof(json_response) - pos, ",");
                first = 0;
                pos += snprintf(json_response + pos, sizeof(json_response) - pos,
                    "{\"id\":%d,\"name\":\"%s\",\"experience\":%d,\"num_skills\":%d,\"skills\":[",
                    freelancers[i].id, freelancers[i].name, freelancers[i].experience, freelancers[i].num_skills);
                for (int k = 0; k < freelancers[i].num_skills; k++) {
                    if (k > 0) pos += snprintf(json_response + pos, sizeof(json_response) - pos, ",");
                    pos += snprintf(json_response + pos, sizeof(json_response) - pos, "\"%s\"", freelancers[i].skills[k]);
                }
                pos += snprintf(json_response + pos, sizeof(json_response) - pos, "]}");
            }
        }
        pos += snprintf(json_response + pos, sizeof(json_response) - pos, "]");
        char headers[BUFFER_SIZE];
        snprintf(headers, sizeof(headers),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n",
            strlen(json_response));
        write(client_socket, headers, strlen(headers));
        write(client_socket, json_response, strlen(json_response));
    } else if (strcmp(method, "GET") == 0 && strncmp(path, "/skill_exists", 13) == 0) {
        // Parse skill from query string
        char* skill_param = strstr(path, "?skill=");
        char skill[100] = "";
        if (skill_param) {
            strncpy(skill, skill_param + 7, sizeof(skill) - 1);
            skill[sizeof(skill) - 1] = '\0';
        }
        int possibly_exists = 0;
        if (strlen(skill) > 0) {
            possibly_exists = bloom_check(&global_bloom.filter, skill);
        }
        char json_response[128];
        snprintf(json_response, sizeof(json_response),
            "{\"skill\":\"%s\",\"possibly_exists\":%s}",
            skill, possibly_exists ? "true" : "false");
        char headers[BUFFER_SIZE];
        snprintf(headers, sizeof(headers),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n",
            strlen(json_response));
        write(client_socket, headers, strlen(headers));
        write(client_socket, json_response, strlen(json_response));
    } else {
        // Handle 404 Not Found
        const char* not_found = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 28\r\n"
            "Connection: close\r\n\r\n"
            "{\"error\":\"Resource not found\"}";
        write(client_socket, not_found, strlen(not_found));
    }
    
    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d...\n", PORT);
    
    // Accept connections
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, 
                                  (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }
        
        handle_request(client_socket);
    }
    
    return 0;
} 