#pragma once

#include "Product.h"
#include "User.h"

#include <memory>
#include <vector>

struct MarketplaceError {
    const wchar_t* message;
};

struct Receipt {
    std::wstring customerName;
    std::wstring sellerName;
    std::wstring productName;
    int quantity;
    Money unitPrice;
    Money total;
    std::wstring paymentMethod;
    Money remainingBalance;
};

class Marketplace {
public:
    const Seller& seller(std::size_t id) const;
    const Customer& customer(std::size_t id) const;
    const std::vector<std::unique_ptr<Seller>>& sellers() const noexcept { return sellers_; }
    const std::vector<std::unique_ptr<Customer>>& customers() const noexcept { return customers_; }
    const std::vector<Product>& products() const noexcept { return products_; }

private:
    friend class Superuser;
    friend class Seller;
    friend class Customer;

    std::size_t addSeller(std::wstring name);
    std::size_t addCustomer(std::wstring name, Money balance);
    std::size_t addProduct(const Seller& seller, std::wstring name, Money price, int quantity);
    Receipt purchase(const Customer& customer, std::size_t productId, int quantity,
                     const PaymentStrategy& payment);

    std::vector<std::unique_ptr<Seller>> sellers_;
    std::vector<std::unique_ptr<Customer>> customers_;
    std::vector<Product> products_;
    std::size_t nextUserId_ = 1;
};
