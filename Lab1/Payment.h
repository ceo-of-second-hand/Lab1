//payment.h
#pragma once
#include <string>
#include <cstdio>

typedef struct Payment {
    int admin_id;               // ID адміністратора
    int local_id;               // унікальний ID виплати
    int size;                   // розмір виплати
	char month[8];              // місяць виплати (mm-yyyy) (8 символів, оскільки включає '\0')
    int next_local_address;     // адреса наступної виплати (0, якщо немає наступної)
    bool deleted;               // відмітка про видалення виплати
} Payment;

void getPaymentInfo(int& admin_id, int& size, char* month);  // функція для запиту даних про платіж
void addPayment(Payment& payment); 
Payment getPayment(int localId);
void writePayment(Payment& payment, int position);
void deletePayment(int localId);
int countPayments(); // підрахунок виплат (для main)

