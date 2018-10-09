#include <sstream>
#include "tokenizer.hpp"

const char* Tokenizer::WHITESPACE = "\\s+";
const char* Tokenizer::WORD_RULE = "[\\l\\u_][\\w]*";
const char* Tokenizer::DECIMAL_RULE = "-?[1-9][\\d]*";
const char* Tokenizer::MALFORMED_DECIMAL_RULE = "(-[0\\l\\u_])|(-?[1-9][\\d]*[\\l\\u_])[\\w]*";
const char* Tokenizer::HEX_RULE = "$|(0x)[\\h]+";
const char* Tokenizer::MALFORMED_HEX_RULE = "$|(0x)([\\h]*[g-zG-Z_][\\w]*)?";
const char* Tokenizer::OCTAL_RULE = "0[0-7]*";
const char* Tokenizer::MALFORMED_OCTAL_RULE = "0[0-7]*[89ac-wyz\\u_][\\w]*";
const char* Tokenizer::BINARY_RULE = "0b[01]+";
const char* Tokenizer::MALFORMED_BINARY_RULE = "0b[01]*[2-9\\l\\u_][\\w]*";
const char* Tokenizer::DQ_STRING_RULE = "\"((\\\\.)|[^\"\\\\])*\"";
const char* Tokenizer::SQ_STRING_RULE = "'((\\\\.)|[^\"\\\\])*'";
const char* Tokenizer::CHARACTER_RULE = "'(\\\\.)|[^'\\\\]'";
const char* Tokenizer::MALFORMED_CHARACTER_RULE = "'(\\\\.)|[^'\\\\]((\\\\.)|[^'\\\\])+'";

void Tokenizer::addRule(std::string rule, int token_type) {
	state_machine.addRule(rule, token_type);
}

void Tokenizer::ignoreType(int token_type) {
	ignore_types.push_back(token_type);
}

bool Tokenizer::tokenize(std::istream* stream, std::vector<Token>* token_list) {
	this->token_list = token_list;
	this->stream = stream;

	// reset position tracker
	row = 1;
	column = 1;
	num_errors = 0;

	while(!stream->eof()) {
		// parse token
		uint token_row = row;
		uint token_column = column;
		std::string cur_token;
		TokenStateMachine::Iterator it = state_machine.begin();
		while(!stream->eof()) {
			char c = stream->peek();
			it.nextState(c);

			if (it.getState() == 0) {
				break;
			} else {
				cur_token += c;
				get();
			}
		}
			
		int type = it.getType();

		bool ignore = false;
		for(uint i = 0; i < ignore_types.size(); i++) {
			if (ignore_types[i] == type) {
				ignore = true;
				break;
			}
		}

		if (!ignore) {
			if (type < 0) {
				num_errors++;
			}

			token_list->push_back(Token(type, cur_token, token_row, token_column));
		}
	}

	return num_errors > 0;
}

bool Tokenizer::tokenize(const std::string& str, std::vector<Token>* token_list) {
	std::stringstream ss(str);
	return tokenize(&ss, token_list);
}

char Tokenizer::get() {
	char c = stream->get();
	if (c == '\n') {
		row++;
		column = 1;
	} else {
		column++;
	}
	return c;
}
/*
uint Tokenizer::errors() {
	return num_errors;
}
*/