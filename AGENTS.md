# module_i2c-mapper — сканер шины I2C

> **Опциональный модуль.** Подключается через `src_filter` + `build_flags`.
> **Работоспособен только в составе сборки, содержащей ядро** (см. `../../TRS.md` §2.1).

Сканирование адресов шины I2C 0x01..0x7E (Wire0) с кормлением watchdog; результат через
web и последовательную ASCII-карту.

- **Репозиторий:** https://github.com/Arcanum753/module_i2c-mapper
- **Папка:** `src/module_i2c-mapper/`
- **Флаг активации:** `-D MODULE_I2C_MAPPER`
- **Registry:** `object=module_i2c_mapper`, `define=MODULE_I2C_MAPPER`, `web=1`, `loop=0` (без `namespace`/`res`/`prio`)
- **Только для платформы:** обе (ESP8266/ESP32); env — `esp32-i2c-mapper`
- **Зависит от модулей:** —
- **Зависит от ядра:** `core_web`, `core_sys`, `core_terminal` (команда `i2c`)

## Назначение

Сканирование адресов шины I2C 0x01..0x7E (Wire0) с кормлением watchdog; результат через
web и последовательную карту.

## Функциональные требования

- FR-I2CMAP-1: `POST /i2cmapper/scan` возвращает JSON-массив найденных адресов вида `"0xNN"`.
- FR-I2CMAP-2: Сканирование с кормлением `esp_task_wdt` (ESP32).
- FR-I2CMAP-3: Выводы `I2C_MAPPER_SDA/SCL` (ESP32 21/22, ESP8266 4/5).
- FR-I2CMAP-4: Терминальная команда `i2c` — `show` (платформа, пины, состояние шины,
  кэш последнего скана), `scan` (ASCII-карта + список адресов), `ping <addr>` (hex,
  `0x01..0x7E`); старое имя `i2c-scan` — скрытый алиас `i2c scan`.

## Аппаратные интерфейсы

| Интерфейс | Выводы по умолчанию | Примечание |
|-----------|---------------------|------------|
| I2C (сканер) | ESP32 21/22, ESP8266 4/5 (SDA/SCL) | Wire0 |

## Веб-интерфейс

Страница `i2cmapper.html` (пункт меню — `web/_menu.html`); маршрут `POST /i2cmapper/scan`.

## Конфигурация

Собственного `config_*.json` нет.

## Терминальные команды

Реализация — `module_i2c-mapper_terminal.cpp`; `terminalRegister()` вызывается из `begin()`.
Компонент в дескрипторе — `module_i2c-mapper` (по нему `sys stat` ставит флаг `term`),
команда верхнего уровня — `i2c`, `editable = false` (конфига у модуля нет). Справка —
`i2c help`.

| Подкоманда | Назначение |
|---|---|
| `i2c show` | платформа, SDA/SCL, состояние Wire, кэш последнего скана (без обращений к шине) |
| `i2c scan` | скан `0x01..0x7E`, ASCII-карта и список найденных адресов; обновляет кэш |
| `i2c ping <addr>` | проверка одного адреса (hex: `3C` или `0x3C`, диапазон `0x01..0x7E`) |
| `i2c help` | справка |

Правила:

- адрес в `i2c ping` — **только hex**, `0x`-префикс опционален, ровно один аргумент;
  невалидный адрес даёт usage-сообщение без обращения к шине;
- кэш последнего скана обновляют и `i2c scan`, и web-эндпоинт `POST /i2cmapper/scan`;
  `i2c show` кэш только читает;
- `i2c`/`i2c scan` блокируют main-loop на время прохода по 126 адресам (~0.1–0.2 с),
  watchdog кормится внутри `scanBus()`;
- конфига нет, поэтому подкоманды `save` нет и `editable = false`;
- старое имя `i2c-scan` — скрытый алиас на `i2c scan` (аргументов не принимает).

## Слоистая структура

Из `../../LAYERS.md`: `module_i2c-mapper` — сканер шины I2C; деление на слои уже выполнено:
define'ы и кэш — в `_types.h`/классе, работа с шиной — в `_engine.cpp`, команды — в
`_terminal.cpp`.

```
module_i2c-mapper_types.h    — define'ы пинов, диапазона адресов и размера кэша
module_i2c-mapper.h          — класс CLASS_MODULE_I2C_MAPPER, debug-макрос, extern
module_i2c-mapper.cpp        — шаблонный блок (begin, web_Init, веб-обработчики, версии)
module_i2c-mapper_engine.cpp — busInit, scanBus, probeBus
module_i2c-mapper_terminal.cpp — терминальные команды i2c
module_i2c-mapper.ini        — env-пример + секция [registry]
web/                         — i2cmapper.html, _menu.html
```

## Тестирование

- HIL-стенд (уровень 5): реальные чипы на шине I2C (см. `../../TESTING.md`).

## Ссылки

- Ядро и конвенции: `../../TRS.md`
- Слоистая структура: `../../LAYERS.md`
- Общие утилиты: `../../TRS.md` §3.1.12 (`common/`)
- Сборка: `../../BUILD.md`
- Реестр компонентов: `../../INVENTORY.md`

> Если модуль читается вне дерева ядра (standalone), корневые документы доступны в
> репозитории ядра avr-fota.
