#include <cstdlib>
#include <iostream>
#include <math.h>
#include <ostream>
#include <string>

using namespace std;

void hextox();
void dectox();
// void bintox();
void hexcomp();
void legal();
void start() {
  cout << "What do you wanna do?" << endl
       << "[0] Hex-comp" << endl
       << "[1] Hex to x" << endl
       << "[2] Dec to x"
       << endl
       //<< "[3] Bin to x"
       << "[3] Is-She-Legal-Calc" << endl
       << "[4] Quit" << endl
       << "Waiting for input: ";
  char x;
  cin >> x;

  switch (x) {
  case '0':
    hexcomp();
    break;
  case '1':
    hextox();
    break;
  case '2':
    dectox();
    break;
    // case '3':
    // bintox();
    // break;
  case '3':
    legal();
    break;
  case '4':
    exit(EXIT_SUCCESS);
  default:
    return;
  }
}

void hextox() {

  int z;
  cout << "Hexstring: ";
  cin >> hex >> z;
  if (cin.fail()) {
    cout << "Illegal input!" << endl;
    return;
  }
  bool neg = false;
  if (z == 0) {
    return;
  }
  if (z <= 0) {
    neg = true;
  }
  int temp = z;
  string b;

  while (temp > 0) {
    b = to_string(temp % 2) + b;
    temp /= 2;
  }
  if (neg == true) {
    b = "-" + b;
  }
  cout << "Decimal: " << z << endl;
  cout << "Binary: " << b << endl;
}

void dectox() {
  int z;
  cout << "Number: ";
  cin >> z;
  if (cin.fail()) {
    cout << "Illegal input!" << endl;
    return;
  }
  bool neg = false;
  if (z == 0) {
    return;
  }
  if (z <= 0) {
    neg = true;
  }
  int temp = z;
  string b;

  while (temp > 0) {
    b = to_string(temp % 2) + b;
    temp /= 2;
  }
  if (neg == true) {
    b = "-" + b;
  }
  cout << "Hexstring: " << hex << z << endl;
  cout << "Binary: " << b << endl;
}
void hexcomp() {
  int a;
  int b;
  cout << "Hexstring one: ";
  cin >> hex >> a;
  if (cin.fail()) {
    cout << "Illegal input!" << endl;
    return;
  }
  cout << "Hexstring two: ";
  cin >> hex >> b;
  if (cin.fail()) {
    cout << "Illegal input!" << endl;
    return;
  }
  if (a != b) {
    cout << "Not equal" << endl;
  } else {
    cout << "Equal" << endl;
  }
}
void legal() {

  double a;
  double b;
  bool haram = false;
  cout << "Your age: ";
  cin >> hex >> a;
  if (cin.fail()) {
    cout << "Illegal input!" << endl;
    return;
  }
  cout << "Her age:  ";
  cin >> hex >> b;
  if (cin.fail()) {
    cout << "Illegal input!" << endl;
    return;
  }
  double calc = a / 2 + 7;
  calc = ceil(calc);
  if (calc > b) {
    haram = true;
  }
  double dif = b - calc;

  if (haram && dif == -1) {
    cout << "You dipped? (y/n) ";
    string chail;
    cin >> chail;
    if (chail == "y" || chail == "yes") {
      cout << "Officer, lock this man up!" << endl;
      return;
    }
    if (chail == "n" || chail == "no") {
      cout << "BRO, WTF. Stop the preorder!" << endl;
      return;
    }
    cout << "Ey yo, wrong input, Pdiddy!" << endl;
    return;
  }

  if (haram && b < 18) {
    cout << "So you have been on the island. Calling 110 ..." << endl;
    return;
  }
  if (haram) {
    cout << "Dude that's fucking weird. Alt least she's not a minor." << endl;
    return;
  }
  if (dif >= 7) {
    cout << "So you are sailing on old ships, huh." << endl;
    return;
  }
  cout << "Good for you, she is legal" << endl;
  return;
}

int main() {
  for (;;) {
    start();
  }
}
