#pragma once

#include <NimBLEDevice.h>
#include <XboxControllerNotificationParser.h>

#include <XboxSeriesXHIDReportBuilder_asukiaaa.hpp>

// #define XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL Serial
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
const unsigned long printInterval = 100UL;
#endif

namespace XboxSeriesXControllerESP32_asukiaaa {

static NimBLEUUID uuidServiceGeneral("1801");
static NimBLEUUID uuidServiceBattery("180f");
static NimBLEUUID uuidServiceHid("1812");
static NimBLEUUID uuidCharaReport("2a4d");
static NimBLEUUID uuidCharaPnp("2a50");
static NimBLEUUID uuidCharaHidInformation("2a4a");
static NimBLEUUID uuidCharaPeripheralAppearance("2a01");
static NimBLEUUID uuidCharaPeripheralControlParameters("2a04");

static NimBLEAdvertisedDevice* advDevice;
static NimBLEClient* pConnectedClient = nullptr;

static const uint16_t controllerAppearance = 964;
static const String controllerManufacturerDataNormal = "060000";
static const String controllerManufacturerDataSearching = "0600030080";

enum class ConnectionState : uint8_t {
  Connected = 0,
  WaitingForFirstNotification = 1,
  Found = 2,
  Scanning = 3,
};

class ClientCallbacks : public NimBLEClientCallbacks {
 public:
  ConnectionState* pConnectionState;
  ClientCallbacks(ConnectionState* pConnectionState) {
    this->pConnectionState = pConnectionState;
  }

  void onConnect(NimBLEClient* pClient) {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println("Connected");
#endif
    *pConnectionState = ConnectionState::WaitingForFirstNotification;
    // pClient->updateConnParams(120,120,0,60);
  };

  void onDisconnect(NimBLEClient* pClient) {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.print(
        pClient->getPeerAddress().toString().c_str());
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(" Disconnected");
#endif
    *pConnectionState = ConnectionState::Scanning;
    pConnectedClient = nullptr;
  };

  /********************* Security handled here **********************
  ****** Note: these are the same return values as defaults ********/
  uint32_t onPassKeyRequest() {
    // Serial.println("Client Passkey Request");
    /** return the passkey to send to the server */
    return 0;
  };

  bool onConfirmPIN(uint32_t pass_key) {
    // Serial.print("The passkey YES/NO number: ");
    // Serial.println(pass_key);
    /** Return false if passkeys don't match. */
    return true;
  };

  /** Pairing process complete, we can check the results in ble_gap_conn_desc */
  void onAuthenticationComplete(ble_gap_conn_desc* desc) {
    // Serial.println("onAuthenticationComplete");
    if (!desc->sec_state.encrypted) {
      // Serial.println("Encrypt connection failed - disconnecting");
      /** Find the client with the connection handle provided in desc */
      NimBLEDevice::getClientByID(desc->conn_handle)->disconnect();
      return;
    }
  };
};

/** Define a class to handle the callbacks when advertisments are received */
class AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
 public:
  AdvertisedDeviceCallbacks(String strTargetDeviceAddress,
                            ConnectionState* pConnectionState) {
    if (strTargetDeviceAddress != "") {
      this->targetDeviceAddress =
          new NimBLEAddress(strTargetDeviceAddress.c_str());
    }
    this->pConnectionState = pConnectionState;
  }

 private:
  NimBLEAddress* targetDeviceAddress = nullptr;
  ConnectionState* pConnectionState;
  void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.print("Advertised Device found: ");
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(
        advertisedDevice->toString().c_str());
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.printf(
        "name:%s, address:%s\n", advertisedDevice->getName().c_str(),
        advertisedDevice->getAddress().toString().c_str());
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.printf(
        "uuidService:%s\n",
        advertisedDevice->haveServiceUUID()
            ? advertisedDevice->getServiceUUID().toString().c_str()
            : "none");
#endif
    char* pHex = NimBLEUtils::buildHexData(
        nullptr, (uint8_t*)advertisedDevice->getManufacturerData().data(),
        advertisedDevice->getManufacturerData().length());
    if ((targetDeviceAddress != nullptr &&
         advertisedDevice->getAddress().equals(*targetDeviceAddress)) ||
        (targetDeviceAddress == nullptr &&
         advertisedDevice->getAppearance() == controllerAppearance &&
         (strcmp(pHex, controllerManufacturerDataNormal.c_str()) == 0 ||
          strcmp(pHex, controllerManufacturerDataSearching.c_str()) == 0) &&
         advertisedDevice->getServiceUUID().equals(uuidServiceHid)))
    // if (advertisedDevice->isAdvertisingService(uuidServiceHid))
    {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
      XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println("Found target");
#endif
      /** stop scan before connecting */
      // NimBLEDevice::getScan()->stop();
      /** Save the device reference in a global for the client to use*/
      *pConnectionState = ConnectionState::Found;
      advDevice = advertisedDevice;
    }
  };
};

class Core {
 public:
  Core(String targetDeviceAddress = "") {
    this->advDeviceCBs =
        new AdvertisedDeviceCallbacks(targetDeviceAddress, &connectionState);
    this->clientCBs = new ClientCallbacks(&connectionState);
  }

  AdvertisedDeviceCallbacks* advDeviceCBs;
  ClientCallbacks* clientCBs;
  uint8_t battery = 0;
  static const int deviceAddressLen = 6;
  uint8_t deviceAddressArr[deviceAddressLen];

  void begin() {
    NimBLEDevice::setScanFilterMode(CONFIG_BTDM_SCAN_DUPL_TYPE_DEVICE);
    // NimBLEDevice::setScanDuplicateCacheSize(200);
    NimBLEDevice::init("");
    NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_PUBLIC);
    NimBLEDevice::setSecurityAuth(true, false, false);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /* +9db */
  }

  void writeHIDReport(uint8_t* dataArr, size_t dataLen) {
    if (pConnectedClient == nullptr) {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
      XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println("no connnected client");
#endif
      return;
    }
    NimBLEClient* pClient = pConnectedClient;
    auto pService = pClient->getService(uuidServiceHid);
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(pService->toString().c_str());
#endif
    for (auto pChara : *pService->getCharacteristics()) {
      if (pChara->canWrite()) {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
        XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(
            "canWrite " + String(pChara->canWrite()));
        XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(
            pChara->toString().c_str());
        writeWithComment(pChara, dataArr, dataLen);
#else
        pChara->writeValue(dataArr, dataLen, false);
#endif
      }
    }
  }

  void writeHIDReport(
      const XboxSeriesXHIDReportBuilder_asukiaaa::ReportBase& repo) {
    writeHIDReport((uint8_t*)repo.arr8t, repo.arr8tLen);
  }

  void writeHIDReport(
      const XboxSeriesXHIDReportBuilder_asukiaaa::ReportBeforeUnion&
          repoBeforeUnion) {
    XboxSeriesXHIDReportBuilder_asukiaaa::ReportBase repo;
    repo.v = repoBeforeUnion;
    writeHIDReport((uint8_t*)repo.arr8t, repo.arr8tLen);
  }

  void onLoop() {
    if (!isConnected()) {
      if (advDevice != nullptr) {
        auto connectionResult = connectToServer(advDevice);
        if (!connectionResult || !isConnected()) {
          NimBLEDevice::deleteBond(advDevice->getAddress());
          ++countFailedConnection;
          // reset();
          connectionState = ConnectionState::Scanning;
        } else {
          countFailedConnection = 0;
        }
        advDevice = nullptr;
      } else if (!isScanning()) {
        // reset();
        startScan();
      }
    }
  }

  String buildDeviceAddressStr() {
    char buffer[18];
    auto addr = deviceAddressArr;
    snprintf(buffer, sizeof(buffer), "%02x:%02x:%02x:%02x:%02x:%02x", addr[5],
             addr[4], addr[3], addr[2], addr[1], addr[0]);
    return String(buffer);
  }

  void startScan() {
    connectionState = ConnectionState::Scanning;
    auto pScan = NimBLEDevice::getScan();
    // pScan->clearResults();
    // pScan->clearDuplicateCache();
    pScan->setDuplicateFilter(false);
    pScan->setAdvertisedDeviceCallbacks(advDeviceCBs);
    // pScan->setActiveScan(true);
    pScan->setInterval(97);
    pScan->setWindow(97);
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println("Start scan");
#endif
    // assign scanCompleteCB to scan on other thread
    pScan->start(scanTime, &Core::scanCompleteCB, false);
  }

  XboxControllerNotificationParser xboxNotif;

  bool isWaitingForFirstNotification() {
    return connectionState == ConnectionState::WaitingForFirstNotification;
  }
  bool isConnected() {
    return connectionState == ConnectionState::WaitingForFirstNotification ||
           connectionState == ConnectionState::Connected;
  }
  unsigned long getReceiveNotificationAt() { return receivedNotificationAt; }
  uint8_t getCountFailedConnection() { return countFailedConnection; }

 private:
  ConnectionState connectionState = ConnectionState::Scanning;
  unsigned long receivedNotificationAt = 0;
  uint32_t scanTime = 4; /** 0 = scan forever */
  uint8_t countFailedConnection = 0;
  uint8_t retryCountInOneConnection = 3;
  unsigned long retryIntervalMs = 100;
  NimBLEClient* pClient = nullptr;

#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
  static void writeWithComment(NimBLERemoteCharacteristic* pChara,
                               uint8_t* data, size_t len) {
    Serial.println("send(print from addr 0) ");
    for (int i = 0; i < len; ++i) {
      Serial.print(data[i]);
      Serial.print(" ");
    }
    if (pChara->writeValue(data, len, true)) {
      XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println("suceeded in writing");
    } else {
      XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println("failed writing");
    }
  }
#endif

  static void readAndPrint(NimBLERemoteCharacteristic* pChara) {
    auto str = pChara->readValue();
    if (str.size() == 0) {
      str = pChara->readValue();
    }
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
    printValue(str);
#endif
  }

  bool isScanning() { return NimBLEDevice::getScan()->isScanning(); }

  // void reset() {
  //   NimBLEDevice::deinit(true);
  //   delay(500);
  //   begin();
  //   delay(500);
  // }

  /** Handles the provisioning of clients and connects / interfaces with the
   * server */
  bool connectToServer(NimBLEAdvertisedDevice* advDevice) {
    NimBLEClient* pClient = nullptr;

    /** Check if we have a client we should reuse first **/
    if (NimBLEDevice::getClientListSize()) {
      pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
      if (pClient) {
        pClient->connect();
      }
    }

    /** No client to reuse? Create a new one. */
    if (!pClient) {
      if (NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
        XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(
            "Max clients reached - no more connections available");
#endif
        return false;
      }

      pClient = NimBLEDevice::createClient();

#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
      XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println("New client created");
#endif

      // default values
      // pClient->setConnectionParams(
      //     BLE_GAP_INITIAL_CONN_ITVL_MIN, BLE_GAP_INITIAL_CONN_ITVL_MAX,
      //     BLE_GAP_INITIAL_CONN_LATENCY, BLE_GAP_INITIAL_SUPERVISION_TIMEOUT,
      //     16, 16);
      // pClient->setConnectionParams(
      //     BLE_GAP_INITIAL_CONN_ITVL_MIN, BLE_GAP_INITIAL_CONN_ITVL_MAX,
      //     BLE_GAP_INITIAL_CONN_LATENCY, BLE_GAP_INITIAL_SUPERVISION_TIMEOUT,
      //     100, 100);
      pClient->setClientCallbacks(clientCBs, true);
      pClient->connect(advDevice, true);
    }

    int retryCount = retryCountInOneConnection;
    while (!pClient->isConnected()) {
      if (retryCount <= 0) {
        return false;
      } else {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
        XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println("Try connection. left: " +
                                                      String(retryCount));
#endif
      }

      // NimBLEDevice::getScan()->stop();
      // pClient->disconnect();
      delay(retryIntervalMs);
      // Serial.println(pClient->toString().c_str());
      pClient->connect(true);
      --retryCount;
    }
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.print("Connected to: ");
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(
        pClient->getPeerAddress().toString().c_str());
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.print("RSSI: ");
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(pClient->getRssi());
#endif

    // pClient->discoverAttributes();

    bool result = afterConnect(pClient);
    if (!result) {
      return result;
    }

#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println("Done with this device!");
#endif
    pConnectedClient = pClient;
    return true;
  }

  bool afterConnect(NimBLEClient* pClient) {
    memcpy(deviceAddressArr, pClient->getPeerAddress().getNative(),
           deviceAddressLen);
    for (auto pService : *pClient->getServices(true)) {
      auto sUuid = pService->getUUID();
      if (!sUuid.equals(uuidServiceHid) && !sUuid.equals(uuidServiceBattery)) {
        continue;  // skip
      }
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
      XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(
          pService->toString().c_str());
#endif
      for (auto pChara : *pService->getCharacteristics(true)) {
        charaHandle(pChara);
        charaSubscribeNotification(pChara);
      }
    }

    return true;
  }

#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
  void charaPrintId(NimBLERemoteCharacteristic* pChara) {
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.printf(
        "s:%s c:%s h:%d",
        pChara->getRemoteService()->getUUID().toString().c_str(),
        pChara->getUUID().toString().c_str(), pChara->getHandle());
  }

  static void printValue(std::__cxx11::string str) {
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.printf("str: %s\n", str.c_str());
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.printf("hex:");
    for (auto v : str) {
      XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.printf(" %02x", v);
    }
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println("");
  }
#endif

  void charaHandle(NimBLERemoteCharacteristic* pChara) {
    if (pChara->canWrite()) {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
      charaPrintId(pChara);
      XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(" canWrite");
#endif
    }
    if (pChara->canRead()) {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
      charaPrintId(pChara);
      XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(" canRead");
#endif
      // Reading value is required for subscribe
      auto str = pChara->readValue();
      if (str.size() == 0) {
        str = pChara->readValue();
      }
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
      printValue(str);
#endif
    }
  }

  void charaSubscribeNotification(NimBLERemoteCharacteristic* pChara) {
    if (pChara->canNotify()) {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
      charaPrintId(pChara);
      XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(" canNotify ");
#endif
      if (pChara->subscribe(
              true,
              std::bind(&Core::notifyCB, this, std::placeholders::_1,
                        std::placeholders::_2, std::placeholders::_3,
                        std::placeholders::_4),
              true)) {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
        XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(
            "succeeded in subscribing");
#endif
      } else {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
        XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println("failed subscribing");
#endif
      }
    }
  }

  void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic,
                uint8_t* pData, size_t length, bool isNotify) {
    auto sUuid = pRemoteCharacteristic->getRemoteService()->getUUID();
    if (connectionState != ConnectionState::Connected) {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
      XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(
          "Received first notification");
#endif
      connectionState = ConnectionState::Connected;
    }
    if (sUuid.equals(uuidServiceHid)) {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
      static bool isPrinting = false;
      static unsigned long printedAt = 0;
      if (isPrinting || millis() - printedAt < printInterval) return;
      isPrinting = true;
      std::string str = (isNotify == true) ? "Notification" : "Indication";
      str += " from ";
      /** NimBLEAddress and NimBLEUUID have std::string operators */
      str += std::string(pRemoteCharacteristic->getRemoteService()
                             ->getClient()
                             ->getPeerAddress());
      str += ": Service = " +
             std::string(pRemoteCharacteristic->getRemoteService()->getUUID());
      str +=
          ", Characteristic = " + std::string(pRemoteCharacteristic->getUUID());
      // str += ", Value = " + std::string((char*)pData, length);
      XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(str.c_str());
      XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.print("value: ");
      for (int i = 0; i < length; ++i) {
        XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.printf(" %02x", pData[i]);
      }
      XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println("");
#endif
      xboxNotif.update(pData, length);
      receivedNotificationAt = millis();
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
      // XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.print(xboxNotif.toString());
      printedAt = millis();
      isPrinting = false;
#endif
    } else {
      if (sUuid.equals(uuidServiceBattery)) {
        battery = pData[0];
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
        XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println("battery notification");
#endif
      } else {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
        XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.printf("s:%s",
                                                     sUuid.toString().c_str());
        XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println(
            "not handled notification");
#endif
      }
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
      for (int i = 0; i < length; ++i) {
        XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.printf(" %02x", pData[i]);
      }
      XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println("");
#endif
    }
  }

  static void scanCompleteCB(NimBLEScanResults results) {
#ifdef XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL
    XBOX_SERIES_X_CONTROLLER_DEBUG_SERIAL.println("Scan Ended");
#endif
  }
};

};  // namespace XboxSeriesXControllerESP32_asukiaaa
