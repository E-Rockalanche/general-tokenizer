#include "tokenizer.hpp"
#include "token_types.hpp"
#include "token.hpp"
#include "testing.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define testSingleToken(message, token_str, token_type, num_errors)\
{\
	it(message, {\
		std::vector<Token> token_list;\
		std::string str = token_str;\
		tokenizer.tokenize(str, &token_list);\
		expect(tokenizer.errors(), num_errors);\
		expect(token_list.size(), 1);\
		expect(token_list[0].str, str);\
		expect(token_list[0].type, token_type);\
	});\
}

void setup(Tokenizer& tokenizer);

int main() {
	Tokenizer tokenizer;
	setup(tokenizer);

	describe("tokenizer", {
		testSingleToken("should parse alphanumeric word",
			"abc123_", TokenType::WORD, 0);

		testSingleToken("should parse underscore",
			"_", TokenType::WORD, 0);

		it("should skip whitesace", {
			std::vector<Token> token_list;
			std::string str = " \n\t\ffoobar \r\n\t\v";
			tokenizer.tokenize(str, &token_list);
			expect(tokenizer.errors(), 0);
			expect(token_list.size(), 1);
			expect(token_list[0].str, "foobar");
			expect(token_list[0].type, TokenType::WORD);
		});

		testSingleToken("should parse directive",
			".start", TokenType::DIRECTIVE, 0);

		describe("integers", {
			testSingleToken("should parse hex number (0x)",
				"0x1234567890abcdef", TokenType::HEX, 0);

			testSingleToken("should parse invalid hex number (0x)",
				"0x1234567890abcdefg", TokenType::MALFORMED_HEX, 1);

			testSingleToken("should parse hex number ($)",
				"$1234567890abcdef", TokenType::HEX, 0);

			testSingleToken("should parse invalid hex number ($)",
				"$1234567890abcdefg", TokenType::MALFORMED_HEX, 1);

			testSingleToken("should parse decimal",
				"1234567890", TokenType::DECIMAL, 0);

			testSingleToken("should parse negative decimal",
				"-1234567890", TokenType::DECIMAL, 0);

			testSingleToken("should parse invalid decimal",
				"1234567890a", TokenType::MALFORMED_DECIMAL, 1);

			testSingleToken("should parse octal",
				"012345670", TokenType::OCTAL, 0);

			testSingleToken("should parse zero",
				"0", TokenType::OCTAL, 0);

			testSingleToken("should parse invalid octal",
				"0123456708", TokenType::MALFORMED_OCTAL, 1);

			testSingleToken("should parse binary",
				"0b01010", TokenType::BINARY, 0);

			testSingleToken("should parse invalid binary",
				"0b010102", TokenType::MALFORMED_BINARY, 1);
		});

		describe("strings", {
			testSingleToken("should parse string",
				"\"Hello, World\"", TokenType::STRING, 0);

			testSingleToken("should parse this wacky string",
				"\"Hi\n, \\tmy \\\\fellow companions!\"", TokenType::STRING, 0);

			testSingleToken("should parse string with escaped characters",
				"\"\\\"Hello,\n\tWorld\\\"\"", TokenType::STRING, 0);

			testSingleToken("should parse unterminated string",
				"\"Hello, World", TokenType::INVALID, 1);
		});

		describe("char", {
			testSingleToken("should parse character",
				"'c'", TokenType::CHARACTER, 0);

			testSingleToken("should parse escaped character",
				"'\\n'", TokenType::CHARACTER, 0);

			testSingleToken("should parse invalid character",
				"'bb'", TokenType::MALFORMED_CHARACTER, 1);

			testSingleToken("should parse unterminated character",
				"'p", TokenType::INVALID, 1);
		});

		describe("symbols", {
			testSingleToken("should parse (",
				"(", TokenType::OPEN_PAREN, 0);

			testSingleToken("should parse )",
				")", TokenType::CLOSE_PAREN, 0);

			testSingleToken("should parse #",
				"#", TokenType::HASH, 0);

			testSingleToken("should parse ,",
				",", TokenType::COMMA, 0);

			testSingleToken("should parse :",
				":", TokenType::COLON, 0);

			testSingleToken("should parse =",
				"=", TokenType::EQUALS, 0);
		});

		it("should ignore comment", {
			std::vector<Token> token_list;
			std::string str = "; this is a comment until new line or eof";
			tokenizer.tokenize(str, &token_list);
			expect(tokenizer.errors(), 0);
			expect(token_list.size(), 0);
		});

		it("should ignore whitespace", {
			std::vector<Token> token_list;
			std::string str = " \t\n\r\f\v";
			tokenizer.tokenize(str, &token_list);
			expect(tokenizer.errors(), 0);
			expect(token_list.size(), 0);
		});

		it("should parse multi token string", {
			std::vector<Token> token_list;
			std::string str = " \t\vabc123_ .data 0x1234567890abcdef \
				; hello this is a comment yeeeeeeeeeeehaaaaaaaaaaaaaa\n\
				$1234567890abcdef 1234567890 -1234567890 01234567 0 0b10 \"Hi\n\
				, \\tmy \\\\fellow companions!\" ()#,:= ;goodbye everybody!";
			tokenizer.tokenize(str, &token_list);
			expect(tokenizer.errors(), 0);
			expect(token_list.size(), 16);
		});
	});

	displayTestResults();

	return failed();
}

void setup(Tokenizer& tokenizer) {
	tokenizer.addRule(Tokenizer::WHITESPACE, TokenType::WHITESPACE, true);

	tokenizer.addRule(";[^\n]*\n?", TokenType::COMMENT, true);

	tokenizer.addRule(Tokenizer::WORD_RULE, TokenType::WORD);
	tokenizer.addRule("\\.[\\w]+", TokenType::DIRECTIVE);

	tokenizer.addRule(Tokenizer::HEX_RULE, TokenType::HEX);
	tokenizer.addRule(Tokenizer::DECIMAL_RULE, TokenType::DECIMAL);
	tokenizer.addRule(Tokenizer::OCTAL_RULE, TokenType::OCTAL);
	tokenizer.addRule(Tokenizer::BINARY_RULE, TokenType::BINARY);

	tokenizer.addRule(Tokenizer::MALFORMED_HEX_RULE, TokenType::MALFORMED_HEX);
	tokenizer.addRule(Tokenizer::MALFORMED_DECIMAL_RULE, TokenType::MALFORMED_DECIMAL);
	tokenizer.addRule(Tokenizer::MALFORMED_OCTAL_RULE, TokenType::MALFORMED_OCTAL);
	tokenizer.addRule(Tokenizer::MALFORMED_BINARY_RULE, TokenType::MALFORMED_BINARY);

	tokenizer.addRule(Tokenizer::DQ_STRING_RULE, TokenType::STRING);

	tokenizer.addRule(Tokenizer::CHARACTER_RULE, TokenType::CHARACTER);
	tokenizer.addRule(Tokenizer::MALFORMED_CHARACTER_RULE, TokenType::MALFORMED_CHARACTER);

	tokenizer.addRule("\\(", TokenType::OPEN_PAREN);
	tokenizer.addRule(")", TokenType::CLOSE_PAREN);
	tokenizer.addRule(",", TokenType::COMMA);
	tokenizer.addRule(":", TokenType::COLON);
	tokenizer.addRule("#", TokenType::HASH);
	tokenizer.addRule("=", TokenType::EQUALS);
}