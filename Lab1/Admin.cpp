// admin.cpp
#include "Admin.h"
#include <iostream>
#include <cstring>
#include <cstdio>

// функція для запиту даних про адміністратора
void getAdminInfo(char* name, int& experience) {
    std::cout << "Enter the name of the admin: ";
    std::cin >> name;
    std::cout << "Enter the experience of the admin (years): ";
    std::cin >> experience;
}

// записуємо індекс адміністратора в вектор, а не файл
void writeAdminIndex(AdminIdentificator index, int position) {
    // перевіряємо, чи позиція в межах вектора
    if (position < 0 || position >= adminIden.size()) {
        std::cerr << "Error: Invalid position to write index!" << std::endl;
        return;
    }

    // оновлюємо індекс у векторі
    adminIden[position] = index;
}

// отримуємо індекс адміністратора з вектора
AdminIdentificator getAdminIndex(int position) {
    AdminIdentificator index{};

    if (position < 0 || position >= adminIden.size()) {
        std::cerr << "Error: Invalid position to get index!" << std::endl;
        return index;
    }

    return adminIden[position];
}


// функція для отримання адміністратора за його ID
Admin getAdmin(int adminId) {
    Admin admin{}; // оголошуємо змінну admin типу Admin, ініціалізуємо її за замовчуванням (пусті значення)
    FILE* adminDB = nullptr;
    fopen_s(&adminDB, "admins.bin", "rb");

    if (!adminDB) {
        std::cerr << "Error: Could not open admins.bin for reading!" << std::endl;
        return admin;
    }

    fseek(adminDB, (adminId - 1) * sizeof(Admin), SEEK_SET); 
    fread(&admin, sizeof(Admin), 1, adminDB);
    fclose(adminDB);
    return admin;
}

//  запис адміністратора в файл
void writeAdmin(Admin& admin, int position) {
    FILE* adminDB = nullptr;
    fopen_s(&adminDB, "admins.bin", "rb+");

    if (!adminDB) {
        std::cerr << "Error: Could not open admins.bin for writing!" << std::endl;
        return;
    }

    fseek(adminDB, position * sizeof(Admin), SEEK_SET); 
    fwrite(&admin, sizeof(Admin), 1, adminDB);
    fclose(adminDB);
}

// функція для додавання адміністратора
void addAdmin(Admin& admin) {
    AdminIdentificator index; // оголошення структури index типу AdminIdentificator
    int freeSpace = getFreeSpace(adminGarbage); // отримання вільного місця в файлі

    if (freeSpace != -1) { // якщо є місце у смітті
        index.adminId = freeSpace; 
        index.adminAddress = (freeSpace - 1) * sizeof(Admin); 
    }
    else {
        // створюємо новий індекс, якщо вільного місця немає
        int count = adminIden.size(); // кількість елементів у векторі adminIden
        index.adminId = count + 1; // інкрементуємо adminId
        index.adminAddress = count * sizeof(Admin); // обчислюємо адресу для нового адміністратора

        // додаємо новий індекс в оперативну пам'ять
        adminIden.push_back(index);
    }

    admin.deleted = false;
    writeAdminIndex(index, index.adminId - 1); // записуємо індекс в оперативну пам'ять (вектор)

    // відкриваємо файл адміністраторів і записуємо нові дані
    FILE* adminDb = nullptr;
    fopen_s(&adminDb, "admins.bin", "rb+");

    if (!adminDb) {
        std::cerr << "Error: Could not open admins.bin for writing!" << std::endl;
        return;
    }

    fseek(adminDb, index.adminAddress, SEEK_SET); // переміщаємо покажчик файлу до адреси запису нового адміністратора
    fwrite(&admin, sizeof(Admin), 1, adminDb); // записуємо нові дані одного адміністратора у файл
    fclose(adminDb);
}

// видалення адміністратора
void deleteAdmin(int adminId) {
    Admin admin = getAdmin(adminId);
    int currentPaymentLocalId = admin.first_payment;

    while (currentPaymentLocalId != 0) { // проходимося по всім виплатам адміністратора
        Payment currentPayment = getPayment(currentPaymentLocalId);
        if (currentPayment.deleted) {  // якщо виплата вже видалена, переходимо до наступної
            currentPaymentLocalId = currentPayment.next_local_address;
            continue;
        }
        currentPayment.deleted = true;
        addFreeSpace(paymentGarbage, currentPayment.local_id);
        writePayment(currentPayment, currentPaymentLocalId - 1); // записуємо зміни в файл, враховуючи видалення
        currentPaymentLocalId = currentPayment.next_local_address;
    }

    AdminIdentificator index = getAdminIndex(adminId - 1);
    admin.deleted = true;
	addFreeSpace(adminGarbage, adminId); // додаємо вільне місце в сміття
    writeAdmin(admin, adminId - 1);
    writeAdminIndex(index, adminId - 1); 
}

std::vector<AdminIdentificator> adminIden; // вектор індексів адміністраторів

void loadIndexesFromFile(const std::string& filename) {
    FILE* IndexDB = nullptr;
    fopen_s(&IndexDB, filename.c_str(), "rb");

    if (!IndexDB) {
        std::cerr << "Error: Could not open " << filename << " for reading!" << std::endl;
        return;
    }

    adminIden.clear();

    AdminIdentificator entry;
    while (fread(&entry, sizeof(AdminIdentificator), 1, IndexDB)) {
         adminIden.push_back(entry);
    }

    fclose(IndexDB);
}

void writeIndexesToFile(const std::string& filename) {
	FILE* IndexDB = nullptr;
	fopen_s(&IndexDB, filename.c_str(), "wb");
	if (!IndexDB) {
		std::cerr << "Error: Could not open " << filename << " for writing!" << std::endl;
		return;
	}
	for (const AdminIdentificator& entry : adminIden) {
		fwrite(&entry, sizeof(AdminIdentificator), 1, IndexDB);
	}
	fclose(IndexDB);
}

int countAdmins() {
    FILE* adminDB = nullptr;
    fopen_s(&adminDB, "admins.bin", "rb");

    if (!adminDB) {
        std::cerr << "Error: Could not open admins.bin for reading!" << std::endl;
        return 0;
    }

    int count = 0;
    Admin admin;

    while (fread(&admin, sizeof(Admin), 1, adminDB)) { 
        if (!admin.deleted) {  // якщо адміністратор не видалений, враховуємо його
            count++;
        }
    }

    fclose(adminDB);
    return count;
}
