document.addEventListener('DOMContentLoaded', () => {
    // --- Data Section ---
    // Store your provided results from the SECOND table ("cliques Density")
    // Structure: resultsData[datasetKey][patternKey] = { exact: value, coreExact: value };
    // IMPORTANT: Using data directly from user's second table.
    const resultsData = {
        yeast: {
            edge: { exact: 1.0, coreExact: 2.5 },
            triangle: { exact: 0.0, coreExact: 3.33 },
            '4-clique': { exact: 0.0, coreExact: 2.5 },
            '5-clique': { exact: 1.0, coreExact: 1.0 }, // Note: Your table showed 1 vs 1 here
            '6-clique': { exact: 0.166, coreExact: 0.166 }
        },
        netscience: {
            edge: { exact: 1.72, coreExact: 9.50 },
            triangle: { exact: 57.0, coreExact: 57.0 },
            '4-clique': { exact: 242.5, coreExact: 242.5 },
            '5-clique': { exact: 775.2, coreExact: 775.2 },
            '6-clique': { exact: 1938.0, coreExact: 1938.0 }
        },
        'as-caida': { // Use quotes for key with hyphen
            edge: { exact: 1.0, coreExact: 15.5 },
            triangle: { exact: 155.0, coreExact: 145.0 },
            '4-clique': { exact: 1123.75, coreExact: 1123.75 },
            '5-clique': { exact: 6293.0, coreExact: 6293.0 },
            '6-clique': { exact: 1938.0, coreExact: 28318.5 } // Very large difference here
        }
    };

    // Map pattern select values to display names
    const patternDisplayNames = {
         edge: "Edge (h=2)",
         triangle: "Triangle (h=3)",
        '4-clique': "4-Clique (h=4)",
        '5-clique': "5-Clique (h=5)",
        '6-clique': "6-Clique (h=6)"
    };


    // --- DOM Elements ---
    const datasetButtons = document.querySelectorAll('.tab-button');
    const patternSelect = document.getElementById('pattern-select');
    const resultsTitle = document.getElementById('results-title');
    const resultExactDensity = document.getElementById('result-exact-density');
    const resultCoreExactDensity = document.getElementById('result-coreexact-density');

    let currentDataset = 'yeast'; // Keep track of selected dataset
    let currentPattern = 'edge'; // Keep track of selected pattern

    // --- Functions ---
    function displayResults() {
        const datasetInfo = resultsData[currentDataset];
        const patternInfo = datasetInfo ? datasetInfo[currentPattern] : null;

        // Update Title
        const datasetName = currentDataset.charAt(0).toUpperCase() + currentDataset.slice(1).replace('-',' ');
        resultsTitle.textContent = `Results for ${datasetName} Dataset (${patternDisplayNames[currentPattern]})`;

        // Update Display Card
        if (patternInfo) {
            resultExactDensity.textContent = patternInfo.exact !== undefined ? patternInfo.exact.toFixed(3) : "N/A";
            resultCoreExactDensity.textContent = patternInfo.coreExact !== undefined ? patternInfo.coreExact.toFixed(3) : "N/A";
        } else {
            console.warn(`Data not found for ${currentDataset} - ${currentPattern}`);
            resultExactDensity.textContent = "N/A";
            resultCoreExactDensity.textContent = "N/A";
        }

        // Update Active Button State (redundant if event handler does it, but safe)
        datasetButtons.forEach(button => {
             button.classList.toggle('active', button.dataset.dataset === currentDataset);
        });
        // Ensure select matches currentPattern (redundant if event handler does it, but safe)
        patternSelect.value = currentPattern;

    }

    // --- Event Listeners ---
    datasetButtons.forEach(button => {
        button.addEventListener('click', () => {
            currentDataset = button.dataset.dataset; // Update state
            // Update active class immediately
            datasetButtons.forEach(btn => btn.classList.remove('active'));
            button.classList.add('active');
            displayResults(); // Refresh display
        });
    });

    patternSelect.addEventListener('change', (event) => {
        currentPattern = event.target.value; // Update state
        displayResults(); // Refresh display
    });

    // --- Initial Load ---
    displayResults(); // Load default data (Yeast, Edge)

    // --- Initialize Particles.js ---
     // (Include the particlesJS initialization code from the previous answer here)
      particlesJS("particles-js", {
      "particles": {
        "number": {
          "value": 80, // Number of particles
          "density": {
            "enable": true,
            "value_area": 800
          }
        },
        "color": {
          "value": "#00aaff" // Particle color
        },
        "shape": {
          "type": "circle", // Shape type
        },
        "opacity": {
          "value": 0.5, // Particle opacity
          "random": false,
        },
        "size": {
          "value": 3, // Particle size
          "random": true,
        },
        "line_linked": {
          "enable": true,
          "distance": 150,
          "color": "#00aaff", // Line color
          "opacity": 0.4,
          "width": 1
        },
        "move": {
          "enable": true,
          "speed": 3, // Movement speed
          "direction": "none",
          "random": false,
          "straight": false,
          "out_mode": "out",
          "bounce": false,
        }
      },
      "interactivity": {
        "detect_on": "canvas",
        "events": {
          "onhover": {
            "enable": true,
            "mode": "repulse" // Interaction mode on hover
          },
          "onclick": {
            "enable": true,
            "mode": "push" // Interaction mode on click
          },
          "resize": true
        },
        "modes": {
          "repulse": {
            "distance": 100,
            "duration": 0.4
          },
          "push": {
            "particles_nb": 4
          },
        }
      },
      "retina_detect": true
    });


});