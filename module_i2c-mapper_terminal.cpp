#include "core_web/FSWebServerLib.h"

#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "module_i2c-mapper.h"

#include "core_terminal/core_terminal.h"
#include "core_terminal/common_module.h"

// Строковые литералы терминала: на ESP8266 держим их в flash.
#if defined(ESP8266)
#define I2C_LOG(fmt, ...) Serial.printf_P(PSTR(fmt), ##__VA_ARGS__)
#define I2C_LOG_LN(msg)   Serial.println(F(msg))
#define I2C_PRINT(msg)    Serial.print(F(msg))
#else
#define I2C_LOG(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#define I2C_LOG_LN(msg)   Serial.println(msg)
#define I2C_PRINT(msg)    Serial.print(msg)
#endif

// ============================================================
// Терминальные команды module_i2c-mapper
//
// Одна команда верхнего уровня i2c, старое имя i2c-scan сохранено скрытым
// алиасом. Конфига у модуля нет, поэтому save отсутствует, а изменения
// касаются только состояния шины и кэша последнего скана.
// ============================================================

static const char* const kI2cCommands[] = {
    "show", "scan", "ping", nullptr
};

static const char kI2cHelp[] PROGMEM =
    "i2c - I2C bus scanner (no config to save)\r\n"
    "  i2c show           platform, SDA/SCL, bus state, last scan result\r\n"
    "  i2c scan           scan 0x01..0x7E, print bus map and found addresses\r\n"
    "  i2c ping <addr>    check one address (hex: 3C or 0x3C, range 0x01..0x7E)\r\n"
    "  i2c help           this help\r\n"
    "\r\n"
    "Old name 'i2c-scan' is a hidden alias of 'i2c scan'.\r\n";

// Строгий разбор адреса: только hex, опциональный префикс 0x/0X,
// 1..2 цифры, вся строка, диапазон I2C_MAPPER_ADDR_MIN..MAX.
static bool parseAddrHex(const String& in, uint8_t& out) {
    String v = ns_core_terminal::trimCopy(in);
    if (v.length() == 0) { return false; }

    const char* s = v.c_str();
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) { s += 2; }
    if (*s == '\0' || strlen(s) > 2) { return false; }

    for (const char* p = s; *p != '\0'; p++) {
        bool isHex = (*p >= '0' && *p <= '9') ||
                     (*p >= 'a' && *p <= 'f') ||
                     (*p >= 'A' && *p <= 'F');
        if (!isHex) { return false; }
    }

    long value = strtol(s, nullptr, 16);
    if (value < I2C_MAPPER_ADDR_MIN || value > I2C_MAPPER_ADDR_MAX) { return false; }

    out = (uint8_t)value;
    return true;
}

void CLASS_MODULE_I2C_MAPPER::terminalRegister() {
    TerminalRegisterModule("module_i2c-mapper", "i2c", "I2C bus scanner",
                           &CLASS_MODULE_I2C_MAPPER::s_terminalCmd,
                           kI2cCommands, kI2cHelp, false);

    // Старое имя — скрытым алиасом, слот не занимает.
    TerminalRegisterAlias("i2c-scan", &CLASS_MODULE_I2C_MAPPER::s_cmdScanAlias);
}

void CLASS_MODULE_I2C_MAPPER::s_terminalCmd() {
    char* sub = term.getNext();
    if (sub == nullptr || strcmp(sub, "help") == 0) {
        TerminalPrintText(kI2cHelp);
        return;
    }

    if (strcmp(sub, "show") == 0) { s_cmdShow(); return; }
    if (strcmp(sub, "scan") == 0) { s_cmdScan(); return; }
    if (strcmp(sub, "ping") == 0) { s_cmdPing(); return; }

    I2C_LOG("Unknown i2c subcommand '%s'. Type 'i2c help'.\r\n", sub);
}

// ============================================================
// show — состояние шины и кэш последнего скана (без обращений к I2C)
// ============================================================

void CLASS_MODULE_I2C_MAPPER::s_cmdShow() {
    if (term.getNext() != nullptr) {
        I2C_LOG_LN("Usage: i2c show");
        return;
    }

#if defined(ESP32)
    I2C_LOG_LN("Platform: ESP32");
#endif
#if defined(ESP8266)
    I2C_LOG_LN("Platform: ESP8266");
#endif

    I2C_LOG("SDA: %d  SCL: %d\r\n", I2C_MAPPER_SDA, I2C_MAPPER_SCL);
    I2C_LOG("Wire: %s\r\n", module_i2c_mapper._wireReady ? "initialized" : "not initialized");

    if (!module_i2c_mapper._scanDone) {
        I2C_LOG_LN("Last scan: none");
        return;
    }

    if (module_i2c_mapper._lastCount == 0) {
        I2C_LOG_LN("Last scan: 0 device(s)");
        return;
    }

    I2C_LOG("Last scan: %d device(s):", (int)module_i2c_mapper._lastCount);
    for (uint8_t i = 0; i < module_i2c_mapper._lastCount; i++) {
        I2C_LOG(" 0x%02X", (unsigned)module_i2c_mapper._lastAddrs[i]);
    }
    Serial.println();
}

// ============================================================
// scan — ASCII-карта шины и список найденных адресов
// ============================================================

void CLASS_MODULE_I2C_MAPPER::s_cmdScan() {
    if (term.getNext() != nullptr) {
        I2C_LOG_LN("Usage: i2c scan");
        return;
    }

    uint8_t count = module_i2c_mapper.scanBus();

    Serial.println();
    I2C_LOG_LN("I2C Bus Map (0x01-0x7E)");
    I2C_LOG_LN("    0 1 2 3 4 5 6 7 8 9 a b c d e f");
    for (uint8_t r = 0; r < 8; r++) {
        I2C_LOG("%02X: ", (unsigned)(r * 0x10));
        for (uint8_t c = 0; c < 16; c++) {
            uint8_t addr = (uint8_t)((r << 4) | c);
            bool foundAddr = false;
            for (uint8_t i = 0; i < count; i++) {
                if (module_i2c_mapper._lastAddrs[i] == addr) { foundAddr = true; break; }
            }
            if (foundAddr) {
                I2C_LOG("%02X ", (unsigned)addr);   // полный адрес на перекрестье строки и столбца
            } else {
                I2C_PRINT("-- ");
            }
        }
        Serial.println();
    }

    if (count == 0) {
        I2C_LOG_LN("No devices found.");
    } else {
        I2C_LOG("Found %d device(s):", (int)count);
        for (uint8_t i = 0; i < count; i++) {
            I2C_LOG(" 0x%02X", (unsigned)module_i2c_mapper._lastAddrs[i]);
        }
        Serial.println();
    }
}

// Скрытый алиас i2c-scan: та же карта, без аргументов.
void CLASS_MODULE_I2C_MAPPER::s_cmdScanAlias() {
    if (term.getNext() != nullptr) {
        I2C_LOG_LN("Usage: i2c-scan");
        return;
    }
    s_cmdScan();
}

// ============================================================
// ping — проверка одного адреса
// ============================================================

void CLASS_MODULE_I2C_MAPPER::s_cmdPing() {
    char* arg = term.getNext();
    if (arg == nullptr || term.getNext() != nullptr) {
        I2C_LOG_LN("Usage: i2c ping <addr>   (hex, e.g. 3C or 0x3C)");
        return;
    }

    uint8_t addr = 0;
    if (!parseAddrHex(String(arg), addr)) {
        I2C_LOG_LN("Invalid address. Use hex 0x01..0x7E (e.g. 3C or 0x3C).");
        return;
    }

    if (module_i2c_mapper.probeBus(addr)) {
        I2C_LOG("0x%02X: present (ACK)\r\n", (unsigned)addr);
    } else {
        I2C_LOG("0x%02X: no response\r\n", (unsigned)addr);
    }
}
