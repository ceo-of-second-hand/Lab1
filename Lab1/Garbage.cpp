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

/*
// Function to remove garbage from admin and payment records
void removeGarbage() {
    // Remove garbage from admin records
    for (auto it = adminIden.begin(); it != adminIden.end(); ) {
        Admin admin = getAdmin(it->adminId);
        if (admin.deleted) {
            it = adminIden.erase(it); // Remove from index table
        }
        else {
            ++it;
        }
    }

    // Remove garbage from payment records
    FILE* paymentsDB = nullptr;
    fopen_s(&paymentsDB, "payments.bin", "rb+");
    if (!paymentsDB) {
        std::cerr << "Error: Could not open payments.bin for reading and writing!" << std::endl;
        return;
    }

    Payment payment;
    int localId = 1;
    while (fread(&payment, sizeof(Payment), 1, paymentsDB)) {
        if (payment.deleted) {
            // Update the previous payment's next_local_address to skip the deleted payment
            if (payment.next_local_address != 0) {
                Payment nextPayment = getPayment(payment.next_local_address);
                if (payment.local_id == getAdmin(payment.admin_id).first_payment) {
                    Admin admin = getAdmin(payment.admin_id);
                    admin.first_payment = nextPayment.local_id;
                    writeAdmin(admin, payment.admin_id - 1);
                }
                else {
                    Payment prevPayment = getPayment(getAdmin(payment.admin_id).first_payment);
                    while (prevPayment.next_local_address != payment.local_id) {
                        prevPayment = getPayment(prevPayment.next_local_address);
                    }
                    prevPayment.next_local_address = nextPayment.local_id;
                    writePayment(prevPayment, prevPayment.local_id - 1);
                }
            }
        }
        localId++;
    }

    fclose(paymentsDB);
}
*/
/*
// Function to update indexes after removing garbage
void updateIndexes() {
    // Update admin indexes
    for (size_t i = 0; i < adminIden.size(); ++i) {
        adminIden[i].adminId = i + 1;
        adminIden[i].adminAddress = i * sizeof(Admin);
    }

    // Write updated indexes to file
    writeIndexesToFile("adminIndex.bin");
}
*/
void removeAdminGarbage() {
    if (!adminGarbage.empty()) {
        int adminId = adminGarbage.back().next_free;  // Беремо ID адміністратора з кінця вектора
        adminGarbage.pop_back();  // Видаляємо його з вектору
        updateAdminAfterCompleteDeletion(adminId);  // Викликаємо функцію для оновлення даних
    }
}

// Функція для видалення одного елементу зі сміття виплат
void removePaymentGarbage() {
    if (!paymentGarbage.empty()) {
        int localId = paymentGarbage.back().next_free;  // Беремо ID виплати з кінця вектора
        paymentGarbage.pop_back();  // Видаляємо його з вектору
        updatePaymentAfterCompleteDeletion(localId);  // Викликаємо функцію для оновлення даних
    }
}