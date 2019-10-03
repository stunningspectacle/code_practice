#include <string>

class Stock
{
	std::string name;
	int shares;
	double price;
	double total_price;
	void calc_total() { total_price = price * shares; }
public:
	Stock();
	//Stock(const Stock &);
	Stock(const std::string &s, int nr = 0, double p = 0.0);
	~Stock();
	void show() const;
	void acquire(const char *s, int nr, double price);
	void sell(int nr);
	const Stock & topval(const Stock &s) const;
	Stock operator+(const Stock &s) const;
	Stock operator*(int n) const;
	Stock operator-() const;
	friend Stock operator*(int n, const Stock &s);
	friend std::ostream & operator<<(std::ostream &os, const Stock &s);
	operator int() const;
};

