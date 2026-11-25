#include "Marketplace.h"

#include "PaymentStrategy.h"

#include <algorithm>
#include <limits>
#include <utility>

namespace {
template<class Participant>
Participant& findParticipant(const std::vector<std::unique_ptr<Participant>>& participants,
                             std::size_t id, const char* error) {
    const auto found = std::find_if(participants.begin(), participants.end(), [id](const auto& user) {
        return user->id() == id;
    });
    if (found == participants.end()) {
        throw MarketplaceError{error};
    }
    return **found;
}

void validateName(const std::string& name, const char* error) {
    if (name.find_first_not_of(" \t\r\n") == std::string::npos) {
        throw MarketplaceError{error};
    }
}
}

const Seller& Marketplace::seller(std::size_t id) const {
    return findParticipant(sellers_, id, "Продавец не найден.");
}

const Customer& Marketplace::customer(std::size_t id) const {
    return findParticipant(customers_, id, "Покупатель не найден.");
}

std::size_t Marketplace::addSeller(std::string name) {
    validateName(name, "Введите имя продавца.");
    auto seller = std::unique_ptr<Seller>(new Seller(nextUserId_, std::move(name)));
    sellers_.push_back(std::move(seller));
    return nextUserId_++;
}

std::size_t Marketplace::addCustomer(std::string name, Money balance) {
    validateName(name, "Введите имя покупателя.");
    if (balance < 0) {
        throw MarketplaceError{"Баланс покупателя должен быть неотрицательным."};
    }
    auto customer = std::unique_ptr<Customer>(new Customer(nextUserId_, std::move(name), balance));
    customers_.push_back(std::move(customer));
    return nextUserId_++;
}

std::size_t Marketplace::addProduct(const Seller& owner, std::string name, Money price, int quantity) {
    if (&seller(owner.id()) != &owner) {
        throw MarketplaceError{"Продавец не зарегистрирован на этой площадке."};
    }
    validateName(name, "Введите название товара.");
    if (price <= 0 || quantity <= 0) {
        throw MarketplaceError{"Цена и количество должны быть больше нуля."};
    }

    const std::size_t id = products_.size() + 1;
    products_.push_back(Product(id, std::move(name), price, quantity, owner.id()));
    return id;
}

Receipt Marketplace::purchase(const Customer& buyer, std::size_t productId, int quantity,
                              const PaymentStrategy& payment) {
    auto& customer = findParticipant(customers_, buyer.id(), "Покупатель не найден.");
    if (&customer != &buyer) {
        throw MarketplaceError{"Покупатель не зарегистрирован на этой площадке."};
    }
    if (productId == 0 || productId > products_.size()) {
        throw MarketplaceError{"Товар не найден."};
    }
    auto& product = products_[productId - 1];
    if (quantity <= 0) {
        throw MarketplaceError{"Количество должно быть больше нуля."};
    }
    if (quantity > product.quantity()) {
        throw MarketplaceError{"На складе недостаточно товара."};
    }
    if (product.price() > std::numeric_limits<Money>::max() / quantity) {
        throw MarketplaceError{"Сумма покупки слишком велика."};
    }

    const Money total = product.price() * quantity;
    if (customer.balance() < total) {
        throw MarketplaceError{"На балансе покупателя недостаточно средств."};
    }

    Receipt receipt{buyer.name(), seller(product.sellerId()).name(), product.name(), quantity,
                    product.price(), total, payment.name(), buyer.balance()};
    if (!payment.pay(receipt.remainingBalance, total)) {
        throw MarketplaceError{"Оплата не прошла."};
    }

    customer.balance_ = receipt.remainingBalance;
    product.quantity_ -= quantity;
    return receipt;
}
