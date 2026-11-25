#pragma once

#include "Money.h"

#include <cstddef>
#include <string>
#include <utility>

class Product {
public:
    std::size_t id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }
    Money price() const noexcept { return price_; }
    int quantity() const noexcept { return quantity_; }
    std::size_t sellerId() const noexcept { return sellerId_; }

private:
    friend class Marketplace;

    Product(std::size_t id, std::string name, Money price, int quantity, std::size_t sellerId)
        : id_(id), name_(std::move(name)), price_(price), quantity_(quantity), sellerId_(sellerId) {}

    std::size_t id_;
    std::string name_;
    Money price_;
    int quantity_;
    std::size_t sellerId_;
};
