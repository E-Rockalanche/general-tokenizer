#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

struct Token {
public:
	int type;
	const std::string str;
	const unsigned int row;
	const unsigned int column;
	
	Token() : type(-1), str(""), row(0), column(0) {}
	Token(int type, std::string str, unsigned int row = 0, unsigned int column = 0)
		: type(type), str(str), row(row), column(column) {}
};

#endif