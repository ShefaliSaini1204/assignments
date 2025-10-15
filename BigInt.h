#ifndef BIGINT_H
#define BIGINT_H

#include <iostream>
#include <vector>
#include <string>

class BigInt {
private:
    std::vector<int> digits;
    bool negative;

    void removeLeadingZeros();
    static int absCompare(const BigInt &a, const BigInt &b);
    static BigInt absAdd(const BigInt &a, const BigInt &b);
    static BigInt absSub(const BigInt &a, const BigInt &b);

public:
    
    BigInt();                                           
    BigInt(const std::vector<int> &v);                  
    BigInt(const std::vector<char> &v);                 
    BigInt(const char *num, size_t size);               
    BigInt(long long num);                          

    
    BigInt operator+(const BigInt &other) const;
    BigInt operator-(const BigInt &other) const;
    BigInt operator*(const BigInt &other) const;
    BigInt operator!() const;

    bool operator==(const BigInt &other) const;
    bool operator!=(const BigInt &other) const { return !(*this == other); }
    bool operator<(const BigInt &other) const;
    bool operator>(const BigInt &other) const { return other < *this; }
    bool operator<=(const BigInt &other) const { return !(other < *this); }
    bool operator>=(const BigInt &other) const { return !(*this < other); }

    
    BigInt &operator++();   
    BigInt operator++(int); 
    BigInt &operator--();   
    BigInt operator--(int); 

    
    friend std::ostream &operator<<(std::ostream &os, const BigInt &num);
    friend std::istream &operator>>(std::istream &is, BigInt &num);
};

#endif
