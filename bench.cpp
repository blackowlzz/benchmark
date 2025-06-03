#include <iostream>
#include <cmath>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#include <winreg.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <fstream>
#endif

using namespace std;
using namespace chrono;

atomic<long long> totale(0);
const int DURATA = 30;

void cpuload(steady_clock::time_point inizio) {
    long long count = 0;
    double tmp = 0;

    while (duration_cast<seconds>(steady_clock::now() - inizio).count() < DURATA) {
        double x = 0;
        for (int j = 1; j < 100; ++j)
            x += sin(count * j) * cos(j) + log(j + 1) * sqrt(j + count);
        tmp += x;
        count++;
    }

    totale += count;
}

string cpuName() {
#if defined(_WIN32)
    char cpu[128];
    DWORD sz = sizeof(cpu);
    RegGetValueA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                 "ProcessorNameString", RRF_RT_REG_SZ, nullptr, &cpu, &sz);
    return string(cpu);
#elif defined(__linux__)
    ifstream f("/proc/cpuinfo");
    string l;
    while (getline(f, l)) {
        if (l.find("model name") != string::npos)
            return l.substr(l.find(":") + 2);
    }
#elif defined(__APPLE__)
    char buf[128];
    size_t sz = sizeof(buf);
    sysctlbyname("machdep.cpu.brand_string", &buf, &sz, nullptr, 0);
    return string(buf);
#endif
    return "??";
}

int main() {
    
    cout << "Starting CPU load...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    unsigned int n = thread::hardware_concurrency();
    if (n == 0) n = 4;

    vector<thread> th;
    auto inizio = steady_clock::now();

    for (unsigned int i = 0; i < n; ++i)
        th.emplace_back(cpuload, inizio);
    for (auto& t : th) t.join();

    auto fine = steady_clock::now();
    double tempo = duration_cast<duration<double>>(fine - inizio).count();
    double punteggio = (totale.load() / tempo) * 100;

    cout << "CPU: " << cpuName() << "\n";
    cout << "Time: " << tempo << "s\n";
    cout << "Calcs: " << totale.load() << "\n";
    cout << "CPU Score: " << punteggio << "\n";

    cin.get();
    return 0;
}
