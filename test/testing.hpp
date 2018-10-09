/*
made by Eric Roberts, 2018

mocha/chai-like test suite macros

ex)
describe("string foobar(int n)", {
	it("should return true", {
		string str;
		expectNoException(str = foobar(7))
		expectEqual(str.size(), 7)
	})

	it("should throw error", {
		expectException(foobar(-1), ExceptionType)
	})
})
*/

#ifndef UNIT_TEST_HPP
#define UNIT_TEST_HPP

#include <iostream>
#include <string>
#include <windows.h>
#include <stdexcept>
#include <exception>

#define RED FOREGROUND_RED
#define GREEN FOREGROUND_GREEN
#define BLUE FOREGROUND_BLUE
#define YELLOW (RED | GREEN)
#define PURPLE (RED | BLUE)
#define CYAN (BLUE | GREEN)
#define WHITE (RED | GREEN | BLUE)

unsigned int TEST_num_tests = 0;
unsigned int TEST_num_success = 0;
unsigned int TEST_num_failed = 0;
unsigned int TEST_print_depth = 0;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

#define print(message)\
std::cout << std::string(TEST_print_depth*2, ' ') << message << '\n';

#define printColour(message, colour)\
SetConsoleTextAttribute(hConsole, colour);\
std::cout << std::string(TEST_print_depth*2, ' ') << message << '\n';\
SetConsoleTextAttribute(hConsole, WHITE);

#define describe(message, code)\
{\
	print(message);\
	TEST_print_depth++;\
	code\
	TEST_print_depth--;\
}

#define it(message, code)\
{\
	print(message);\
	bool TEST_ok = true;\
	bool TEST_threw = false;\
	TEST_num_tests++;\
	TEST_print_depth++;\
	std::string TEST_test_error_message;\
	try {\
		code\
	} catch (std::exception& e) {\
		TEST_test_error_message = e.what();\
		TEST_threw = true;\
	} catch(...) {\
		TEST_threw = true;\
	}\
	if (TEST_threw) {\
		TEST_ok = false;\
		SetConsoleTextAttribute(hConsole, RED);\
		if (TEST_test_error_message.size() > 0) {\
			print("test threw exception: " << TEST_test_error_message);\
		} else {\
			print("test threw an exception");\
		}\
		SetConsoleTextAttribute(hConsole, WHITE);\
	}\
	if (TEST_ok) TEST_num_success++;\
	else TEST_num_failed++;\
	TEST_print_depth--;\
}

#define expect(actual, expected) TEST_expect(actual, ==, "equal", expected)
#define expectEqual(actual, expected) TEST_expect(actual, ==, "equal", expected)
#define expectNotEqual(actual, expected) TEST_expect(actual, !=, "not equal", expected)
#define expectGreaterThan(actual, expected) TEST_expect(actual, >, "greater than", expected)
#define expectGreaterThanOrEqual(actual, expected) TEST_expect(actual, >=, "greater than or equal", expected)
#define expectLesserThan(actual, expected) TEST_expect(actual, <, "lesser than", expected)
#define expectLesserThanOrEqual(actual, expected) TEST_expect(actual, <=, "lesser than or equal", expected)

#define TEST_expect(actual, comparison, comparison_text, expected)\
{\
	if (TEST_ok) {\
		try{\
			auto TEST_actual = actual;\
			if (!(TEST_actual comparison expected)) {\
				TEST_ok = false;\
				printColour("expected " << #actual << " (" << TEST_actual\
					<< ") to be " << comparison_text << " to " << expected, RED);\
			}\
		} catch (...) {\
			TEST_ok = false;\
			printColour(#actual << " threw an exception", RED);\
		}\
	}\
}

#define expectException(throwing_code, exception_type)\
{\
	bool TEST_correct_exception = false;\
	try {\
		throwing_code;\
	} catch (exception_type e) {\
		TEST_correct_exception = true;\
	} catch(...) {}\
	if (!TEST_correct_exception) {\
		TEST_ok = false;\
		printColour("expected " << #throwing_code << " to throw " << #exception_type, RED);\
	}\
}

#define expectNoException(throwing_code)\
{\
	bool TEST_exception = false;\
	std::string TEST_error_message;\
	try {\
		throwing_code;\
	} catch (std::exception& e) {\
		TEST_error_message = e.what();\
		TEST_exception = true;\
	} catch(...) {\
		TEST_exception = true;\
	}\
	if (TEST_exception) {\
		TEST_ok = false;\
		SetConsoleTextAttribute(hConsole, RED);\
		print("expected " << #throwing_code << " to not throw an exception");\
		if (TEST_error_message.size()) print("exception: " << TEST_error_message);\
		SetConsoleTextAttribute(hConsole, WHITE);\
	}\
}

#define displayTestResults()\
{\
	if (TEST_num_failed > 0) {\
		SetConsoleTextAttribute(hConsole, RED);\
	} else {\
		SetConsoleTextAttribute(hConsole, GREEN);\
	}\
	print("\nnumber of tests: " << TEST_num_tests);\
	print("number of successes: " << TEST_num_success);\
	print("number of failures: " << TEST_num_failed << '\n');\
	SetConsoleTextAttribute(hConsole, WHITE);\
}

#define failed() TEST_num_failed

#endif