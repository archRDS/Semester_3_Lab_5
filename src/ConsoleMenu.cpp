#include "ConsoleMenu.h"

#include "PaymentStrategy.h"

#include <array>
#include <iostream>
#include <limits>
#include <string_view>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {
struct EndOfInput {};

bool readInput(std::wstring& line) {
#ifdef _WIN32
    const HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(input, &mode)) {
        // Читаем строку в UTF-16 напрямую, включая завершающий CRLF.
        wchar_t buffer[256];
        DWORD length = 0;
        do {
            if (!ReadConsoleW(input, buffer, 256, &length, nullptr) || length == 0) {
                return false;
            }
            line.append(buffer, length);
        } while (line.back() != L'\n');
        return line.front() != L'\x1a';
    }
#endif
    return static_cast<bool>(std::getline(std::wcin, line));
}

std::wstring readLine(const wchar_t* prompt) {
    std::wcout << prompt << std::flush;
    std::wstring line;
    if (!readInput(line)) {
        throw EndOfInput{};
    }
    const auto first = line.find_first_not_of(L" \t\r\n");
    if (first == std::wstring::npos) {
        return {};
    }
    return line.substr(first, line.find_last_not_of(L" \t\r\n") - first + 1);
}

Money parseDigits(std::wstring_view text, Money max, const wchar_t* error) {
    if (text.empty()) {
        throw MarketplaceError{error};
    }

    Money value = 0;
    for (const wchar_t symbol : text) {
        const int digit = symbol - L'0';
        if (digit < 0 || digit > 9 || value > (max - digit) / 10) {
            throw MarketplaceError{error};
        }
        value = value * 10 + digit;
    }
    return value;
}

int readNumber(const wchar_t* prompt) {
    return static_cast<int>(parseDigits(readLine(prompt), std::numeric_limits<int>::max(),
                                        L"Введите целое неотрицательное число."));
}

Money readMoney(const wchar_t* prompt) {
    const auto* error = L"Введите сумму в рублях с точностью до копеек, например 179,90.";
    const std::wstring text = readLine(prompt);
    const std::wstring_view value(text);
    const auto separator = value.find_first_of(L",.");
    Money kopecks = 0;
    if (separator != std::wstring_view::npos) {
        const auto fraction = value.substr(separator + 1);
        if (fraction.size() > 2) {
            throw MarketplaceError{error};
        }
        kopecks = parseDigits(fraction, 99, error);
        if (fraction.size() == 1) {
            kopecks *= 10;
        }
    }
    const Money maxRubles = (std::numeric_limits<Money>::max() - kopecks) / 100;
    return parseDigits(value.substr(0, separator), maxRubles, error) * 100 + kopecks;
}

std::wstring formatMoney(Money amount) {
    const Money kopecks = amount % 100;
    return std::to_wstring(amount / 100) + (kopecks < 10 ? L",0" : L",") +
           std::to_wstring(kopecks) + L" руб.";
}

void printReceipt(const Receipt& receipt) {
    std::wcout << L"\nЧек покупки\n"
               << L"Покупатель: " << receipt.customerName << L'\n'
               << L"Продавец: " << receipt.sellerName << L'\n'
               << L"Товар: " << receipt.productName << L'\n'
               << L"Количество: " << receipt.quantity << L" шт.\n"
               << L"Цена за штуку: " << formatMoney(receipt.unitPrice) << L'\n'
               << L"Итого: " << formatMoney(receipt.total) << L'\n'
               << L"Оплата: " << receipt.paymentMethod << L'\n'
               << L"Остаток на балансе: " << formatMoney(receipt.remainingBalance) << L'\n';
}
}

void ConsoleMenu::run() {
    std::wcout << L"Торговая площадка. Лабораторная работа №5, вариант 11.\n";
    for (;;) {
        std::wcout << L"\n1. Добавить продавца\n"
                      L"2. Добавить покупателя\n"
                      L"3. Выставить товар\n"
                      L"4. Показать каталог\n"
                      L"5. Купить товар\n"
                      L"0. Выход\n";
        try {
            switch (readNumber(L"Ваш выбор: ")) {
            case 1: addSeller(); break;
            case 2: addCustomer(); break;
            case 3: addProduct(); break;
            case 4: showCatalog(); break;
            case 5: buyProduct(); break;
            case 0: return;
            default: std::wcout << L"Выберите пункт от 0 до 5.\n";
            }
        } catch (const MarketplaceError& error) {
            std::wcout << L"Ошибка: " << error.message << L'\n';
        } catch (const EndOfInput&) {
            std::wcout << L'\n';
            return;
        }
    }
}

void ConsoleMenu::addSeller() {
    const auto name = readLine(L"Имя продавца: ");
    const auto id = Superuser::instance().addSeller(marketplace_, name);
    std::wcout << L"Продавец добавлен. Номер: " << id << L'\n';
}

void ConsoleMenu::addCustomer() {
    const auto name = readLine(L"Имя покупателя: ");
    const Money balance = readMoney(L"Начальный баланс, руб.: ");
    const auto id = Superuser::instance().addCustomer(marketplace_, name, balance);
    std::wcout << L"Покупатель добавлен. Номер: " << id << L'\n';
}

void ConsoleMenu::addProduct() {
    if (marketplace_.sellers().empty()) {
        throw MarketplaceError{L"Сначала добавьте продавца через пункт 1."};
    }
    std::wcout << L"Продавцы\n";
    for (const auto& seller : marketplace_.sellers()) {
        std::wcout << seller->id() << L". " << seller->name() << L'\n';
    }
    const auto& seller = marketplace_.seller(readNumber(L"Номер продавца: "));
    const auto name = readLine(L"Название товара: ");
    const Money price = readMoney(L"Цена за штуку, руб.: ");
    const int quantity = readNumber(L"Количество на складе: ");
    const auto id = seller.add_product(marketplace_, name, price, quantity);
    std::wcout << L"Товар выставлен. Номер: " << id << L'\n';
}

void ConsoleMenu::showCatalog() const {
    if (marketplace_.products().empty()) {
        std::wcout << L"Каталог пуст. Выставьте товар через пункт 3.\n";
        return;
    }
    std::wcout << L"Каталог товаров\n";
    for (const auto& product : marketplace_.products()) {
        const auto& seller = marketplace_.seller(product.sellerId());
        std::wcout << product.id() << L". " << product.name() << L" | " << formatMoney(product.price())
                   << L" | Остаток: " << product.quantity() << L" шт. | Продавец: " << seller.name()
                   << L" (#" << seller.id() << L")\n";
    }
}

void ConsoleMenu::buyProduct() {
    if (marketplace_.customers().empty()) {
        throw MarketplaceError{L"Сначала добавьте покупателя через пункт 2."};
    }
    if (marketplace_.products().empty()) {
        throw MarketplaceError{L"Сначала выставьте товар через пункт 3."};
    }
    std::wcout << L"Покупатели\n";
    for (const auto& customer : marketplace_.customers()) {
        std::wcout << customer->id() << L". " << customer->name() << L" | Баланс: "
                   << formatMoney(customer->balance()) << L'\n';
    }
    const auto& buyer = marketplace_.customer(readNumber(L"Номер покупателя: "));
    showCatalog();
    const auto productId = static_cast<std::size_t>(readNumber(L"Номер товара: "));
    const int quantity = readNumber(L"Количество для покупки: ");

    const CashPayment cash;
    const CardPayment card;
    const CryptoPayment crypto;
    const std::array<const PaymentStrategy*, 3> payments{&cash, &card, &crypto};
    for (std::size_t index = 0; index < payments.size(); ++index) {
        std::wcout << index + 1 << L". " << payments[index]->name() << L'\n';
    }
    const int choice = readNumber(L"Способ оплаты: ");
    if (choice < 1 || choice > static_cast<int>(payments.size())) {
        throw MarketplaceError{L"Выберите способ оплаты от 1 до 3."};
    }
    printReceipt(buyer.buy(marketplace_, productId, quantity, *payments[choice - 1]));
}
