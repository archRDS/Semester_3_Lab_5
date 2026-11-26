#include "PaymentStrategy.h"

bool PaymentStrategy::debit(Money& balance, Money amount) noexcept {
    if (amount <= 0 || balance < amount) {
        return false;
    }
    balance -= amount;
    return true;
}

const wchar_t* CashPayment::name() const noexcept {
    return L"Наличные";
}

bool CashPayment::pay(Money& balance, Money amount) const noexcept {
    return debit(balance, amount);
}

const wchar_t* CardPayment::name() const noexcept {
    return L"Банковская карта";
}

bool CardPayment::pay(Money& balance, Money amount) const noexcept {
    return debit(balance, amount);
}

const wchar_t* CryptoPayment::name() const noexcept {
    return L"Криптовалюта";
}

bool CryptoPayment::pay(Money& balance, Money amount) const noexcept {
    return debit(balance, amount);
}
