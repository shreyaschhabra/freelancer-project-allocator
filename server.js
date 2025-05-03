const express = require('express');
const path = require('path');
const fs = require('fs').promises;
const http = require('http');
const app = express();
const port = 3000;

// Serve static files from frontend directory
app.use(express.static('frontend'));

// Function to parse CSV to JSON
function csvToJson(csv) {
    const lines = csv.split('\n');
    const headers = lines[0].split(',');
    const result = [];
    
    for (let i = 1; i < lines.length; i++) {
        if (!lines[i].trim()) continue;
        const obj = {};
        const currentline = lines[i].split(',');
        
        for (let j = 0; j < headers.length; j++) {
            let value = currentline[j];
            // Handle skills as array
            if (headers[j] === 'skills' && value) {
                value = value.split(' ');
            }
            obj[headers[j].trim()] = value;
        }
        result.push(obj);
    }
    return result;
}

// Serve data files
app.get('/data/:file', async (req, res) => {
    try {
        const filePath = path.join(__dirname, 'data', req.params.file);
        const data = await fs.readFile(filePath, 'utf8');
        const jsonData = csvToJson(data);
        res.json(jsonData);
    } catch (error) {
        console.error('Error loading data:', error);
        res.status(404).send('File not found');
    }
});

// Proxy requests to the C backend
app.get('/matches', (req, res) => {
    const options = {
        hostname: 'localhost',
        port: 8080,
        path: '/matches',
        method: 'GET',
        headers: {
            'Accept': 'application/json',
            'Content-Type': 'application/json'
        }
    };

    const proxyReq = http.request(options, (proxyRes) => {
        let data = '';
        proxyRes.on('data', (chunk) => {
            data += chunk;
        });
        proxyRes.on('end', () => {
            res.setHeader('Content-Type', 'application/json');
            res.setHeader('Access-Control-Allow-Origin', '*');
            res.send(data);
        });
    });

    proxyReq.on('error', (error) => {
        console.error('Error proxying request:', error);
        res.status(500).json({ error: 'Internal server error' });
    });

    proxyReq.end();
});

// Serve the main page
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'frontend', 'index.html'));
});

// Start the server
app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
}); 