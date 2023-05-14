const int motorPin = 12;
const int vibrationInterval = 500;
const int dailyContributions[] = {0, 2, 3, 2, 4, 2, 40, 2, 3, 2, 1, 0, 4, 7, 9, 10, 4, 0, 8, 0, 10, 0};
const int contributionsCount = sizeof(dailyContributions) / sizeof(dailyContributions[0]);
const int minimumIntensity = 100;
const int vibrationDuration = 500;
const int contributionCap = 10;

void setup() {
  Serial.begin(115200);
  pinMode(motorPin, OUTPUT);
}

void loop() {
  int* motorIntensities = mapContributionsToMotorIntensity(dailyContributions, contributionsCount, minimumIntensity, contributionCap);

  for (int i = 0; i < contributionsCount; i++) {
    vibrateMotor(motorIntensities[i], vibrationInterval, vibrationDuration);
  }

  delete[] motorIntensities;
}

int* mapContributionsToMotorIntensity(const int* contributions, int count, int minIntensity, int cap) {
  int* motorIntensities = new int[count];

  for (int i = 0; i < count; i++) {
    if (contributions[i] >= cap) {
      motorIntensities[i] = 255;
    } else {
      motorIntensities[i] = map(contributions[i], 1, cap, minIntensity, 255);
    }
  }
  
  return motorIntensities;
}

void vibrateMotor(int intensity, int interval, int duration) {
  Serial.print("Vibration intensity: ");
  Serial.println(intensity);
  analogWrite(motorPin, intensity);
  delay(duration);
  analogWrite(motorPin, 0);
  delay(interval - duration);
}
