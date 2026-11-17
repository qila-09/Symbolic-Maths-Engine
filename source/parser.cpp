#include "classes.h"

void Parser::skipSpaces() {
    while (position < input.size() && isspace(input[position])) position++;
}

bool Parser::match(char character) {
    skipSpaces();
    if (position < input.size() && input[position] == character) {
        position++;
        return true;
    }
    return false;
}

string Parser::readName() {
    skipSpaces();
    size_t start = position;
    while (position < input.size() && isalpha(input[position])) position++;
    return input.substr(start, position - start);
}

Expr Parser::parseNumber() {
    skipSpaces();
    size_t start = position;
    bool decimalFound = false;
    while (position < input.size()) {
        char character = input[position];
        if (isdigit(character)) position++;
        else if (character == '.' && !decimalFound) {
            decimalFound = true;
            position++;
        } else break;
    }
    if (start == position) throw runtime_error("Expected a number.");
    return makeNumber(stod(input.substr(start, position - start)));
}

Expr Parser::parsePrimary() {
    skipSpaces();
    if (match('(')) {
        Expr result = parseExpression();
        if (!match(')')) throw runtime_error("Missing closing bracket.");
        return result;
    }

    if (position < input.size() && (isdigit(input[position]) || input[position] == '.'))
        return parseNumber();

    string name = readName();
    if (name == "x") return makeVariable();
    if (name == "pi") return makeNumber(PI);

    if (name == "sin" || name == "cos" || name == "exp" || name == "ln" || name == "abs") {
        if (!match('(')) throw runtime_error("Expected '(' after function name.");
        Expr argument = parseExpression();
        if (!match(')')) throw runtime_error("Missing ')' after function argument.");
        return makeUnary(name, move(argument));
    }

    throw runtime_error("Unknown expression or function: " + name);
}

Expr Parser::parsePower() {
    Expr left = parsePrimary();
    if (match('^')) return makeBinary("^", move(left), parseUnary());
    return left;
}

Expr Parser::parseUnary() {
    if (match('+')) return parseUnary();
    if (match('-')) return makeBinary("*", makeNumber(-1), parseUnary());
    return parsePower();
}

Expr Parser::parseTerm() {
    Expr result = parseUnary();
    while (true) {
        if (match('*')) result = makeBinary("*", move(result), parseUnary());
        else if (match('/')) result = makeBinary("/", move(result), parseUnary());
        else break;
    }
    return result;
}

Expr Parser::parseExpression() {
    Expr result = parseTerm();
    while (true) {
        if (match('+')) result = makeBinary("+", move(result), parseTerm());
        else if (match('-')) result = makeBinary("-", move(result), parseTerm());
        else break;
    }
    return result;
}

Expr Parser::parse() {
    Expr result = parseExpression();
    skipSpaces();
    if (position != input.size()) throw runtime_error("Unexpected character in expression.");
    return result;
}
