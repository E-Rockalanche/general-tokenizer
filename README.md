# general-tokenizer

## Tokenizer

### void Tokenizer::addRule(std::string rule, int token_type)

Encodes a series of state changes in the underlying finite state machine. If a parsed token matches the given rule the resulting token will be of the given type. Negative types are considered invalid tokens. The function will throw if the finite state machine tries to override existing state changes or types.
Rule is expected to be a regular expression with implied beginning and end anchors

example:
```cpp
enum TokenType {
	WORD,
	HEX,
	DECIMAL,
	OCTAL,
	IF
};
addRule("[\\l\\u_][\\w]*", WORD);
addRule("$|(0x)[\\h]+", HEX);
addRule("-?[1-9][\\d]*", DECIMAL);
addRule("0[0-7]*, OCTAL");

// this call will throw an exception because it conflicts with the rule WORD
addRule("if", IF)
```

Many common regular expressions are already defined

```cpp
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
```

### void Tokenizer::ignoreType(int token_type)

Sets the given type to be excluded from the token list (std::vector). Usefull for ignoring comments and whitespace

example:
```cpp
enum TokenType {
	COMMENT
};
ignoreRule(COMMENT);
```

### bool Tokenizer::tokenize(std::istream* stream, std::vector<Token>* token_list)

Tokenizes the given stream using the defined set of rules. Each token records the row and column in the text, raw string parsed, and the type of rule it matched

example:
```cpp
std::vector<Token> token_list;
ifstream fin("example.txt");
if (fin.is_open()) {
	bool succeeded = tokenizer.tokenize(&fin, &token_list);
	fin.close();
}
```

### bool Tokenizer::tokenize(const std::string& str, std::vector<Token>* token_list)

Same as above expect overloaded to take a string (a stringstream is constructed and the istream version is called

example:
```cpp
std::vector<Token> token_list;
std::string text = "text to be tokenized";
bool succeeded = tokenizer.tokenize(text, &token_list);
```

### unsigned int Tokenizer::errors()

returns the number of invalid tokens parsed

example:
```cpp
int errors = tokenizer.errors();
if (errors > 0) {
	std::cout << "Number of invalid tokens: " << errors << '\n';
}
```

## Token

Records the type, string parsed, and row and column found. The type is not constant so that the type can be refined or modified. For example, the tokenizer will throw if a keyword rule is added after a catch-all word rule. To remmedy this some custom code must be defined to recognize that a word is actually a keyword

## Contributors

[Eric Roberts](https://github.com/E-Rockalanche)