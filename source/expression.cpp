#include "classes.h"

Expr makeNumber(double value) {
    Expr result = make_unique<Expression>("number", value);
    return result;
}

Expr makeVariable() {
    return make_unique<Expression>("x");
}

Expr makeUnary(const string& operation, Expr child) {
    Expr result = make_unique<Expression>(operation);
    result->left = move(child);
    return result;
}

Expr makeBinary(const string& operation, Expr left, Expr right) {
    Expr result = make_unique<Expression>(operation);
    result->left = move(left);
    result->right = move(right);
    return result;
}

Expr cloneExpression(const Expression& expression) {
    Expr result = make_unique<Expression>(expression.type, expression.value);
    if (expression.left) result->left = cloneExpression(*expression.left);
    if (expression.right) result->right = cloneExpression(*expression.right);
    return result;
}

bool isNumber(const Expression& expression) { return expression.type == "number"; }
bool isZero(const Expression& expression) { return isNumber(expression) && abs(expression.value) < EPSILON; }
bool isOne(const Expression& expression) { return isNumber(expression) && abs(expression.value - 1) < EPSILON; }

Expr simplify(Expr expression) {
    if (!expression) return nullptr;
    if (expression->left) expression->left = simplify(move(expression->left));
    if (expression->right) expression->right = simplify(move(expression->right));

    string type = expression->type;
    if (type == "number" || type == "x") return expression;

    if (type == "sin" || type == "cos" || type == "exp" || type == "ln" || type == "abs") {
        if (isNumber(*expression->left)) {
            double value = expression->left->value;
            if (type == "sin") return makeNumber(sin(value));
            if (type == "cos") return makeNumber(cos(value));
            if (type == "exp") return makeNumber(exp(value));
            if (type == "ln" && value > 0) return makeNumber(log(value));
            if (type == "abs") return makeNumber(abs(value));
        }
        return expression;
    }

    Expression& a = *expression->left;
    Expression& b = *expression->right;
    if (isNumber(a) && isNumber(b)) {
        double x = a.value, y = b.value;
        if (type == "+") return makeNumber(x + y);
        if (type == "-") return makeNumber(x - y);
        if (type == "*") return makeNumber(x * y);
        if (type == "/" && abs(y) > EPSILON) return makeNumber(x / y);
        if (type == "^" && !(x == 0 && y < 0)) return makeNumber(pow(x, y));
    }

    if (type == "+") {
        if (isZero(a)) return move(expression->right);
        if (isZero(b)) return move(expression->left);
    } else if (type == "-") {
        if (isZero(b)) return move(expression->left);
        if (isZero(a)) return makeBinary("*", makeNumber(-1), move(expression->right));
    } else if (type == "*") {
        if (isZero(a) || isZero(b)) return makeNumber(0);
        if (isOne(a)) return move(expression->right);
        if (isOne(b)) return move(expression->left);
    } else if (type == "/") {
        if (isZero(a)) return makeNumber(0);
        if (isOne(b)) return move(expression->left);
    } else if (type == "^") {
        if (isZero(b)) return makeNumber(1);
        if (isOne(b)) return move(expression->left);
        if (isOne(a)) return makeNumber(1);
    }
    return expression;
}

Expr differentiate(const Expression& expression) {
    const string& type = expression.type;
    if (type == "number") return makeNumber(0);
    if (type == "x") return makeNumber(1);

    Expr u = cloneExpression(*expression.left);
    Expr du = differentiate(*expression.left);

    if (type == "sin") return makeBinary("*", makeUnary("cos", move(u)), move(du));
    if (type == "cos") return makeBinary("*", makeBinary("*", makeNumber(-1), makeUnary("sin", move(u))), move(du));
    if (type == "exp") return makeBinary("*", makeUnary("exp", move(u)), move(du));
    if (type == "ln") return makeBinary("/", move(du), move(u));

    Expr v = cloneExpression(*expression.right);
    Expr dv = differentiate(*expression.right);
    if (type == "+" || type == "-") return makeBinary(type, move(du), move(dv));
    if (type == "*") {
        return makeBinary("+", makeBinary("*", move(du), move(v)),
                          makeBinary("*", move(u), move(dv)));
    }
    if (type == "/") {
        Expr numerator = makeBinary("-", makeBinary("*", move(du), move(v)),
                                     makeBinary("*", move(u), move(dv)));
        return makeBinary("/", move(numerator), makeBinary("^", cloneExpression(*expression.right), makeNumber(2)));
    }
    if (type == "^") {
        if (isNumber(*expression.right)) {
            double power = expression.right->value;
            return makeBinary("*", makeBinary("*", makeNumber(power),
                makeBinary("^", move(u), makeNumber(power - 1))), move(du));
        }
        // General power rule: u^v * (v' * ln(u) + v * u'/u).
        Expr first = makeBinary("*", move(dv), makeUnary("ln", cloneExpression(*expression.left)));
        Expr second = makeBinary("*", move(v), makeBinary("/", move(du), cloneExpression(*expression.left)));
        return makeBinary("*", cloneExpression(expression), makeBinary("+", move(first), move(second)));
    }
    throw runtime_error("Differentiation does not support this expression.");
}

double evaluate(const Expression& expression, double x) {
    const string& type = expression.type;
    if (type == "number") return expression.value;
    if (type == "x") return x;
    double a = evaluate(*expression.left, x);
    if (type == "sin") return sin(a);
    if (type == "cos") return cos(a);
    if (type == "exp") return exp(a);
    if (type == "abs") return abs(a);
    if (type == "ln") {
        if (a <= 0) throw runtime_error("ln is undefined for this input.");
        return log(a);
    }
    double b = evaluate(*expression.right, x);
    if (type == "+") return a + b;
    if (type == "-") return a - b;
    if (type == "*") return a * b;
    if (type == "/") {
        if (abs(b) < EPSILON) throw runtime_error("Division by zero.");
        return a / b;
    }
    if (type == "^") return pow(a, b);
    throw runtime_error("Unknown expression type.");
}

int precedence(const string& type) {
    if (type == "+" || type == "-") return 1;
    if (type == "*" || type == "/") return 2;
    if (type == "^") return 3;
    return 4;
}

string formatNumber(double value) {
    if (abs(value) < EPSILON) value = 0;
    ostringstream output;
    output << setprecision(8) << value;
    return output.str();
}

string toString(const Expression& expression, int parentPrecedence) {
    const string& type = expression.type;
    if (type == "number") return formatNumber(expression.value);
    if (type == "x") return "x";
    if (type == "sin" || type == "cos" || type == "exp" || type == "ln" || type == "abs")
        return type + "(" + toString(*expression.left) + ")";

    int current = precedence(type);
    string result = toString(*expression.left, current) + " " + type + " " + toString(*expression.right, current + (type == "^" ? -1 : 1));
    if (current < parentPrecedence) return "(" + result + ")";
    return result;
}

// Returns a constant value when the whole expression is independent of x.
bool getConstant(const Expression& expression, double& value) {
    if (expression.type == "number") {
        value = expression.value;
        return true;
    }
    if (expression.type == "x") return false;
    try {
        // A small recursive check avoids treating an expression containing x as constant.
        if (expression.left) {
            double ignored;
            if (!getConstant(*expression.left, ignored)) return false;
        }
        if (expression.right) {
            double ignored;
            if (!getConstant(*expression.right, ignored)) return false;
        }
        value = evaluate(expression, 0);
        return isfinite(value);
    } catch (...) { return false; }
}

bool getLinearForm(const Expression& expression, double& a, double& b) {
    if (expression.type == "x") { a = 1; b = 0; return true; }
    if (expression.type == "number") { a = 0; b = expression.value; return true; }
    if (expression.type == "+" || expression.type == "-") {
        double leftA, leftB, rightA, rightB;
        if (!getLinearForm(*expression.left, leftA, leftB) || !getLinearForm(*expression.right, rightA, rightB)) return false;
        double sign = expression.type == "+" ? 1 : -1;
        a = leftA + sign * rightA;
        b = leftB + sign * rightB;
        return true;
    }
    if (expression.type == "*") {
        double leftA, leftB, rightA, rightB;
        if (!getLinearForm(*expression.left, leftA, leftB) || !getLinearForm(*expression.right, rightA, rightB)) return false;
        if (abs(leftA) < EPSILON) { a = leftB * rightA; b = leftB * rightB; return true; }
        if (abs(rightA) < EPSILON) { a = rightB * leftA; b = rightB * leftB; return true; }
    }
    if (expression.type == "/") {
        double numeratorA, numeratorB, denominator;
        if (getLinearForm(*expression.left, numeratorA, numeratorB) && getConstant(*expression.right, denominator) && abs(denominator) > EPSILON) {
            a = numeratorA / denominator; b = numeratorB / denominator; return true;
        }
    }
    return false;
}

Expr integrate(const Expression& expression) {
    const string& type = expression.type;
    if (type == "number") return makeBinary("*", makeNumber(expression.value), makeVariable());
    if (type == "x") return makeBinary("/", makeBinary("^", makeVariable(), makeNumber(2)), makeNumber(2));

    if (type == "+" || type == "-")
        return makeBinary(type, integrate(*expression.left), integrate(*expression.right));

    if (type == "*") {
        double constant;
        if (getConstant(*expression.left, constant))
            return makeBinary("*", makeNumber(constant), integrate(*expression.right));
        if (getConstant(*expression.right, constant))
            return makeBinary("*", makeNumber(constant), integrate(*expression.left));
    }

    if (type == "^") {
        double power;
        if (expression.left->type == "x" && getConstant(*expression.right, power)) {
            if (abs(power + 1) < EPSILON) return makeUnary("ln", makeUnary("abs", makeVariable()));
            return makeBinary("/", makeBinary("^", makeVariable(), makeNumber(power + 1)), makeNumber(power + 1));
        }
    }

    if (type == "sin" || type == "cos" || type == "exp") {
        // Recognise a linear argument a*x + b and divide by its gradient a.
        const Expression& argument = *expression.left;
        double a = 0, b = 0;
        bool linear = getLinearForm(argument, a, b);
        if (linear && abs(a) > EPSILON) {
            Expr inner = cloneExpression(argument);
            Expr result;
            if (type == "sin") result = makeBinary("/", makeBinary("*", makeNumber(-1), makeUnary("cos", move(inner))), makeNumber(a));
            else if (type == "cos") result = makeBinary("/", makeUnary("sin", move(inner)), makeNumber(a));
            else result = makeBinary("/", makeUnary("exp", move(inner)), makeNumber(a));
            return result;
        }
    }

    throw runtime_error("Cannot integrate this expression using the available rules.");
}

double simpson(const Expression& expression, double start, double end, int intervals) {
    if (intervals < 2) intervals = 2;
    if (intervals % 2 != 0) intervals++;
    double width = (end - start) / intervals;
    double total = evaluate(expression, start) + evaluate(expression, end);
    for (int i = 1; i < intervals; i++) {
        double multiplier = (i % 2 == 0) ? 2 : 4;
        total += multiplier * evaluate(expression, start + i * width);
    }
    return total * width / 3;
}

double numericalDerivative(const Expression& expression, double x) {
    const double h = 1e-5;
    return (evaluate(expression, x + h) - evaluate(expression, x - h)) / (2 * h);
}
