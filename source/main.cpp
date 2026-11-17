#include "classes.h"

void runTests() {
    vector<string> derivativeTests = {"x^3 + 2*x", "sin(x)", "x*cos(x)", "exp(x)", "ln(x)"};
    int derivativePassed = 0;
    int derivativeTotal = 0;

    for (const string& input : derivativeTests) {
        try {
            Expr original = Parser(input).parse();
            Expr derivative = simplify(differentiate(*original));
            bool passed = true;
            for (double x : {0.7, 1.3, 2.1}) {
                if (abs(evaluate(*derivative, x) - numericalDerivative(*original, x)) > 1e-4) passed = false;
            }
            derivativeTotal++;
            if (passed) derivativePassed++;
            cout << "Derivative test: " << input << " - " << (passed ? "PASS" : "FAIL") << '\n';
        } catch (...) {
            derivativeTotal++;
            cout << "Derivative test: " << input << " - FAIL\n";
        }
    }

    vector<string> integralTests = {"x^2", "sin(3*x + 1)", "cos(2*x)", "exp(2*x)", "3*x"};
    int integralPassed = 0;
    int integralTotal = 0;
    for (const string& input : integralTests) {
        try {
            Expr original = Parser(input).parse();
            Expr antiDerivative = simplify(integrate(*original));
            Expr check = simplify(differentiate(*antiDerivative));
            bool passed = true;
            for (double x : {0.7, 1.3, 2.1}) {
                if (abs(evaluate(*check, x) - evaluate(*original, x)) > 1e-4) passed = false;
            }
            integralTotal++;
            if (passed) integralPassed++;
            cout << "Integral test: " << input << " - " << (passed ? "PASS" : "FAIL") << '\n';
        } catch (...) {
            integralTotal++;
            cout << "Integral test: " << input << " - FAIL\n";
        }
    }

    Expr testFunction = Parser("x^3 + 2*x").parse();
    double exact = simpson(*testFunction, 0, 2);
    bool simpsonPassed = abs(exact - 8.0) < 1e-6;
    cout << "Simpson test: x^3 + 2*x from 0 to 2 - " << (simpsonPassed ? "PASS" : "FAIL") << '\n';
    cout << "\nTests passed: " << derivativePassed + integralPassed + (simpsonPassed ? 1 : 0)
         << "/" << derivativeTotal + integralTotal + 1 << '\n';
}

int main() {
    cout << "     SYMBOLIC MATHS ENGINE\n";

    while (true) {
        cout << "\n1. Differentiate an expression\n";
        cout << "2. Find a definite integral (Simpson's rule)\n";
        cout << "3. Find a symbolic integral\n";
        cout << "4. Run automatic tests\n";
        cout << "5. Exit\n";
        cout << "Choice: ";

        string choice;
        getline(cin, choice);
        if (choice == "5") break;

        try {
            if (choice == "4") {
                runTests();
                continue;
            }
            if (choice != "1" && choice != "2" && choice != "3") {
                cout << "Please enter a valid menu option.\n";
                continue;
            }

            cout << "Enter an expression (use x, +, -, *, /, ^, sin(), cos(), exp(), ln()): ";
            string input;
            getline(cin, input);
            Expr expression = simplify(Parser(input).parse());

            if (choice == "1") {
                Expr result = simplify(differentiate(*expression));
                cout << "Derivative: " << toString(*result) << '\n';
            } else if (choice == "2") {
                double start, end;
                cout << "Lower limit: ";
                if (!(cin >> start)) throw runtime_error("Invalid lower limit.");
                cout << "Upper limit: ";
                if (!(cin >> end)) throw runtime_error("Invalid upper limit.");
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Approximate integral: " << setprecision(12) << simpson(*expression, start, end) << '\n';
            } else {
                Expr result = simplify(integrate(*expression));
                cout << "Integral: " << toString(*result) << " + C\n";
            }
        } catch (const exception& error) {
            cout << "Error: " << error.what() << '\n';
        }
    }

    return 0;
}
