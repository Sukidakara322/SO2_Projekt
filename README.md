Projekt 1: Problem Jedzących Filozofów

Spis treści:
1. Opis problemu
2. Instrukcja uruchomenia
3. Struktura i omówienie projektu
4. Wątki i ich reprezentacja
5. Sekcje krytyczne i rozwiązanie


Opis problemu
Problem jedzących filozofów to klasyczny problem synchronizacji w programowaniu współbieżnym. Mamy N filozofów siedzących przy okrągłym stole, gdzie każdy filozof ma po lewej i prawej stronie pałeczki (lub widelce). Każdy filozof przechodzi cyklicznie przez stany:

Myślenie (THINKING),
Bycie glodnym (HUNGRY),
Jedzenie (EATING).

Kluczowe jest takie zsynchronizowanie dostępu do wspólnych zasobów (pałeczek), aby:
Nigdy nie doszło do trwałego zakleszczenia (deadlock), w którym wszyscy filozofowie czekają na zasoby i żaden nie może jeść
Każdy filozof miał szansę jeść

W tym projekcie rozwiązanie tego problemu zostało zrealizowane z użyciem:

Pthreadów do tworzenia i obsługi wątków,
Własnej implementacji semaforów (opartej na pthread_mutex_t i pthread_cond_t),
Mechanizmu „testowania” (funkcja test(i)) decydującego, czy filozof może przejść w stan JEDZENIA.


Instrukcja uruchomenia
Projekt zostal zrealizowany w za pomocą Visual Studio, dlatego plik wykonywalny tego projektu został też załączony w tym repozytorium.
W katalogu z plikiem .exe również powinny być umieszczone pliki .dll (pthread).
Podczas uruchomenia pliku wykonywalnego do argumentów trzeba podać tylko liczbę filozofów


Struktura i omówienie projektu
W repozytorium znajdują się następujące główne pliki:

Semaphore.h / Semaphore.cpp
Zawierają implementację prostego semafora opartego na pthread_mutex_t i pthread_cond_t

DiningPhilosophers.h / DiningPhilosophers.cpp
Zawierają kluczowe funkcje realizujące logikę problemu (pickup(), putdown(), test()) oraz tablice z aktualnymi stanami filozofów

main.cpp
Główny plik z funkcją main(), który tworzy wątki, inicjuje struktury danych i uruchamia program

CompatPthread.h
Plik pomocniczy, który definiuje _TIMESPEC_DEFINED i dołącza pthread.h, co zapobiega błędom redefinicji timespec w MSVC


Wątki i ich reprezentacja
Wątek główny (main)
Odpowiada za odczyt liczby filozofów z argumentów wiersza poleceń, zainicjowanie tablic (stanów filozofów, semaforów) i utworzenie N wątków filozofów.

Wątki filozofów (każdy reprezentowany przez pthread_t)
Każdy wątek filozofa działa w pętli nieskończonej, przechodząc przez stany:
  Myślenie (THINKING) – symulowane
  Próba podniesienia pałeczek (pickup(i)) – tu może nastąpić blokada, jeśli sąsiad je
  Jedzenie (EATING) – ponownie symulowane przez krótki czas uśpienia wątku
  Odkładanie pałeczek (putdown(i)) – zwalnia pałeczki i potencjalnie umożliwia jedzenie sąsiadom


Sekcje krytyczne i rozwiązanie

Sekcje krytyczne
Zmiana stanu filozofa (z THINKING na HUNGRY lub EATING, i odwrotnie)
Sprawdzanie warunku czy sąsiedzi nie jedzą w funkcji test(i)
Każda z tych operacji jest wykonywana w sekcji krytycznej chronionej globalnym pthread_mutex_t

Rozwiązanie
Tablica stanów stateArray[] przechowuje aktualny stan każdego filozofa.
Semafor prywatny blokuje danego filozofa, gdy nie może jeść (sąsiedzi jedzą).
pickup(i) ustawia stan filozofa na HUNGRY, wywołuje test(i) i jeśli filozóf nie może jeść, wątek blokuje się na semaforze semPhilosopher[i].
putdown(i) ustawia stan na THINKING i wywołuje test(left(i)) i test(right(i)), aby umożliwić sąsiadom przejście w stan EATING, jeśli to możliwe.
Unikanie deadlocka zapewnia się przez to, że każdy filozof przechodzi przez test i może blokować się na semaforze tylko w stanie HUNGRY, a także przez „monitorowe” wywołania test() sąsiadów po zwolnieniu pałeczek.

Wykonawca: Aliaksandr Afanasyeu 273018.
