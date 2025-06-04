# Projekt 1: Problem Jedzących Filozofów

## Spis treści
1. [Opis problemu](#opis-problemu)
2. [Instrukcja uruchomienia](#instrukcja-uruchomienia)
3. [Struktura i omówienie projektu](#struktura-i-omówienie-projektu)
4. [Wątki i ich reprezentacja](#wątki-i-ich-reprezentacja)
5. [Sekcje krytyczne i rozwiązanie](#sekcje-krytyczne-i-rozwiązanie)
6. [Autor](#autor)

---

## Opis problemu

**Problem jedzących filozofów** to klasyczny problem synchronizacji w programowaniu współbieżnym.

Mamy `N` filozofów siedzących przy okrągłym stole. Każdy z nich ma po lewej i prawej stronie pałeczki (lub widelce). Filozof cyklicznie przechodzi przez trzy stany:

- `THINKING` – myślenie,
- `HUNGRY` – oczekiwanie na możliwość jedzenia,
- `EATING` – jedzenie.

### Celem rozwiązania jest zapewnienie, że:
- **nie dochodzi do zakleszczenia** (ang. *deadlock*),
- **każdy filozof ma szansę jeść** (brak głodzenia – *starvation*).

### W projekcie zastosowano:
- wątki POSIX (`pthreads`) do realizacji równoległości,
- **własną implementację semaforów** (na bazie `pthread_mutex_t` i `pthread_cond_t`),
- funkcję `test(i)`, która decyduje, czy filozof może zacząć jeść.

---

## Instrukcja uruchomienia

- Projekt został wykonany w **Visual Studio** (Windows).
- W repozytorium znajduje się gotowy plik `.exe`.
- W katalogu z plikiem `.exe` należy umieścić wymagane pliki `.dll` biblioteki pthread.
- Aby uruchomić program, należy podać **liczbę filozofów** jako argument:


---

## Struktura i omówienie projektu

| Plik | Opis |
|------|------|
| `main.cpp` | Główna funkcja `main()`, tworzy wątki i uruchamia logikę programu |
| `DiningPhilosophers.cpp / .h` | Logika działania filozofów – obsługa stanów, synchronizacja, funkcje `pickup()`, `putdown()` i `test()` |
| `Semaphore.cpp / .h` | Własna implementacja semafora z użyciem `pthread_mutex_t` i `pthread_cond_t` |
| `CompatPthread.h` | Plik kompatybilności dla systemu Windows, zabezpiecza przed konfliktem definicji `timespec` |

---

## Wątki i ich reprezentacja

### Wątek główny (`main`)
- Odczytuje liczbę filozofów z argumentów,
- Inicjalizuje wszystkie struktury danych,
- Tworzy `N` wątków filozofów.

### Wątki filozofów (`pthread_t`)
Każdy filozof działa w osobnym wątku i przechodzi w pętli nieskończonej przez stany:

1. `THINKING` – symulowane przez opóźnienie (`sleep`),
2. `HUNGRY` – próba zdobycia pałeczek (`pickup(i)`),
3. `EATING` – jedzenie (również opóźnienie),
4. `putdown(i)` – zwolnienie pałeczek i sprawdzenie, czy sąsiedzi mogą teraz jeść.

---

## Sekcje krytyczne i rozwiązanie

### Sekcje krytyczne

Operacje wykonywane wewnątrz sekcji krytycznych (chronionych przez `stateMutex`):

- Zmiana stanu filozofa (`THINKING`, `HUNGRY`, `EATING`),
- Sprawdzanie, czy sąsiedzi jedzą (`test(i)`),
- Budzenie innych filozofów (sygnalizacja semafora).

### Rozwiązanie

- `stateArray[]` – dynamiczna tablica przechowująca stan każdego filozofa.
- `semPhilosopher[i]` – prywatny semafor każdego filozofa, pozwala na czekanie, gdy nie może on jeść.
- `pickup(i)`:
  - Ustawia filozofa jako `HUNGRY`,
  - Wywołuje `test(i)` – jeśli może jeść, przechodzi do `EATING`; jeśli nie – czeka.
- `putdown(i)`:
  - Zmienia stan filozofa na `THINKING`,
  - Wywołuje `test()` dla lewego i prawego sąsiada – jeśli któryś z nich był głodny i teraz może jeść, zostaje odblokowany.
- Unikanie zakleszczenia i głodzenia jest osiągnięte dzięki kolejności operacji oraz monitorowemu zarządzaniu semaforami i stanami.

---

## Autor

**Aliaksandr Afanasyeu**  
Numer indeksu: **273018**

---
---

## 📡 Projekt 2: Wielowątkowy Serwer Czatu

### Spis treści
1. [Opis problemu](#opis-problemu-1)
2. [Instrukcja uruchomienia](#instrukcja-uruchomienia-1)
3. [Struktura i omówienie projektu](#struktura-i-omówienie-projektu-1)
4. [Wątki i ich reprezentacja](#wątki-i-ich-reprezentacja-1)
5. [Sekcje krytyczne i rozwiązanie](#sekcje-krytyczne-i-rozwiązanie-1)

---

## Opis problemu

Projekt polega na stworzeniu serwera (TCP), który obsługuje **czat wielu klientów** w czasie rzeczywistym. Każdy klient po połączeniu się:

- podaje swój **nickname**,
- może wysyłać wiadomości, które trafiają do wszystkich innych klientów.

Serwer działa **wielowątkowo** – każdy klient obsługiwany jest w oddzielnym wątku, a przekazywanie wiadomości odbywa się przez wspólną kolejkę z synchronizacją.

---

## Instrukcja uruchomienia

1. Zbuduj projekt

2. Uruchom serwer z podaniem portu jako argument:
   ```bash
   Project_SO2_Task_2.exe 9000
   ```

3. Połącz się z drugiego terminala lub narzędzia (np. PuTTY):

   - **Hostname**: `127.0.0.1`
   - **Port**: `9000`
   - **Connection type**: `Raw`

   Można też użyć `ncat`:
   ```bash
   ncat 127.0.0.1 9000
   ```

4. Aby się **rozłączyć po stronie klienta**, wpisz:
   ```bash
   /quit
   ```

5. Aby **zatrzymać serwer i rozłączyć wszystkich klientów**, użyj:
   ```
   CTRL + C
   ```

---

## Struktura i omówienie projektu

| Plik | Opis |
|------|------|
| `main.cpp`           | Odczyt portu, inicjalizacja Winsock, uruchomienie serwera |
| `server.cpp / .h`    | Właściwa logika serwera: połączenia, wątki klientów, dispatcher |
| `message.h`          | Struktura `Message` zawierająca nadawcę i treść |
| `spinlock.h`         | Prosty `SpinLock` oparty na `std::atomic_flag` (synchronizacja) |

---

## Wątki i ich reprezentacja

| Wątek | Rola |
|-------|------|
| `main`              | Uruchamia serwer i nasłuchuje połączeń |
| `client_thread()`   | Obsługuje jednego klienta: nickname, odbiór wiadomości, `/quit` |
| `dispatcher()`      | Centralny wątek – przekazuje wiadomości z kolejki do wszystkich innych klientów |

---

## Sekcje krytyczne i rozwiązanie

### Sekcje krytyczne

Chronione przez własną implementację `SpinLock`:

- `clients`: lista połączeń z klientami,
- `nicks`: mapa SOCKET → nickname,
- `msg_queue`: wspólna kolejka wiadomości.

### Rozwiązanie

- `SpinLock` używa `std::atomic_flag` do stworzenia lekkiego mechanizmu synchronizacji bez użycia `mutex`.
- `dispatcher` działa w tle, obsługując wiadomości, które pojawiają się w `msg_queue`.

---
## Autorzy:

**Aliaksandr Afanasyeu**  
Numer indeksu: **273018**,
**Dzmitry Kuzma**
Numer indeksu: **276246**

---
