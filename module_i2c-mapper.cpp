#include "core_web/FSWebServerLib.h"

#include "module_i2c-mapper.h"
#include "module_i2c-mapper_version.h"

CLASS_MODULE_I2C_MAPPER module_i2c_mapper;

CLASS_MODULE_I2C_MAPPER::CLASS_MODULE_I2C_MAPPER() {
    _wireReady = false;
    _scanDone = false;
    _lastCount = 0;
}

// ============================================================
// begin()
// ============================================================
void CLASS_MODULE_I2C_MAPPER::begin() {
    DEBUGI2CMAPPER("%s\r\n", __FUNCTION__);

    terminalRegister();
}

void CLASS_MODULE_I2C_MAPPER::begin(ModContext& ctx) {
    (void)ctx;
    begin();
}

// ============================================================
// web_Init()
// ============================================================
void CLASS_MODULE_I2C_MAPPER::web_Init() {
    DEBUGI2CMAPPER("%s\r\n", __FUNCTION__);

    ESPHTTPServer.on("/i2cmapper/scan", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (!ESPHTTPServer.checkAuth(request)) { return request->requestAuthentication(); }
        this->handleScan(request);
    });

    ESPHTTPServer.on("/i2cmapper/ver", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->html_ver_get(request);
    });
}

// ============================================================
// Веб-обработчики
// ============================================================
void CLASS_MODULE_I2C_MAPPER::handleScan(AsyncWebServerRequest *request) {
    DEBUGI2CMAPPER("%s\r\n", __FUNCTION__);

    uint8_t count = scanBus();

    String result = "[";
    bool first = true;
    for (uint8_t i = 0; i < count; i++) {
        if (!first) { result += ","; }
        first = false;
        char buf[8];
        snprintf(buf, sizeof(buf), "\"0x%02X\"", _lastAddrs[i]);
        result += buf;
    }
    result += "]";
    request->send(200, "application/json", result);
}

// ============================================================
// Версионные методы
// ============================================================
String CLASS_MODULE_I2C_MAPPER::getVersionStr() {
    return String(MODULE_I2C_MAPPER_VERSION);
}

String CLASS_MODULE_I2C_MAPPER::getGeneratedTime() {
    return String(MODULE_I2C_MAPPER_GENERATED_TIME);
}

String CLASS_MODULE_I2C_MAPPER::getCommitDateStr() {
    return String(MODULE_I2C_MAPPER_COMMIT_DATE_STR);
}

void CLASS_MODULE_I2C_MAPPER::html_ver_get(AsyncWebServerRequest *request) {
    DEBUGI2CMAPPER("%s\r\n", __FUNCTION__);
    String values = "";
    values += "i2cmapperversion|" + getVersionStr()    + "|div\n";
    values += "i2cmappergentime|" + getGeneratedTime() + "|div\n";
    values += "i2cmappergendate|" + getCommitDateStr() + "|div\n";
    request->send(200, "text/plain", values);
}
