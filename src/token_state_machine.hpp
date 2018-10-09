#ifndef TOKEN_STATE_MACHINE_HPP
#define TOKEN_STATE_MACHINE_HPP

#include <vector>
#include <map>
#include <string>
#include <stdexcept>

typedef unsigned int uint;
typedef uint State;
typedef std::vector<State> States;

class TokenStateMachine {
public:
	class Iterator {
	public:
		Iterator(TokenStateMachine* my_machine);
		void nextState(char c);
		uint getState() { return state; }
		int getType() { return type; }
		bool atEnd() { return state == 0; }

	private:
		State state;
		int type;
		TokenStateMachine* my_machine;
	};

	TokenStateMachine();
	TokenStateMachine(uint num_expressions, const std::string* expressions);
	TokenStateMachine(uint rows, const std::map<char, uint>* state_changes, const int* types);
	void addRule(std::string simple_regex, int type);
	Iterator begin();
	void debug();
	bool saveToFile(std::string filename);
	bool loadFromFile(std::string filename);

private:
	enum RegexGroupType {
		SINGLE,
		SEQUENCE,
		OPTIONS
	};

	static const std::string DIGITS;
	static const std::string WORD;
	static const std::string WHITESPACE;
	static const std::string LOWERCASE;
	static const std::string UPPERCASE;
	static const std::string HEXDIGITS;

	std::vector<std::map<char, uint>> state_transitions;
	std::vector<int> state_types;

	void setStateType(uint state, int type);
	void setStateChange(uint state, char c, uint next_state);
	uint getNextState(uint state, char c) const;

	States compileRegexSequence(States start_states, std::string str);
	States compileRegexGroup(States start_states, std::string str);
	States compileRegexBracketExpression(States start_states, std::string str);
	States compileRegexQuantifier(States start_states, std::string str);

	State newState() const { return state_transitions.size(); }
	State chooseState(State cur_state, char c) const;

	static char getEscapedCharacter(char c);
	static bool isQuantifier(char c);
	static std::string parseRegexGroup(const std::string& str, uint& index);
	static std::string parseMatchingBrackets(const std::string& str, uint& index);
	static std::string getExclusion(const std::string& excluded);
	static void machineAssert(bool condition, std::string message);
};

#endif