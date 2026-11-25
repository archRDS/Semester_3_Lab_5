#include "ConsoleMenu.h"

#include "PaymentStrategy.h"

#include <array>
#include <iostream>
#include <limits>
#include <string_view>

namespace {
struct EndOfInput {};

std::string readLine(const char* prompt) {
    std::cout << prompt << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) {
        throw EndOfInput{};
    }
    const auto first = line.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    return line.substr(first, line.find_last_not_of(" \t\r\n") - first + 1);
}

Money parseDigits(std::string_view text, Money max, const char* error) {
    if (text.empty()) {
        throw MarketplaceError{error};
    }

    Money value = 0;
    for (const char symbol : text) {
        const int digit = symbol - '0';
        if (digit < 0 || digit > 9 || value > (max - digit) / 10) {
            throw MarketplaceError{error};
        }
        value = value * 10 + digit;
    }
    return value;
}

int readNumber(const char* prompt) {
    return static_cast<int>(parseDigits(readLine(prompt), std::numeric_limits<int>::max(),
                                        "Введите целое неотрицательное число."));
}

Money readMoney(const char* prompt) {
    const auto* error = "Введите сумму в рублях с точностью до копеек, например 179,90.";
    const std::string text = readLine(prompt);
    const std::string_view value(text);
    const auto separator = value.find_first_of(",.");
    Money kopecks = 0;
    if (separator != std::string_view::npos) {
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

std::string formatMoney(Money amount) {
    const Money kopecks = amount % 100;
    return std::to_string(amount / 100) + (kopecks < 10 ? ",0" : ",") +
           std::to_string(kopecks) + " руб.";
}

void printReceipt(const Receipt& receipt) {
    std::cout << "\nЧек покупки\n"
               << "Покупатель: " << receipt.customerName << '\n'
               << "Продавец: " << receipt.sellerName << '\n'
               << "Товар: " << receipt.productName << '\n'
               << "Количество: " << receipt.quantity << " шт.\n"
               << "Цена за штуку: " << formatMoney(receipt.unitPrice) << '\n'
               << "Итого: " << formatMoney(receipt.total) << '\n'
               << "Оплата: " << receipt.paymentMethod << '\n'
               << "Остаток на балансе: " << formatMoney(receipt.remainingBalance) << '\n';
}
}

void ConsoleMenu::run() {
    std::cout << "Торговая площадка. Лабораторная работа №5, вариант 11.\n";
    for (;;) {
        std::cout << "\n1. Добавить продавца\n"
                      "2. Добавить покупателя\n"
                      "3. Выставить товар\n"
                      "4. Показать каталог\n"
                      "5. Купить товар\n"
                      "0. Выход\n";
        try {
            switch (readNumber("Ваш выбор: ")) {
            case 1: addSeller(); break;
            case 2: addCustomer(); break;
            case 3: addProduct(); break;
            case 4: showCatalog(); break;
            case 5: buyProduct(); break;
            case 0: return;
            default: std::cout << "Выберите пункт от 0 до 5.\n";
            }
        } catch (const MarketplaceError& error) {
            std::cout << "Ошибка: " << error.message << '\n';
        } catch (const EndOfInput&) {
            std::cout << '\n';
            return;
        }
    }
}

void ConsoleMenu::addSeller() {
    const auto name = readLine("Имя продавца: ");
    const auto id = Superuser::instance().addSeller(marketplace_, name);
    std::cout << "Продавец добавлен. Номер: " << id << '\n';
}

void ConsoleMenu::addCustomer() {
    const auto name = readLine("Имя покупателя: ");
    const Money balance = readMoney("Начальный баланс, руб.: ");
    const auto id = Superuser::instance().addCustomer(marketplace_, name, balance);
    std::cout << "Покупатель добавлен. Номер: " << id << '\n';
}

void ConsoleMenu::addProduct() {
    if (marketplace_.sellers().empty()) {
        throw MarketplaceError{"Сначала добавьте продавца через пункт 1."};
    }
    std::cout << "Продавцы\n";
    for (const auto& seller : marketplace_.sellers()) {
        std::cout << seller->id() << ". " << seller->name() << '\n';
    }
    const auto& seller = marketplace_.seller(readNumber("Номер продавца: "));
    const auto name = readLine("Название товара: ");
    const Money price = readMoney("Цена за штуку, руб.: ");
    const int quantity = readNumber("Количество на складе: ");
    const auto id = seller.add_product(marketplace_, name, price, quantity);
    std::cout << "Товар выставлен. Номер: " << id << '\n';
}

void ConsoleMenu::showCatalog() const {
    if (marketplace_.products().empty()) {
        std::cout << "Каталог пуст. Выставьте товар через пункт 3.\n";
        return;
    }
    std::cout << "Каталог товаров\n";
    for (const auto& product : marketplace_.products()) {
        const auto& seller = marketplace_.seller(product.sellerId());
        std::cout << product.id() << ". " << product.name() << " | " << formatMoney(product.price())
                   << " | Остаток: " << product.quantity() << " шт. | Продавец: " << seller.name()
                   << " (#" << seller.id() << ")\n";
    }
}

void ConsoleMenu::buyProduct() {
    if (marketplace_.customers().empty()) {
        throw MarketplaceError{"Сначала добавьте покупателя через пункт 2."};
    }
    if (marketplace_.products().empty()) {
        throw MarketplaceError{"Сначала выставьте товар через пункт 3."};
    }
    std::cout << "Покупатели\n";
    for (const auto& customer : marketplace_.customers()) {
        std::cout << customer->id() << ". " << customer->name() << " | Баланс: "
                   << formatMoney(customer->balance()) << '\n';
    }
    const auto& buyer = marketplace_.customer(readNumber("Номер покупателя: "));
    showCatalog();
    const auto productId = static_cast<std::size_t>(readNumber("Номер товара: "));
    const int quantity = readNumber("Количество для покупки: ");

    const CashPayment cash;
    const CardPayment card;
    const CryptoPayment crypto;
    const std::array<const PaymentStrategy*, 3> payments{&cash, &card, &crypto};
    for (std::size_t index = 0; index < payments.size(); ++index) {
        std::cout << index + 1 << ". " << payments[index]->name() << '\n';
    }
    const int choice = readNumber("Способ оплаты: ");
    if (choice < 1 || choice > static_cast<int>(payments.size())) {
        throw MarketplaceError{"Выберите способ оплаты от 1 до 3."};
    }
    printReceipt(buyer.buy(marketplace_, productId, quantity, *payments[choice - 1]));
}
