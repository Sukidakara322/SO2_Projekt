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
