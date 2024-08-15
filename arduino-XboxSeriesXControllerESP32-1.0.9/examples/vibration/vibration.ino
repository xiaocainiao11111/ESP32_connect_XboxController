#include <XboxSeriesXControllerESP32_asukiaaa.hpp>

// Required to replace with your xbox address
// XboxSeriesXControllerESP32_asukiaaa::Core
// xboxController("44:16:22:5e:b2:d4");

// any xbox controller
XboxSeriesXControllerESP32_asukiaaa::Core xboxController;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting NimBLE Client");
  xboxController.begin();
}

void demoVibration() {
  XboxSeriesXHIDReportBuilder_asukiaaa::ReportBase repo;
  Serial.println("full power for 1 sec");
  xboxController.writeHIDReport(repo);
  delay(2000);

  repo.v.select.center = true;
  repo.v.select.left = false;
  repo.v.select.right = false;
  repo.v.select.shake = false;
  repo.v.power.center = 30;  // 30% power
  repo.v.timeActive = 50;    // 0.5 second
  Serial.println("run center 30\% power in half second");
  xboxController.writeHIDReport(repo);
  delay(2000);

  repo.v.select.center = false;
  repo.v.select.left = true;
  repo.v.power.left = 30;
  Serial.println("run left 30\% power in half second");
  xboxController.writeHIDReport(repo);
  delay(2000);

  repo.v.select.left = false;
  repo.v.select.right = true;
  repo.v.power.right = 30;
  Serial.println("run right 30\% power in half second");
  xboxController.writeHIDReport(repo);
  delay(2000);

  repo.v.select.right = false;
  repo.v.select.shake = true;
  repo.v.power.shake = 30;
  Serial.println("run shake 30\% power in half second");
  xboxController.writeHIDReport(repo);
  delay(2000);

  repo.v.select.shake = false;
  repo.v.select.center = true;
  repo.v.power.center = 50;
  repo.v.timeActive = 20;
  repo.v.timeSilent = 20;
  repo.v.countRepeat = 2;
  Serial.println("run center 50\% power in 0.2 sec 3 times");
  xboxController.writeHIDReport(repo);
  delay(2000);
}

void loop() {
  xboxController.onLoop();
  if (xboxController.isConnected()) {
    if (xboxController.isWaitingForFirstNotification()) {
      Serial.println("waiting for first notification");
    } else {
      demoVibration();
    }
  } else {
    Serial.println("not connected");
    if (xboxController.getCountFailedConnection() > 2) {
      ESP.restart();
    }
  }
  Serial.println("at " + String(millis()));
  delay(500);
}
