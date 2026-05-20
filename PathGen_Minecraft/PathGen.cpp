#include <iostream>
#include <cstdlib>
#include "time.h"
#include <cctype>
#include <cstring>
#include "errno.h"
#include <fstream>
#include <sstream>

using namespace std;

const string f_name = "inventar.txt";

class Block
{
  friend ostream &operator<<(ostream &outFile, const Block &a);
  friend ostream &operator<<=(ostream &outFile, const Block &a);
  friend class List;

protected:
  int id;
  string name;
  char c;
  Block *next;

public:
  Block(string n_name, char n_c) : id(0), name(n_name), c(n_c), next(nullptr) {};
  ~Block() {};

  Block *gnext() { return next; };
  int gid() { return id; };
  char getc() {return c;};
  string getname() {return name;};
};

ostream &operator<<(ostream &outFile, const Block &a)
{
  // outFile << "Block ID" << " || " << "      Blockname      " << " || " << "Kürzel" << endl; // 8 21 6
  if (a.id < 10)
    outFile << "       " << a.id << " || ";
  if (a.id >= 10)
    outFile << "      " << a.id << " || ";
  int len = a.name.length();
  int l = (21 - len) / 2;
  int mod = (21 - len) % 2;
  string print = a.name;
  if (l != 0)
  {
    for (int i = 0; i < l; i++)
    {
      print = " " + print + " ";
    }
  }
  if (mod != 0)
    print = print + " ";

  print = print + " || " + a.c;
  outFile << print << endl;
  return outFile;
}

ostream &operator<<=(ostream &outFile, const Block &a)
{
  outFile << a.id << ";" << a.name << ";" << a.c << endl;
  return outFile;
}
class List
{
  friend ostream &operator<<(ostream &outFile, const List &a);
  friend ostream &operator<<=(ostream &outFile, const List &a);

private:
  Block *head;
  Block *tail;

public:
  List(Block *neu) : head(neu), tail(neu) {};
  List(const List &a)
  {
    if (a.head == nullptr)
    {
      head = nullptr;
      tail = nullptr;
      return;
    }
    head = new Block(a.head->name, a.head->c);

    Block *neu = head;
    Block *alt = a.head->next;
    while (alt != nullptr)
    {
      neu->next = new Block(alt->next->name, alt->next->c);

      neu = neu->next;
      alt = alt->next;
    }

    tail = neu;
  }
  ~List()
  {

    Block *p = head;

    while (p != nullptr)
    {
      Block *next = p->next;
      delete p;
      p = next;
    }
  }
  Block *gethead() {return head;};

  void operator+=(Block *neu);
  void operator-=(string name);
  void operator-=(int z);
  void operator -= (char z);
  void set_default();
  int cnt() const;
  void reset();
};

void List::operator+=(Block *neu)
{
  if (head == nullptr)
  {
    head = tail = neu;
    neu->id = 0;
    return;
  }
  Block *p = this->head;
  Block *tmp = nullptr;

  while (p->next != nullptr)
  {
    if (p->name == neu->name || p->c == neu->c)
    {
      cout << endl
           << "Fehler: Name oder Kuerzel bereits vergeben." << endl;
      return;
    }
    int diff = (p->next->id) - (p->id);
    if (diff != 1)
    {
      tmp = p->next;
      p->next = neu;
      neu->next = tmp;
      neu->id = (p->id) + 1;
      return;
    }

    p = p->next;
  }
  tail->next = neu;
  neu->id = (p->id) + 1;
  tail = neu;
}

void List::operator-=(string name)
{
  if (this->head == nullptr)
  {
    cout << "Fehler: Liste ist leer!" << endl;
    return;
  }

  Block *p = this->head;
  Block *prev = nullptr;

  while (p != nullptr && p->name != name)
  {
    prev = p;
    p = p->next;
  }

  if (p == nullptr)
  {
    cout << "Fehler: Block nicht gefunden!" << endl;
    return;
  }

  if (prev == nullptr)
  {
    this->head = p->next;

    if (this->head == nullptr)
    {
      this->tail = nullptr;
    }
  }
  else
  {
    prev->next = p->next;

    if (p == this->tail)
    {
      this->tail = prev;
    }
  }

  delete p;
}
void List::operator-=(int z)
{
  if (this->head == nullptr)
  {
    cout << "Fehler: Liste ist leer!" << endl;
    return;
  }

  Block *p = this->head;
  Block *prev = nullptr;

  while (p != nullptr && p->id != z)
  {
    prev = p;
    p = p->next;
  }

  if (p == nullptr)
  {
    cout << "Fehler: Block nicht gefunden!" << endl;
    return;
  }

  if (prev == nullptr)
  {
    this->head = p->next;

    if (this->head == nullptr)
    {
      this->tail = nullptr;
    }
  }
  else
  {
    prev->next = p->next;

    if (p == this->tail)
    {
      this->tail = prev;
    }
  }

  delete p;
}
void List::operator-=(char z)
{
  if (this->head == nullptr)
  {
    cout << "Fehler: Liste ist leer!" << endl;
    return;
  }

  Block *p = this->head;
  Block *prev = nullptr;

  while (p != nullptr && p->c != z)
  {
    prev = p;
    p = p->next;
  }

  if (p == nullptr)
  {
    cout << "Fehler: Block nicht gefunden!" << endl;
    return;
  }

  if (prev == nullptr)
  {
    this->head = p->next;

    if (this->head == nullptr)
    {
      this->tail = nullptr;
    }
  }
  else
  {
    prev->next = p->next;

    if (p == this->tail)
    {
      this->tail = prev;
    }
  }

  delete p;
}
int List::cnt() const
{
  Block *p = head;
  int i = 0;
  while (p != nullptr)
  {
    p = p->next;
    i++;
  }
  return i;
}
void List::reset()
{
while (head!=nullptr)
{
  Block *tmp = head;
  head = head->next;
  delete tmp;
}
Block *neu = new Block("gravel", 'g');
*this += neu;
set_default();
cout << "Standardwerte wurden wiederhergestellt!"<<endl;
}

ostream &operator<<(ostream &outFile, const List &a)
{
  if (a.head == nullptr)
  {
    outFile << "Fehler: Liste ist leer!" << endl;
    return outFile;
  }
  Block *p = a.head;
  outFile << "Anzahl gespeicherter Bloecke: " << a.cnt() << endl
          << "--------------------------------" << endl
          << "Block ID" << " || " << "      Blockname      " << " || " << "Kuerzel" << endl
          << "--------------------------------------------" << endl;
  while (p != nullptr)
  {
    outFile << *p;
    p = p->gnext();
  }
  return outFile;
}

ostream &operator<<=(ostream &outFile, const List &a)
{
  Block *p = a.head;
  while (p != nullptr)
  {
    outFile <<= *p;
    p = p->gnext();
  }
  return outFile;
}

void List::set_default()
{
  string sarr[5] = {"stone", "stonebrick", "cobblestone", "(chiseled)_dirt", "andesite"};
  char carr[5] = {'s', 'b', 'c', 'd', 'a'};

  for (int i = 0; i < 5; i++)
  {
    Block *neu = new Block(sarr[i], carr[i]);
    (*this) += neu;
  }
}

class Path
{
  friend ostream &operator<<(ostream &outFile, const Path &a);

private:
  int path_len;
  int path_wide;
  char *path;
  Path *next;

public:
  Path(int len, int wide) : path_len(len), path_wide(wide), path(nullptr), next(nullptr) {};
  ~Path()
  {
    delete[] path;
    delete next;
  }

  char *set_arr(List *inventory);
  void chain(int len, int wide, List *inventory);
};

ostream &operator<<(ostream &outFile, const Path &a)
{
  int z = 0;
  const Path *current = &a;
  while (current != nullptr)
  {
    for (int i = 0; i < current->path_wide; ++i)
    {
      outFile << current->path[i] << " ";
    }
    outFile << endl;
    current = current->next;
    z++;
    if (z % 5 == 0)
      outFile << '\n';
  }
  return outFile;
}


char *Path::set_arr(List *inventory)
{
  path = new char[path_wide];
  for (int i = 0; i <= (path_wide); i++)
  {
    int n = rand() % inventory->cnt();

    Block *p = inventory->gethead();
    for (int z = 0; z < n; z++) 
    {p = p ->gnext(); }

    path[i] = (p->getc());
  }
  return path;
}

void Path::chain(int len, int wide, List *inventory)
{
  Path *p = this;
  for (int i = 1; i < len; ++i)
  {
    p->next = new Path(len - i, wide);
    p = p->next;
    p->set_arr(inventory);
  }
}

void gen(List *iventory);
void add(List *inventory);
void ls(List *inventory);
void del(List *inventory);
List *read();
void write(List *inventory);
bool changed = false;

int main(int argc, const char **argv)
{
  srand(time(nullptr));

  List *inventory = read();
  if (inventory->cnt() == 0)
  {
    Block *gravel = new Block("gravel", 'g');
    *inventory += gravel;
    inventory->set_default();
    cin.clear();
    changed = true;
  }
  string word;

  if (argc != 2)
  {
    cout << "Funktionen:" << endl
         << "Pfad generieren              : " << "gen" << endl
         << "Blockbibliothek erweitern    : " << "add" << endl
         << "Blockbibliothek verkleinern  : " << "del" << endl
         << "Blockbibliothek zurücksetzen : " << "res" << endl
         << "Blockbibliothek ausgeben     : " << "ls" << endl
         << "Prgramm beenden              : " << "end" << endl;
    cout << endl
         << "Wenn mir jemals langweilig werden sollte baue ich vllt eine grafische Oberflaeche." << endl;
    cout << "Warte auf Eingabe: ";
    cin >> word;
    cin.clear();
    cin.ignore(30, '\n');
    cout << endl;
  }
  else
  {
    word = argv[1];
  }

  for (;;)
  {
    if (word == "gen") // tatsächlich können switch cases keine strings, nur ints und chars
    {
      gen(inventory);
    }
    else if (word == "add")
    {
      add(inventory);
    }
    else if (word == "del")
    {
      del(inventory);
    }else if (word == "res")
    {
      inventory->reset();
      changed = true;
    }
    else if (word == "ls")
    {
      ls(inventory);
    }
    else if (word == "end")
    {
      if (changed == true)
        write(inventory);
      delete inventory;
      exit(EXIT_SUCCESS);
    }
    else
    {
      cout << "Fehler, falsche Eingabe!" << endl
           << endl;
    }

    cout << "Funktionen:" << endl
         << "Pfad generieren              : " << "gen" << endl
         << "Blockbibliothek erweitern    : " << "add" << endl
         << "Blockbibliothek verkleinern  : " << "del" << endl
         << "Blockbibliothek zurücksetzen : " << "res" << endl
         << "Blockbibliothek ausgeben     : " << "ls" << endl
         << "Prgramm beenden              : " << "end" << endl
         << "Warte auf Eingabe            : ";
    cin >> word;
    cin.ignore(30, '\n');
    cout << endl;
  }
}

void gen(List *inventory)
{
  int len, wide = 0;
  cout << "Warte auf Eingabe, Laenge: ";
  cin >> len;
  cin.ignore(30, '\n');
  if (cin.fail())
  {
    cout << endl
         << "Fehler: Eingabe muss numerisch sein\n\n";
    cin.clear();
    return;
  }
  cout << "Warte auf Eingabe, Breite: ";
  cin >> wide;
  cin.ignore(30, '\n');
  if (cin.fail())
  {
    cout << endl
         << "Fehler: Eingabe muss numerisch sein\n\n";
    return;

  }

  if (len <= 0 || wide <= 0)
  {

    cout << endl
         << "Fehler: Werte > 0 erforderlich\n\n";
    return;
  }

  if (wide > 60)
  {
    cout << endl
         << "Jetzt uebertreibst du es aber." << endl
         << endl;
    return;
  }
  if (wide > 50)
  {
    cout << endl
         << "Das ist kein(e) Strasse/Weg mehr, das ist ein Platz!" << endl;
  }
  Block *p = inventory->gethead();
  cout << endl<< "Kuerzel" << " || " << "Blockname" << endl;
  cout << "--------------------------------" << endl;
  for (int i = 0; i < inventory->cnt(); i++)
  {
    cout << "   " << p->getc() << "   " << " || " << p->getname() << endl; 
    p = p ->gnext();
  }
  cout << endl; 

  Path *head = new Path(len, wide);
  head->set_arr(inventory);
  head->chain(len, wide, inventory);

  cout << *head;
  delete head;
  return;
}

void add(List *inventory)
{
  string name = " ";
  char c = ' ';
  do
  {
    cout << "'end' eingeben um zurueckzukehren" << endl;
    cout << "Erwarte Eingabe, Blockname: ";
    cin >> name;
    cin.ignore(30, '\n');
    if (name == "end") return;
    if (name.length() > 21)
    {
      cout << endl
           << "Fehler: Name darf maximal 21 Zeichen lang sein.\n\n";
      cin.clear();
      name = " ";
    }
  } while (name == " ");

  do
  {
    cout << "Erwarte Eingabe, Blockkuerzel: ";
    cin >> c;
    cin.ignore(30, '\n');
  } while (c == ' ');

  Block *neu = new Block(name, c);
  // cout << neu;
  (*inventory) += neu;
  cin.clear();
  changed = true;
}

void ls(List *inventory)
{
  cout << *inventory << endl;
}

void del(List *inventory)
{
  ls(inventory);

  cout << endl
       << "Welcher Block soll entfernt werden?" << endl;
  string name = " ";
  int id;
  char c;
  do
  {
    cout << "'end' eingeben um zurueckzukehren" << endl;
    cout << "Erwarte Eingabe, Blockname/ID/Blockkuerzel: ";
    cin >> name;
    cin.ignore(30, '\n');
    if (name == "end") return;
    if (name.length() <= 2)
    {
      char z1 = name[0];
      char z2 = name[1];
      if (isdigit(z1))
      {
        id =(z1-'0');
        if (isdigit((z2)))
        {
          id *=10;
        id += (z2-'0');
        }
        (*inventory) -= id;
        changed = true;
        return;
      }
        (*inventory) -= z1;
        changed = true;
        return;
    }
    if (name.length() > 21)
    {
      cout << endl
           << "Fehler: Name darf maximal 21 Zeichen lang sein.\n\n";
      cin.clear();
      name = " ";
    }
  } while (name == " ");

  (*inventory) -= name;
  changed = true;
}

List *read()
{
  List *inventory = new List(nullptr);
  ifstream file(f_name);

  if (!file.is_open())
  {
    cout << "Fehler: Blockbibliothek konnte nicht geoeffnet werden." << endl
         << "Lege neue Bibliothek an." << endl;
    //   cout << "lebe noch" << endl;
    return inventory;
  }
  string line;
  while (getline(file, line))
  {
    stringstream ss(line);
    string name;
    string id;
    string kr;

    getline(ss, id, ';');
    getline(ss, name, ';');
    getline(ss, kr, '\n');

    char c = kr[0];
    Block *neu = new Block(name, c);
    (*inventory) += neu;
  }
  file.close();
  return inventory;
}

void write(List *inventory)
{
  ofstream file(f_name);

  if (!file.is_open())
  {
    std::cerr << "Fehler beim Erstellen der Datei\n";
    std::cerr << "errno: " << errno << "\n";
    std::cerr << "Beschreibung: "
              << std::strerror(errno) << "\n";
    exit(EXIT_FAILURE);
  }
  file <<= *inventory;
  cout << "Bibliothek wurde erfolreich gespeichert!" <<endl;
  file.close();
}

