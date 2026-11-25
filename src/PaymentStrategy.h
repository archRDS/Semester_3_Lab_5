#pragma once

#include "Money.h"

class PaymentStrategy {
public:
    virtual ~PaymentStrategy() = default;
    virtual const char* name() const noexcept = 0;
    virtual bool pay(Money& balance, Money amount) const noexcept = 0;

protected:
    static bool debit(Money& balance, Money amount) noexcept;
};

class CashPayment final : public PaymentStrategy {
public:
    const char* name() const noexcept override;
    bool pay(Money& balance, Money amount) const noexcept override;
};

class CardPayment final : public PaymentStrategy {
public:
    const char* name() const noexcept override;
    bool pay(Money& balance, Money amount) const noexcept override;
};

class CryptoPayment final : public PaymentStrategy {
public:
    const char* name() const noexcept override;
    bool pay(Money& balance, Money amount) const noexcept override;
};
