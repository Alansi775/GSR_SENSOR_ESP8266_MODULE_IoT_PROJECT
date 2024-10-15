#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// const char* ssid = "HUAWEI-100A7N";  // Your WiFi SSID
// const char* password = "Trabzon6167";  // Your WiFi password

//HOME WIFI AND PASSWORD
const char* ssid = "SUPERONLINE_Wi-Fi_1075";  // Your WiFi SSID
const char* password = "R92GxdyDScyC";  // Your WiFi password

//SAF WIFI AND PASSWORD
// const char* ssid = "FiberHGW_ZTKE59_2.4GHz";
// const char* password = "CsehqJzeg7";
c:\Users\aymnb\Documents\Arduino\ESP8266_LCD_VOICERECOGNITION\ESP8266_VOICERECOGNITION_EN.ino

ESP8266WebServer server(80);  // Web server on port 80

float previousConductance = 0;  // Store previous conductance value
String comment = "Waiting for data...";
const int maxDataPoints = 20;  // Limit the number of data points in the chart
float conductanceData[maxDataPoints] = { 0 };
int dataIndex = 0;

String htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>GSR Sensor Readings Done By Mohammed Saleh</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    body { background-color: #95D2B3; color: #FDFFE2; text-align: center; }
    canvas { width: 100%; max-width: 600px; }
  </style>
</head>
<body>
  <h2>GSR Conductance Level</h2>
  <p id="conductance">Waiting for data...</p>
  <p id="comment">Comment: Waiting for data...</p>
  <canvas id="conductanceChart"></canvas>
  <audio id="alertSound" src="data:audio/wav;base64,UklGRhQAAABXQVZFZm10IBAAAAABAAEAAQAAACAAAAAA..."></audio> <!-- Replace with actual sound data -->
  <script>
    const ctx = document.getElementById('conductanceChart').getContext('2d');
    const conductanceChart = new Chart(ctx, {
      type: 'line',
      data: {
        labels: [],
        datasets: [{
          label: 'Conductance (µS)',
          data: [],
          borderColor: 'rgba(75, 192, 192, 1)',
          borderWidth: 1,
          fill: false
        }]
      },
      options: {
        scales: {
          x: {
            type: 'linear',
            position: 'bottom',
            title: {
              display: true,
              text: 'Time (s)'
            }
          },
          y: {
            beginAtZero: true,
            title: {
              display: true,
              text: 'Conductance (µS)'
            }
          }
        }
      }
    });

    let timeIndex = 0; // Initialize time index
    let previousValue = null; // Track the previous conductance value

    setInterval(() => {
      fetch('/conductance')
        .then(response => response.json())
        .then(data => {
          document.getElementById('conductance').innerText = "Conductance: " + data.value;
          document.getElementById('comment').innerText = "Comment: " + data.comment;

          // Update chart data
          if (conductanceChart.data.labels.length >= 20) {
            conductanceChart.data.labels.shift(); // Remove the first label
            conductanceChart.data.datasets[0].data.shift(); // Remove the first data point
          }
          conductanceChart.data.labels.push(timeIndex++); // Increment time index
          conductanceChart.data.datasets[0].data.push(data.value); // Add new data point

          // Change line color based on stress level
          if (data.value > 50.0) {
            conductanceChart.data.datasets[0].borderColor = 'rgba(255, 99, 132, 1)'; // Red for high stress
          } else if (data.value > 30.0) {
            conductanceChart.data.datasets[0].borderColor = 'rgba(255, 206, 86, 1)'; // Yellow for mid stress
          } else {
            conductanceChart.data.datasets[0].borderColor = 'rgba(75, 192, 192, 1)'; // Green for normal
          }

          // Play alert sound if value changes and crosses thresholds
          if (previousValue !== null && previousValue !== data.value) {
            if (data.value > 50.0 || data.value < 30.0) {
              document.getElementById('alertSound').play(); // Play sound alert on significant change
            }
          }

          previousValue = data.value; // Update previous value
          conductanceChart.update(); // Update the chart
        });
    }, 1000);
  </script>
</body>
</html>
)rawliteral";


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  server.on("/", []() {
    server.send(200, "text/html", htmlPage);
  });

  server.on("/conductance", []() {
    int sensorValue = analogRead(A0);  // Read GSR sensor from A0
    float resistance = ((1024.0 + 2 * sensorValue - 1) * 10000.0) / (512.0 - sensorValue);
    float conductance = (resistance != 0) ? (1 / resistance) * 1e6 : 0;

    // Determine comment based on conductance value
    String comment;
    if (conductance == 29.4 || conductance == 29.53) {
    comment = "normal";
} else if (conductance > 30.0 && conductance < 43.0) {
    comment = "Relaxed";
} else if (conductance >= 43.0 && conductance < 47.0) {
    comment = "Mid Stressed";
} else if (conductance >= 50.0) {
    comment = "Stressed";
} else {
    comment = "No Connection";
}


    // Prepare JSON response
    String jsonResponse = "{";
    jsonResponse += "\"value\":" + String(conductance) + ",";
    jsonResponse += "\"comment\":\"" + comment + "\"";
    jsonResponse += "}";

    server.send(200, "application/json", jsonResponse);
  });

  server.begin();
  Serial.println("HTTP server started");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
}