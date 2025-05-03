// Global data
const matchingData = {
    freelancers: [],
    projects: [],
    assignments: []
};

// Chart instances
let skillsChart, experienceChart, matchQualityChart;

// Fetch and load the data
async function loadData(retryCount = 0, maxRetries = 3) {
    try {
        console.log('Starting data load...');
        
        // Clear existing data
        matchingData.freelancers = [];
        matchingData.projects = [];
        matchingData.assignments = [];
        
        console.log(`Fetching data from API (attempt ${retryCount + 1} of ${maxRetries})...`);
        const response = await fetch('http://localhost:8080/matches');

        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }

        const data = await response.json();
        console.log('Raw data received:', data);

        if (!data || !Array.isArray(data.matches)) {
            throw new Error('Invalid data format received from server');
        }

        // Process matches
        const validMatches = data.matches.filter(match => 
            match && 
            match.freelancer && 
            match.freelancer.id && 
            match.project && 
            match.project.id
        );

        console.log(`Found ${validMatches.length} valid matches out of ${data.matches.length} total`);

        // Store all freelancers and projects, including unmatched ones
        const freelancerMap = new Map();
        const projectMap = new Map();

        // First, add all freelancers and projects from matches
        data.matches.forEach(match => {
            if (match.freelancer) {
                freelancerMap.set(match.freelancer.id, match.freelancer);
            }
            if (match.project) {
                projectMap.set(match.project.id, match.project);
            }
        });

        // Store all entities and valid matches
        matchingData.freelancers = Array.from(freelancerMap.values());
        matchingData.projects = Array.from(projectMap.values());
        matchingData.assignments = validMatches;

        // Populate tables for visualization
        populateMatchesAndUnmatchedTables();

        console.log('Processed data:', {
            freelancers: matchingData.freelancers.length,
            projects: matchingData.projects.length,
            assignments: matchingData.assignments.length
        });

        // Initialize UI
        try {
            console.log('Initializing visualization...');
            
            console.log('Updating statistics...');
            updateStatistics();
            console.log('Populating matches list...');
            populateMatchesList();
            console.log('Initializing charts...');
            initializeCharts();
            console.log('Updating charts...');
            updateCharts();
            console.log('UI initialization complete');
        } catch (uiError) {
            console.error('Error initializing UI:', uiError);
            throw uiError;
        }

    } catch (error) {
        console.error('Error loading data:', error);
        console.error('Error details:', {
            name: error.name,
            message: error.message,
            stack: error.stack
        });

        if (retryCount < maxRetries) {
            const delay = Math.pow(2, retryCount) * 1000;
            console.log(`Retrying in ${delay/1000} seconds...`);
            setTimeout(() => loadData(retryCount + 1, maxRetries), delay);
            return;
        }

        const errorMessage = error.name === 'TypeError' ?
            'Unable to connect to the server. Please check if the backend is running.' :
            `Error: ${error.message}`;

        document.getElementById('error-message').innerHTML = `
            <h3>Error Loading Data</h3>
            <p>${errorMessage}</p>
            <p>Server Status: ${error.name === 'TypeError' ? 'Unreachable' : 'Error'}</p>
            <p>Attempted ${retryCount + 1} times</p>
            <button onclick="loadData()" class="btn primary">Try Again</button>
        `;
    }
}

// Create list item element
function createListItem(item, type) {
    if (!item || !item.id) {
        console.warn('Invalid item:', item);
        return null;
    }
    
    const div = document.createElement('div');
    div.className = `list-item ${type}`;
    div.setAttribute('data-id', item.id);
    
    const skillsHtml = item.skills ? 
        item.skills.map(skill => `<span class="skill-tag">${skill}</span>`).join('') :
        '';
    
    const experienceHtml = type === 'freelancer' ?
        `<div class="experience">Experience: ${item.experience || 0} years</div>` :
        type === 'project' ?
        `<div class="experience">Required: ${item.min_experience || 0} years</div>` :
        '';
    
    div.innerHTML = `
        <h4>${item.name || 'Unnamed'}</h4>
        <div class="skills">${skillsHtml}</div>
        ${experienceHtml}
    `;
    
    return div;
}

// Setup canvas for drawing connections
function setupCanvas() {
    const canvas = document.getElementById('connectionsCanvas');
    const ctx = canvas.getContext('2d');
    
    function resizeCanvas() {
        canvas.width = canvas.offsetWidth;
        canvas.height = canvas.offsetHeight;
    }
    
    resizeCanvas();
    window.addEventListener('resize', () => {
        resizeCanvas();
        drawConnections();
    });
}

// Draw connections between matched pairs
function drawConnections() {
    const canvas = document.getElementById('connectionsCanvas');
    const ctx = canvas.getContext('2d');
    
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    matchingData.assignments.forEach(assignment => {
        if (!assignment.freelancer || !assignment.project) return;
        
        const freelancerElement = document.querySelector(`.freelancer[data-id="${assignment.freelancer.id}"]`);
        const projectElement = document.querySelector(`.project[data-id="${assignment.project.id}"]`);
        
        if (freelancerElement && projectElement) {
            const start = getElementCenter(freelancerElement);
            const end = getElementCenter(projectElement);
            
            drawConnection(ctx, start, end, assignment.score);
        }
    });
}

// Get center coordinates of an element
function getElementCenter(element) {
    const rect = element.getBoundingClientRect();
    const canvasRect = document.getElementById('connectionsCanvas').getBoundingClientRect();
    
    return {
        x: rect.left + rect.width / 2 - canvasRect.left,
        y: rect.top + rect.height / 2 - canvasRect.top
    };
}

// Draw a single connection line
function drawConnection(ctx, start, end, score) {
    const gradient = ctx.createLinearGradient(start.x, start.y, end.x, end.y);
    const color = getScoreColor(score);
    gradient.addColorStop(0, color);
    gradient.addColorStop(1, color);
    
    ctx.beginPath();
    ctx.moveTo(start.x, start.y);
    ctx.lineTo(end.x, end.y);
    ctx.strokeStyle = gradient;
    ctx.lineWidth = 2;
    ctx.stroke();
}

// Get color based on match score
function getScoreColor(score) {
    if (score >= 80) return '#2ecc71';  // Green
    if (score >= 60) return '#3498db';  // Blue
    if (score >= 40) return '#f1c40f';  // Yellow
    return '#e74c3c';  // Red
}

// Update statistics display
function updateStatistics() {
    const actualMatches = matchingData.assignments.filter(a => 
        a.freelancer && 
        a.project && 
        a.score >= 60  // Only count matches with score >= 60%
    );

    document.getElementById('totalFreelancers').textContent = matchingData.freelancers.length;
    document.getElementById('totalProjects').textContent = matchingData.projects.length;
    document.getElementById('matchedPairs').textContent = actualMatches.length;
    
    // Success rate is based on how many projects were successfully matched
    const successRate = matchingData.projects.length > 0
        ? (actualMatches.length / Math.min(matchingData.projects.length, matchingData.freelancers.length) * 100).toFixed(1)
        : 0;
    document.getElementById('successRate').textContent = `${successRate}%`;
}

// Populate matches list
function populateMatchesList() {
    const matchesList = document.getElementById('matchesList');
    matchesList.innerHTML = '';
    
    matchingData.assignments
        .filter(assignment => assignment.freelancer && assignment.project) 
        .forEach(assignment => {
            const matchElement = createMatchElement(assignment);
            matchesList.appendChild(matchElement);
        });
}

// Create match element
function createMatchElement(assignment) {
    const div = document.createElement('div');
    div.className = 'match-item slide-in';
    
    div.innerHTML = `
        <div class="match-header">
            <h4>${assignment.freelancer.name} ↔ ${assignment.project.name}</h4>
            <div class="match-score" style="color: ${getScoreColor(assignment.score)}">
                Score: ${assignment.score}%
            </div>
        </div>
        <div class="match-details">
            <div class="skills-match">
                ${getSkillsMatchHTML(assignment.freelancer.skills, assignment.project.required_skills)}
            </div>
            <div class="experience-match">
                Required: ${assignment.project.min_experience}y | 
                Actual: ${assignment.freelancer.experience}y
            </div>
        </div>
    `;
    
    div.addEventListener('click', () => showDetails(assignment));
    return div;
}

// Get skills match HTML
function getSkillsMatchHTML(freelancerSkills, projectSkills) {
    const matchedSkills = freelancerSkills.filter(skill => projectSkills.includes(skill));
    const unmatchedSkills = projectSkills.filter(skill => !freelancerSkills.includes(skill));
    
    return `
        <div class="matched-skills">
            ${matchedSkills.map(skill => `<span class="skill-tag matched">${skill}</span>`).join('')}
        </div>
        <div class="unmatched-skills">
            ${unmatchedSkills.map(skill => `<span class="skill-tag unmatched">${skill}</span>`).join('')}
        </div>
    `;
}

// Show details in modal
function showDetails(assignment) {
    const modal = document.getElementById('detailsModal');
    const modalContent = modal.querySelector('.modal-content');
    
    modalContent.innerHTML = `
        <span class="close-button">&times;</span>
        <h2>Match Details</h2>
        <div class="modal-body">
            <div class="modal-section">
                <h3>Freelancer</h3>
                <p><strong>Name:</strong> ${assignment.freelancer.name}</p>
                <p><strong>Skills:</strong> ${assignment.freelancer.skills.join(', ')}</p>
                <p><strong>Experience:</strong> ${assignment.freelancer.experience} years</p>
            </div>
            <div class="modal-section">
                <h3>Project</h3>
                <p><strong>Name:</strong> ${assignment.project.name}</p>
                <p><strong>Required Skills:</strong> ${assignment.project.required_skills.join(', ')}</p>
                <p><strong>Minimum Experience:</strong> ${assignment.project.min_experience} years</p>
            </div>
            <div class="modal-section">
                <h3>Match Score</h3>
                <p><strong>Score:</strong> ${assignment.score}%</p>
                <div class="score-bar">
                    <div class="score-fill" style="width: ${assignment.score}%; background-color: ${getScoreColor(assignment.score)}"></div>
                </div>
            </div>
        </div>
    `;
    
    modal.style.display = 'block';
    
    // Close button handler
    modal.querySelector('.close-button').onclick = () => {
        modal.style.display = 'none';
    };
    
    // Close when clicking outside
    window.onclick = (event) => {
        if (event.target == modal) {
            modal.style.display = 'none';
        }
    };
}

// Initialize charts
function initializeCharts() {
    // Skills distribution chart
    const skillsCtx = document.getElementById('skillsChart').getContext('2d');
    skillsChart = new Chart(skillsCtx, {
        type: 'bar',
        data: {
            labels: [],
            datasets: [{
                label: 'Skills Distribution',
                data: [],
                backgroundColor: '#3498db'
            }]
        },
        options: {
            responsive: true,
            scales: {
                y: {
                    beginAtZero: true
                }
            }
        }
    });
    
    // Experience distribution chart
    const expCtx = document.getElementById('experienceChart').getContext('2d');
    experienceChart = new Chart(expCtx, {
        type: 'bar',
        data: {
            labels: [],
            datasets: [{
                label: 'Experience Distribution',
                data: [],
                backgroundColor: '#2ecc71'
            }]
        },
        options: {
            responsive: true,
            scales: {
                y: {
                    beginAtZero: true
                }
            }
        }
    });
    
    // Match quality chart
    const qualityCtx = document.getElementById('matchQualityChart').getContext('2d');
    matchQualityChart = new Chart(qualityCtx, {
        type: 'pie',
        data: {
            labels: ['Excellent (80-100%)', 'Good (60-79%)', 'Fair (40-59%)', 'Poor (<40%)'],
            datasets: [{
                data: [0, 0, 0, 0],
                backgroundColor: ['#2ecc71', '#3498db', '#f1c40f', '#e74c3c']
            }]
        },
        options: {
            responsive: true
        }
    });
}

// Update charts with current data
function updateCharts() {
    updateSkillsChart();
    updateExperienceChart();
    updateMatchQualityChart();
}

// Update skills distribution chart
function updateSkillsChart() {
    const skillCounts = {};
    
    matchingData.freelancers.forEach(freelancer => {
        freelancer.skills.forEach(skill => {
            skillCounts[skill] = (skillCounts[skill] || 0) + 1;
        });
    });
    
    skillsChart.data.labels = Object.keys(skillCounts);
    skillsChart.data.datasets[0].data = Object.values(skillCounts);
    skillsChart.update();
}

// Update experience distribution chart
function updateExperienceChart() {
    const expCounts = {};
    
    matchingData.freelancers.forEach(freelancer => {
        const exp = freelancer.experience;
        expCounts[exp] = (expCounts[exp] || 0) + 1;
    });
    
    experienceChart.data.labels = Object.keys(expCounts).map(exp => `${exp} years`);
    experienceChart.data.datasets[0].data = Object.values(expCounts);
    experienceChart.update();
}

// Update match quality chart
function updateMatchQualityChart() {
    const qualityCounts = [0, 0, 0, 0];
    
    // Count all assignments including null matches
    matchingData.assignments.forEach(assignment => {
        if (!assignment.freelancer || !assignment.project) {
            qualityCounts[3]++; // Count unmatched as poor quality
        } else if (assignment.score >= 80) {
            qualityCounts[0]++;
        } else if (assignment.score >= 60) {
            qualityCounts[1]++;
        } else if (assignment.score >= 40) {
            qualityCounts[2]++;
        } else {
            qualityCounts[3]++;
        }
    });
    
    // Also count unmatched freelancers and projects
    const unmatched = Math.abs(matchingData.freelancers.length - matchingData.projects.length);
    qualityCounts[3] += unmatched;
    
    matchQualityChart.data.datasets[0].data = qualityCounts;
    matchQualityChart.update();
}

// Visualization state
let isVisualizationRunning = false;
let animationFrame = null;

// Function to toggle visualization
function startVisualization() {
    const button = document.getElementById('startVisualization');
    if (isVisualizationRunning) {
        resetVisualization();
        button.textContent = 'Start Visualization';
        return;
    }
    button.textContent = 'Stop Visualization';
    if (isVisualizationRunning) return;
    isVisualizationRunning = true;

    // Get DOM elements
    const freelancersList = document.getElementById('freelancersList');
    const projectsList = document.getElementById('projectsList');
    const canvas = document.getElementById('connectionsCanvas');
    const ctx = canvas.getContext('2d');

    // Reset canvas
    resetVisualization();

    // Get valid matches (where both freelancer and project exist)
    const validMatches = matchingData.assignments.filter(match => 
        match.freelancer && 
        match.project && 
        match.score > 0
    );

    console.log('Starting visualization with matches:', validMatches.length);

    // Animate connections
    let currentIndex = 0;
    
    function animate() {
        if (!isVisualizationRunning || currentIndex >= validMatches.length) {
            isVisualizationRunning = false;
            animationFrame = null;
            return;
        }

        const match = validMatches[currentIndex];
        console.log('Processing match:', match);

        // Find elements
        const freelancerElement = freelancersList.querySelector(`[data-id="${match.freelancer.id}"]`);
        const projectElement = projectsList.querySelector(`[data-id="${match.project.id}"]`);

        console.log('Found elements:', {
            freelancer: freelancerElement?.getAttribute('data-id'),
            project: projectElement?.getAttribute('data-id')
        });

        if (freelancerElement && projectElement) {
            // Highlight elements
            freelancerElement.classList.add('highlight');
            projectElement.classList.add('highlight');

            // Calculate positions
            const start = getElementCenter(freelancerElement);
            const end = getElementCenter(projectElement);

            console.log('Drawing connection:', { start, end, score: match.score });

            // Draw connection
            drawConnection(ctx, start, end, match.score);

            // Schedule next animation frame
            setTimeout(() => {
                // Remove highlights
                freelancerElement.classList.remove('highlight');
                projectElement.classList.remove('highlight');
                
                // Move to next match
                currentIndex++;
                animationFrame = requestAnimationFrame(animate);
            }, 800);
        } else {
            console.log('Skipping invalid match - elements not found');
            currentIndex++;
            animationFrame = requestAnimationFrame(animate);
        }
    }

    // Start animation
    animationFrame = requestAnimationFrame(animate);
}

// Function to get the center coordinates of an element
function getElementCenter(element) {
    const rect = element.getBoundingClientRect();
    const canvasRect = document.getElementById('connectionsCanvas').getBoundingClientRect();
    return {
        x: rect.left + rect.width / 2 - canvasRect.left,
        y: rect.top + rect.height / 2 - canvasRect.top
    };
}

// Function to draw a connection between two elements
function drawConnection(ctx, start, end, score) {
    const gradient = ctx.createLinearGradient(start.x, start.y, end.x, end.y);
    const color = getScoreColor(score);
    const alpha = Math.min(score / 100, 1);
    
    gradient.addColorStop(0, color.replace('rgb', 'rgba').replace(')', `, ${alpha})`));
    gradient.addColorStop(1, color.replace('rgb', 'rgba').replace(')', `, ${alpha})`));
    
    ctx.beginPath();
    ctx.moveTo(start.x, start.y);
    ctx.lineTo(end.x, end.y);
    ctx.strokeStyle = gradient;
    ctx.lineWidth = 2;
    ctx.stroke();
}

// Function to reset visualization
function resetVisualization() {
    if (animationFrame) {
        cancelAnimationFrame(animationFrame);
    }
    isVisualizationRunning = false;

    const canvas = document.getElementById('connectionsCanvas');
    const ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // Remove all highlights
    document.querySelectorAll('.highlight').forEach(el => {
        el.classList.remove('highlight');
    });

    // Reset canvas size
    const container = canvas.parentElement;
    canvas.width = container.offsetWidth;
    canvas.height = container.offsetHeight;
}

// Populate the matched and unmatched tables
function populateMatchesAndUnmatchedTables() {
    const matchesTableBody = document.querySelector('#matchesTable tbody');
    const unmatchedTableBody = document.querySelector('#unmatchedTable tbody');
    if (!matchesTableBody || !unmatchedTableBody) return;
    // Clear tables
    matchesTableBody.innerHTML = '';
    unmatchedTableBody.innerHTML = '';

    // Sort assignments by score descending
    const sortedAssignments = [...matchingData.assignments].sort((a, b) => b.score - a.score);
    const matchedFreelancerIds = new Set();

    // Fill matched table
    sortedAssignments.forEach(assignment => {
        const freelancer = assignment.freelancer;
        const project = assignment.project;
        if (freelancer && project) {
            matchedFreelancerIds.add(freelancer.id);
            const row = document.createElement('tr');
            row.innerHTML = `
                <td>${freelancer.name}</td>
                <td>${project.name}</td>
                <td>${assignment.score}</td>
                <td>${freelancer.skills ? freelancer.skills.join(', ') : ''}</td>
                <td>${freelancer.experience}</td>
            `;
            matchesTableBody.appendChild(row);
        }
    });

    // Fill unmatched table
    matchingData.freelancers.forEach(freelancer => {
        if (!matchedFreelancerIds.has(freelancer.id)) {
            const row = document.createElement('tr');
            row.innerHTML = `
                <td>${freelancer.name}</td>
                <td>${freelancer.skills ? freelancer.skills.join(', ') : ''}</td>
                <td>${freelancer.experience}</td>
            `;
            unmatchedTableBody.appendChild(row);
        }
    });
}

// Event listeners
document.addEventListener('DOMContentLoaded', () => {
    loadData();

    // Bloom filter skill check UI logic
    const bloomSkillInput = document.getElementById('bloomSkillInput');
    const bloomSkillCheckBtn = document.getElementById('bloomSkillCheckBtn');
    const bloomSkillResult = document.getElementById('bloomSkillResult');
    const bloomSkillTable = document.getElementById('bloomSkillTable');
    const bloomSkillTableBody = bloomSkillTable ? bloomSkillTable.querySelector('tbody') : null;
    if (bloomSkillCheckBtn) {
        bloomSkillCheckBtn.addEventListener('click', async () => {
            const skill = bloomSkillInput.value.trim();
            bloomSkillResult.textContent = '';
            if (bloomSkillTable) bloomSkillTable.style.display = 'none';
            if (!skill) {
                bloomSkillResult.textContent = 'Please enter a skill.';
                return;
            }
            bloomSkillResult.textContent = 'Searching...';
            try {
                const res = await fetch(`http://localhost:8080/freelancers_with_skill?skill=${encodeURIComponent(skill)}`);
                if (!res.ok) throw new Error('Network error');
                const freelancers = await res.json();
                if (Array.isArray(freelancers) && freelancers.length > 0) {
                    bloomSkillResult.textContent = `Found ${freelancers.length} freelancer(s) with skill "${skill}".`;
                    bloomSkillResult.style.color = 'green';
                    if (bloomSkillTableBody) {
                        bloomSkillTableBody.innerHTML = '';
                        freelancers.forEach(f => {
                            const row = document.createElement('tr');
                            row.innerHTML = `<td>${f.id}</td><td>${f.name}</td><td>${f.experience}</td><td>${f.skills.join(', ')}</td>`;
                            bloomSkillTableBody.appendChild(row);
                        });
                        bloomSkillTable.style.display = '';
                    }
                } else {
                    bloomSkillResult.textContent = `No freelancers found with skill "${skill}".`;
                    bloomSkillResult.style.color = 'red';
                    if (bloomSkillTableBody) bloomSkillTableBody.innerHTML = '';
                    if (bloomSkillTable) bloomSkillTable.style.display = 'none';
                }
            } catch (err) {
                bloomSkillResult.textContent = 'Error searching for skill.';
                bloomSkillResult.style.color = 'orange';
                if (bloomSkillTableBody) bloomSkillTableBody.innerHTML = '';
                if (bloomSkillTable) bloomSkillTable.style.display = 'none';
            }
        });
    }
    
    // Visualization controls
    document.getElementById('startVisualization').addEventListener('click', startVisualization);
    document.getElementById('resetVisualization').addEventListener('click', resetVisualization);
    
    // Search functionality
    document.getElementById('searchInput').addEventListener('input', (e) => {
        const searchTerm = e.target.value.toLowerCase();
        const matches = document.querySelectorAll('.match-item');
        
        matches.forEach(match => {
            const text = match.textContent.toLowerCase();
            match.style.display = text.includes(searchTerm) ? 'block' : 'none';
        });
    });
    
    // Filter functionality
    document.getElementById('filterType').addEventListener('change', (e) => {
        const filter = e.target.value;
        const matches = document.querySelectorAll('.match-item');
        
        matches.forEach(match => {
            const score = parseInt(match.querySelector('.match-score').textContent.split(':')[1]);
            
            switch (filter) {
                case 'matched':
                    match.style.display = score >= 80 ? 'block' : 'none';
                    break;
                case 'unmatched':
                    match.style.display = score < 80 ? 'block' : 'none';
                    break;
                default:
                    match.style.display = 'block';
            }
        });
    });

    // Handle window resize
    window.addEventListener('resize', () => {
        if (isVisualizationRunning) {
            resetVisualization();
            startVisualization();
        }
    });
}); 