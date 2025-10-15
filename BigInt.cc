#include "BigInt.h"
#include <algorithm>
#include <cctype>


void BigInt::removeLeadingZeros() {
    while (digits.size() > 1 && digits.back() == 0)
        digits.pop_back();
    if (digits.size() == 1 && digits[0] == 0)
        negative = false;
}


BigInt::BigInt() : digits(1, 0), negative(false) {}


BigInt::BigInt(const std::vector<int> &v) {
    if (v.empty()) {
        digits = {0};
        negative = false;
    } else {
        negative = (v[0] < 0);
        digits.clear();
        for (size_t i = (negative ? 1 : 0); i < v.size(); ++i)
            digits.insert(digits.begin(), std::abs(v[i]));
        removeLeadingZeros();
    }
}


BigInt::BigInt(const std::vector<char> &v) {
    if (v.empty()) {
        digits = {0};
        negative = false;
    } else {
        negative = (v[0] == '-');
        digits.clear();
        for (size_t i = (negative ? 1 : 0); i < v.size(); ++i) {
            if (isdigit(v[i]))
                digits.insert(digits.begin(), v[i] - '0');
        }
        removeLeadingZeros();
    }
}


BigInt::BigInt(const char *num, size_t size) {
    if (size == 0) {
        digits = {0};
        negative = false;
        return;
    }
    negative = (num[0] == '-');
    digits.clear();
    for (size_t i = (negative ? 1 : 0); i < size; ++i)
        if (isdigit(num[i]))
            digits.insert(digits.begin(), num[i] - '0');
    removeLeadingZeros();
}


BigInt::BigInt(long long num) {
    negative = (num < 0);
    num = std::llabs(num);
    if (num == 0)
        digits = {0};
    else {
        while (num > 0) {
            digits.push_back(num % 10);
            num /= 10;
        }
    }
}


int BigInt::absCompare(const BigInt &a, const BigInt &b) {
    if (a.digits.size() != b.digits.size())
        return (a.digits.size() < b.digits.size()) ? -1 : 1;
    for (int i = a.digits.size() - 1; i >= 0; --i) {
        if (a.digits[i] != b.digits[i])
            return (a.digits[i] < b.digits[i]) ? -1 : 1;
    }
    return 0;
}


BigInt BigInt::absAdd(const BigInt &a, const BigInt &b) {
    BigInt result;
    result.digits.clear();
    int carry = 0;
    size_t n = std::max(a.digits.size(), b.digits.size());
    for (size_t i = 0; i < n; ++i) {
        int sum = carry;
        if (i < a.digits.size()) sum += a.digits[i];
        if (i < b.digits.size()) sum += b.digits[i];
        result.digits.push_back(sum % 10);
        carry = sum / 10;
    }
    if (carry) result.digits.push_back(carry);
    return result;
}

BigInt BigInt::absSub(const BigInt &a, const BigInt &b) {
    BigInt result;
    result.digits.clear();
    int borrow = 0;
    for (size_t i = 0; i < a.digits.size(); ++i) {
        int diff = a.digits[i] - (i < b.digits.size() ? b.digits[i] : 0) - borrow;
        if (diff < 0) { diff += 10; borrow = 1; }
        else borrow = 0;
        result.digits.push_back(diff);
    }
    result.removeLeadingZeros();
    return result;
}

BigInt BigInt::operator+(const BigInt &other) const {
    BigInt result;
    if (negative == other.negative) {
        result = absAdd(*this, other);
        result.negative = negative;
    } else {
        int cmp = absCompare(*this, other);
        if (cmp == 0) return BigInt(0);
        if (cmp > 0) {
            result = absSub(*this, other);
            result.negative = negative;
        } else {
            result = absSub(other, *this);
            result.negative = other.negative;
        }
    }
    result.removeLeadingZeros();
    return result;
}


BigInt BigInt::operator-(const BigInt &other) const {
    BigInt negOther = other;
    negOther.negative = !other.negative;
    return *this + negOther;
}


BigInt BigInt::operator*(const BigInt &other) const {
    BigInt result;
    result.digits.assign(digits.size() + other.digits.size(), 0);
    for (size_t i = 0; i < digits.size(); ++i)
        for (size_t j = 0; j < other.digits.size(); ++j)
            result.digits[i + j] += digits[i] * other.digits[j];

    for (size_t i = 0; i < result.digits.size(); ++i) {
        if (result.digits[i] >= 10) {
            if (i + 1 == result.digits.size())
                result.digits.push_back(0);
            result.digits[i + 1] += result.digits[i] / 10;
            result.digits[i] %= 10;
        }
    }

    result.negative = (negative != other.negative);
    result.removeLeadingZeros();
    return result;
}


BigInt BigInt::operator!() const {
    BigInt temp = *this;
    if (!(temp.digits.size() == 1 && temp.digits[0] == 0))
        temp.negative = !temp.negative;
    return temp;
}


bool BigInt::operator==(const BigInt &other) const {
    return (negative == other.negative && digits == other.digits);
}

bool BigInt::operator<(const BigInt &other) const {
    if (negative != other.negative)
        return negative;
    int cmp = absCompare(*this, other);
    return (negative ? cmp > 0 : cmp < 0);
}


BigInt &BigInt::operator++() {
    *this = *this + BigInt(1);
    return *this;
}

BigInt BigInt::operator++(int) {
    BigInt temp = *this;
    ++(*this);
    return temp;
}

BigInt &BigInt::operator--() {
    *this = *this - BigInt(1);
    return *this;
}

BigInt BigInt::operator--(int) {
    BigInt temp = *this;
    --(*this);
    return temp;
}


std::ostream &operator<<(std::ostream &os, const BigInt &num) {
    if (num.negative) os << '-';
    for (int i = num.digits.size() - 1; i >= 0; --i)
        os << num.digits[i];
    return os;
}

std::istream &operator>>(std::istream &is, BigInt &num) {
    std::string s; is >> s;
    num = BigInt(s.c_str(), s.size());
    return is;
}
