#pragma once

#include "Money.h"

#include <cstddef>
#include <string>

class Marketplace;
class PaymentStrategy;
struct Receipt;

class User {
public:
    virtual ~User() = default;
    std::size_t id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }

    User(const User&) = delete;
    User& operator=(const User&) = delete;

protected:
    User(std::size_t id, std::string name);

private:
    std::size_t id_;
    std::string name_;
};

class Superuser final : public User {
public:
    static const Superuser& instance();

    std::size_t addSeller(Marketplace& market, std::string name) const;
    std::size_t addCustomer(Marketplace& market, std::string name, Money balance) const;

private:
    Superuser();
};

class Seller final : public User {
public:
    std::size_t add_product(Marketplace& market, std::string name, Money price, int quantity) const;

private:
    friend class Marketplace;
    Seller(std::size_t id, std::string name);
};

class Customer final : public User {
public:
    Money balance() const noexcept { return balance_; }
    Receipt buy(Marketplace& market, std::size_t productId, int quantity,
                const PaymentStrategy& payment) const;

private:
    friend class Marketplace;
    Customer(std::size_t id, std::string name, Money balance);

    Money balance_;
};
