#ifndef __XBOX_CONTROLLER_NOTIFICATION_PARSER_H__
#define __XBOX_CONTROLLER_NOTIFICATION_PARSER_H__

#include <Arduino.h>

#define XBOX_CONTROLLER_ERROR_INVALID_LENGTH 1

class XboxControllerNotificationParser {
 public:
  XboxControllerNotificationParser();

  bool btnA, btnB, btnX, btnY;
  bool btnShare, btnStart, btnSelect, btnXbox;
  // side top button
  bool btnLB, btnRB;
  // button on joy stick
  bool btnLS, btnRS;
  bool btnDirUp, btnDirLeft, btnDirRight, btnDirDown;
  uint16_t joyLHori = maxJoy / 2;
  uint16_t joyLVert = maxJoy / 2;
  uint16_t joyRHori = maxJoy / 2;
  uint16_t joyRVert = maxJoy / 2;
  uint16_t trigLT, trigRT;
  uint8_t update(uint8_t* data, size_t length);
  uint8_t toArr(uint8_t* data, size_t length);
  String toString();

  static const size_t expectedDataLen = 16;
  static const uint16_t maxJoy = 0xffff;
  static const uint16_t maxTrig = 0x3ff;

 private:
  static void convertU16TU8Arr(uint16_t vU16, uint8_t* data);
};

#endif
