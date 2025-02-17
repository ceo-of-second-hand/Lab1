//garbage.cpp
#include "Garbage.h"

// вектор для зберігання вільних місць в оперативній пам'яті
std::vector<GarbageSpace> adminGarbage;
std::vector<GarbageSpace> paymentGarbage;


// функція для отримання вільного простору
int getFreeSpace(std::vector<GarbageSpace>& garbageContainer) {
    if (garbageContainer.empty()) {  // якщо вектор порожній
        return -1; 
    }

    // отримуємо елемент вектору, що є вільним місцем
    int freeId = garbageContainer.back().next_free;
    garbageContainer.pop_back();  // видаляємо його з вектору

    return freeId;  // повертаємо вільний ID
}

// функція для додавання нового вільного місця 
void addFreeSpace(std::vector<GarbageSpace>& garbageContainer, int localId) {
    // Створюємо нове вільне місце
    GarbageSpace newFreeEntry;
    newFreeEntry.next_free = localId;  // прив'язуємо нове вільне місце до списку

    // додаємо його в кінець вектору
    garbageContainer.push_back(newFreeEntry);
}


void loadAdminGarbageFromFile(const std::string& filename) {
    FILE* garbageDB = nullptr;
    fopen_s(&garbageDB, filename.c_str(), "rb");

    if (!garbageDB) {
        std::cerr << "Error: Could not open " << filename << " for reading!" << std::endl;
        return;
    }

    adminGarbage.clear();  // очищаємо контейнер сміття адмінів

    GarbageSpace entry;
    while (fread(&entry, sizeof(GarbageSpace), 1, garbageDB)) {
        adminGarbage.push_back(entry);
    }

    fclose(garbageDB);
}

void loadPayoutGarbageFromFile(const std::string& filename) {
    FILE* garbageDB = nullptr;
    fopen_s(&garbageDB, filename.c_str(), "rb");

    if (!garbageDB) {
        std::cerr << "Error: Could not open " << filename << " for reading!" << std::endl;
        return;
    }

	paymentGarbage.clear();  // очищаємо контейнер виплат, щоб не виниколо проблем з дублюванням

    GarbageSpace entry;
    while (fread(&entry, sizeof(GarbageSpace), 1, garbageDB)) {
        paymentGarbage.push_back(entry);
    }

    fclose(garbageDB);
}

// записуємо сміття адмінів у файл
void writeAdminGarbageToFile(const std::string& filename) {
    FILE* garbageDB = nullptr;
    fopen_s(&garbageDB, filename.c_str(), "wb");

    if (!garbageDB) {
        std::cerr << "Error: Could not open " << filename << " for writing!" << std::endl;
        return;
    }

    for (const auto& entry : adminGarbage) {
        fwrite(&entry, sizeof(GarbageSpace), 1, garbageDB);
    }

    fclose(garbageDB);
}

void writePayoutGarbageToFile(const std::string& filename) {
    FILE* garbageDB = nullptr;
    fopen_s(&garbageDB, filename.c_str(), "wb");

    if (!garbageDB) {
        std::cerr << "Error: Could not open " << filename << " for writing!" << std::endl;
        return;
    }

    for (const auto& entry : paymentGarbage) {
        fwrite(&entry, sizeof(GarbageSpace), 1, garbageDB);
    }

    fclose(garbageDB);
}


