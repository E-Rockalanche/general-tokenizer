/*
made by Eric Roberts, 2018

Tokenizer uses token state machine to encode a set of rules and parse a sequence
of tokens. Rules are added with addRule() which requires a regex like string and
a type value. Negative types are considered invalid tokens. Tokens of specific
types can be excluded from the vector by calling ignoreType(). Each token
records the raw string parsed, its type, row, and column. Each token is added to
the vector provided. For each invalid token parsed the number of errors is
incremented.
*/

#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <istream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>
#include "token.hpp"
#include "token_state_machine.hpp"

typedef unsigned int uint;

class Tokenizer {
public:
	void addRule(std::string rule, int token_type, bool ignore = false);
	bool tokenize(std::istream* stream, std::vector<Token>* token_list);
	bool tokenize(const std::string& str, std::vector<Token>* token_list);
	unsigned int errors(){ return num_errors; }

	// predefined rules you can use
	static const char* WHITESPACE; // \s+
	static const char* WORD_RULE; // [\l\u][\w]*
	static const char* DECIMAL_RULE; // -?[1-9][0-9]*
	static const char* MALFORMED_DECIMAL_RULE;
	static const char* HEX_RULE; // ($|0x)[\\x]+
	static const char* MALFORMED_HEX_RULE;
	static const char* OCTAL_RULE; // 0[0-7]*
	static const char* MALFORMED_OCTAL_RULE;
	static const char* BINARY_RULE; // 0b[01]+
	static const char* MALFORMED_BINARY_RULE;
	static const char* DQ_STRING_RULE;
	static const char* SQ_STRING_RULE;
	static const char* CHARACTER_RULE; // single or escaped character in single quotes
	static const char* MALFORMED_CHARACTER_RULE; // more than 1 character in single quotes

private:
	TokenStateMachine state_machine;
	std::vector<Token>* token_list;
	std::istream* stream;

	std::vector<int> ignore_types;

	uint row;
	uint column;
	uint num_errors;

	char get();
};

#endif