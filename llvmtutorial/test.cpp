#include <iostream>

using namespace std;

void simpleLoop(int n);
void nestedLoop(int rows, int cols);
int factorial(int n);
void conditionalBranch(int x);
void switchCase(int choice);

int main() {
    simpleLoop(5);
    nestedLoop(3, 4);

    conditionalBranch(10);
    conditionalBranch(-5);

    switchCase(1);
    switchCase(2);
    switchCase(3);
    switchCase(4);

    cout << "Factorial of 5: " << factorial(5) << endl;

    return 0;
}


void simpleLoop(int n) {
    cout << "simpleLoop(" << n << "):\n";
    for (int i = 0; i < n; ++i) {
        cout << i << " ";
    }
    cout << endl;
}


void nestedLoop(int rows, int cols) {
    cout << "nestedLoop(" << rows << ", " << cols << "):\n";
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cout << "* ";
        }
        cout << endl;
    }
}


int factorial(int n) {
    if (n <= 1)
        return 1;
    return n * factorial(n - 1);
}


void conditionalBranch(int x) {
    cout << "conditionalBranch(" << x << "): ";
    if (x > 0)
        cout << "Positive" << endl;
    else if (x < 0)
        cout << "Negative" << endl;
    else
        cout << "Zero" << endl;
}


void switchCase(int choice) {
    cout << "switchCase(" << choice << "): ";
    switch (choice) {
        case 1:
            cout << "Option One" << endl;
            break;
        case 2:
            cout << "Option Two" << endl;
            break;
        case 3:
            cout << "Option Three" << endl;
            break;
        default:
            cout << "Unknown Option" << endl;
    }
}