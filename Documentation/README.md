# Freelancer-Project Matching System

This project implements an optimal matching system for assigning freelancers to projects using the Hungarian Algorithm. The system takes into account various factors such as skills, experience, and availability to make the best possible assignments.

## Features

- Optimal matching using the Hungarian Algorithm
- Consideration of multiple factors:
  - Skill matches/mismatches
  - Experience level requirements
  - Availability constraints
- CSV-based data input for easy configuration
- Detailed matching results output
- Support for large datasets (70+ freelancers and projects)

## Project Structure

```
/freelancer-matching
│
├── /data
│   ├── freelancers.csv    # Freelancer information (70 entries)
│   ├── projects.csv       # Project requirements (70 entries)
│   ├── availability.csv   # Freelancer availability
│
├── /backend
│   ├── match_allocator.c  # Hungarian Algorithm implementation
│   ├── utils.c           # Utility functions
│   ├── utils.h           # Header file
│   ├── main.c            # Main program
│
└── Makefile              # Build configuration
```

## Data Format

### freelancers.csv
```
Freelancer ID,Name,Skills,Experience (Years)
1,John,Python Data_Science,3
```

### projects.csv
```
Project ID,Name,Required Skills,Min Exp (Years),Deadline
101,AI Chatbot,Python ML,2,5
```

### availability.csv
```
Freelancer ID,Day,Available
1,Monday,Yes
```

## Building and Running

1. Clone the repository
2. Navigate to the project directory
3. Build the project:
   ```bash
   make
   ```
4. Run the program:
   ```bash
   ./freelancer_matcher
   ```

## Cost Matrix Generation

The system generates a cost matrix based on the following factors:
- Skill mismatch (weight: 3)
- Experience mismatch (weight: 2)
- Availability mismatch (weight: 4)

Lower costs indicate better matches.

## Output

The program outputs detailed matching results showing:
- Summary statistics
- Freelancer information
- Assigned project details
- Project requirements
- Matching quality indicators
- Assignment statistics

## Performance

The system is optimized to handle large datasets:
- Supports up to 100 freelancers and 100 projects
- Each freelancer can have up to 15 skills
- Efficient Hungarian Algorithm implementation for optimal matching

## Contributing

Feel free to submit issues and enhancement requests! 