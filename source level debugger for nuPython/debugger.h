/*debugger.h*/
//
// defines the debugger class
// 
// Irene Ha
// Northwestern University
// CS 211

// Original author: Prof. Joe Hummel

#pragma once

#include <string>
#include <set>
#include <map>

#include "programgraph.h"
#include "ram.h"

using namespace std;

class Debugger
{
private:
  string State;
  struct STMT* Program;
  struct RAM*  Memory;
  map<int, bool> Breakpoints; // key is the line number and bool is false if never hit before and true if hit before
  map<int, struct STMT*> int_to_stmt; // map of all lines in the program wiht the key being a int line number and the value being a stmt
  //
  // controls where we start execution from:
  //
  struct STMT* curStmt; // this was moved from its place originally in Run()
  struct STMT* nextStmt; 


  
  void print_value(string varname, struct RAM_VALUE* value);
  bool findBreakpoint(struct STMT*& prev, struct STMT*& breakpoint, int lineNum);
  void linkOrUnlinkStmts(struct STMT* prev, struct STMT* cur);
  /* get_next_statement
  when getting the next statement it checks what type it is as every type has a differnet way of getting next statement
  Moves to the next statement by checking the type of the current statement (curr).

  parameter: stmt
  returns: struct
  */
  struct STMT* get_next_statement(struct STMT* stmt);

  /* find_prevStmt
  finds the previous statement from a current statement or "target"

  parameters: stmt
  returns: stmt
  */
  struct STMT* find_prevStmt(struct STMT* target);

  /* execute_steps
  checks what to do based on if the command is s or r

  parameters: string
  returns: bool
  */
  //bool execute_steps(string cmd, bool s_or_r);

  /* execute_single_step
  executes single step. this is used for our s and r command

  parameters: string
  returns: bool
  */
  bool execute_single_step(const string& cmd);

public:
  Debugger(struct STMT* program);

  ~Debugger();

  void run();
};

