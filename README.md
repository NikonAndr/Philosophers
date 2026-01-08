1. Synchronizacja wątków

mutex + unique_lock

2. Zakleszczenie (deadlock)

Nie ma deadlocka, bo:

nikt nie bierze „jednego widelca”

albo bierzesz oba naraz

albo czekasz

3. Zagłodzenie (starvation)

Nie ma starvation, bo:

po oddaniu widelców robisz notify_all()

wszyscy czekający dostają szansę

scheduler + losowe czasy = sprawiedliwość

4. Zmienna warunkowa

✅ użyta poprawnie:

z predykatem

pod mutexem

bez busy-waitingu

kompilacja na Ubuntu 

sudo apt install g++ libncurses-dev
g++ -std=c++20 main.cpp -pthread -lncurses -o main
