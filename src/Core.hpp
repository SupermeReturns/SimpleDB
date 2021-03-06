/*
 * Core.hpp
 *
 *  Created on: Nov 21, 2014
 *      Author: thomas
 */

#ifndef CORE_HPP_
#define CORE_HPP_

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Exceptions.hpp"
#include <set>
#include <vector>
#include <iostream>

struct Row {
	int key;
	std::vector<int> keys;
	std::vector<int> cols;
	bool operator==(const Row& r) const {
		if (sizeof(keys) == 0 || sizeof(r.keys) == 0) {
			return key == r.key;
		}
		for (auto& i : keys) {
			if (cols[i] == r.cols[i])
				return true;
		}
		return false;
	}

	bool operator<(const Row& r) const {
		return key < r.key;
	}

	Row(const std::vector<int>& c, std::vector<int> k, int count) {
		cols = c;
		keys = k;
		if (keys.size() != 0) {
			key = cols[keys[0]];
		} else {
			key = count;
		}
	}

	//Dummy usage
	Row(int k) {
		key = k;
	}
};

struct Table {
	//table id
	std::string id;
	//table props
	std::vector<Property> props;
	//primary key index
	std::vector<int> key_idx;
	//if no primary key, used for search
	unsigned long long count;
	//use string to store various type of values
	std::set<Row> rows;
	//insert a record to table
	//if value not specified, use default value
	void Insert(const std::vector<int>& record);
	void Delete(const Condition* cond);

	// return keys of rows that matches the condition
	std::vector<int> Query(const Condition* cond);

	Table(const std::string& i, const std::vector<std::string>& ki,
			const std::list<Property>& p) {
		count = 1;
		int n = 0;
		for (auto& i : ki) {
			for (auto& j : p) {
				if (j.id == i) {
					key_idx.push_back(n);
					break;
				}
			}
			n++;
		}
		for (auto& i : p) {
			props.push_back(i);
		}
		id = i;
	}

	bool operator<(const Table& t) const {
		return id < t.id;
	}
};

/*
 *
 * CREATE
 *    table name doesn't match with any existed table
 *
 * INSERT
 *    the table should exist
 *    no primary key constraint violation
 *    all columns should be in the schema of the table
 *    For a column without specified value, default
 *    value is used.
 *
 * WHERE
 *    columns occurring in the where clause (if any)
 *    should be in the schema of the table
 *
 * DELETE
 *    If there is a where clause, delete all rows
 *    whose where clause is evaluated to be TRUE.
 *    Otherwise, delete all rows in the table.
 *
 * SELECT
 *    If a where clause is present, those rows whose
 *    where clauses are evaluated to FALSE should
 *    be omitted. Otherwise, none should be omitted.
 *
 *    If '*' is present in the select list, all columns
 *    should be returned. Otherwise, return only
 *    those columns specified in the select list.
 */

class SimpleDB {
private:
	void Execute(const Statement& stmt);
	Parser* parser;

public:
	std::set<Table> tables;
	SimpleDB() {
		parser = new Parser();
	}
	void Execute(const std::string& stmt) {
		Statement s;
		try {
			s = parser->Parse(stmt);
		} catch (SDBException& e) {
			e.Print();
			return;
		}
		Execute(s);
	}

};

#endif /* CORE_HPP_ */
