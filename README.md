# ESP32 Seed

Стартовый проект для **ESP32-DevKit** на ESP-IDF + FreeRTOS: плата подключается к Wi-Fi (или поднимает свою точку доступа) и отдаёт по HTTP статическую страницу «Hello from ESP32!».

## Плата

| Параметр     | Значение                                         |
|--------------|--------------------------------------------------|
| Модуль / чип | ESP32-D0WDQ6                                     |
| CPU          | 2 × Xtensa LX6, до 240 МГц                       |
| Память       | 4 МБ Flash, 520 КБ SRAM, 448 КБ ROM              |
| Радио        | Wi-Fi 2.4 GHz 802.11 b/g/n, Bluetooth 4.2 + BLE  |
| USB          | USB-UART мост (CP2102 или CH340/CH9102)          |

> ESP32 работает только в диапазоне **2.4 GHz** — к сети 5 GHz плата не подключится.

## Структура проекта

```
ESP32_seed/
├── CMakeLists.txt          # корневой CMake-файл проекта ESP-IDF
├── sdkconfig.defaults      # настройки по умолчанию (чип, 4 МБ flash, 240 МГц, разделы)
├── main/
│   ├── CMakeLists.txt      # компонент main: исходники, зависимости, встраиваемые файлы
│   ├── Kconfig.projbuild   # пункт "ESP32 Seed" в menuconfig (Wi-Fi, GPIO светодиода)
│   ├── main.c              # app_main: NVS, сеть, задача-мигалка FreeRTOS, запуск Wi-Fi и HTTP
│   ├── wifi.c / wifi.h     # Wi-Fi: STA (подключение к сети) или SoftAP (своя точка доступа)
│   ├── http_server.c / .h  # HTTP-сервер (esp_http_server), отдаёт index.html на "/"
│   └── www/index.html      # статическая страница, вшивается в прошивку при сборке
└── .clangd                 # настройка clangd для подсветки кода в редакторе
```

Файл `sdkconfig` генерируется при первой сборке из `sdkconfig.defaults` и настроек `menuconfig`. Он в `.gitignore`, потому что в нём лежит пароль Wi-Fi.

---

## Тулчейн

Всё собирается официальным фреймворком Espressif — **ESP-IDF** (IoT Development Framework). Проект рассчитан на **ESP-IDF v5.5** (минимум v5.3).

| Компонент | Что делает |
|-----------|------------|
| **ESP-IDF** | Фреймворк: SDK, драйверы, сетевой стек lwIP, Wi-Fi/BT, HTTP-сервер и **FreeRTOS** (входит в ESP-IDF, отдельно ставить не нужно). |
| **xtensa-esp-elf-gcc** | Кросс-компилятор GCC под архитектуру Xtensa LX6. Скачивается скриптом `install.sh`. |
| **CMake + Ninja** | Система сборки. ESP-IDF генерирует проект CMake, Ninja выполняет сборку. |
| **Python 3 + venv** | ESP-IDF держит свои Python-зависимости в отдельном виртуальном окружении (`~/.espressif/python_env`). |
| **idf.py** | Главная утилита-обёртка: `build`, `flash`, `monitor`, `menuconfig`, `clean` и т.д. |
| **esptool.py** | Прошивка flash по UART через встроенный в ROM загрузчик ESP32. Вызывается из `idf.py flash`. |
| **OpenOCD (esp32)** | Отладка по JTAG. Опционально — на DevKit нет JTAG-адаптера, нужен внешний (например ESP-Prog). |

Как проходит сборка и прошивка:

1. `idf.py build` — CMake собирает три образа: загрузчик второй стадии (`bootloader.bin`), таблицу разделов (`partition-table.bin`) и приложение (`esp32_seed.bin`).
2. `idf.py flash` — esptool через USB-UART переводит чип в режим загрузки (линии DTR/RTS дёргают EN и GPIO0) и записывает образы во flash.
3. `idf.py monitor` — последовательный монитор на 115200 бод: показывает логи и декодирует адреса в стектрейсах при падениях.

---

## Установка тулчейна

### Linux (Debian / Ubuntu)

**1. Системные пакеты**

```bash
sudo apt-get update
sudo apt-get install -y git wget flex bison gperf python3 python3-pip python3-venv \
    cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
```

**2. ESP-IDF**

```bash
mkdir -p ~/esp
cd ~/esp
git clone -b release/v5.5 --recursive https://github.com/espressif/esp-idf.git
```

`release/v5.5` — ветка стабильного релиза с исправлениями. Можно указать конкретный тег (список: <https://github.com/espressif/esp-idf/releases>).

**3. Компилятор и Python-окружение**

```bash
cd ~/esp/esp-idf
./install.sh esp32
```

Скрипт скачает тулчейн только для ESP32 в `~/.espressif/` (около 1–2 ГБ вместе с окружением).

**4. Переменные окружения**

Эту команду нужно выполнять в каждом новом терминале перед работой с проектом:

```bash
. ~/esp/esp-idf/export.sh
```

Удобно завести алиас в `~/.bashrc`:

```bash
alias get_idf='. $HOME/esp/esp-idf/export.sh'
```

Проверка:

```bash
idf.py --version
xtensa-esp-elf-gcc --version
```

**5. Доступ к последовательному порту**

```bash
sudo usermod -aG dialout $USER
```

После этого нужно перелогиниться (или перезагрузиться). Драйверы CP210x и CH34x в ядре Linux уже есть, плата появится как `/dev/ttyUSB0`:

```bash
ls /dev/ttyUSB*        # или: dmesg | tail
```

### macOS

```bash
brew install cmake ninja dfu-util python3
```

Дальше шаги 2–4 как для Linux. Порт будет называться `/dev/cu.usbserial-*` или `/dev/cu.SLAB_USBtoUART`. Для CH340 на старых версиях macOS может понадобиться драйвер WCH.

### Windows

1. Установить драйвер USB-UART: [CP210x](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) или [CH340/CH9102](https://www.wch-ic.com/downloads/CH341SER_EXE.html) — смотря какой чип стоит на плате.
2. Установить ESP-IDF через официальный установщик: <https://dl.espressif.com/dl/esp-idf/> (ESP-IDF Installation Manager), выбрать версию v5.5.
3. Работать из ярлыка **ESP-IDF PowerShell / CMD**, в нём окружение уже настроено. Порт — `COMx` (см. Диспетчер устройств).

### VS Code (опционально)

Расширение **ESP-IDF** (`espressif.esp-idf-extension`) умеет ставить тулчейн само и даёт кнопки build/flash/monitor. Для подсветки кода через clangd в проекте лежит `.clangd` — он заработает после первой сборки (`build/compile_commands.json`).

---

## Сборка и прошивка

Все команды выполняются из корня проекта после `. ~/esp/esp-idf/export.sh`.

### 1. Настройка Wi-Fi

```bash
idf.py menuconfig
```

Раздел **ESP32 Seed**:

| Параметр | По умолчанию | Описание |
|----------|--------------|----------|
| Wi-Fi SSID | *(пусто)* | Сеть 2.4 GHz для подключения. **Если пусто — плата поднимает свою точку доступа.** |
| Wi-Fi password | *(пусто)* | Пароль сети. |
| SoftAP SSID | `ESP32-seed` | Имя собственной точки доступа. |
| SoftAP password | `esp32seed` | Пароль точки доступа (не короче 8 символов, пусто — открытая сеть). |
| Status LED GPIO | `2` | Светодиод, которым мигает задача FreeRTOS. `-1` — выключить. |

Сохранить — `S`, выйти — `Q`. Настройки попадают в `sdkconfig`.

### 2. Сборка

```bash
idf.py build
```

Первая сборка идёт несколько минут (собирается весь ESP-IDF), последующие — секунды.

### 3. Прошивка и монитор

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

Выход из монитора — `Ctrl+]`.

Если прошивка не стартует с ошибкой `Failed to connect to ESP32: Wrong boot mode detected`: зажать кнопку **BOOT** (IO0), нажать и отпустить **EN**, запустить `flash`, отпустить **BOOT**, когда пойдёт запись.

### 4. Проверка

В мониторе появится адрес страницы:

- режим STA (задан SSID): `wifi: Connected, open http://192.168.x.x/`
- режим SoftAP: `wifi: SoftAP "ESP32-seed" started, open http://192.168.4.1/` — подключитесь к сети `ESP32-seed` с паролем `esp32seed`.

Откройте адрес в браузере — должна отобразиться страница «Hello from ESP32!». Светодиод на GPIO2 мигает раз в полсекунды — значит, планировщик FreeRTOS работает.

> На плате Espressif ESP32-DevKitC пользовательского светодиода нет (только индикатор питания), на клонах DevKit V1 синий светодиод обычно висит на GPIO2.

### Полезные команды

| Команда | Что делает |
|---------|------------|
| `idf.py build` | Собрать проект |
| `idf.py -p PORT flash` | Прошить |
| `idf.py -p PORT monitor` | Открыть монитор порта |
| `idf.py -p PORT erase-flash` | Полностью очистить flash (включая NVS с настройками Wi-Fi) |
| `idf.py menuconfig` | Конфигурация проекта и ESP-IDF |
| `idf.py size` | Размер прошивки по секциям |
| `idf.py fullclean` | Удалить `build/` целиком |

### Ручная прошивка через esptool

Если нужно прошить готовые бинарники без `idf.py` (например на другой машине), вот карта памяти для ESP32:

| Адрес | Файл |
|-------|------|
| `0x1000` | `build/bootloader/bootloader.bin` |
| `0x8000` | `build/partition_table/partition-table.bin` |
| `0x10000` | `build/esp32_seed.bin` |

```bash
cd build
python -m esptool --chip esp32 -p /dev/ttyUSB0 -b 460800 write_flash @flash_args
```

Файл `build/flash_args` генерируется при сборке и содержит адреса и параметры flash.

---

## Частые проблемы

| Симптом | Причина / решение |
|---------|-------------------|
| `idf.py: command not found` | Не выполнен `. ~/esp/esp-idf/export.sh` в текущем терминале. |
| `Permission denied: '/dev/ttyUSB0'` | Пользователь не в группе `dialout` — см. установку, шаг 5. |
| Порт `/dev/ttyUSB0` не появляется | Кабель только для зарядки (без линий данных) — взять другой. На Ubuntu с CH340 порт может забирать `brltty`: `sudo apt remove brltty`. |
| `Wrong boot mode detected` | Прошивать с зажатой кнопкой BOOT (см. выше). |
| Бесконечные `Disconnected, reconnecting...` | Неверный SSID/пароль или сеть 5 GHz. |
| `Brownout detector was triggered` | Не хватает питания от USB-порта — подключить в другой порт или через активный хаб. |
