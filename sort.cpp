// compilare: g++ -std=c++20 -O3 sort.cpp -o sort
// rulare: ./sort_bench <algoritm> <numar_rulari>

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

using Vec = std::vector<long long>;

// algoritm bubble sort
void bubble_sort(Vec& vector) {
    for (size_t i = 0; i + 1 < vector.size(); ++i) {
        bool schimbare = false;
        for (size_t j = 0; j + 1 < vector.size() - i; ++j) {
            if (vector[j] > vector[j + 1]) {
                std::swap(vector[j], vector[j + 1]);
                schimbare = true;
            }
        }
        if (!schimbare) break;
    }
}

// algoritm heap sort
void heap_sort(Vec& vector) {
    std::make_heap(vector.begin(), vector.end());
    std::sort_heap(vector.begin(), vector.end());
}

// algoritm selection sort
void selection_sort(Vec& vector) {
    for (size_t i = 0; i + 1 < vector.size(); ++i) {
        size_t minim = i;
        for (size_t j = i + 1; j < vector.size(); ++j) {
            if (vector[j] < vector[minim])
                minim = j;
        }
        std::swap(vector[i], vector[minim]);
    }
}

// algoritm insertion sort
void insertion_sort(Vec& vector) {
    for (size_t i = 1; i < vector.size(); ++i) {
        long long element_curent = vector[i];
        size_t pozitie = i;
        while (pozitie > 0 && vector[pozitie - 1] > element_curent) {
            vector[pozitie] = vector[pozitie - 1];
            --pozitie;
        }
        vector[pozitie] = element_curent;
    }
}

// functie recursiva pentru merge sort
void interclasare_recursiva(Vec& vector, Vec& temporar, size_t stanga, size_t dreapta) {
    if (dreapta - stanga <= 1) return;
    size_t mijloc = (stanga + dreapta) / 2;
    interclasare_recursiva(vector, temporar, stanga, mijloc);
    interclasare_recursiva(vector, temporar, mijloc, dreapta);

    size_t i = stanga, j = mijloc, k = stanga;
    while (i < mijloc && j < dreapta)
        temporar[k++] = (vector[i] <= vector[j]) ? vector[i++] : vector[j++];
    while (i < mijloc) temporar[k++] = vector[i++];
    while (j < dreapta) temporar[k++] = vector[j++];

    for (size_t p = stanga; p < dreapta; ++p)
        vector[p] = temporar[p];
}

// algoritm merge sort
void merge_sort(Vec& vector) {
    Vec temporar(vector.size());
    interclasare_recursiva(vector, temporar, 0, vector.size());
}

// functie recursiva pentru quick sort
void sortare_rapida_recursiva(Vec& vector, size_t stanga, size_t dreapta) {
    if (stanga >= dreapta) return;
    long long pivot = vector[(stanga + dreapta) / 2];
    size_t i = stanga, j = dreapta;
    while (i <= j) {
        while (vector[i] < pivot) ++i;
        while (vector[j] > pivot) --j;
        if (i <= j) {
            std::swap(vector[i], vector[j]);
            ++i;
            if (j > 0) --j;
        }
    }
    if (stanga < j) sortare_rapida_recursiva(vector, stanga, j);
    if (i < dreapta) sortare_rapida_recursiva(vector, i, dreapta);
}

// algoritm quick sort
void quick_sort(Vec& vector) {
    if (!vector.empty())
        sortare_rapida_recursiva(vector, 0, vector.size() - 1);
}

// selector de algoritmi
using FunctieSortare = void(*)(Vec&);
FunctieSortare alege_algoritm(const std::string& nume) {
    if (nume == "bubble") return bubble_sort;
    if (nume == "selection") return selection_sort;
    if (nume == "insertion") return insertion_sort;
    if (nume == "merge") return merge_sort;
    if (nume == "quick") return quick_sort;
    if (nume == "heap") return heap_sort;
    return nullptr;
}

// incarcare lista din csv
Vec incarca_lista(const std::string& cale) {
    Vec vector;
    long long valoare;
    std::ifstream fisier(cale);
    while (fisier >> valoare)
        vector.push_back(valoare);
    return vector;
}

// calculeaza numarul de secunde intre doua momente
double secunde(auto start, auto stop) {
    return std::chrono::duration<double>(stop - start).count();
}

// structura pentru fisiere de test
struct FisierTest {
    std::string tip_lista;
    long dimensiune;
    std::string cale;
};

// ordonare tipuri de liste
std::map<std::string, int> ordine_tip_lista {
    {"inversat", 0}, {"sortat", 1}, {"aproape_sortat", 2}, {"aleator", 3}, {"plat", 4}
};

// denumiri afisate frumos
std::map<std::string, std::string> eticheta_tip_lista {
    {"inversat", "Inversat"}, {"sortat", "Sortat"},
    {"aproape_sortat", "Aproape sortat"}, {"aleator", "Aleator"}, {"plat", "Plat"}
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Utilizare: " << argv[0] << " <algoritm> <numar_rulari>\n";
        return 1;
    }

    std::string nume_algoritm = argv[1];
    FunctieSortare sortare = alege_algoritm(nume_algoritm);
    if (!sortare) {
        std::cerr << "Algoritm necunoscut\n";
        return 1;
    }

    int numar_rulari = std::stoi(argv[2]);
    std::vector<FisierTest> fisiere_test;

    namespace fs = std::filesystem;
    for (const auto& element : fs::directory_iterator("liste")) {
        if (!element.is_regular_file() || element.path().extension() != ".csv") continue;
        auto nume = element.path().stem().string();
        auto pozitie = nume.find_last_of('_');
        std::string tip = nume.substr(0, pozitie);
        long dimensiune = std::stol(nume.substr(pozitie + 1));
        fisiere_test.push_back({tip, dimensiune, element.path().string()});
    }

    std::sort(fisiere_test.begin(), fisiere_test.end(), [](auto& a, auto& b) {
        if (ordine_tip_lista[a.tip_lista] != ordine_tip_lista[b.tip_lista])
            return ordine_tip_lista[a.tip_lista] < ordine_tip_lista[b.tip_lista];
        return a.dimensiune < b.dimensiune;
    });

    std::ofstream rezultate("rezultate.csv");
    std::ofstream rezultate_medie("rezultate2.csv");
    if (!rezultate || !rezultate_medie) {
        std::cerr << "Eroare la deschiderea fisierelor de iesire\n";
        return 1;
    }

    rezultate << "lista,run,timp_sec\n";
    rezultate_medie << "lista,medie_timp\n";

    for (const auto& fisier : fisiere_test) {
        if ((nume_algoritm == "bubble" || nume_algoritm == "selection" || nume_algoritm == "insertion") && fisier.dimensiune > 100000) {
            std::cout << "[Sarit] " << fisier.cale << "\n";
            continue;
        }

        double suma_timpi = 0;
        std::string denumire_lista = eticheta_tip_lista[fisier.tip_lista] + " " + std::to_string(fisier.dimensiune);

        for (int r = 1; r <= numar_rulari; ++r) {
            auto start = std::chrono::high_resolution_clock::now();
            int pid = fork();
            if (pid == 0) {
                Vec vector = incarca_lista(fisier.cale);
                sortare(vector);
                _exit(0);
            }
            int status;
            waitpid(pid, &status, 0);
            auto stop = std::chrono::high_resolution_clock::now();

            double durata = secunde(start, stop);
            rezultate << denumire_lista << ',' << r << ',' << durata << '\n';
            suma_timpi += durata;

            // afisare progres
            int procent = static_cast<int>(100.0 * r / numar_rulari + 0.5);
            std::cout << "\rProcesare " << std::setw(20) << std::left << denumire_lista << ": " << procent << "%   " << std::flush;
        }

        rezultate_medie << denumire_lista << ',' << (suma_timpi / numar_rulari) << '\n';
        std::cout << "\rProcesare " << std::setw(20) << std::left << denumire_lista << ": 100%   \n";
    }

    std::cout << "Testare finalizata -> rezultate.csv si rezultate2.csv\n";
    return 0;
}
