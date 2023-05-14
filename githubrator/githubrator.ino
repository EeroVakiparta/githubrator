#include <WiFi.h>
#include <HTTPClient.h>
#include <algorithm>

const char* ssid = "SSID";
const char* password = "salasana";

struct Contribution {
  int score;
  String date;
};

const int motorPin = 12;
const int vibrationInterval = 50;
const int minimumIntensity = 120;
const int vibrationDuration = 500;
// TODO: figure out a good cap, search average daily contributions for a year?
// Or introduce new vibration patterns like pulse and wave when reaching higher daily contributions.
const int contributionCap = 10; 

std::vector<Contribution> parseSvgResponse(String svgResponse);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  pinMode(motorPin, OUTPUT);
  performSvgRequest();
}

void loop() {}

void performSvgRequest() {
  HTTPClient http;
  String url = "https://ghchart.rshah.org/EeroVakiparta";
  http.begin(url);
  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    String response = http.getString();
    Serial.println("Received SVG response:");
    std::vector<Contribution> contributions = parseSvgResponse(response);
    Serial.println("Parsed Contribution Data:");
    for (const auto& contribution : contributions) {
      Serial.print("Contribution Score: ");
      Serial.print(contribution.score);
      Serial.print(", Contribution Date: ");
      Serial.println(contribution.date);
    }
    for (const auto& contribution : contributions) {
      int intensity = mapContributionToMotorIntensity(contribution.score, minimumIntensity, contributionCap);
      vibrateMotor(intensity, vibrationInterval, vibrationDuration);
    }
  } else {
    Serial.print("Error - HTTP response code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

std::vector<Contribution> parseSvgResponse(String svgResponse) {
  std::vector<Contribution> contributions;
  int startIndex = svgResponse.indexOf("data-score=\"");
  while (startIndex != -1) {
    int endIndex = svgResponse.indexOf("\"", startIndex + 12);
    if (endIndex != -1) {
      String scoreStr = svgResponse.substring(startIndex + 12, endIndex);
      int score = scoreStr.toInt();
      startIndex = svgResponse.indexOf("data-date=\"", endIndex);
      endIndex = svgResponse.indexOf("\"", startIndex + 11);
      if (endIndex != -1) {
        String date = svgResponse.substring(startIndex + 11, endIndex);
        contributions.push_back({score, date});
      }
    }
    startIndex = svgResponse.indexOf("data-score=\"", endIndex);
  }
  std::sort(contributions.begin(), contributions.end(),
            [](const Contribution& a, const Contribution& b) {
                return a.date < b.date;
            });
  return contributions;
}

int mapContributionToMotorIntensity(int contribution, int minIntensity, int cap) {
  if (contribution >= cap) {
    return 255;
  } else {
    return map(contribution, 1, cap, minIntensity, 255);
  }
}

void vibrateMotor(int intensity, int interval, int duration) {
  Serial.print("Vibration intensity: ");
  Serial.println(intensity);
  analogWrite(motorPin, intensity);
  delay(duration);
  analogWrite(motorPin, 0);
  delay(interval);
}
