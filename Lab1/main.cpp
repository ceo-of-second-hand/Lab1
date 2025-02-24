#include <iostream>
#include <iomanip>
#include <cstdio> // для remove()
#include <limits> // для numeric_limits
#include "Admin.h"
#include "Payment.h"
#include "Garbage.h"

// функція для ініціалізації файлів
void initializeFiles() {
    FILE* file;

    const char* files[] = {
        "adminIndex.bin",
        "admins.bin",
        "payments.bin",
        "adminGarbage.bin",
        "paymentGarbage.bin"
    };

    for (const char* filename : files) {
        fopen_s(&file, filename, "rb");
        if (!file) {
            fopen_s(&file, filename, "wb");
            if (!file) {
                std::cerr << "Error: Could not create " << filename << "!\n";
                exit(1); // завершення програми у разі помилки
            }
        }
        if (file) fclose(file);
    }
}

// функція для очищення файлів
void cleanupFiles() {
    const char* files[] = {
        "adminIndex.bin",
        "admins.bin",
        "payments.bin",
        "adminGarbage.bin",
        "paymentGarbage.bin"
    };

    for (const char* file : files) {
        if (remove(file)) {
            std::cerr << "Error: Could not delete file " << file << "!\n";
        }
        else {
            std::cout << "File " << file << " deleted successfully.\n";
        }
    }
}

void displayAdminTable() {
    // виводимо таблицю адміністраторів
    std::cout << "\nAdmin Table:\n";
    std::cout << std::setw(5) << "ID"
        << std::setw(20) << "Name"
        << std::setw(12) << "Experience"
        << std::setw(15) << "Num of Payments"
        << std::setw(15) << "First Payment"
        << std::setw(15) << "Last Payment"
        << std::setw(10) << "Deleted" << std::endl;
    std::cout << std::string(92, '-') << std::endl;

    // проходимо по всіх індексах у векторі adminIden
    for (const AdminIdentificator& index : adminIden) {
        Admin admin = getAdmin(index.adminId); // отримуємо адміністратора за його ID
        std::cout << std::setw(5) << index.adminId
            << std::setw(20) << admin.name
            << std::setw(12) << admin.experience
            << std::setw(15) << admin.num_of_payments
            << std::setw(15) << admin.first_payment
            << std::setw(15) << admin.last_payment
            << std::setw(10) << (admin.deleted ? "Yes" : "No") << std::endl;
    }
}

// виводимо таблицю виплат
void displayPaymentTable() {
    FILE* paymentsDB;
    fopen_s(&paymentsDB, "payments.bin", "rb");
    if (!paymentsDB) {
        std::cerr << "Error: Could not open payments.bin for reading!" << std::endl;
        return;
    }

    std::cout << "\nPayments Table:\n";
    std::cout << std::setw(5) << "ID" << std::setw(10) << "Admin ID" << std::setw(10) << "Size"
        << std::setw(12) << "Month" << std::setw(15) << "Next Payment"
        << std::setw(10) << "Deleted" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    Payment payment;
    int localId = 1;

    while (fread(&payment, sizeof(Payment), 1, paymentsDB)) {
        std::cout << std::setw(5) << localId
            << std::setw(10) << payment.admin_id
            << std::setw(10) << payment.size
            << std::setw(12) << payment.month
            << std::setw(15) << payment.next_local_address
            << std::setw(10) << (payment.deleted ? "Yes" : "No") << std::endl;
        localId++;
    }

    fclose(paymentsDB);
}

// виводимо всі дані 
void displayTable() {
    displayAdminTable();
    displayPaymentTable();
}

int main() {
    initializeFiles();

    FILE* file;
    if (fopen_s(&file, "adminGarbage.bin", "rb") == 0) {
        loadAdminGarbageFromFile("adminGarbage.bin");
        fclose(file);
    }

    if (fopen_s(&file, "paymentGarbage.bin", "rb") == 0) {
        loadPayoutGarbageFromFile("paymentGarbage.bin");
        fclose(file);
    }

    if (fopen_s(&file, "adminIndex.bin", "rb") == 0) {
        loadIndexesFromFile("adminIndex.bin");
        fclose(file);
    }

    while (true) {
        int choice = 0;
        std::cout << "\nMenu:\n";
        std::cout << "1. Add an admin(insert-m)\n";
        std::cout << "2. Add a payment(insert-s)\n";
        std::cout << "3. Delete an admin(del-s)\n";
        std::cout << "4. Delete a payment(del-s)\n";
        std::cout << "5. Get certain admin(get-m)\n";
        std::cout << "6. Get certain payment(get-s)\n";
        std::cout << "7. Update admin's value (update-m)\n";
        std::cout << "8. Update payment's value (upldate-s)\n";
        std::cout << "9. Count admins (calc-m)\n";
        std::cout << "10. Count paymnets (calc-s)\n";
        std::cout << "11. Display tables (display)\n";
        std::cout << "12. Display admin (ut-m)\n";
        std::cout << "13. Display payments (ut-s)\n";
        std::cout << "14. Exit (without cleanup)\n";
        std::cout << "15. Exit (with cleanup)\n";
		std::cout << "16. Remove admin garbage\n";
		std::cout << "17. Remove payment garbage\n";
        std::cout << "Choose an option: ";

        if (!(std::cin >> choice)) {
            std::cin.clear(); // clear the error flag
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard invalid input
            std::cout << "Invalid input! Please enter a number.\n";
            continue;
        }

        switch (choice) {
        case 1: {
            Admin admin;
            getAdminInfo(admin.name, admin.experience);
            admin.num_of_payments = 0;
            admin.first_payment = 0;
            admin.last_payment = 0;
            addAdmin(admin);
            std::cout << "Admin added successfully!\n";
            break;
        }
        case 2: {
            Payment payment;
            getPaymentInfo(payment.admin_id, payment.size, payment.month);
            payment.next_local_address = 0;
            payment.deleted = false;
            addPayment(payment);
            std::cout << "Payment added successfully!\n";
            break;
        }
        case 3: {
            int adminId;
            std::cout << "Enter admin ID to delete: ";
            if (!(std::cin >> adminId)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input! Please enter a number.\n";
                break;
            }
            Admin admin = getAdmin(adminId);
            if (admin.deleted) {
                std::cout << "Admin already deleted!\n";
                break;
            }
            if (adminId < 0 || adminId > adminIden.size()) {
                std::cout << "Admin does not exist\n";
                break;
            }
            deleteAdmin(adminId);
            std::cout << "Admin deleted!\n";
            break;
        }
        case 4: {
            int localId;
            std::cout << "Enter payment local ID to delete: ";
            if (!(std::cin >> localId)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input! Please enter a number.\n";
                break;
            }
            Payment payment = getPayment(localId);
            if (payment.deleted) {
                std::cout << "Payment already deleted!\n";
                break;
            }
            if (localId < 0 || localId > countPayments()) {
                std::cout << "Payment does not exist\n";
                break;
            }
            deletePayment(localId);
            std::cout << "Payment deleted!\n";
            break;
        }
        case 5: {
            int adminId;
            std::cout << "Enter admin ID to look at: ";
            if (!(std::cin >> adminId)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input! Please enter a number.\n";
                break;
            }
            Admin admin = getAdmin(adminId);
            if (admin.deleted) {
                std::cout << "Admin already deleted!\n";
                break;
            }
            if (adminId < 0 || adminId > adminIden.size()) {
                std::cout << "Admin does not exist\n";
                break;
            }
            std::cout << std::setw(5) << adminId
                << std::setw(20) << admin.name
                << std::setw(12) << admin.experience
                << std::setw(15) << admin.num_of_payments
                << std::setw(15) << admin.first_payment
                << std::setw(15) << admin.last_payment
                << std::setw(10) << (admin.deleted ? "Yes" : "No") << std::endl;
            break;
        }
        case 6: {
            int adminId;
            std::cout << "Enter admin ID, whose payment you want to look at: ";
            if (!(std::cin >> adminId)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input! Please enter a number.\n";
                break;
            }
            Admin admin = getAdmin(adminId);
            if (admin.deleted) {
                std::cout << "Admin already deleted!\n";
                break;
            }
            if (adminId < 0 || adminId > adminIden.size()) {
                std::cout << "Admin does not exist\n";
                break;
            }
            std::cout << "Number of admin's payments: " << admin.num_of_payments << "\n";
            if (admin.num_of_payments == 0) {
                std::cout << "No payments for this admin\n";
                break;
            }
            std::cout << "Enter number of admin's payment you want to look at: ";
            int veryLocalId;
            if (!(std::cin >> veryLocalId)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input! Please enter a number.\n";
                break;
            }
            if (veryLocalId > admin.num_of_payments || veryLocalId < 0) {
                std::cout << "No payments for this admin\n";
                break;
            }
            Payment currentPayment;
            int currentPaymentLocalId = admin.first_payment;
            for (int i = 1; i <= veryLocalId; i++) { // проходимося по всім виплатам адміністратора
                currentPayment = getPayment(currentPaymentLocalId);
                currentPaymentLocalId = currentPayment.next_local_address;
            }
            std::cout << std::setw(5) << currentPayment.local_id
                << std::setw(10) << currentPayment.admin_id
                << std::setw(10) << currentPayment.size
                << std::setw(12) << currentPayment.month
                << std::setw(15) << currentPayment.next_local_address
                << std::setw(10) << (currentPayment.deleted ? "Yes" : "No") << std::endl;

            break;
        }
        case 7: {
            int adminId;
            std::cout << "Enter admin ID to change: ";
            if (!(std::cin >> adminId)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input! Please enter a number.\n";
                break;
            }
            Admin admin = getAdmin(adminId);
            if (admin.deleted) {
                std::cout << "Admin already deleted!\n";
                break;
            }
            if (adminId < 0 || adminId > adminIden.size()) {
                std::cout << "Admin does not exist\n";
                break;
            }
            int field;
            std::cout << "Enter 1 to change name or 2 to change experience : ";
            if (!(std::cin >> field)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input! Please enter a number.\n";
                break;
            }
            if (field == 1) {
                char altered_name[40];

                std::cout << "Enter new name: ";
                std::cin >> altered_name;
                strcpy_s(admin.name, altered_name);
                writeAdmin(admin, adminId - 1);

            }
            else if (field == 2) {
                int altered_experience;
                std::cout << "Enter new experience: ";
                if (!(std::cin >> altered_experience)) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid input! Please enter a number.\n";
                    break;
                }
                admin.experience = altered_experience;
                writeAdmin(admin, adminId - 1);
            }
            else {
                std::cout << "Unknown field! Try again.\n";
            }
            break;
        }

        case 8: {
            int localId;
            std::cout << "Enter payment ID to change: ";
            if (!(std::cin >> localId)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input! Please enter a number.\n";
                break;
            }
            Payment payment = getPayment(localId);
            if (payment.deleted) {
                std::cout << "Payment already deleted!\n";
                break;
            }
            if (localId < 0 || localId > countPayments()) {
                std::cout << "Payment does not exist\n";
                break;
            }
            int field;
            std::cout << "Enter 1 to change size or 2 to change month : ";
            if (!(std::cin >> field)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input! Please enter a number.\n";
                break;
            }
            if (field == 1) {
                int altered_size;

                std::cout << "Enter new size: ";
                if (!(std::cin >> altered_size)) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid input! Please enter a number.\n";
                    break;
                }
                payment.size = altered_size;
                writePayment(payment, localId - 1);

            }
            else if (field == 2) {
                char altered_month[8];
                std::cout << "Enter new month: ";
                std::cin >> altered_month;
                strcpy_s(payment.month, altered_month);
                writePayment(payment, localId - 1);
            }
            else {
                std::cout << "Unknown field! Try again.\n";
            }
            break;
        }

        case 9:
            std::cout << countAdmins() << std::endl;
            break;
        case 10: {
            std::cout << countPayments() << std::endl;
            std::cout << "Enter admin's Id to count their payments: ";
            int adminId;
            if (!(std::cin >> adminId)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input! Please enter a number.\n";
                break;
            }
            Admin admin = getAdmin(adminId);
            if (admin.deleted) {
                std::cout << "Admin already deleted!\n";
                break;
            }
            if (adminId < 0 || adminId > adminIden.size()) {
                std::cout << "Admin does not exist\n";
                break;
            }
            std::cout << "Admin's payments: " << admin.num_of_payments << std::endl;
            break;
        }
        case 11:
            displayTable();
            break;
        case 12:
            displayAdminTable();
            break;
        case 13:
            displayPaymentTable();
            break;
        case 14:
            std::cout << "Saving garbage and exiting...\n";
            writeAdminGarbageToFile("adminGarbage.bin");
            writePayoutGarbageToFile("paymentGarbage.bin");
            writeIndexesToFile("adminIndex.bin");
            return 0;
        case 15:
            std::cout << "Exiting and cleaning up files...\n";
            cleanupFiles();
            return 0;
        case 16:
            removeAdminGarbage();
            std::cout << "Admin arbage removed and indexes updated successfully!\n";
            break;
        case 17:
            removePaymentGarbage();
            std::cout << "Payment arbage removed and indexes updated successfully!\n";
            break;
        default:
            std::cout << "Unknown option! Try again.\n";

        }
    }
}

