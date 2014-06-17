STM32F4_AccelerometersProject
=============================

Projekt ma na celu stworzenie jednolitego API dla STM32 do obsługi różnych modeli akcelerometrów przy pomocy SPI oraz I2C.

### Dostępne funkcje

    struct Connection Accelerometer_Init(enum Accel accel, enum Interface iface);
    
Funkcja inicjalizująca połączenie. Jako parametry przyjmuje model danego akcelerometru (`LIS` bądź `ADXL`) oraz interfejs po którym prowadzona ma być komunikacja (`SPI` lub `I2C`). Funkcja zwraca strukturę przechowującą te dane, która jest wykorzystywana w poniższej funkcji, a także informacje o powodzeniu (bądź nie) nawiązania komunikacji.
    
    struct Axes Accelerometer_readAxes(struct Connection conn);

Funkcja zwracająca strukturę zawierającą wartości przyspieszenia dla poszczególnych osi.

### Cele na przyszłość

- Wykrywanie błędów w komunikacji
- Normalizacja wyników dla wszystkich typów akcelerometrów
