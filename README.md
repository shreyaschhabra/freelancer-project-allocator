
# 🚀 Optimized Freelancer-Project Allocation System

A comprehensive and optimized system designed to **intelligently match freelancers to projects** using the **Hungarian Algorithm**, incorporating skills, experience, and availability constraints. Built with a **high-performance C backend**, a **Node.js bridge**, and an interactive **JavaScript-based frontend**, this system ensures **efficient and fair allocation** even at scale.

---

## 🧠 Problem Statement

Organizations often struggle to assign the right people to the right projects due to varying skillsets, experience levels, and schedule constraints. This project solves that problem by:
- Creating a **cost matrix** based on real-world parameters.
- Applying the **Hungarian Algorithm** for optimal matching.
- Providing a simple interface to visualize and customize matches.

---

## 🌟 Key Features

✅ **Optimal Matching** using the Hungarian Algorithm  
📊 **Multi-Factor Cost Modeling** (Skill Mismatch, Experience Gap, Availability)  
🗂️ **CSV-Based Input/Output** for portability and ease of configuration  
⚙️ **High-Performance C Core** for real-time matching  
💻 **Frontend UI** for uploading data and viewing results  
📈 **Scalable** to 70+ freelancers/projects

---

## 🏗️ Project Architecture

```
freelancer-matching/
│
├── data/
│   ├── freelancers.csv       # Freelancer data: ID, skills, experience
│   ├── projects.csv          # Project data: required skills, experience
│   ├── availability.csv      # Weekly availability slots
│
├── backend/
│   ├── match_allocator.c     # Core matching logic using Hungarian Algorithm
│   ├── utils.c / utils.h     # File reading, matrix construction, I/O handling
│   ├── cost_matrix.csv       # Auto-generated cost matrix for verification
│
├── frontend/
│   ├── index.html            # Web interface to upload CSVs
│   ├── script.js             # Client-side logic and fetch API
│
├── server.js                 # Node.js Express server to connect UI ↔ Backend
├── Makefile                  # To compile the C code
├── README.md                 # Project documentation
```

---

## 🛠️ Technologies Used

- **C** – Efficient matching logic using Hungarian Algorithm  
- **Node.js** – Middleware server to run C executables  
- **JavaScript + HTML/CSS** – Browser-based UI  
- **CSV Format** – Portable and editable data source

---

## 📦 Installation & Usage

### Step 1: Clone the Repo

```bash
git clone https://github.com/yourusername/freelancer-matching.git
cd freelancer-matching
```

### Step 2: Install Dependencies

```bash
npm install
```

### Step 3: Compile the C Code

```bash
make
```

### Step 4: Run the Application

```bash
node server.js
```

Now open your browser and go to `http://localhost:3000`

---

## 🔢 Matching Logic Breakdown

- Each freelancer is evaluated against each project.
- A **cost score** is calculated using:
  - **Skill mismatch** penalties
  - **Experience level** gaps
  - **Availability** conflicts
- A **cost matrix** is built and passed to the Hungarian Algorithm
- The algorithm ensures **minimum total cost** while assigning one freelancer per project

---

## 📈 Example Output

| Freelancer    | Assigned Project | Cost Score |
|---------------|------------------|------------|
| Alice Verma   | Web Dashboard    | 8          |
| Rohan Singh   | AI Engine Dev    | 6          |
| Anjali Kumar  | Mobile App UI    | 5          |

---

## 🧪 Testing

- The system has been tested on **>70 freelancers and projects**.
- Various edge cases are considered including:
  - Full availability overlap
  - Multiple freelancers with the same skills
  - Projects with strict experience requirements

---

## 📚 Documentation

- 📄 **Project Report**: Full system explanation, design decisions, and performance analysis.
- 📊 **Presentation PDF**: Summary of key findings and algorithm behavior.

---

## 👨‍🎓 Author

**Shreyas Chhabra**   
IIT Patna

---

## 📄 License

This project is licensed under the MIT License.

---

## 🙌 Acknowledgements

- Hungarian Algorithm - Kuhn-Munkres method
- CSV Parsing References from C Libraries
- Node.js Child Process Integration
