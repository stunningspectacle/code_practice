#include <iostream>
#include <string>
using std::cout;
using std::endl;
using std::string;
using std::ostream;

class Player
{
	string m_firstname;
	string m_lastname;
public:
	Player(const string &fs = "none", const string &ls = "none"):
		m_firstname(fs), m_lastname(ls) { };
	virtual ~Player() { cout << "~Player()" << endl; };
	virtual void showPlayer() const = 0;
	friend ostream & operator<<(ostream &os, const Player &p);
};

void Player::showPlayer() const
{
	cout << m_firstname << " " << m_lastname << endl;
}

class TableTenisPlayer : public Player
{
	bool m_hasTable;

public:
	TableTenisPlayer(const string &fs = "none",
			const string &ls = "none", bool hasTable=false) :
		Player(fs, ls), m_hasTable(hasTable) { };
	virtual ~TableTenisPlayer() { cout << "~TableTenisPlayer()" << endl; };
	virtual void showPlayer() const;
	bool hasTable() const { return m_hasTable; }
};

ostream & operator<<(ostream &os, const Player &p)
{
	os << p.m_firstname << " " << p.m_lastname;

	return os;
}

void TableTenisPlayer::showPlayer() const
{
	Player::showPlayer();
	cout << "m_hasTable:" << m_hasTable << endl;
}

int main()
{
	Player *p0, *p1;

	//Player p("Leo", "Yan"); 
	//cout << p << endl;

	TableTenisPlayer t("Jack", "Wang");
	cout << t << endl;

	//p0 = &p;
	p1 = &t;

	//p0->showPlayer();
	p1->showPlayer();

	double pi = 3.1415;
	int *i0 = (int *)&pi;

	cout << pi << " " << i0 << endl;

}
