#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include "token_state_machine.hpp"

const std::string TokenStateMachine::DIGITS = "0123456789"; // \d
const std::string TokenStateMachine::WORD = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"; // \w
const std::string TokenStateMachine::WHITESPACE = " \t\r\f\n\v"; // \s
const std::string TokenStateMachine::LOWERCASE = "abcdefghijklmnopqrstuvwxyz"; // \l
const std::string TokenStateMachine::UPPERCASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; // \u
const std::string TokenStateMachine::HEXDIGITS = "0123456789abcdefABCDEF"; // \h

TokenStateMachine::Iterator::Iterator(TokenStateMachine* my_machine) {
	TokenStateMachine::machineAssert(my_machine != NULL, "token state machine is null");
	this->my_machine = my_machine;
	this->state = 1;
	this->type = -1;
}

void TokenStateMachine::Iterator::nextState(char c) {
	machineAssert(state < my_machine->state_transitions.size(), "state out of bounds");
	const std::map<char, uint>& transitions = my_machine->state_transitions[state];
	auto it = transitions.find(c);
	if (it != transitions.end()) {
		state = it->second;
	} else {
		state = 0;
	}
	int new_type = my_machine->state_types[state];
	type = (new_type != -1) ? new_type : type;
}

TokenStateMachine::TokenStateMachine() {
	// state 0 is the end state
	state_transitions.resize(2);
	state_types.resize(2, -1);
}

TokenStateMachine::TokenStateMachine(uint num_expressions, const std::string* expressions) : TokenStateMachine() {
	for(uint i = 0; i < num_expressions; i++) {
		addRule(expressions[i], i);
	}
}

TokenStateMachine::TokenStateMachine(uint rows, const std::map<char, uint>* state_changes, const int* types) {
	machineAssert(rows > 0, "state array cannot have 0 rows");
	state_transitions.resize(1 + rows);
	state_types.resize(1 + rows, -1);
	for(uint r = 0; r < rows; r++) {
		state_transitions[r+1] = state_changes[r];
		state_types[r+1] = types[r];
	}
}

bool TokenStateMachine::saveToFile(std::string filename) {
	std::ofstream fout(filename.c_str());
	if (fout.is_open()) {
		fout << (state_transitions.size()-1) << ' ';
		for(uint row = 1; row < state_transitions.size(); row++) {
			fout << state_types[row] << ' ';
			const std::map<char, uint>& transitions = state_transitions[row];
			fout << transitions.size() << ' ';
			for(auto it = transitions.begin(); it != transitions.end(); it++) {
				fout << it->first <<  ' ' << it->second << ' ';
			}
		}
		fout.close();
		return true;
	}
	return false;
}

bool TokenStateMachine::loadFromFile(std::string filename) {
	std::ifstream fin(filename.c_str());
	uint rows;
	if (fin.is_open()) {
		fin >> rows;
		state_transitions.resize(rows + 1);
		state_types.resize(rows + 1);

		for(uint row = 0; row < rows; row++) {
			fin >> state_types[row + 1];
			uint changes;
			fin >> changes;
			std::map<char, uint>& transitions = state_transitions[row + 1];
			for(uint i = 0; i < changes; i++) {
				char c;
				uint state;
				fin >> c >> state;
				transitions[c] = state;
			}
		}
		fin.close();
		return true;
	}
	return false;
}

void TokenStateMachine::machineAssert(bool condition, std::string message) {
	if (!condition) throw std::runtime_error(message);
}

void TokenStateMachine::addRule(std::string str, int type) {
	machineAssert(str.size() > 0, "string cannot be empty");
	States end_states = compileRegexSequence(States(1, 1), str);

	for(uint i = 0; i < end_states.size(); i++) {
		setStateType(end_states[i], type);
	}
}

TokenStateMachine::Iterator TokenStateMachine::begin() {
	return Iterator(this);
}

void TokenStateMachine::setStateType(State state, int type) {
	if (state >= state_types.size()) state_types.resize(state + 1, -1);
	int old_type = state_types[state];
	if (type != old_type) {
		machineAssert(old_type == -1 , "trying to override state "
			+ std::to_string(state) + " type " + std::to_string(old_type)
			+ " with " + std::to_string(type));
		state_types[state] = type;
	}
}

void TokenStateMachine::setStateChange(State from_state, char c, State to_state) {
	machineAssert(from_state != 0, "cannot change end state");
	machineAssert(to_state != 1, "cannot go back to start state");

	// resize matrix
	State max_state = (from_state > to_state) ? from_state : to_state;
	if (max_state >= state_transitions.size()) {
		state_transitions.resize(max_state + 1);
	}
		
	std::map<char, uint>& transitions = state_transitions[from_state];
	auto it = transitions.find(c);
	if (it == transitions.end()) {
		transitions[c] = to_state;
	} else {
		machineAssert(it->second == to_state, "trying to override state change "
			+ std::to_string(from_state) + " on " + std::string(1, c) + " from "
			+ std::to_string(it->second) + " to " + std::to_string(to_state));
	}
}

uint TokenStateMachine::getNextState(uint state, char c) const {
	uint next_state = 0;
	machineAssert(state < state_transitions.size(), "state does not exist");
	const std::map<char, uint>& transitions = state_transitions[state];
	auto it = transitions.find(c);
	if (it != transitions.end()) {
		next_state = it->second;
	}
	return next_state;
}

char TokenStateMachine::getEscapedCharacter(char c) {
	switch(c) {
		case 'a': c = '\a'; break;
		case 'b': c = '\b'; break;
		case 'f': c = '\f'; break;
		case 'n': c = '\n'; break;
		case 'r': c = '\r'; break;
		case 't': c = '\t'; break;
		case 'v': c = '\v'; break;
		default: break;
	}
	return c;
}

/*
ex) "[a-zA-Z_][a-zA-Z0-9_]*", WORD
ex) "0x[0-9a-fA-F]+", HEX
ex) "++", INCREMENT
ex) "-128", DECIMAL
ex) "\"((\\.)|[^\\\"])*\""
*/

States TokenStateMachine::compileRegexSequence(States start_states, std::string str) {
	States end_states;
	uint index = 0;

	while(index < str.size()) {
		end_states.clear();

		// parse all group options
		// ex) (ab)|(de)|(fg)
		std::vector<std::string> groups;
		bool add_option;

		do {
			std::string group = parseRegexGroup(str, index);
			add_option = false;
			if (group.size() > 0) {
				groups.push_back(group);

				if (index < str.size() && str[index] == '|') {
					index++;
					machineAssert(index < str.size(), "no group on right side of bar");
					add_option = true;
				}
			}
		} while(add_option);

		// compile each sub-sequence
		for(uint i = 0; i < groups.size(); i++) {
			States cur_end_states = compileRegexGroup(start_states, groups[i]);
			end_states.insert(end_states.end(), cur_end_states.begin(), cur_end_states.end());
		}

		// prepare for next iteration
		start_states = end_states;
	}
	return end_states;
}

std::string TokenStateMachine::parseRegexGroup(const std::string& str, uint& index) {
	std::string substr;
	if (index < str.size()) {
		char c = str[index++];
		machineAssert(!isQuantifier(c),
			"group cannot start with quantifier");

		if (c == '(' || c == '[') {
			index--;
			substr = parseMatchingBrackets(str, index);
		} else {
			if (c == '\\') {
				machineAssert(index < str.size(),
					"no character after escape");

				substr += '\\';
				c = str[index++];
			}

			substr += c;
		}

		while((index < str.size()) && isQuantifier(str[index])){
			substr += str[index++];
		}
	}

	return substr;
}

std::string TokenStateMachine::parseMatchingBrackets(const std::string& str, uint& index) {
	std::string substr;
	machineAssert(index < str.size(), "index out of bounds");
	char open_bracket = str[index];
	char close_bracket;
	switch(open_bracket) {
		case '(': close_bracket = ')'; break;
		case '[': close_bracket = ']'; break;
		default: machineAssert(false, "invalid open bracket");
	}
	substr += open_bracket;
	uint bracket_depth = 1;
	index++;
	bool escaped = false;
	while(index < str.size() && bracket_depth > 0) {
		char c = str[index++];
		substr += c;

		if (!escaped) {
			if (c == '\\') escaped = true;
			else if (c == open_bracket) bracket_depth++;
			else if (c == close_bracket) bracket_depth--;
		} else {
			escaped = false;
		}
	}
	machineAssert(bracket_depth == 0, "number of brackets do not match");

	return substr;
}

States TokenStateMachine::compileRegexGroup(States start_states, std::string str) {
	States end_states;

	machineAssert(str.size() > 0, "group string is empty");
	char back = str.back();
	char front = str.front();

	if (isQuantifier(back) && (str.size() > 1) && (str[str.size()-2] != '\\')) {
		end_states = compileRegexQuantifier(start_states, str);
	} else if (front == '[') {
		std::string substr(str, 1, str.size()-2);
		end_states = compileRegexBracketExpression(start_states, substr);
	} else if (front == '(') {
		std::string substr(str, 1, str.size()-2);
		end_states = compileRegexSequence(start_states, substr);
	} else {
		char c = front;
		if (c == '.') {
			std::string char_group;
			for(int c = 1; c < 128; c++) {
				if (c == '\\') char_group += (char)c;
				char_group += (char)c;
			}
			end_states = compileRegexBracketExpression(start_states, char_group);
		} else {
			bool special_char = false;
			std::string char_class;

			if (c == '\\') {
				machineAssert(str.size() == 2, "no character after escape");
				c = str[1];
				special_char = true;
				switch(c) {
					case 'd': char_class = DIGITS; break;
					case 'w': char_class = WORD; break;
					case 's': char_class = WHITESPACE; break;
					case 'l': char_class = LOWERCASE; break;
					case 'u': char_class = UPPERCASE; break;
					case 'h': char_class = HEXDIGITS; break;

					default:
						special_char = false;
						c = getEscapedCharacter(c);
				}
			}

			if (special_char) {
				machineAssert(char_class.size(), "character class is empty");
				end_states = compileRegexBracketExpression(start_states, char_class);
			} else {
				// encode state change on input c
				State next_state = chooseState(start_states[0], c);

				for(uint i = 0; i < start_states.size(); i++) {
					setStateChange(start_states[i], c, next_state);
				}

				end_states.push_back(next_state);
			}
		}
	}
	return end_states;
}

States TokenStateMachine::compileRegexQuantifier(States start_states, std::string str) {
	States end_states;
	States second_pass_start_states;
	States should_be_the_same;
	std::string substr;

	char q = str.back();
	machineAssert(isQuantifier(q), "back character is not a quantifier");

	int min_passes = (q == '+') ? 1 : 0;
	bool infinite_passes = (q == '?') ? false : true;

	substr = std::string(str, 0, str.size()-1);
	end_states = compileRegexGroup(start_states, substr);
	if (infinite_passes) {
		second_pass_start_states = end_states;
		second_pass_start_states.insert(second_pass_start_states.begin(), start_states[0]);
		should_be_the_same = compileRegexGroup(second_pass_start_states, substr);
		machineAssert(end_states == should_be_the_same, "states should be the same");
	}

	if (min_passes == 0) {
		// concatenate end states with start states
		end_states.insert(end_states.end(), start_states.begin(), start_states.end());
	}

	return end_states;
}

States TokenStateMachine::compileRegexBracketExpression(States start_states, std::string str) {
	machineAssert(str.size() > 0, "bracket expression string is empty");
	States end_states;
	std::string char_group;
	bool excluded = false;
	bool spanning = false;
	bool escaped = false;
	for(uint i = 0; i < str.size(); i++) {
		char c = str[i];
		if (c == '\\' && !escaped) {
			escaped = true;
		} else {
			bool special_char = false;
			std::string char_class;
			if (escaped) {
				special_char = true;
				switch(c) {
					case 'd': char_class = DIGITS; break;
					case 'w': char_class = WORD; break;
					case 's': char_class = WHITESPACE; break;
					case 'l': char_class = LOWERCASE; break;
					case 'u': char_class = UPPERCASE; break;
					case 'h': char_class = HEXDIGITS; break;

					default:
						special_char = false;
						c = getEscapedCharacter(c);
				}
				escaped = false;
			}

			if (special_char) {
				machineAssert(char_class.size(), "character class is empty");
				for(uint i = 0; i < char_class.size(); i++) {
					char_group += char_class[i];
				}
			} else if (i == 0 && c == '^') {
				excluded = true;
			} else if ((i > excluded) && (i < str.size()-1) && (c == '-')) {
				spanning = true;
			} else if (spanning) {
				for(char cc = char_group.back()+1; cc <= c; cc++) {
					char_group += cc;
				}
				spanning = false;
			} else {
				char_group += c;
			}
		}
	}

	if (excluded) {
		char_group = getExclusion(char_group);
	}

	State next_state = chooseState(start_states[0], char_group[0]);
	for(uint s = 0; s < start_states.size(); s++) {
		State state = start_states[s];
		for(uint i = 0; i < char_group.size(); i++) {
			setStateChange(state, char_group[i], next_state);
		}
	}
	end_states.push_back(next_state);
	return end_states;
}

std::string TokenStateMachine::getExclusion(const std::string& excluded) {
	std::string included;
	bool include_chars[128];
	for(uint i = 1; i < 128; i++) include_chars[i] = true;
	for(uint i = 0; i < excluded.size(); i++) {
		char c = excluded[i];
		include_chars[(uint)c] = false;
	}
	for(uint i = 1; i < 128; i++) {
		if (include_chars[i]) {
			included += (char)i;
		}
	}

	return included;
}

void TokenStateMachine::debug() {
	std::cout << "\nState | Type | Transitions\n";

	for(unsigned int state = 0; state < state_transitions.size(); state++) {
		std::cout << std::setw(5) << state << " | ";
		std::cout << std::setw(4) << state_types[state] << " | ";
		const std::map<char, uint>& transitions = state_transitions[state];
		for(auto it = transitions.begin(); it != transitions.end(); it++) {
			std:: cout << it->first << ": " << it->second << " ";
		}
		std::cout << '\n';
	}
	std::cout << '\n';
}

// static
bool TokenStateMachine::isQuantifier(char c) {
	return (c == '?' || c == '+' || c == '*');
}

State TokenStateMachine::chooseState(State cur_state, char c) const {
	State existing_state = getNextState(cur_state, c);
	return (existing_state == 0) ? newState() : existing_state;
}