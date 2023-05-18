#include <WiFi.h>
#include <HTTPClient.h>
#include <algorithm>
#include <WiFiManager.h>

struct Contribution {
  int score;
  String date;
};

// PWM
const int motorPin = 12;
const int channel = 0;
const int resolution = 8;

String username;

// Time between vibrations (in milliseconds)
const int vibrationInterval = 5;

// Minimum intensity for the motor
// To make sure the motor is always vibrating a bit, the minimum intensity should be
const int minimumIntensity = 120;

// Duration of vibration (in milliseconds)
const int vibrationDuration = 500;

// TODO: figure out a good cap, search average daily contributions for a year?
// Or introduce new vibration patterns like pulse and wave when reaching higher daily contributions.
const int contributionCap = 4; 

std::vector<Contribution> contributions;  // Global variable to hold contributions

std::vector<Contribution> parseSvgResponse(String svgResponse);

void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);

  WiFiManager wm;
  
  //wm.setDebugOutput(false); // comment out during development
  wm.resetSettings(); // comment out during production
  WiFiManagerParameter contributorName("contributor","Enter contributor username", "torvalds", 50);
  wm.addParameter(&contributorName);
  bool res;
  res = wm.autoConnect("GitHubrator");

  if(!res){
    Serial.println("Failed to conect");
  }else{
    Serial.println("Connected...");
  }
  ledcSetup(channel, 5000, resolution);
  ledcAttachPin(motorPin, channel);
  
  username = contributorName.getValue();
  Serial.println(username);
  performSvgRequest();
}

void loop() {
  // Cycle through contributions continuously
  for (const auto& contribution : contributions) {
    int intensity = mapContributionToMotorIntensity(contribution.score, minimumIntensity, contributionCap);
    vibrateMotor(intensity, vibrationInterval, vibrationDuration);
  }
}

void performSvgRequest() {
  HTTPClient http;
  //TODO: change to your own username
  String url = "https://ghchart.rshah.org/" + username;
  http.begin(url);
  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    String response = http.getString();
    Serial.println("Received SVG response:");
    contributions = parseSvgResponse(response);  // Store contributions in global variable
    Serial.println("Parsed Contribution Data:");
    for (const auto& contribution : contributions) {
      Serial.print("Contribution Score: ");
      Serial.print(contribution.score);
      Serial.print(", Contribution Date: ");
      Serial.println(contribution.date);
    }
  } else {
    Serial.print("Error - HTTP response code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

std::vector<Contribution> parseSvgResponse(String svgResponse) {
  std::vector<Contribution> contributions;

  Serial.println("Parsing SVG Response:");

  int startIndex = svgResponse.indexOf("data-score=\"");
  while (startIndex != -1) {
    int endIndex = svgResponse.indexOf("\"", startIndex + 12);
    if (endIndex != -1) {
      String scoreStr = svgResponse.substring(startIndex + 12, endIndex);
      int score = scoreStr.toInt();

      Serial.print("Score: ");
      Serial.println(score);

      startIndex = svgResponse.indexOf("data-date=\"", endIndex);
      endIndex = svgResponse.indexOf("\"", startIndex + 11);
      if (endIndex != -1) {
        String date = svgResponse.substring(startIndex + 11, endIndex);

        Serial.print("Date: ");
        Serial.println(date);

        contributions.push_back({score, date});
      }
    }
    startIndex = svgResponse.indexOf("data-score=\"", endIndex);
  }

  // Sort contributions by date
   std::sort(contributions.begin(), contributions.end(),
            [](const Contribution& a, const Contribution& b) {
                return a.date < b.date;
            });

  return contributions;
}

int mapContributionToMotorIntensity(int contribution, int minIntensity, int cap) {
  if (contribution >= cap) {
    return 255;  // Maximum intensity for contributions at or above the cap
  } else {
    return map(contribution, 1, cap, minIntensity, 255);
  }
}

void vibrateMotor(int intensity, int interval, int duration) {
  Serial.print("Vibration intensity: ");
  Serial.println(intensity);
  ledcWrite(channel, intensity);
  delay(duration);
  ledcWrite(channel, 0);
  delay(interval);
}
