#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <random>
#include <ncurses.h>

// Philosophers state
enum class State {
    THINKING,
    HUNGRY,
    EATING
};

// Global state 
std::mutex mtx;                     
std::condition_variable cv;         

std::vector<State> states;          
std::vector<bool> forks;            
std::vector<int> actionDuration;    // NEW
std::vector<int> actionElapsed;     // NEW
int N;                              

int leftFork(int i) {
    return i;
}

int rightFork(int i) {
    return (i + 1) % N;
}

bool canEat(int i) {
    return forks[leftFork(i)] && forks[rightFork(i)];
}

// Philosopher func
void philosopher(int id) {
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> thinkTime(2, 5);
    std::uniform_int_distribution<> eatTime(1, 3);

    while (true) {
        // THINKING
        int t = thinkTime(gen);
        {
            std::lock_guard<std::mutex> lock(mtx);
            states[id] = State::THINKING;
            actionDuration[id] = t;
            actionElapsed[id] = 0;
        }

        for (int i = 0; i < t; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::lock_guard<std::mutex> lock(mtx);
            actionElapsed[id]++;
        }

        // HUNGRY
        std::unique_lock<std::mutex> lock(mtx);
        states[id] = State::HUNGRY;

        // WAIT UNTIL CAN EAT
        cv.wait(lock, [&]() {
            return canEat(id);
        });

        // TAKE FORKS
        forks[leftFork(id)] = false;
        forks[rightFork(id)] = false;

        int e = eatTime(gen);
        states[id] = State::EATING;
        actionDuration[id] = e;
        actionElapsed[id] = 0;

        lock.unlock();

        // EATING
        for (int i = 0; i < e; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::lock_guard<std::mutex> lock(mtx);
            actionElapsed[id]++;
        }

        // PUT FORKS BACK
        lock.lock();
        forks[leftFork(id)] = true;
        forks[rightFork(id)] = true;
        states[id] = State::THINKING;
        lock.unlock();

        cv.notify_all();
    }
}

// Render Thread
void render() {
    const int BAR_WIDTH = 20;

    while (true) {
        std::vector<State> localStates;
        std::vector<bool> localForks;
        std::vector<int> localDur;
        std::vector<int> localElap;

        {
            std::lock_guard<std::mutex> lock(mtx);
            localStates = states;
            localForks  = forks;
            localDur    = actionDuration;
            localElap   = actionElapsed;
        }

        clear();
        mvprintw(0, 0, "Problem ucztujacych filozofow");

        for (int i = 0; i < N; i++) {
            const char* stateStr;
            switch (localStates[i]) {
                case State::THINKING: stateStr = "THINKING"; break;
                case State::HUNGRY:   stateStr = "HUNGRY";   break;
                case State::EATING:   stateStr = "EATING";  break;
            }

            int filled = 0;
            if (localDur[i] > 0) {
                filled = (localElap[i] * BAR_WIDTH) / localDur[i];
                if (filled > BAR_WIDTH) filled = BAR_WIDTH;
            }

            mvprintw(
                2 + i,
                0,
                "Filozof %d | %-8s | L:%c R:%c | [",
                i,
                stateStr,
                localForks[leftFork(i)]  ? 'O' : 'X',
                localForks[rightFork(i)] ? 'O' : 'X'
            );

            for (int j = 0; j < BAR_WIDTH; j++) {
                if (j < filled) addch('#');
                else addch(' ');
            }
            addch(']');
        }

        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Podaj liczbe filozofow (>= 5)\n";
        return 1;
    }

    N = std::stoi(argv[1]);
    if (N < 5) {
        std::cerr << "Liczba filozofow musi byc >= 5\n";
        return 1;
    }

    // State init
    states.resize(N, State::THINKING);
    forks.resize(N, true);
    actionDuration.resize(N, 0);   // NEW
    actionElapsed.resize(N, 0);    // NEW

    // ncurses init
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    // Render thread
    std::thread renderThread(render);

    // Start philosophers
    std::vector<std::thread> threads;
    for (int i = 0; i < N; i++) {
        threads.emplace_back(philosopher, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    renderThread.join();
    endwin();

    return 0;
}

