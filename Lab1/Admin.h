//admin.h
#pragma once
#include <string>
#include <cstdio>
#include "Garbage.h"
#include "Payment.h"

typedef struct Admin {
    char name[40];
    int experience;
    int num_of_payments; 
    int first_payment;
	int last_payment;
	bool deleted;
} Admin;

typedef struct AdminIdentificator {
    int adminId; //id в таблиці індексів
	int adminAddress; //адреса в файлі адміністраторів
} AdminIdentificator;

extern std::vector<AdminIdentificator> adminIden;

void addAdmin(Admin& admin);
void getAdminInfo(char* name, int& experience); //дані про адміна, що вводяться користувачем
void writeAdminIndex(AdminIdentificator index, int position);  //обновлення структури індексу адміна
AdminIdentificator getAdminIndex(int position); //отримання індексу адміна за позицією
Admin getAdmin(int adminId); //отримання адміна за його ID
void writeAdmin(Admin& admin, int position); //запис адміна в файл
void deleteAdmin(int adminId); //видалення адміна
void loadIndexesFromFile(const std::string& filename); //завантаження індексів з файлу
void writeIndexesToFile(const std::string& filename); //запис індексів в файл
int countAdmins(); //підрахунок адміністраторів (для main)
