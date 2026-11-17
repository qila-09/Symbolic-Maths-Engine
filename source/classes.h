#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <random>
#include <algorithm>
#include <limits>
#include <cctype>

using namespace std;

const double EPSILON = 1e-10;
const double PI = acos(-1.0);

struct Expression {
    string type;
    double value;
    unique_ptr<Expression> left;
    unique_ptr<Expression> right;

    Expression(string type, double value = 0) : type(type), value(value) {}
};

using Expr = unique_ptr<Expression>;

// Expression construction
Expr makeNumber(double value);
Expr makeVariable();
Expr makeUnary(const string& operation, Expr child);
Expr makeBinary(const string& operation, Expr left, Expr right);
Expr cloneExpression(const Expression& expression);

class Parser {
private:
    string input;
    size_t position = 0;

    void skipSpaces();
    bool match(char character);
    string readName();
    Expr parseNumber();
    Expr parsePrimary();
    Expr parsePower();
    Expr parseUnary();
    Expr parseTerm();
    Expr parseExpression();

public:
    explicit Parser(const string& input) : input(input) {}

    Expr parse();
};

// Expression helpers
bool isNumber(const Expression& expression);
bool isZero(const Expression& expression);
bool isOne(const Expression& expression);

// Maths operations
Expr simplify(Expr expression);
Expr differentiate(const Expression& expression);
double evaluate(const Expression& expression, double x);

// Printing
int precedence(const string& type);
string formatNumber(double value);
string toString(const Expression& expression, int parentPrecedence = 0);

// Integration
bool getConstant(const Expression& expression, double& value);
bool getLinearForm(const Expression& expression, double& a, double& b);
Expr integrate(const Expression& expression);

// Numerical methods
double simpson(const Expression& expression, double start, double end, int intervals = 1000);
double numericalDerivative(const Expression& expression, double x);

// Tests
void runTests();
