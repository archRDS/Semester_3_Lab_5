#include "User.h"

#include "Marketplace.h"

#include <utility>

User::User(std::size_t id, std::string name) : id_(id), name_(std::move(name)) {}

Superuser::Superuser() : User(0, "Администратор") {}

const Superuser& Superuser::instance() {
    static const Superuser superuser;
    return superuser;
}

std::size_t Superuser::addSeller(Marketplace& market, std::string name) const {
    return market.addSeller(std::move(name));
}

std::size_t Superuser::addCustomer(Marketplace& market, std::string name, Money balance) const {
    return market.addCustomer(std::move(name), balance);
}

Seller::Seller(std::size_t id, std::string name) : User(id, std::move(name)) {}

std::size_t Seller::add_product(Marketplace& market, std::string name, Money price, int quantity) const {
    return market.addProduct(*this, std::move(name), price, quantity);
}

Customer::Customer(std::size_t id, std::string name, Money balance)
    : User(id, std::move(name)), balance_(balance) {}

Receipt Customer::buy(Marketplace& market, std::size_t productId, int quantity,
                      const PaymentStrategy& payment) const {
    return market.purchase(*this, productId, quantity, payment);
}
