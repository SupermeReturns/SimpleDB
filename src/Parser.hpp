/*
 * Parser.hpp
 *
 *  Created on: Nov 21, 2014
 *      Author: thomas
 */

#ifndef PARSER_HPP_
#define PARSER_HPP_

#include "Lexer.hpp"
#include <iostream>
#include <map>
#include <stack>
#include <queue>
#include <list>
#include <string>
#include <stdlib.h>

enum Action {
	CREATE, DELETE, INSERT, QUERY, INVALID,
};

class Polish {
private:
	std::list<std::string> ops;
	std::queue<std::string> rp;
	std::map<std::string, int> level;
	std::set<std::string> _ops;
public:
	Polish() {
		level.insert(std::pair<std::string, int>("+", 0));
		level.insert(std::pair<std::string, int>("-", 0));
		level.insert(std::pair<std::string, int>("*", 1));
		level.insert(std::pair<std::string, int>("/", 1));
		level.insert(std::pair<std::string, int>("(", -1));
		level.insert(std::pair<std::string, int>(")", -1));
		_ops.insert("+");
		_ops.insert("-");
		_ops.insert("*");
		_ops.insert("/");
	}
	void Insert(const std::string& item) {
		if (_ops.find(item) != _ops.end()) {
			while (level.find(item)->second <= level.find(ops.back())->second) {
				rp.push(ops.back());
				ops.pop_back();
			}
			ops.push_back(item);
		} else if (item == "(") {
			ops.push_back("(");
		} else if (item == ")") {
			while (ops.back() != "(") {
				rp.push(ops.back());
				ops.pop_back();
			}
			ops.pop_back();
		} else {
			rp.push(item);
		}
	}
	int Calculate() {
		std::stack<int> nums;
		while (!ops.empty()) {
			rp.push(ops.back());
			ops.pop_back();
		}
		while (!rp.empty()) {
			std::string to_ret = rp.front();
			rp.pop();
			if (_ops.find(to_ret) != _ops.end()) {
				if (nums.size() < 2) {
					std::cerr << "Wrong expression.\n";
					return 0;
				} else {
					int b = nums.top();
					nums.pop();
					int a = nums.top();
					nums.pop();
					if (to_ret == "+") {
						nums.push(a + b);
					} else if (to_ret == "*") {
						nums.push(a * b);
					} else if (to_ret == "-") {
						nums.push(a - b);
					} else if (to_ret == "/") {
						nums.push(a / b);
					}
				}
			} else {
				nums.push(atoi(to_ret.c_str()));
			}
		}
		if (nums.size() != 1) {
			std::cerr << "Wrong expression.\n";
			return 0;
		} else {
			int t = nums.top();
			nums.pop();
			return t;
		}
	}
};

//arithmetic operators
//for parser only
enum ArithOp {
	PLUS, // +, both unary and binary
	MINUS, // -, both unary and binary
	MULTIPLY, // *
	DIVIDE, // /
	E, // =
	LB, // (
	RB, // )
	COMMA, // ,
};

//boolean operators
enum BoolOp {
	LT, // <
	GT, // >
	NE, // <>
	EQ, // ==
	GTE, // >=
	LTE, // <=
	AND, // &&
	OR, // ||
	NOT, // !
};

/*
 * reduce to boolean tree
 * only contains boolean operations for <id, num> or <id, id> pairs
 */

struct Condition {
	/*
	 * operand
	 * 		ID
	 * 		NUM
	 */
	// operator
	BoolOp op;

	//TODO:
	//use isNum and isId to check opd
	std::string opd;

	// left operand
	Condition* lc;

	//right
	Condition* rc;

	Condition() {
		op = NOT;
		lc = rc = NULL;
	}

	Condition(BoolOp o) {
		op = o;
		lc = rc = NULL;
	}
};

/*
 * parser tree:
 * tree->ID OP NUM
 * 		| ID OP ID
 * 		| NUM OP NUM
 * 		| NUM OP ID
 * 		| tree OP tree
 * NOT operator generate right subtree
 */

struct Property {
	/*
	 * id (identifier) is a sequence of digits, underline and letters. All
	 * identifiers should start with a letter or an underline. The maximum
	 * length of an identifier is 64.
	 */
	std::string id;

	/*
	 default 0 for default value if no default specified
	 */
	int default_value;

	bool operator==(const Property& o) {
		return o.id == id;
	}
};

struct Statement {
	/*
	 * action
	 */
	Action act;

	/*
	 * operand table
	 */
	std::string table;

	/*
	 * list of properties to return
	 * used with select and create
	 */
	std::list<Property> prop_list;

	/*
	 * location of primary key
	 * used with create
	 * -1 for no primary key
	 */
	std::vector<std::string> key_idx;

	/*
	 * value list, only for insert
	 */
	std::vector<int> value_list;

	/*
	 * boolean tree
	 * NULL for none(insert and
	 */
	Condition* cond;

	void treeInsert(Token node);
	Condition* insertNodeSearch(Condition* &cond);
	void rotate(Condition* &cond);
	Statement() {
		act = INVALID;
		cond = NULL;
	}

};

struct state {
	int count;
	std::string sta;
};

/*ERROR Handling
 * The parser should handle the ill-formed statements, but
 * not check its consistency.
 * To check:
 *   illegal grammar
 * Not to check:
 *   table existence
 *   value consistency
 * TODO:
 * Print meaningful error prompts
 * When an error is encountered, stop parsing.
 * what's the error
 * LINE NUMBER HANDLED BY MAIN.CPP
 */

/*
 * ssql_stmt -> create_stmt
 *    | insert_stmt
 *    | delete_stmt
 *    | query_stmt
 */

class Parser {
private:

	/*CREATE
	 *
	 *create_stmt → create table id (decl_list);
	 *    decl_list -> decl | decl_list, decl
	 *    decl -> id int default_spec | primary key(column_list)
	 *    default_spec → default = num | ε
	 *    column_list →id | column_list, id
	 *
	 * EXAMPLE:
	 * CREATE TABLE Student(sid INT, age INT DEFAULT = 18, PRIMARY KEY (sid));
	 *
	 * To check:
	 *   no duplicate column names
	 *   no two or more primary key declarations
	 *   primary key contains only columns in the table
	 *   # of columns in a table should <= 100
	 *   # of columns in primary key declaration <= 100
	 */

	/*INSERT
	 *
	 * insert_stmt → insert into id(column_list) values (value_list);
	 *     value_list → num | value_list, num
	 *
	 * To check:
	 *    no duplicate columns (see the example below)
	 *    # of columns should equal to # of values
	 */

	/*DELETE
	 *
	 * delete_stmt → delete from id where_clause;
	 *
	 * To check:
	 *
	 */

	/*SELECT
	 *
	 * select_stmt → select select_list from id where_clause;
	 *      select_list → column_list | *
	 *
	 * To check:
	 *
	 */

	Condition* parseWhere(std::list<Token> ts);

	/*WHERE
	 * where_clause → where conjunct_list | ε
	 *     conjunct_list → bool | conjunct_list && bool
	 *     bool →operand rop operand
	 *     operand → num | id
	 *     rop → <> | == | > | < | >= | <=
	 *
	 * To check
	 *
	 */
	Lexer* lexer;
	void initTable();
	void initTerminal();
	void initAction();
	typedef void (*act)(Statement &s, Token t, std::string father);
	std::map<std::pair<std::string, std::string>, std::list<std::string> > table;
	std::set<std::string> terminal;
	std::map<std::string, act> action;
	std::stack<std::string> procedure;
	std::string getTokenSymbol(Token t);
public:
	Parser();
	Statement Parse(const std::string& s);
};

#endif /* PARSER_HPP_ */
