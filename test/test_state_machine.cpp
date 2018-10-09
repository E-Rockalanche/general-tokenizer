#include <string>
#include <fstream>
#include "token_state_machine.hpp"
#include "testing.hpp"

typedef unsigned int uint;

const uint num_keywords = 8;
const std::string keywords[num_keywords] = {
	"foobar",
	"fantastic",
	"funkalicious",
	"flubber",
	"erratic",
	"eric",
	"erroneous",
	"epic"
};

const uint num_int_expressions = 4;
const std::string int_expressions[num_int_expressions] = {
	"0x[0-9a-fA-F]+",
	"0b[01]+",
	"0[0-7]*",
	"-?[1-9][0-9]*"
};
const std::string int_tokens[num_int_expressions] = {
	"0x123abc ",
	"0b1010010010 ",
	"0572635 ",
	"-191837460 "
};

const uint num_assembly_expressions = 19;
const std::string assembly_expressions[num_assembly_expressions] = {
	"[a-zA-Z_][a-zA-Z0-9_]*", // word
	"\\.[a-z]+", // directive
	"$|(0x)[0-9a-fA-F]+", // hex
	"-?[1-9][0-9]*", // decimal
	"0[0-7]*", // octal
	"0b[01]+", // binary
	"\"((\\\\.)|[^\\\\\"])*\"", // string
	"'((\\\\.)|[^\\\\'])'", // char
	"\\(",
	")",
	"#",
	",",
	":",
	"=",
	";[^\n]*\n", // comment
	"($|(0x)[0-9a-fA-F]+[g-zG-Z_]+)|(-?[1-9][0-9]*[a-zA-Z_]+)|(0[0-7]*[89ac-wyzA-Z_]+)|(0b[01]+[2-9a-zA-Z_]+)", //invalid integer
	"\"((\\\\.)|[^\\\\\"])*", // unterminated string
	"'((\\\\.)|[^\\\\'])((\\\\.)|[^\\\\'])+'", // invalid char
	"'((\\\\.)|[^\\\\'])" // unterminated char
};

int main() {
	describe("token state machine", {
		describe("addRule()", {
			it("should accept simple regex", {
				TokenStateMachine sm;
				expectNoException(sm.addRule("keyword", 5));
			});

			it("should accept sequence group", {
				TokenStateMachine sm;
				expectNoException(sm.addRule("(group)", 5));
			});

			it("should accept option group", {
				TokenStateMachine sm;
				expectNoException(sm.addRule("[group]", 5));
			});

			it("should accept quantifiers (?+*)", {
				TokenStateMachine sm;
				expectNoException(sm.addRule("a?b+c*", 5));
			});

			it("should accept character class", {
				TokenStateMachine sm;
				expectNoException(sm.addRule("\\d", 5));
			});

			it("should accept string regex", {
				TokenStateMachine sm;
				expectNoException(sm.addRule("\"((\\\\.)|[^\\\\\"])*\"", 8));
			});

			it("should accept multiple simple tokens", {
				TokenStateMachine sm;
				for(uint i = 0; i < num_keywords; i++) {
					expectNoException(sm.addRule(keywords[i], i));
				}
			});

			it("should accept multiple complex tokens", {
				TokenStateMachine sm;
				for(uint i = 0; i < num_int_expressions; i++) {
					expectNoException(sm.addRule(int_expressions[i], i));
				}
			});

			it("should parse set of assembly expressions", {
				TokenStateMachine sm;
				for(uint i = 0; i < num_assembly_expressions; i++) {
					expectNoException(sm.addRule(assembly_expressions[i], i));
				}
			});

			it("should throw error on unmatched brackets", {
				TokenStateMachine sm;
				expectException(sm.addRule("bad regex[", 5), std::runtime_error);
			});

			it("should throw error on bad escape character", {
				TokenStateMachine sm;
				expectException(sm.addRule("bad regex\\", 5), std::runtime_error);
			});
		});

		describe("state iteration", {
			it("should get correct type from simple regex", {
				TokenStateMachine sm;
				int type = 5;
				std::string str = "foobar";
				sm.addRule(str, type);
				TokenStateMachine::Iterator iterator = sm.begin();
				for(unsigned int i = 0; i < str.size(); i++) {
					iterator.nextState(str[i]);
				}
				expect(iterator.getType(), type);
			});

			describe("character classes", {
				it("should get correct type from \\d", {
					TokenStateMachine sm;
					int type = 5;
					std::string expression = "\\d";
					std::string str = "6";
					sm.addRule(expression, type);
					TokenStateMachine::Iterator iterator = sm.begin();
					for(unsigned int i = 0; i < str.size(); i++) {
						iterator.nextState(str[i]);
					}
					expect(iterator.getType(), type);
				});

				it("should get correct type from \\w", {
					TokenStateMachine sm;
					int type = 5;
					std::string expression = "\\w";
					std::string str = "_";
					sm.addRule(expression, type);
					TokenStateMachine::Iterator iterator = sm.begin();
					for(unsigned int i = 0; i < str.size(); i++) {
						iterator.nextState(str[i]);
					}
					expect(iterator.getType(), type);
				});

				it("should get correct type from \\s", {
					TokenStateMachine sm;
					int type = 5;
					std::string expression = "\\s";
					std::string str = " ";
					sm.addRule(expression, type);
					TokenStateMachine::Iterator iterator = sm.begin();
					for(unsigned int i = 0; i < str.size(); i++) {
						iterator.nextState(str[i]);
					}
					expect(iterator.getType(), type);
				});

				it("should get correct type from \\l", {
					TokenStateMachine sm;
					int type = 5;
					std::string expression = "\\l";
					std::string str = "p";
					sm.addRule(expression, type);
					TokenStateMachine::Iterator iterator = sm.begin();
					for(unsigned int i = 0; i < str.size(); i++) {
						iterator.nextState(str[i]);
					}
					expect(iterator.getType(), type);
				});

				it("should get correct type from \\u", {
					TokenStateMachine sm;
					int type = 5;
					std::string expression = "\\u";
					std::string str = "P";
					sm.addRule(expression, type);
					TokenStateMachine::Iterator iterator = sm.begin();
					for(unsigned int i = 0; i < str.size(); i++) {
						iterator.nextState(str[i]);
					}
					expect(iterator.getType(), type);
				});

				it("should get correct type from \\h", {
					TokenStateMachine sm;
					int type = 5;
					std::string expression = "\\h";
					std::string str = "F";
					sm.addRule(expression, type);
					TokenStateMachine::Iterator iterator = sm.begin();
					for(unsigned int i = 0; i < str.size(); i++) {
						iterator.nextState(str[i]);
					}
					expect(iterator.getType(), type);
				});
			});

			describe("quantifiers", {
				describe("?", {
					it("should choose optional path", {
						TokenStateMachine sm;
						int type = 8;
						sm.addRule("a?b", type);
						std::string str = "ab";
						TokenStateMachine::Iterator iterator = sm.begin();
						for(unsigned int i = 0; i < str.size(); i++) {
							iterator.nextState(str[i]);
						}
						expect(iterator.getType(), type);
					});

					it("should not choose optional path", {
						TokenStateMachine sm;
						int type = 8;
						sm.addRule("a?b", type);
						std::string str = "b";
						TokenStateMachine::Iterator iterator = sm.begin();
						for(unsigned int i = 0; i < str.size(); i++) {
							iterator.nextState(str[i]);
						}
						expect(iterator.getType(), type);
					});
				});

				describe("*", {
					it("should choose optional paths", {
						TokenStateMachine sm;
						int type = 8;
						sm.addRule("a*b", type);
						std::string str = "aaaab";
						TokenStateMachine::Iterator iterator = sm.begin();
						for(unsigned int i = 0; i < str.size(); i++) {
							iterator.nextState(str[i]);
						}
						expect(iterator.getType(), type);
					});

					it("should choose optional path", {
						TokenStateMachine sm;
						int type = 8;
						sm.addRule("a*b", type);
						std::string str = "ab";
						TokenStateMachine::Iterator iterator = sm.begin();
						for(unsigned int i = 0; i < str.size(); i++) {
							iterator.nextState(str[i]);
						}
						expect(iterator.getType(), type);
					});

					it("should not choose optional path", {
						TokenStateMachine sm;
						int type = 8;
						sm.addRule("a*b", type);
						std::string str = "b";
						TokenStateMachine::Iterator iterator = sm.begin();
						for(unsigned int i = 0; i < str.size(); i++) {
							iterator.nextState(str[i]);
						}
						expect(iterator.getType(), type);
					});
				});

				describe("+", {
					it("should choose optional paths", {
						TokenStateMachine sm;
						int type = 8;
						sm.addRule("a+b", type);
						std::string str = "aaaab";
						TokenStateMachine::Iterator iterator = sm.begin();
						for(unsigned int i = 0; i < str.size(); i++) {
							iterator.nextState(str[i]);
						}
						expect(iterator.getType(), type);
					});

					it("should not choose optional path", {
						TokenStateMachine sm;
						int type = 8;
						sm.addRule("a+b", type);
						std::string str = "ab";
						TokenStateMachine::Iterator iterator = sm.begin();
						for(unsigned int i = 0; i < str.size(); i++) {
							iterator.nextState(str[i]);
						}
						expect(iterator.getType(), type);
					});

					it("should require at least one", {
						TokenStateMachine sm;
						int type = 8;
						sm.addRule("a+b", type);
						std::string str = "b";
						TokenStateMachine::Iterator iterator = sm.begin();
						for(unsigned int i = 0; i < str.size(); i++) {
							iterator.nextState(str[i]);
						}
						expect(iterator.getType(), -1);
					});
				});
			});

			describe("character groups", {
				it("should get correct type from option group", {
					TokenStateMachine sm;
					int type = 5;
					std::string expression = "[abc]";
					std::string str = "b ";
					sm.addRule(expression, type);
					TokenStateMachine::Iterator iterator = sm.begin();
					for(unsigned int i = 0; i < str.size(); i++) {
						iterator.nextState(str[i]);
					}
					expect(iterator.getType(), type);
				});

				it("should get correct type from multiple option groups", {
					TokenStateMachine sm;
					int type = 5;
					std::string expression = "[abc][123][def]";
					std::string str = "b3d ";
					sm.addRule(expression, type);
					TokenStateMachine::Iterator iterator = sm.begin();
					for(unsigned int i = 0; i < str.size(); i++) {
						iterator.nextState(str[i]);
					}
					expect(iterator.getType(), type);
				});

				it("should get correct type from option group with span", {
					TokenStateMachine sm;
					int type = 5;
					std::string expression = "[a-z]";
					std::string str = "g ";
					sm.addRule(expression, type);
					TokenStateMachine::Iterator iterator = sm.begin();
					for(unsigned int i = 0; i < str.size(); i++) {
						iterator.nextState(str[i]);
					}
					expect(iterator.getType(), type);
				});

				it("should get correct type from option group with multiple spans", {
					TokenStateMachine sm;
					int type = 5;
					std::string expression = "[a-zA-Z]";
					std::string str = "G ";
					sm.addRule(expression, type);
					TokenStateMachine::Iterator iterator = sm.begin();
					for(unsigned int i = 0; i < str.size(); i++) {
						iterator.nextState(str[i]);
					}
					expect(iterator.getType(), type);
				});

				it("should get correct type from multiple span option groups", {
					TokenStateMachine sm;
					int type = 5;
					std::string expression = "[a-z][0-9][A-CT-Z]";
					std::string str = "l4U ";
					sm.addRule(expression, type);
					TokenStateMachine::Iterator iterator = sm.begin();
					for(unsigned int i = 0; i < str.size(); i++) {
						iterator.nextState(str[i]);
					}
					expect(iterator.getType(), type);
				});

				it("should get correct type from expressions containing '\\.'", {
					TokenStateMachine sm;

					std::string expressions[4];
					expressions[0] = "[a-z]";
					expressions[1] = "[A-Z]";
					expressions[2] = "[0-9]";
					expressions[3] = "\\.";

					std::string tokens[4];
					tokens[0] = "h";
					tokens[1] = "U";
					tokens[2] = "7";
					tokens[3] = ".";

					for(uint i = 0; i < 4; i++) {
						expectNoException(sm.addRule(expressions[i], i));
					}
					for(uint i = 0; i < 4; i++) {
						TokenStateMachine::Iterator iterator = sm.begin();
						for(unsigned int j = 0; j < tokens[i].size(); j++) {
							iterator.nextState(tokens[i][j]);
						}
						expect(iterator.getType(), i);
					}
				});
			});

			it("should get correct type from or groups", {
				TokenStateMachine sm;
				int type = 7;
				sm.addRule("$|(0x)[\\h]+", type);
				std::string str = "$fb";
				TokenStateMachine::Iterator iterator = sm.begin();
				for(unsigned int i = 0; i < str.size(); i++) {
					iterator.nextState(str[i]);
				}
				expect(iterator.getType(), type);
			});

			it("should get correct type from string regex", {
				TokenStateMachine sm;
				int type = 7;
				sm.addRule("\"((\\\\.)|[^\\\\\"])*\"", type);
				std::string str = "\"Hey there, didn't\nnotice\tyou, \\\"FELLOW\\\"\" ";
				TokenStateMachine::Iterator iterator = sm.begin();
				for(unsigned int i = 0; i < str.size(); i++) {
					iterator.nextState(str[i]);
				}
				expect(iterator.getType(), type);
			});

			it("should get correct type from string regex (the wacky one)", {
				TokenStateMachine sm;
				int type = 7;
				sm.addRule("\"((\\\\.)|[^\\\\\"])*\"", type);
				std::string str = "\"Hi\n, \\tmy \\\\fellow companions!\"";
				TokenStateMachine::Iterator iterator = sm.begin();
				for(unsigned int i = 0; i < str.size(); i++) {
					iterator.nextState(str[i]);
				}
				expect(iterator.getType(), type);
			});

			it("should get correct types from multiple simple tokens", {
				TokenStateMachine sm;
				for(uint i = 0; i < num_keywords; i++) {
					sm.addRule(keywords[i], i);
				}
				for(uint i = 0; i < num_keywords; i++) {
					TokenStateMachine::Iterator iterator = sm.begin();
					for(unsigned int j = 0; j < keywords[i].size(); j++) {
						iterator.nextState(keywords[i][j]);
					}
					expect(iterator.getType(), i);
				}
			});

			it("should get correct types from multiple tokens with option groups", {
				TokenStateMachine sm;

				for(uint i = 0; i < num_int_expressions; i++) {
					sm.addRule(int_expressions[i], i);
				}

				for(uint i = 0; i < num_int_expressions; i++) {
					TokenStateMachine::Iterator iterator = sm.begin();
					for(unsigned int j = 0; j < int_tokens[i].size(); j++) {
						iterator.nextState(int_tokens[i][j]);
					}
					expect(iterator.getType(), i);
				}
			});
		});

		it("should be able to save and load", {
			std::string filename = "temp.txt";

			TokenStateMachine sm;
			for(uint i = 0; i < num_keywords; i++) {
				sm.addRule(keywords[i], i);
			}
			bool saved = sm.saveToFile(filename);
			expect(saved, true);

			if (saved) {
				TokenStateMachine sm2;
				bool loaded = sm2.loadFromFile(filename);
				expect(loaded, true);

				if (loaded) {
					for(uint i = 0; i < num_keywords; i++) {
						TokenStateMachine::Iterator iterator = sm2.begin();
						for(unsigned int j = 0; j < keywords[i].size(); j++) {
							iterator.nextState(keywords[i][j]);
						}
						expect(iterator.getType(), i);
					}
				}
			}
		});
	});

	displayTestResults();

	return failed();
}