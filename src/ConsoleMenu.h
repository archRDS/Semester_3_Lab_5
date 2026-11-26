#pragma once

#include "Marketplace.h"

class ConsoleMenu {
public:
    void run();

private:
    void addSeller();
    void addCustomer();
    void addProduct();
    void showCatalog() const;
    void buyProduct();

    Marketplace marketplace_;
};
