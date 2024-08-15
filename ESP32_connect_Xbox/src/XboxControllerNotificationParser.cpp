#include "XboxControllerNotificationParser.h"

#define XBOX_CONTROLLER_INDEX_BUTTONS_DIR 12
#define XBOX_CONTROLLER_INDEX_BUTTONS_MAIN 13
#define XBOX_CONTROLLER_INDEX_BUTTONS_CENTER 14
#define XBOX_CONTROLLER_INDEX_BUTTONS_SHARE 15

XboxControllerNotificationParser::XboxControllerNotificationParser() {
  btnA = btnB = btnX = btnY = false;
  btnShare = btnStart = btnSelect = btnXbox = false;
  btnLB = btnRB = btnLS = btnRS = false;
  btnDirUp = btnDirLeft = btnDirRight = btnDirDown = false;
  joyLHori = joyLVert = joyRHori = joyRVert = 0xffff / 2;
  trigLT = trigRT = 0;
}

uint8_t XboxControllerNotificationParser::update(uint8_t* data, size_t length) {
  if (length != expectedDataLen) {
    return XBOX_CONTROLLER_ERROR_INVALID_LENGTH;
  }
  uint8_t btnBits;
  btnBits = data[XBOX_CONTROLLER_INDEX_BUTTONS_MAIN];
  btnA = btnBits & 0b00000001;
  btnB = btnBits & 0b00000010;
  btnX = btnBits & 0b00001000;
  btnY = btnBits & 0b00010000;
  btnLB = btnBits & 0b01000000;
  btnRB = btnBits & 0b10000000;

  btnBits = data[XBOX_CONTROLLER_INDEX_BUTTONS_CENTER];
  btnSelect = btnBits & 0b00000100;
  btnStart = btnBits & 0b00001000;
  btnXbox = btnBits & 0b00010000;
  btnLS = btnBits & 0b00100000;
  btnRS = btnBits & 0b01000000;

  btnBits = data[XBOX_CONTROLLER_INDEX_BUTTONS_SHARE];
  btnShare = btnBits & 0b00000001;

  btnBits = data[XBOX_CONTROLLER_INDEX_BUTTONS_DIR];
  btnDirUp = btnBits == 1 || btnBits == 2 || btnBits == 8;
  btnDirRight = 2 <= btnBits && btnBits <= 4;
  btnDirDown = 4 <= btnBits && btnBits <= 6;
  btnDirLeft = 6 <= btnBits && btnBits <= 8;

  joyLHori = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
  joyLVert = (uint16_t)data[2] | ((uint16_t)data[3] << 8);
  joyRHori = (uint16_t)data[4] | ((uint16_t)data[5] << 8);
  joyRVert = (uint16_t)data[6] | ((uint16_t)data[7] << 8);
  trigLT = (uint16_t)data[8] | ((uint16_t)data[9] << 8);
  trigRT = (uint16_t)data[10] | ((uint16_t)data[11] << 8);
  return 0;
}

void XboxControllerNotificationParser::convertU16TU8Arr(uint16_t vU16,
                                                        uint8_t* data) {
  data[0] = vU16 & 0xff;
  data[1] = vU16 >> 8;
}

uint8_t XboxControllerNotificationParser::toArr(uint8_t* data, size_t length) {
  if (length < expectedDataLen) {
    return XBOX_CONTROLLER_ERROR_INVALID_LENGTH;
  }
  convertU16TU8Arr(joyLHori, &data[0]);
  convertU16TU8Arr(joyLVert, &data[2]);
  convertU16TU8Arr(joyRHori, &data[4]);
  convertU16TU8Arr(joyRVert, &data[6]);
  convertU16TU8Arr(trigLT, &data[8]);
  convertU16TU8Arr(trigRT, &data[10]);
  {
    uint8_t btnBits = 0;
    if (btnA) btnBits |= 0b00000001;
    if (btnB) btnBits |= 0b00000010;
    if (btnX) btnBits |= 0b00001000;
    if (btnY) btnBits |= 0b00010000;
    if (btnLB) btnBits |= 0b01000000;
    if (btnRB) btnBits |= 0b10000000;
    data[XBOX_CONTROLLER_INDEX_BUTTONS_MAIN] = btnBits;
  }
  {
    uint8_t btnBits = 0;
    if (btnSelect) btnBits |= 0b00000100;
    if (btnStart) btnBits |= 0b00001000;
    if (btnXbox) btnBits |= 0b00010000;
    if (btnLS) btnBits |= 0b00100000;
    if (btnRS) btnBits |= 0b01000000;
    data[XBOX_CONTROLLER_INDEX_BUTTONS_CENTER] = btnBits;
  }
  {
    uint8_t btnBits = 0;
    if (btnShare) btnBits |= 0b00000001;
    data[XBOX_CONTROLLER_INDEX_BUTTONS_SHARE] = btnBits;
  }
  {
    uint8_t btnBits = 0;
    if (btnDirUp) {
      if (btnDirRight) {
        btnBits = 2;
      } else if (btnDirLeft) {
        btnBits = 8;
      } else {
        btnBits = 1;
      }
    } else if (btnDirDown) {
      if (btnDirRight) {
        btnBits = 4;
      } else if (btnDirLeft) {
        btnBits = 6;
      } else {
        btnBits = 5;
      }
    } else {
      if (btnDirRight) {
        btnBits = 3;
      } else if (btnDirLeft) {
        btnBits = 7;
      } else {
        btnBits = 0;
      }
    }
    data[XBOX_CONTROLLER_INDEX_BUTTONS_DIR] = btnBits;
  }
  return 0;
}

String XboxControllerNotificationParser::toString() {
  // clang-format off
  String str = String("") +
    "btnY: " + String(btnY) + " " +
    "btnX: " + String(btnX) + " " +
    "btnB: " + String(btnB) + " " +
    "btnA: " + String(btnA) + " " +
    "btnLB: " + String(btnLB) + " " +
    "btnRB: " + String(btnRB) + "\n" +
    "btnSelect: " + String(btnSelect) + " " +
    "btnStart: " + String(btnStart) + " " +
    "btnXbox: " + String(btnXbox) + " " +
    "btnShare: " + String(btnShare) + " " +
    "btnLS: " + String(btnLS) + " " +
    "btnRS: " + String(btnRS) + "\n" +
    "btnDirUp: " + String(btnDirUp) + " " +
    "btnDirRight: " + String(btnDirRight) + " " +
    "btnDirDown: " + String(btnDirDown) + " " +
    "btnDirLeft: " + String(btnDirLeft) + "\n"
    "joyLHori: " + String(joyLHori) + "\n" +
    "joyLVert: " + String(joyLVert) + "\n" +
    "joyRHori: " + String(joyRHori) + "\n" +
    "joyRVert: " + String(joyRVert) + "\n" +
    "trigLT: " + String(trigLT) + "\n" +
    "trigRT: " + String(trigRT) + "\n";
  // clang-format on
  return str;
}
