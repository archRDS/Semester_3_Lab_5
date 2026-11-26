#include "Marketplace.h"

#include "PaymentStrategy.h"

#include <algorithm>
#include <limits>
#include <utility>

namespace {
template<class Participant>
Participant& findParticipant(const std::vector<std::unique_ptr<Participant>>& participants,
                             std::size_t id, const wchar_t* error) {
    const auto found = std::find_if(participants.begin(), participants.end(), [id](const auto& user) {
        return user->id() == id;
    });
    if (found == participants.end()) {
        throw MarketplaceError{error};
    }
    return **found;
}

void validateName(const std::wstring& name, const wchar_t* error) {
    if (name.find_first_not_of(L" \t\r\n") == std::wstring::npos) {
        throw MarketplaceError{error};
    }
}
}

const Seller& Marketplace::seller(std::size_t id) const {
    return findParticipant(sellers_, id, L"Продавец не найден.");
}

const Customer& Marketplace::customer(std::size_t id) const {
    return findParticipant(customers_, id, L"Покупатель не найден.");
}

std::size_t Marketplace::addSeller(std::wstring name) {
    validateName(name, L"Введите имя продавца.");
    auto seller = std::unique_ptr<Seller>(new Seller(nextUserId_, std::move(name)));
    sellers_.push_back(std::move(seller));
    return nextUserId_++;
}

std::size_t Marketplace::addCustomer(std::wstring name, Money balance) {
    validateName(name, L"Введите имя покупателя.");
    if (balance < 0) {
        throw MarketplaceError{L"Баланс покупателя должен быть неотрицательным."};
    }
    auto customer = std::unique_ptr<Customer>(new Customer(nextUserId_, std::move(name), balance));
    customers_.push_back(std::move(customer));
    return nextUserId_++;
}

std::size_t Marketplace::addProduct(const Seller& owner, std::wstring name, Money price, int quantity) {
    if (&seller(owner.id()) != &owner) {
        throw MarketplaceError{L"Продавец не зарегистрирован на этой площадке."};
    }
    validateName(name, L"Введите название товара.");
    if (price <= 0 || quantity <= 0) {
        throw MarketplaceError{L"Цена и количество должны быть больше нуля."};
    }

    const std::size_t id = products_.size() + 1;
    products_.push_back(Product(id, std::move(name), price, quantity, owner.id()));
    return id;
}

Receipt Marketplace::purchase(const Customer& buyer, std::size_t productId, int quantity,
                              const PaymentStrategy& payment) {
    auto& customer = findParticipant(customers_, buyer.id(), L"Покупатель не найден.");
    if (&customer != &buyer) {
        throw MarketplaceError{L"Покупатель не зарегистрирован на этой площадке."};
    }
    if (productId == 0 || productId > products_.size()) {
        throw MarketplaceError{L"Товар не найден."};
    }
    auto& product = products_[productId - 1];
    if (quantity <= 0) {
        throw MarketplaceError{L"Количество должно быть больше нуля."};
    }
    if (quantity > product.quantity()) {
        throw MarketplaceError{L"На складе недостаточно товара."};
    }
    if (product.price() > std::numeric_limits<Money>::max() / quantity) {
        throw MarketplaceError{L"Сумма покупки слишком велика."};
    }

    const Money total = product.price() * quantity;
    if (customer.balance() < total) {
        throw MarketplaceError{L"На балансе покупателя недостаточно средств."};
    }

    Receipt receipt{buyer.name(), seller(product.sellerId()).name(), product.name(), quantity,
                    product.price(), total, payment.name(), buyer.balance()};
    if (!payment.pay(receipt.remainingBalance, total)) {
        throw MarketplaceError{L"Оплата не прошла."};
    }

    customer.balance_ = receipt.remainingBalance;
    product.quantity_ -= quantity;
    return receipt;
}
