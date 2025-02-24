//garbage.h
#pragma once
#include <iostream>
#include <vector>  
#include <cstdio>  //  fopen, fseek, fread, fwrite
#include "Admin.h"
#include "Payment.h"

// структурf для сміттєвого простору
struct GarbageSpace {
    int next_free;  // fдреса  вільного місця
};

// вектори для зберігання вільних місць
extern std::vector<GarbageSpace> adminGarbage;
extern std::vector<GarbageSpace> paymentGarbage;

// функція для отримання вільного простору з певного контейнера
int getFreeSpace(std::vector<GarbageSpace>& garbageContainer);

// функція для додавання нового вільного місця в певний контейнер
void addFreeSpace(std::vector<GarbageSpace>& garbageContainer, int localId);

// функції для запису та завантаження сміття адмінів
void writeAdminGarbageToFile(const std::string& filename);
void loadAdminGarbageFromFile(const std::string& filename);

void writePayoutGarbageToFile(const std::string& filename);
void loadPayoutGarbageFromFile(const std::string& filename);

void removeAdminGarbage();
void removePaymentGarbage();
void updateIndexes();
