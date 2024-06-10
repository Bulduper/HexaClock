### Other modifications
To `wled.h` add `WLED_GLOBAL int8_t currentLedmap _INIT(-1);` (line 240)

And to `wled.cpp` add `currentLedmap = loadLedmap;` (line 177)

Copy `platformio_override.ini` to the main folder (next to `platformio.ini`) and choose your board.
