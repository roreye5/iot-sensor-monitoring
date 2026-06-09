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

    const formElement = document.querySelector('#data-form');
    const endTimeInput = document.querySelector('#end-time');
    let tempChart = null;
    let humidChart = null;

    if (endTimeInput) {
        const nowLocal = new Date();
        endTimeInput.value = nowLocal.toISOString().slice(0, 16);
    }

    function fetchSensorData(userId, location, startTimeUtc, endTimeUtc, sensorType) {
        const url = `/api/sensor-data?user_id=${userId}&location=${encodeURIComponent(location)}&start_time=${encodeURIComponent(startTimeUtc)}&end_time=${encodeURIComponent(endTimeUtc)}&sensor_type=${encodeURIComponent(sensorType)}`;
        return fetch(url)
          .then((response) => {
              if (!response.ok) {
                  throw new Error(`HTTP Error: ${response.status}`);
              }
              return response.json();
          })
          .then((responseData) => {
              return (responseData['results'] || []).map((dataPoint) => {
                  return Object.assign({}, dataPoint, {
                      time: new Date(dataPoint.time)
                  });
              });
          });
    }

    function buildLineChart(ctx, label, chartData, yAxisLabel, color) {
        if (ctx.chartInstance) {
            ctx.chartInstance.destroy();
        }

        ctx.chartInstance = new Chart(ctx, {
            type: 'line',
            data: {
                datasets: [{
                    label: label,
                    data: chartData,
                    borderColor: color,
                    backgroundColor: color,
                    fill: false,
                    tension: 0.25,
                    pointRadius: 4,
                    pointHoverRadius: 8,
                    pointHitRadius: 10,
                    hoverRadius: 8
                }]
            },
            options: {
                interaction: {
                    mode: 'nearest',
                    intersect: true
                },
                plugins: {
                    tooltip: {
                        enabled: true,
                        mode: 'nearest',
                        intersect: true,
                        callbacks: {
                            label: function(context) {
                                const value = context.parsed.y;
                                const time = context.parsed.x ? new Date(context.parsed.x).toLocaleString() : '';
                                return `${context.dataset.label}: ${value} at ${time}`;
                            }
                        }
                    }
                },
                onClick: (event, elements, chart) => {
                    if (elements.length > 0) {
                        const element = elements[0];
                        const dataset = chart.data.datasets[element.datasetIndex];
                        const pointData = dataset.data[element.index];
                        const pointTime = pointData.x ? new Date(pointData.x).toLocaleString() : '';
                        const pointValue = pointData.y;
                        alert(`${dataset.label}: ${pointValue} at ${pointTime}`);
                    }
                },
                scales: {
                    x: {
                        type: 'time',
                        time: {
                            unit: 'minute',
                            displayFormats: {
                                second: 'h:mm a',
                                minute: 'h:mm a',
                                hour: 'h:mm a',
                                day: 'MMM D',
                                month: 'MMM YYYY'
                            }
                        },
                        title: {
                            display: true,
                            text: 'Time'
                        },
                        ticks: {
                            autoSkip: true,
                            maxTicksLimit: 10
                        }
                    },
                    y: {
                        title: {
                            display: true,
                            text: yAxisLabel
                        }
                    }
                }
            }
        });

        return ctx.chartInstance;
    }

    // Event listener for form submission
    formElement.addEventListener('submit', (event) => {
        event.preventDefault();
        const data = Object.fromEntries(new FormData(formElement).entries());
        const location = data['location'];
        const startTime = data['start-time'];
        const endTime = data['end-time'];
        const userId = data['user-id'];

        if (!startTime || !endTime) {
            alert('Please select both start and end times.');
            return;
        }

        const startTimeUtc = new Date(startTime).toISOString();
        const endTimeUtc = new Date(endTime).toISOString();

        Promise.all([
            fetchSensorData(userId, location, startTimeUtc, endTimeUtc, 'temperature'),
            fetchSensorData(userId, location, startTimeUtc, endTimeUtc, 'humidity')
        ])
        .then(([tempResults, humidResults]) => {
            if (!tempResults.length && !humidResults.length) {
                alert('No temperature or humidity data found for the selected range.');
                return;
            }

            const tempData = tempResults.map((dataPoint) => ({ x: dataPoint.time, y: dataPoint.value }));
            const humidData = humidResults.map((dataPoint) => ({ x: dataPoint.time, y: dataPoint.value }));

            const tempCtx = document.getElementById('tempChart').getContext('2d');
            const humidCtx = document.getElementById('humidChart').getContext('2d');

            tempChart = buildLineChart(tempCtx, 'Temperature', tempData, tempResults[0]?.unit || 'Temperature', 'rgb(255, 99, 132)');
            humidChart = buildLineChart(humidCtx, 'Humidity', humidData, humidResults[0]?.unit || 'Humidity', 'rgb(54, 162, 235)');
        })
        .catch((error) => {
            console.error(error);
            alert('Graph Error');
        });
    });


});