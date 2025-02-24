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

void fullyRemoveAdmin(int adminId) {
    // Відкриваємо оригінальний файл адміністраторів для читання
    FILE* adminDB = nullptr;
    fopen_s(&adminDB, "admins.bin", "rb");

    if (!adminDB) {
        std::cerr << "Error: Could not open admins.bin for reading!" << std::endl;
        return;
    }

    // Відкриваємо тимчасовий файл для запису
    FILE* tempDB = nullptr;
    fopen_s(&tempDB, "temp_admins.bin", "wb");

    if (!tempDB) {
        std::cerr << "Error: Could not create temporary file!" << std::endl;
        fclose(adminDB);
        return;
    }

    Admin admin;
    int currentId = 1; // Лічильник для нових ID

    // Копіюємо всі записи, крім видаленого, у тимчасовий файл
    while (fread(&admin, sizeof(Admin), 1, adminDB)) {
        if (currentId != adminId) { // Пропускаємо запис, який потрібно видалити
            fwrite(&admin, sizeof(Admin), 1, tempDB);
        }
        currentId++;
    }

    // Закриваємо файли
    fclose(adminDB);
    fclose(tempDB);

    // Видаляємо оригінальний файл
    if (remove("admins.bin") != 0) {
        std::cerr << "Error: Could not delete original admins.bin file!" << std::endl;
        return;
    }

    // Перейменовуємо тимчасовий файл у оригінальний
    if (rename("temp_admins.bin", "admins.bin") != 0) {
        std::cerr << "Error: Could not rename temporary file to admins.bin!" << std::endl;
        return;
    }

    // Видаляємо індекс з контейнера adminIden
    auto it = std::find_if(adminIden.begin(), adminIden.end(),
        [adminId](const AdminIdentificator& id) { return id.adminId == adminId; });

    if (it != adminIden.end()) {
        adminIden.erase(it); // Видаляємо індекс
        std::cout << "Admin index " << adminId << " removed from adminIden.\n";
    }
    else {
        std::cerr << "Error: Admin index " << adminId << " not found in adminIden!\n";
    }

    std::cout << "Admin " << adminId << " has been from the file.\n";
}

/*
void updateConnectedIndexesAdmin(int adminId) {
	int size = adminIden.size();
	int current_payment_id = 0;
    for (adminId; adminId <= size; adminId++) {
        Admin admin = getAdmin(adminId);
        admin.first_payment = current_payment_id;
        while (current_payment_id != 0) {
            Payment current_payment =  getPayment(current_payment_id);
			current_payment.admin_id = adminId-1;
			current_payment_id = current_payment.next_local_address;
        }
    }
}
*/

void updateConnectedIndexesAdmin(int adminId) {
    
    int size = adminIden.size();
    Admin admin = getAdmin(adminId);
    if (admin.num_of_payments > 0) {
        int changable_current_id = admin.first_payment;
        int changable_current_id_double = 0;
        while (changable_current_id != 0)
        {
            Payment current_payment = getPayment(changable_current_id);
            current_payment.admin_id = -1;
            writePayment(current_payment, changable_current_id - 1);
            changable_current_id_double = changable_current_id;
            changable_current_id = current_payment.next_local_address;
            current_payment.next_local_address = -1;
            writePayment(current_payment, changable_current_id_double - 1);

        }
    }
    
    // проходимо по всіх адміністраторах, починаючи з adminId
    for (int i = adminId+1; i <= size; i++) {
        Admin admin = getAdmin(i);
        int current_payment_id = admin.first_payment;

        // Проходимо по всіх виплатах адміністратора
        while (current_payment_id != 0) {
            // Отримуємо поточну виплату
            Payment current_payment = getPayment(current_payment_id);

            // Оновлюємо admin_id для поточної виплати
            current_payment.admin_id =  i-1;

            // Записуємо оновлену виплату назад у файл
            writePayment(current_payment, current_payment.local_id - 1);

            // Переходимо до наступної виплати
            current_payment_id = current_payment.next_local_address;
        }
    }

        for (auto& garbage : adminGarbage) {
            if (garbage.next_free > adminId) {
                garbage.next_free -= 1;
            }
        }
}
/*
void updateConnectedIndexesAdmin(int adminId) {
    int size = adminIden.size();
    int current_payment_id = 0;
    // Відкриваємо файл виплат для запису
    
    for (int i = adminId; i <= size; i++) {
        Admin admin = getAdmin(i);
        //admin.first_payment = current_payment_id;
		current_payment_id = admin.first_payment;
        // Відкриваємо файл виплат для запису

        FILE* paymentDB = nullptr;
        fopen_s(&paymentDB, "payments.bin", "rb+");
        if (!paymentDB) {
            std::cerr << "Error: Could not open payments.bin for updating!" << std::endl;
            return;
        }
        while (current_payment_id != 0) {
            Payment current_payment = getPayment(current_payment_id);
            current_payment.admin_id = adminId - 1; // оновлюємо admin_id для поточної виплати
            fseek(paymentDB, (current_payment.local_id - 1) * sizeof(Payment), SEEK_SET); // переміщаємо на адресу виплати
            fwrite(&current_payment, sizeof(Payment), 1, paymentDB); // записуємо оновлену виплату
            current_payment_id = current_payment.next_local_address;
        }

        fclose(paymentDB);
    }
}
*/

void updateIndexes() { //ПОТІМ МОЖЛИВО ВИПРАВИТИ 
    int newId = 1; // Початковий індекс для відновлення порядку

    // Ітеруємо по вектору і присвоюємо нові індекси
    for (auto& admin : adminIden) {
        admin.adminId = newId;
        newId++;
    }
}

void updateAdminAfterCompleteDeletion(int adminId) {
    updateConnectedIndexesAdmin(adminId);
	fullyRemoveAdmin(adminId);
    updateIndexes();



	std::cout << "Admin " << adminId << " has been fully removed from the file.\n"; //ПОТІМ МОЖЛИВО ВИДАЛИТИ РЯДОК
}