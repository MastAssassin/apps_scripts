#include <iostream>
#include <cstdlib>
#include <random>
#include "time.h"
#include <cctype>

using namespace std;

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

  char *set_arr();
  void chain(int len, int wide);
};

char *Path::set_arr()
{
  path = new char[path_wide];
  for (int i = 0; i <= (path_wide); i++)
  {
    int n = rand() % 6;
    char c;
    switch (n)
    {
    case 0:
      c = 'g';
      break;
    case 1:
      c = 's';
      break;
    case 2:
      c = 'b';
      break;
    case 3:
      c = 'c';
      break;
    case 4:
      c = 'd';
      break;
    case 5:
      c = 'a';
      break;
    default:
      c = '?';
      break;
    }

    path[i] = c;
  }
  return path;
}

void Path::chain(int len, int wide)
{
  Path *p = this;
  for (int i = 1; i < len; ++i)
  {
    p->next = new Path(len - i, wide);
    p = p->next;
    p->set_arr();
  }
}

ostream &operator<<(ostream &outFile, const Path &a)
{
  outFile << "g = gravel\n"
          << "s = stone\n"
          << "b = stonebrick\n"
          << "c = cobblestone\n"
          << "d = (chiseled) dirt\n"
          << "a = andesite\n"
          << endl;
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

int main(int argc, const char **argv)
{
  srand(time(nullptr));

  if (argc != 3)
  {
    cout << "Syntaxfehler: PathGen.exe <Laenge des Pfades> <Breite des Pfades>" << endl;
    return 1;
  }
  int len, wide;

  if (isdigit(*argv[1]) == false || isdigit(*argv[2]) == false)
  {
    cout << "Fehler: Argumente muessen ganze Zahlen sein." << endl;
    return 1;
  }
  len = atoi(argv[1]);
  wide = atoi(argv[2]);
  if (len <= 0 || wide <= 0)
  {
    cout << "Fehler: Argumente muessen groesser als 0 sein" << endl;
    return 1;
  }

  Path *head = new Path(len, wide);
  head->set_arr();
  head->chain(len, wide);

  cout << *head;
  delete head;
  return 0;
}
