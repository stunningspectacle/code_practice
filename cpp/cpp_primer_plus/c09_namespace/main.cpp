#include "namespace.h"

int main()
{
	debts::Debt d0, d1, d2;
	debts::getDebt(d0);
	debts::getDebt(d1);
	debts::getDebt(d2);

	debts::showDebt(d0);
	debts::showDebt(d1);
	debts::showDebt(d2);

	debts::Debt debts[] = {d0, d1, d2};
	std::cout << debts::sumDebts(debts, 3) << std::endl;
}
	
