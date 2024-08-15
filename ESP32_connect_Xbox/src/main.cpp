#include <Arduino.h>
#include <XboxSeriesXControllerESP32_asukiaaa.hpp>

// Required to replace with your xbox address
// 需要在此替换成自己的手柄蓝牙MAC地址
XboxSeriesXControllerESP32_asukiaaa::Core
    xboxController("f4:6a:d7:a0:5c:59");

String xbox_string()
{
  String str = String(xboxController.xboxNotif.btnY) + "," +
               String(xboxController.xboxNotif.btnX) + "," +
               String(xboxController.xboxNotif.btnB) + "," +
               String(xboxController.xboxNotif.btnA) + "," +
               String(xboxController.xboxNotif.btnLB) + "," +
               String(xboxController.xboxNotif.btnRB) + "," +
               String(xboxController.xboxNotif.btnSelect) + "," +
               String(xboxController.xboxNotif.btnStart) + "," +
               String(xboxController.xboxNotif.btnXbox) + "," +
               String(xboxController.xboxNotif.btnShare) + "," +
               String(xboxController.xboxNotif.btnLS) + "," +
               String(xboxController.xboxNotif.btnRS) + "," +
               String(xboxController.xboxNotif.btnDirUp) + "," +
               String(xboxController.xboxNotif.btnDirRight) + "," +
               String(xboxController.xboxNotif.btnDirDown) + "," +
               String(xboxController.xboxNotif.btnDirLeft) + "," +
               String(xboxController.xboxNotif.joyLHori) + "," +
               String(xboxController.xboxNotif.joyLVert) + "," +
               String(xboxController.xboxNotif.joyRHori) + "," +
               String(xboxController.xboxNotif.joyRVert) + "," +
               String(xboxController.xboxNotif.trigLT) + "," +
               String(xboxController.xboxNotif.trigRT) + "\n";
  return str;
};

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting NimBLE Client");
  xboxController.begin();
}

void loop()
{
  xboxController.onLoop();
  if (xboxController.isConnected())
  {
    if (xboxController.isWaitingForFirstNotification())
    {
      Serial.println("waiting for first notification");
    }
    else
    {
      Serial.print(xbox_string());
    }
  }
  else
  {
    Serial.println("not connected");
    if (xboxController.getCountFailedConnection() > 2)
    {
      ESP.restart();
    }
  }
  delay(20);
}
