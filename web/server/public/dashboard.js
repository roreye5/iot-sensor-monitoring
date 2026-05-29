var scatterChart = null;

document.addEventListener("DOMContentLoaded", () => {

    //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
    // Define the 'request' function to handle interactions with the server
    function server_request(url, data={}, verb, callback) {
    return fetch(url, {
        credentials: 'same-origin',
        method: verb,
        body: JSON.stringify(data),
        headers: {'Content-Type': 'application/json'}
    })
    .then(response => response.json())
    .then(response => {
        if(callback)
        callback(response);
    })
    .catch(error => console.error('Error:', error));
    }



    // Handle logout POST request
    document.querySelector('.logout_button').addEventListener('click', (event) => {
        const confirmLogout = confirm("Are you sure you want to logout?");
        if (confirmLogout) {
            server_request('/logout', {}, 'POST', (response) => {
                if (response.session_id == 0) {
                    location.replace('/login');
                }
            });
        }

    });

    // Create graph

    // Get form to get form data
    const form = document.querySelectorAll("form");

    // Event listener for form submission
    form[0].addEventListener("submit", (event) => {
        event.preventDefault();
        // Get data from the form and send it to the server
        const data = Object.fromEntries(new FormData(form[0]).entries());
        var location = data['location'];
        var sensorType = data['sensor-type'];
        var endTime = data['end-time'];
        var startTime = data['start-time'];

        // Get data from the menu item from the form and get its cost
        fetch(`https://ece140.frosty-sky-f43d.workers.dev/api/query?auth=A16359906&sensorType=${sensorType}`)
        .then((response) => {
            if (!response.ok) {
                throw new Error(`HTTP Error: ${response.status}`);
            }
            return response.json();
            })
        .then((data) => {
            // Filter data by location
            var locationdata = data['results'].filter(function(dataPoint) {
                return dataPoint.location == location;
            });
            // Filter data by start time
            var begindata = locationdata.filter(function(dataPoint) {
                return dataPoint.time > startTime;
            });

            // Filter data by end time
            var enddata = begindata.filter(function(dataPoint) {
                return dataPoint.time < endTime;
            });

            // Array to store extracted values
            var chartValues = [];
            var chartTimes = [];

            // Iterate through the properties of the object
            for (var entry in enddata) {
                for (var key in enddata[entry]) {
                    // Extract values
                    if (key === "value") {
                        // Push the value into the extracted values array
                        chartValues.push(enddata[entry][key]);
                    }
                    // Extract times
                    if (key === "time") {
                        // Push the value into the extracted values array
                        // chartTimes.push(new Date(enddata[entry][key]));
                        chartTimes.push(enddata[entry][key]);
                    }
                }
            }

            // // Get the canvas element
            var ctx = document.getElementById('myChart').getContext('2d');

            // Check if the chart instance exists
            if (typeof scatterChart !== 'undefined' && scatterChart !== null) {
                // If it exists, destroy the chart
                scatterChart.destroy();
            }

            // Combine time and value data into an array of objects
            var scatterData = [];
            for (var i = 0; i < chartTimes.length; i++) {
                scatterData.push({ x: chartTimes[i], y: chartValues[i] });
            }
            
            // Create the scatter plot
            scatterChart = new Chart(ctx, {
                type: 'scatter',
                data: {
                    datasets: [{
                        label: 'Requested Data',
                        data: scatterData,
                        backgroundColor: 'rgba(255, 99, 132, 0.5)' // Color of the data points
                    }]
                },
                options: {
                    scales: {
                        x: {
                            type: 'time', // Use time scale for x-axis
                            time: {
                                displayFormats: {
                                    hour: 'MMM D, h:mm a' // Format for displaying hours and minutes
                                }
                            },
                            title: {
                                display: true,
                                text: 'Time' // X-axis label
                            }
                        },
                        y: {
                            title: {
                                display: true,
                                text: enddata[0]['unit'] // Y-axis label
                            }
                        }
                    }
                }
            });

        })
        .catch((error) => {
            console.error(error);
            alert('Graph Error');
        });
    });


});