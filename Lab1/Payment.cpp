// payment.cpp
#include "Payment.h"
#include "Admin.h"
#include "Garbage.h"
#include <iostream>
#include <cstring>

void getPaymentInfo(int& admin_id, int& size, char* month) {
    std::cout << "Enter the admin ID: ";
    std::cin >> admin_id;
    std::cout << "Enter the payment size: ";
    std::cin >> size;
    std::cout << "Enter the payment month (mm-yyyy): ";
    std::cin >> month;
}

Payment getPayment(int localId) {
    Payment payment{};
    FILE* paymentsDB = nullptr;
    fopen_s(&paymentsDB, "payments.bin", "rb");

    if (!paymentsDB) {
        std::cerr << "Error: Could not open payments.bin for reading!" << std::endl;
        return payment;
    }

    fseek(paymentsDB, (localId - 1) * sizeof(Payment), SEEK_SET);
    fread(&payment, sizeof(Payment), 1, paymentsDB);
    fclose(paymentsDB);
    return payment;
}

void writePayment(Payment& payment, int position) {
    FILE* paymentsDB = nullptr;
    fopen_s(&paymentsDB, "payments.bin", "rb+");

    if (!paymentsDB) {
        std::cerr << "Error: Could not open payments.bin for writing!" << std::endl;
        return;
    }

    fseek(paymentsDB, position * sizeof(Payment), SEEK_SET);
    fwrite(&payment, sizeof(Payment), 1, paymentsDB);
    fclose(paymentsDB);
}

void addPayment(Payment& payment) {
    int freeSpace = getFreeSpace(paymentGarbage);
    FILE* paymentsDB = nullptr;
    fopen_s(&paymentsDB, "payments.bin", "rb+");

    if (!paymentsDB) {
        std::cerr << "Error: Could not open payments.bin for writing!" << std::endl;
        return;
    }

	if (freeSpace != -1) { // якщо є вільне місце
		fseek(paymentsDB, (freeSpace - 1) * sizeof(Payment), SEEK_SET); // переміщаємо покажчик файлу до вільного місця
		payment.local_id = freeSpace; // присвоюємо виплаті вільний ID
    }
	else { // якщо вільного місця немає
        fseek(paymentsDB, 0, SEEK_END);
        payment.local_id = ftell(paymentsDB) / sizeof(Payment) + 1;
    }

    fwrite(&payment, sizeof(Payment), 1, paymentsDB);
    fclose(paymentsDB);

    Admin admin = getAdmin(payment.admin_id);
    if (admin.num_of_payments == 0) {
        admin.first_payment = payment.local_id;
    }
    else {
        Payment lastPayment = getPayment(admin.last_payment);
        lastPayment.next_local_address = payment.local_id;
        writePayment(lastPayment, lastPayment.local_id - 1);
    }
    admin.last_payment = payment.local_id;
    admin.num_of_payments++;
    writeAdmin(admin, payment.admin_id - 1);
}

void deletePayment(int localId) {
    if (localId < 1) {
        std::cerr << "Error: Invalid localId!" << std::endl;
        return;
    }

    Payment payment = getPayment(localId);
    payment.deleted = true;
    writePayment(payment, localId - 1);

Admin admin = getAdmin(payment.admin_id);
if (admin.first_payment == localId) {
    admin.first_payment = payment.next_local_address;
}
else {
    Payment currentPayment = getPayment(admin.first_payment);  // отримуємо поточний платіж
    // цикл, в якому ми змінюємо ітеруємо, поки не знайдемо того, хто був перд тим, кого ми хочемо видалити
    while (currentPayment.next_local_address != localId) {
        currentPayment = getPayment(currentPayment.next_local_address);  // оновлюємо currentPayment
    }
    currentPayment.next_local_address = payment.next_local_address;  // оновлюємо наступний платіж
    writePayment(currentPayment, currentPayment.local_id - 1);  // записуємо зміни
    if (admin.last_payment == localId) {
        admin.last_payment = currentPayment.local_id;  // використовуємо currentPayment
    }
}

if (admin.last_payment == localId) {
    admin.last_payment = 0;
}
admin.num_of_payments--;
addFreeSpace(paymentGarbage, localId);
writeAdmin(admin, payment.admin_id - 1);
}

int countPayments() {
    FILE* paymentsDB = nullptr;
    fopen_s(&paymentsDB, "payments.bin", "rb");

    if (!paymentsDB) {
        std::cerr << "Error: Could not open payments.bin for reading!" << std::endl;
        return 0;
    }

    Payment payment;
    int count = 0;

    while (fread(&payment, sizeof(Payment), 1, paymentsDB)) {
        if (!payment.deleted) {
            count++;
        }
    }

    fclose(paymentsDB);
    return count;
}

void fullyRemovePayment(int localId) {
    // Відкриваємо оригінальний файл виплат для читання
    FILE* paymentsDB = nullptr;
    fopen_s(&paymentsDB, "payments.bin", "rb");

    if (!paymentsDB) {
        std::cerr << "Error: Could not open payments.bin for reading!" << std::endl;
        return;
    }

    // Відкриваємо тимчасовий файл для запису
    FILE* tempDB = nullptr;
    fopen_s(&tempDB, "temp_payments.bin", "wb");

    if (!tempDB) {
        std::cerr << "Error: Could not create temporary file!" << std::endl;
        fclose(paymentsDB);
        return;
    }

    Payment payment;
    int currentId = 1; // Лічильник для нових ID

    // Копіюємо всі записи, крім видаленого, у тимчасовий файл
    while (fread(&payment, sizeof(Payment), 1, paymentsDB)) {
        if (currentId != localId) { // Пропускаємо запис, який потрібно видалити
            fwrite(&payment, sizeof(Payment), 1, tempDB);
        }
        currentId++;
    }

    // Закриваємо файли
    fclose(paymentsDB);
    fclose(tempDB);

    // Видаляємо оригінальний файл
    if (remove("payments.bin") != 0) {
        std::cerr << "Error: Could not delete original payments.bin file!" << std::endl;
        return;
    }

    // Перейменовуємо тимчасовий файл у оригінальний
    if (rename("temp_payments.bin", "payments.bin") != 0) {
        std::cerr << "Error: Could not rename temporary file to payments.bin!" << std::endl;
        return;
    }

    std::cout << "Payment " << localId << " has been fully removed from the file.\n";
}



void updateConnectedIndexesPayment(int localId) {
    for (auto& garbage : paymentGarbage) {
        if (garbage.next_free > localId) {
            garbage.next_free -= 1;
        }
    }
    // Оновлюємо зв'язки в адміністраторах
    for (int i = 1; i <= adminIden.size(); i++) {
        Admin admin = getAdmin(i);

        // якщо first_payment або last_payment більше за видалений ID, зменшуємо їх
        if (admin.first_payment > localId) {
            admin.first_payment -= 1;
            writeAdmin(admin, i - 1);
        }

        if (admin.last_payment > localId) {
            admin.last_payment -= 1;
            writeAdmin(admin, i - 1);
        }
    }

    // Оновлюємо зв'язки в виплатах
    for (int i = 1; i <= countPayments(); i++) {
        Payment payment = getPayment(i);

        // Якщо наступна виплата має більший локальний ID, коригуємо його
        if (payment.next_local_address > localId && payment.next_local_address != 0) {
            payment.next_local_address -= 1;
            writePayment(payment, i - 1); // Записуємо оновлену виплату
        }
    }
    Payment payment = getPayment(localId);
    Admin admin = getAdmin(payment.admin_id);
    if (admin.first_payment == localId) {
        admin.first_payment = payment.next_local_address;
    }
    else {
        Payment currentPayment = getPayment(admin.first_payment);  // отримуємо поточний платіж
        // цикл, в якому ми змінюємо ітеруємо, поки не знайдемо того, хто був перд тим, кого ми хочемо видалити
        while (currentPayment.next_local_address != localId) {
            currentPayment = getPayment(currentPayment.next_local_address);  // оновлюємо currentPayment
        }
        currentPayment.next_local_address = payment.next_local_address;  // оновлюємо наступний платіж
        writePayment(currentPayment, currentPayment.local_id - 1);  // записуємо зміни
        if (admin.last_payment == localId) {
            admin.last_payment = currentPayment.local_id;  // використовуємо currentPayment
        }
    }

    if (admin.last_payment == localId) {
        admin.last_payment = 0;
    }
    admin.num_of_payments--;
    writeAdmin(admin, payment.admin_id - 1);
}
/*
void updateConnectedIndexesPayment(int localId) {
    FILE* paymentsDB = nullptr;
    fopen_s(&paymentsDB, "payments.bin", "rb+");

    if (!paymentsDB) {
        std::cerr << "Error: Could not open payments.bin for updating!" << std::endl;
        return;
    }

    Payment current_payment;
    fseek(paymentsDB, 0, SEEK_SET);

    // Оновлюємо кожну виплату
    while (fread(&current_payment, sizeof(Payment), 1, paymentsDB)) {
        // Якщо наступна виплата має більший локальний ID, коригуємо його
        if (current_payment.next_local_address > localId && current_payment.next_local_address != 0) {
            current_payment.next_local_address -= 1;
            fseek(paymentsDB, -static_cast<long>(sizeof(Payment)), SEEK_CUR);  // Повертаємось до поточної позиції
            fwrite(&current_payment, sizeof(Payment), 1, paymentsDB);  // Перезаписуємо змінений запис
        }
    }

    fclose(paymentsDB);

    // Оновлюємо зв'язки в адміністраторах
    for (int i = 1; i <= adminIden.size(); i++) {
        Admin current_admin = getAdmin(i);

        // Якщо first_payment або last_payment більше за видалений ID, зменшуємо їх
        if (current_admin.first_payment > localId) {
            current_admin.first_payment -= 1;
            writeAdmin(current_admin, i - 1);
        }

        if (current_admin.last_payment > localId) {
            current_admin.last_payment -= 1;
            writeAdmin(current_admin, i - 1);
        }
    }
}
*/
/*
void updatePaymentIndexes() {
        FILE* paymentsDB = nullptr;
        fopen_s(&paymentsDB, "payments.bin", "rb+");

        if (!paymentsDB) {
            std::cerr << "Error: Could not open payments.bin for updating indexes!" << std::endl;
            return;
        }

        Payment payment;
        int newId = 1; // Початковий індекс для відновлення порядку

        // Переміщуємо покажчик на початок файлу
        fseek(paymentsDB, 0, SEEK_SET);

        // Читаємо всі виплати з файлу
        while (fread(&payment, sizeof(Payment), 1, paymentsDB)) {
                payment.local_id = newId; // Присвоюємо новий індекс
                fseek(paymentsDB, -static_cast<long>(sizeof(Payment)), SEEK_CUR); // Повертаємося на позицію для запису
                fwrite(&payment, sizeof(Payment), 1, paymentsDB); // Записуємо оновлену виплату
                newId++; // Інкрементуємо новий індекс
        }

        fclose(paymentsDB);
    }
*/
void updatePaymentIndexes() {
    int newId = 1; // Початковий індекс для відновлення порядку

    // Проходимо по всіх виплатах у файлі payments.bin
    // Проходимо по всіх виплатах у файлі payments.bin
    for (int localId = 1; localId <= countPayments(); localId++) {
        Payment payment = getPayment(localId);
// Якщо виплата не видалена
            payment.local_id = newId; // Присвоюємо новий індекс
            writePayment(payment, newId - 1); // Записуємо оновлену виплату
            newId++; // Інкрементуємо новий індекс
        
    }
}

void updatePaymentAfterCompleteDeletion(int localId) {
    updateConnectedIndexesPayment(localId);
	fullyRemovePayment(localId);
    updatePaymentIndexes();
}
