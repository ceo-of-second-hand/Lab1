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
