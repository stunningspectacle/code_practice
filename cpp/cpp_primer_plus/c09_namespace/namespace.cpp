#include "namespace.h"

using std::cout;
using std::cin;
using std::endl;

namespace pers
{

	void getPerson(Person &p)
	{
		cout << "Input first name: ";
		cin >> p.fname;
		cout << "Input last name: ";
		cin >> p.lname;
	}

	void showPerson(const Person &p)
	{
		cout << p.fname << " " << p.lname << endl;
	}
}

namespace debts
{
	void getDebt(Debt & debt)
	{
		getPerson(debt.name);
		cout << "Input amount: ";
		cin >> debt.amount;
	}

	void showDebt(const Debt & debt)
	{
		showPerson(debt.name);
		cout << debt.amount << "$" << endl;
	}

	double sumDebts(const Debt debts[], int n)
	{
		double sum = 0.0;

		for (int i = 0; i < n; i++)
			sum += debts[i].amount;

		return sum;
	}
}

