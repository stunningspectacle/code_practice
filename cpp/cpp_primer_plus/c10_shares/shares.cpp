#include <iostream>
#include "shares.h"

Stock::Stock()
{
	name = "no name";
	shares = 0;
	price = 0.0;
	calc_total();
}

/*
Stock::Stock(const Stock &s)
{
	name = s.name;
	shares = s.shares;
	price = s.price;
	calc_total();
}
*/

Stock::Stock(const std::string &s, int nr, double p)
{
	name = s;
	shares = nr;
	price = p;
	calc_total();
}

Stock::~Stock()
{
	std::cout << "Bye..." << name << std::endl;
}

void Stock::show() const
{
	std::cout.setf(std::ios_base::fixed, std::ios_base::showpoint);
	std::cout.precision(5);
	std::cout << name << ": " << " shares: " << shares
		<< ", price: " << price
		<< ", total price: " << total_price << std::endl;
}

void Stock::acquire(const char *s, int nr, double p)
{
	name = s;
	shares = nr;
	price = p;
	calc_total();
}

void Stock::sell(int nr)
{
	shares -= nr;
	calc_total();
}

const Stock & Stock::topval(const Stock &s) const
{
	if (s.price > price) {
		std::cout << s.name << " is higher" << std::endl;
		return s;
	}

	std::cout << name << " is higher" << std::endl;
	return *this;
}

Stock Stock::operator+(const Stock &s) const
{
	Stock t;

	t.name = name + s.name;
	t.price = price + s.price;
	t.shares = shares + s.shares;
	t.calc_total();

	return t;
}

Stock Stock::operator*(int n) const
{
	Stock t;

	t.price = price * n;
	t.shares = shares * n;
	t.calc_total();

	return t;
}

Stock operator*(int n, const Stock &s)
{
	Stock t;

	t.price = s.price * n;
	t.shares = s.shares * n;
	t.calc_total();

	return t;
}

std::ostream & operator<<(std::ostream &os, const Stock &s)
{
	os << "Name:" << s.name << " price:"
		<< s.price << " shares:" << s.shares << std::endl;
	return os;
}

Stock Stock::operator-() const
{
	return Stock(name, -shares, -price);
}

Stock::operator int() const
{
	return shares;
}
