#ifndef _MODULE_I2C_MAPPER_h
#define _MODULE_I2C_MAPPER_h

#include "main.h"

#include "mod_context.h"

#ifdef DEBUG_I2C_MAPPER
#define DEBUGI2CMAPPER(...) DBG_MOD("[M_I2C_MAPPER] ", __VA_ARGS__)
#else
#define DEBUGI2CMAPPER(...)
#endif

#if defined(ESP32)
#include <LittleFS.h>
#include <esp_task_wdt.h>
#endif
#if defined(ESP8266)
#include <LittleFS.h>
#endif

#include "module_i2c-mapper_types.h"

class CLASS_MODULE_I2C_MAPPER {
public:
    CLASS_MODULE_I2C_MAPPER();

    void begin();
    void begin(ModContext& ctx);
    void web_Init();
    void terminalRegister();

private:
    // Терминальные команды: статические методы-члены, поэтому имеют
    // прямой доступ к приватному состоянию модуля (см. _terminal.cpp).
    static void s_terminalCmd();
    static void s_cmdShow();
    static void s_cmdScan();
    static void s_cmdPing();
    static void s_cmdScanAlias();

    // Версионные методы
    String getVersionStr();
    String getGeneratedTime();
    String getCommitDateStr();
    void html_ver_get(AsyncWebServerRequest *request);

    // Веб-обработчики
    void handleScan(AsyncWebServerRequest *request);

    // Логика шины
    void busInit();
    uint8_t scanBus();                                   // скан шины, результат — в кэш
    bool probeBus(uint8_t addr);                         // проверка одного адреса (ACK)

    bool _wireReady;
    bool _scanDone;
    uint8_t _lastCount;
    uint8_t _lastAddrs[I2C_MAPPER_MAX_FOUND];
};

extern CLASS_MODULE_I2C_MAPPER module_i2c_mapper;

#endif // _MODULE_I2C_MAPPER_h
