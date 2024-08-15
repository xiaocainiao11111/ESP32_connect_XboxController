#pragma once

namespace XboxSeriesXHIDReportBuilder_asukiaaa {

struct InfoSelect {
  uint8_t center : 1;
  uint8_t shake : 1;
  uint8_t right : 1;
  uint8_t left : 1;
  uint8_t _unkown_bit_4 : 1;
  uint8_t _unkown_bit_5 : 1;
  uint8_t _unkown_bit_6 : 1;
  uint8_t _unkown_bit_7 : 1;
};

// 0 to 0x64 (100), over 0x64 becomes 0x64
struct InfoPower {
  uint8_t left;
  uint8_t right;
  uint8_t shake;
  uint8_t center;
};

struct ReportBeforeUnion {
  InfoSelect select;
  InfoPower power;
  // value * 0.01 seconds, max 2.55 seconds
  uint8_t timeActive;
  uint8_t timeSilent;
  // max 0xff
  uint8_t countRepeat;
};

class ReportBase {
 public:
  static const size_t arr8tLen = 8;
  union {
    ReportBeforeUnion v;
    uint8_t arr8t[arr8tLen];
  };

  ReportBase() { setFullPowerFor1Sec(); }

  void setFullPowerFor1Sec() {
    v.select.center = true;
    v.select.left = true;
    v.select.right = true;
    v.select.shake = true;
    v.power.center = 100;
    v.power.left = 100;
    v.power.right = 100;
    v.power.shake = 100;
    v.timeActive = 100;
    v.timeSilent = 0;
    v.countRepeat = 0;
  }

  void setAllOff() {
    v.select.center = false;
    v.select.left = false;
    v.select.right = false;
    v.select.shake = false;
    v.power.center = 0;
    v.power.left = 0;
    v.power.right = 0;
    v.power.shake = 0;
    v.timeActive = 0;
    v.timeSilent = 0;
    v.countRepeat = 0;
  }
};

}  // namespace XboxSeriesXHIDReportBuilder_asukiaaa
