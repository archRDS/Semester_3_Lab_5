#include "PaymentStrategy.h"

bool PaymentStrategy::debit(Money& balance, Money amount) noexcept {
    if (amount <= 0 || balance < amount) {
        return false;
    }
    balance -= amount;
    return true;
}

const char* CashPayment::name() const noexcept {
    return "Наличные";
}

bool CashPayment::pay(Money& balance, Money amount) const noexcept {
    return debit(balance, amount);
}

const char* CardPayment::name() const noexcept {
    return "Банковская карта";
}

bool CardPayment::pay(Money& balance, Money amount) const noexcept {
    return debit(balance, amount);
}

const char* CryptoPayment::name() const noexcept {
    return "Криптовалюта";
}

bool CryptoPayment::pay(Money& balance, Money amount) const noexcept {
    return debit(balance, amount);
}
