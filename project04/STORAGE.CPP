// getting s s s w to work

/*debugger.cpp*/

//
// Debugger for nuPython, C++ edition! Provides a simple gdb-like
// interface, with support for one breakpoint in straight-line code.
// Uses nuPython interpreter as execution engine.
//
// Authors:
//    Ata Guvendi
//    Prof. Hummel
// 
// Northwestern University
// CS 211
//

#include <iostream>
#include <cassert>

#include "debugger.h"
#include "execute.h"

using namespace std;


//
// print_value
//
// Prints the contents of a RAM cell, both type and value.
//
void Debugger::print_value(string varname, struct RAM_VALUE* value)
{
  cout << varname << " ("; 
  
  switch (value->value_type) {
    
    case RAM_TYPE_INT:
      cout << "int): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_REAL:
      cout << "real): " << value->types.d << endl;
      break;
      
    case RAM_TYPE_STR:
      cout << "str): " << value->types.s << endl;
      break;
      
    case RAM_TYPE_PTR:
      cout << "ptr): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_BOOLEAN:
      cout << "bool): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_NONE:
      cout << "none): " << "None" << endl;
      break;
  }//switch
}


//
// findBreakpoint
//
// starts from the stmt denoted by breakpoint, and searches forward
// for a stmt with the given line number. As it advances, prev is
// updated to always denote the previous statement. The parameters
// are passed-by-reference so they are updated back in the caller.
//
// Note that if breakpoint happens to match immediately, without advancing,
// then prev does not get set. So the caller should probably set prev 
// appropriately before calling (either nullptr, or to the previous stmt).
//
bool Debugger::findBreakpoint(struct STMT*& prev, struct STMT*& breakpoint, int lineNum)
{
  while (breakpoint != nullptr) {
    
    // cout << breakpoint->line << endl;
    
    if (breakpoint->line == lineNum) // found it!
      return true;
      
    prev = breakpoint;
    
    if (breakpoint->stmt_type == STMT_ASSIGNMENT)
      breakpoint = breakpoint->types.assignment->next_stmt;
    else if (breakpoint->stmt_type == STMT_FUNCTION_CALL)
      breakpoint = breakpoint->types.function_call->next_stmt;
    else {
      assert(breakpoint->stmt_type == STMT_PASS);
      breakpoint = breakpoint->types.pass->next_stmt;
    }
  }//while
  
  // if get here, not found
  return false;
}


//
// linkOrUnlinkStmts
//
// updates prev stmt to point to whatever cur is --- this can be 
// nullptr to unlink, or an actual stmt to link them back together.
//
void Debugger::linkOrUnlinkStmts(struct STMT* prev, struct STMT* cur)
{
  if (prev == nullptr) // nothing to link or unlink:
    return;
    
  if (prev->stmt_type == STMT_ASSIGNMENT)
    prev->types.assignment->next_stmt = cur;
  else if (prev->stmt_type == STMT_FUNCTION_CALL)
    prev->types.function_call->next_stmt = cur;
  else {
    assert(prev->stmt_type == STMT_PASS);
    prev->types.pass->next_stmt = cur;
  }
}



/* get_next_statement
when getting the next statement it checks what type it is as every type has a differnet way of getting next statement
Moves to the next statement by checking the type of the current statement (curr).

parameter: stmt
returns: struct
*/
struct STMT* Debugger::get_next_statement(struct STMT* stmt) {
  if (stmt->stmt_type == STMT_ASSIGNMENT) {
      return stmt->types.assignment->next_stmt;
  } else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
      return stmt->types.function_call->next_stmt;
  } else if (stmt->stmt_type == STMT_PASS) {
      return stmt->types.pass->next_stmt;
  } else {
      return nullptr;
  }
} 

/* find_prevStmt
finds the previous statement from a current statement or "target"

parameters: stmt
returns: stmt
*/
struct STMT* Debugger::find_prevStmt(struct STMT* target) {
  struct STMT* prev = nullptr;
  struct STMT* current = this->Program;

  while (current != nullptr && current != target) {
    prev = current;
    current = get_next_statement(current);  // Use get_next_statement to move to the next
  }

  return prev;  // Returns nullptr if target is the first statement
};

/* execute_steps
checks what to do based on if the command is s or r

parameters: string
returns: bool
*/
bool Debugger::execute_steps(string cmd){
  // If we're just starting, initialize nextStmt
  if (nextStmt == nullptr) {
    if (this->State == "Loaded") { // Beginning of program
      nextStmt = get_next_statement(curStmt); // Initialize first step
      this->State = "Running";
      cout << "Program started, state changed to " << this->State << endl;
    }
    else if (this->State == "Running") { // End of program
      this->State = "Completed";
      cout << "Program completed, state changed to " << this->State << endl;
      return true; // End program
    }
  }
  
  if (cmd == "s") {
    cout << "entered an s command. calling execute_single_step()"<< endl;
    return execute_single_step(cmd); // Return true if the program completes, otherwise false
    //return false; // exit after a single step if the command is "s"
  }
  
  // for the r command, continue to run until completion or breakpoint is hit
  while (!execute_single_step(cmd)) {} // the loop will continue until execute_single_step() returns true indicating the program has completed

  return true; // default return if command is r and the loop should continue
}

/* execute_single_step
executes single step. this is used for our s and r command

parameters: string
returns: bool
*/
bool Debugger::execute_single_step(const string& cmd) {
  // 1. breakpoint check: if the curStmt is a breakpoint then outputs false and stops
  if (Breakpoints.find(curStmt->line) != Breakpoints.end()) {
    cout << "hit breakpoint on line " << curStmt->line << endl;
    return false;
  } 

  // 2. set a temp breakpoint for s command
  //struct STMT* breakStmt = nullptr;
  if (cmd == "s") { // If the command is "s", set a temporary breakpoint at the next statement
    cout << "If the command is s, set a temporary breakpoint at the next statement"<< endl; // DEBUG
    cout << "curStmt right now is ... " << curStmt->line << endl; //DEBUG
    //breakStmt = get_next_statement(curStmt);
    cout << "single step bp is set" << endl;
    linkOrUnlinkStmts(curStmt, nullptr); //unlink curStmt from breakStmt temporarily
    cout << "unlinked at this spot" << endl; // debug
  }

  // 3. Execution check: if the execution of the curStmt fails, repair the graph and set state to completed and return true
  struct ExecuteResult er = execute(curStmt, this->Memory);
  cout << "execution is run... " << endl; // debug
  if (!er.Success) { // execution failed aka semantic error, repair the graph
    cout << "execution failed" << endl; // debug
    struct STMT* prevStmt = find_prevStmt(curStmt);
    linkOrUnlinkStmts(prevStmt, curStmt); //repair graph. use the find_prevStmt method 
    this->State = "Completed";
    return true;
  }
 
  // step 3.5) relink the graph
  if (cmd=="s" && nextStmt != nullptr) { // if we click s and we havent reached the end or are starting from the beginning
    cout << "link retored" << endl; // debug
    linkOrUnlinkStmts(curStmt, nextStmt); // restore link between curStmt and breakstmt 
  }

  // 4. Advance to nextStmt: if there is a next stmt aka not nullptr, then break the graph and advance to the next statement by updating curStmt and returning false
  //struct STMT* nextStmt = get_next_statement(curStmt);
  cout << "nextStmt is set the the next stmt of curstmt" << endl; //debug
  cout << "next stmt: " << nextStmt->line << endl; // DEBUG
  if (nextStmt == nullptr) { // no more stmts program is complete
    cout << "nextStmt was a nullptr. program is complete" << endl; // debug
    this->State = "Completed";
    return true;
  } else {
    // linkOrUnlinkStmts(curStmt, nullptr); // Break the graph between curStmt and nextstmt
    curStmt = nextStmt;
    nextStmt = get_next_statement(curStmt); //IDKKKKKKKKK 
    cout << "curstmt is set to the nextstmt" << endl;
  }

  // // 5. stop at the temporary breakpoint for s
  // if (curStmt == breakStmt) {
  //   cout << "reached temp break for single step" << endl; // DEBUG
  //   return false; // stop at the single step
  // }
  return false; // continue execution if no breakpoint or end of program
  cout << "false is returned for execute_single_step" << endl;
}


//
// constructor:
//
Debugger::Debugger(struct STMT* program)
  : State("Loaded"), Program(program), Memory(nullptr), Breakpoints(), curStmt(this->Program), nextStmt(nullptr)
{
  this->Memory = ram_init();
}

//
// destructor:
//
Debugger::~Debugger()
{
  ram_destroy(this->Memory);
}


//
// run:
//
// Run the debugger for one execution run of the input program.
//
void Debugger::run()
{
  string cmd;
  
  // //
  // // controls where we start execution from:
  // //
  // struct STMT* curStmt = this->Program;
  
  //
  // if we have a breakpoint, these will be set to the breakpoint,
  // and the statement preceding it:
  //
  struct STMT* prevBreakpoint = nullptr;
  struct STMT* breakpoint = nullptr;
  
  //
  // command loop until quit is entered:
  //
  while (true) {
    cout << endl;
    cout << "Enter a command, type h for help. Type r to run. > ";
    cin >> cmd;
    
    if (cmd == "q") { // consider including something to relink the graph and reset breakpoint data to prevent memory leaks
      break;
    }
    else if (cmd == "h") {
      
      cout << "Available commands:"
      << endl << "r -> Run the program / continue from a breakpoint"
      << endl << "s -> Step to the next stmt by executing current stmt"
      << endl << "b n -> Breakpoint at line n" 
      << endl << "rb n -> Remove breakpoint at line n"
      << endl << "lb -> List all breakpoints"
      << endl << "cb -> Clear all breakpoints"
      << endl << "p varname -> Print variable"
      << endl << "sm -> Show memory contents"
      << endl << "ss -> Show state of debugger"
      << endl << "w -> What line are we on?"
      << endl << "q -> Quit the debugger"
      << endl;
    } else if (cmd == "r" or cmd == "s") {
      if (this->State == "Completed") {
        cout << "program has completed" << endl;
        continue;
      } 

      //moved to execute
      // if (this->State == "Loaded") {
      //   this->State = "Running";
      //   cout << "state was changed to " << this->State << endl;
      // }

      execute_steps(cmd);



    // else if (cmd == "r") {
      
    //   //
    //   // run, or continue running, the program:
    //   //
    //   if (this->State == "Completed") {
    //     cout << "program has completed" << endl;
    //     continue; // skip the code below and repeat the loop for next cmd:
    //   }
      
    //   if (this->State == "Loaded")
    //     this->State = "Running";
      
    //   //
    //   // execute via nuPython interpreter:
    //   //
    //   struct ExecuteResult er = execute(curStmt, this->Memory);
      
    //   //
    //   // what happened during execution?
    //   //
    //   if (!er.Success) { // there was an error, we've complete execution:
    //     this->State = "Completed";
    //     continue;  // skip code below and continue loop with next cmd:
    //   }
      
    //   if (er.LastStmt != prevBreakpoint) { // then we ran to completion:
    //     this->State = "Completed";
    //     continue;  // skip code below and continue loop with next cmd:
    //   }
      
    //   //
    //   // if we get here, we've stopped at our breakpoint, so put the 
    //   // program graph back together and setup to continue execution
    //   // from breakpoint:
    //   //
    //   linkOrUnlinkStmts(prevBreakpoint, breakpoint);
     
    //   // start from breakpoint:
    //   curStmt = breakpoint;
      
    //   // and clear breakpoint because it was reached:
    //   prevBreakpoint = nullptr;
    //   breakpoint = nullptr;
    // }
    } else if (cmd == "b") {
      
      int lineNum;
      cin >> lineNum;
      
      //search the set to see if this breakpoint already exists
      if (Breakpoints.find(lineNum) != Breakpoints.end()) { // if it already exists in set
        cout << "breakpoint already set" << endl;
        continue;
      } 

      // if (breakpoint != nullptr) { // sorry, we already have a breakpoint set:
      //   cout << "breakpoint already set" << endl;
      //   continue;  // skip code below and continue with next cmd:
      // }
      
      //
      // Okay, so no breakpoint is set, so we have a valid program
      // graph that we can traverse.
      //
      // first, search the program for the breakpoint, along with
      // the stmt that precedes it:
      //
      prevBreakpoint = nullptr;
      breakpoint = this->Program; // start from first stmt so prev will get set:
      
      bool exists = findBreakpoint(prevBreakpoint, breakpoint, lineNum);
      
      if (!exists) {
        cout << "no such line" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      //
      // okay, the breakpoint line exists, so let's set the breakpoint
      // by severing the program graph between prevStmt and curStmt:
      //
      linkOrUnlinkStmts(prevBreakpoint, nullptr);
      Breakpoints.insert(lineNum); // add line number to breakpoints set if it checked the line exists check
      cout << "breakpoint set" << endl; //DEBUG CHECK
    }
    else if (cmd == "rb") {
      
      int lineNum;
      cin >> lineNum;
      
      if (breakpoint == nullptr) // we have no breakpoint
        cout << "no such breakpoint" << endl;
      else if (breakpoint->line != lineNum) // not the same breakpoint
        cout << "no such breakpoint" << endl;
      else {  // removing current breakpoint:
        //
        // we remove by linking the program graph back together:
        //
        linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
        prevBreakpoint = nullptr;
        breakpoint = nullptr;
     
        cout << "breakpoint removed" << endl;
      }
    }
    else if (cmd == "lb") {
      
      if (breakpoint == nullptr) {
        cout << "no breakpoints" << endl;
      } else {// we have a breakpoint:
        cout << "breakpoints on lines:"; 
        // for each breakpoint line number in set of breakpoints
        for (auto iter : Breakpoints) { // iter in this case is a line number 
          cout << " " + iter;
        }
        cout << endl;
      }
    }
    else if (cmd == "cb") {
      
      if (breakpoint != nullptr) { // we have a breakpoint to clear:
        //
        // we clear by linking the program graph back together:
        //
        linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
        //reset breakpoint data
        prevBreakpoint = nullptr;
        breakpoint = nullptr;
      }
      
      cout << "breakpoints cleared" << endl;
    }
    else if (cmd == "p") {
      
      string varname;
      cin >> varname;
      
      const char* name = varname.c_str();
      
      struct RAM_VALUE* value = ram_read_cell_by_name(this->Memory, (char*) name);
      
      if (value == nullptr) {
        cout << "no such variable" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      print_value(varname, value);
      ram_free_value(value);
    }
    else if (cmd == "sm") {
      
      ram_print(this->Memory);
    }
    else if (cmd == "ss") {
      
      cout << this->State << endl;
    }
    else if (cmd == "w") {
      
      if (this->State == "Loaded") {
        cout << "line_loaded " << this->Program->line << endl;
        // struct STMT* stop = get_next_statement(curStmt);
        // linkOrUnlinkStmts(curStmt, stop);
        programgraph_print(curStmt);
        
      } else if (this->State == "Completed") {
        cout << "completed execution" << endl;
      } else {// we are running:
        //struct STMT* nextStmt = get_next_statement(curStmt); // create a variable that stores the next stmt
        if (nextStmt != nullptr) {
          cout << "line " << curStmt->line << endl;
          //cout << "next" << nextStmt->line << endl; //DEBUG
          linkOrUnlinkStmts(curStmt, nullptr); // unlink
          //cout << (nullptr == get_next_statement(curStmt)) << endl; //DEBUG 0 means its not unlinked prop and 1 means that its unlinked
          programgraph_print(curStmt); // print one line
          linkOrUnlinkStmts(curStmt, nextStmt); // link again
        } else { // reached the end of the program 
          cout << "completed execution" << endl; 
        } 

      }



    // } else if (cmd == "s") { // this will execute a ingle statement at a time
    //   if (this->State == "Completed") {
    //     cout << "program has completed" << endl;
    //     return;
    //   } else if (this->State == "Loaded") {
    //     this->State = "Running"; // change state to running
    //     curStmt = this->Program; // set the current stmt equal to the beginning of the program
    //   } 
    //   // no else statement for if the state is Running is necessary becausr curStmt is already pointing to the curStmt 
    //   // and there is no need to reset it in the running state when we just did that if it was in the loading state
      
    //   execute_single_step(); // call on helper function
    } else {
      cout << "unknown command" << endl;
    }

  }//while
  
  //
  // at this point execution has completed (or the user quit
  // early). If there's still a breakpoint, we need to repair
  // the program graph so the memory is freed properly when
  // we return to main:
  //
  if (breakpoint != nullptr) { // there's still a breakpoint
    linkOrUnlinkStmts(prevBreakpoint, breakpoint);
    
    prevBreakpoint = nullptr; // not really necessary, but just to be safe
    breakpoint = nullptr;
  }
  
}//run

































// this is where I left off with jeremy
// this is where we set b 7 and then r but then it keeps running until 7 continueously
/*debugger.cpp*/

//
// Debugger for nuPython, C++ edition! Provides a simple gdb-like
// interface, with support for one breakpoint in straight-line code.
// Uses nuPython interpreter as execution engine.
//
// Authors:
//    Ata Guvendi
//    Prof. Hummel
// 
// Northwestern University
// CS 211
//

#include <iostream>
#include <cassert>

#include "debugger.h"
#include "execute.h"

using namespace std;


//
// print_value
//
// Prints the contents of a RAM cell, both type and value.
//
void Debugger::print_value(string varname, struct RAM_VALUE* value)
{
  cout << varname << " ("; 
  
  switch (value->value_type) {
    
    case RAM_TYPE_INT:
      cout << "int): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_REAL:
      cout << "real): " << value->types.d << endl;
      break;
      
    case RAM_TYPE_STR:
      cout << "str): " << value->types.s << endl;
      break;
      
    case RAM_TYPE_PTR:
      cout << "ptr): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_BOOLEAN:
      cout << "bool): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_NONE:
      cout << "none): " << "None" << endl;
      break;
  }//switch
}


//
// findBreakpoint
//
// starts from the stmt denoted by breakpoint, and searches forward
// for a stmt with the given line number. As it advances, prev is
// updated to always denote the previous statement. The parameters
// are passed-by-reference so they are updated back in the caller.
//
// Note that if breakpoint happens to match immediately, without advancing,
// then prev does not get set. So the caller should probably set prev 
// appropriately before calling (either nullptr, or to the previous stmt).
//
bool Debugger::findBreakpoint(struct STMT*& prev, struct STMT*& breakpoint, int lineNum)
{
  while (breakpoint != nullptr) {
    
    // cout << breakpoint->line << endl;
    
    if (breakpoint->line == lineNum) // found it!
      return true;
      
    prev = breakpoint;
    
    if (breakpoint->stmt_type == STMT_ASSIGNMENT)
      breakpoint = breakpoint->types.assignment->next_stmt;
    else if (breakpoint->stmt_type == STMT_FUNCTION_CALL)
      breakpoint = breakpoint->types.function_call->next_stmt;
    else {
      assert(breakpoint->stmt_type == STMT_PASS);
      breakpoint = breakpoint->types.pass->next_stmt;
    }
  }//while
  
  // if get here, not found
  return false;
}


//
// linkOrUnlinkStmts
//
// updates prev stmt to point to whatever cur is --- this can be 
// nullptr to unlink, or an actual stmt to link them back together.
//
void Debugger::linkOrUnlinkStmts(struct STMT* prev, struct STMT* cur)
{
  if (prev == nullptr) // nothing to link or unlink:
    return;
    
  if (prev->stmt_type == STMT_ASSIGNMENT)
    prev->types.assignment->next_stmt = cur;
  else if (prev->stmt_type == STMT_FUNCTION_CALL)
    prev->types.function_call->next_stmt = cur;
  else {
    assert(prev->stmt_type == STMT_PASS);
    prev->types.pass->next_stmt = cur;
  }
}



/* get_next_statement
when getting the next statement it checks what type it is as every type has a differnet way of getting next statement
Moves to the next statement by checking the type of the current statement (curr).

parameter: stmt
returns: struct
*/
struct STMT* Debugger::get_next_statement(struct STMT* stmt) {
  if (stmt->stmt_type == STMT_ASSIGNMENT) {
      return stmt->types.assignment->next_stmt;
  } else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
      return stmt->types.function_call->next_stmt;
  } else if (stmt->stmt_type == STMT_PASS) {
      return stmt->types.pass->next_stmt;
  } else {
      return nullptr;
  }
} 

/* find_prevStmt
finds the previous statement from a current statement or "target"

parameters: stmt
returns: stmt
*/
struct STMT* Debugger::find_prevStmt(struct STMT* target) {
  struct STMT* prev = nullptr;
  struct STMT* current = this->Program;

  while (current != nullptr && current != target) {
    prev = current;
    current = get_next_statement(current);  // Use get_next_statement to move to the next
  }

  return prev;  // Returns nullptr if target is the first statement
};

/* execute_steps
checks what to do based on if the command is s or r

parameters: string
returns: bool
*/
bool Debugger::execute_steps(string cmd){
  // If we're just starting, initialize nextStmt
  if (nextStmt == nullptr) {
    if (this->State == "Loaded") { // Beginning of program
      nextStmt = get_next_statement(curStmt); // Initialize first step
      this->State = "Running";
      cout << "Program started, state changed to " << this->State << endl;
    }
    else if (this->State == "Running") { // End of program
      this->State = "Completed";
      cout << "Program completed, state changed to " << this->State << endl;
      return true; // End program
    }
  }
  
  if (cmd == "s") {
    cout << "entered an s command. calling execute_single_step()"<< endl;
    return execute_single_step(cmd); // Return true if the program completes, otherwise false
    //return false; // exit after a single step if the command is "s"
  }
  
  // for the r command, continue to run until completion or breakpoint is hit
  while (!execute_single_step(cmd)) {} // the loop will continue until execute_single_step() returns true indicating the program has completed

  return true; // default return if command is r and the loop should continue
}

/* execute_single_step
executes single step. this is used for our s and r command

parameters: string
returns: bool
*/
bool Debugger::execute_single_step(const string& cmd) {
  // 1. breakpoint check: if the curStmt is a breakpoint then outputs false and stops
  cout << curStmt->line;
  if (Breakpoints.find(curStmt->line) != Breakpoints.end()) {
    cout << "hit breakpoint on line " << curStmt->line << endl;
    return false;
  } 

  // 2. set a temp breakpoint for s command
  //struct STMT* breakStmt = nullptr;
  if (cmd == "s") { // If the command is "s", set a temporary breakpoint at the next statement
    cout << "If the command is s, set a temporary breakpoint at the next statement"<< endl; // DEBUG
    cout << "curStmt right now is ... " << curStmt->line << endl; //DEBUG
    //breakStmt = get_next_statement(curStmt);
    cout << "single step bp is set" << endl;
    linkOrUnlinkStmts(curStmt, nullptr); //unlink curStmt from breakStmt temporarily
    cout << "unlinked at this spot" << endl; // debug
  }

  // 3. Execution check: if the execution of the curStmt fails, repair the graph and set state to completed and return true
  struct ExecuteResult er = execute(curStmt, this->Memory);
  cout << "execution is run... " << endl; // debug
  if (!er.Success) { // execution failed aka semantic error, repair the graph
    cout << "execution failed" << endl; // debug
    struct STMT* prevStmt = find_prevStmt(curStmt);
    linkOrUnlinkStmts(prevStmt, curStmt); //repair graph. use the find_prevStmt method 
    this->State = "Completed";
    return true;
  }
 
  // step 3.5) relink the graph
  if (cmd=="s" && nextStmt != nullptr) { // if we click s and we havent reached the end or are starting from the beginning
    cout << "link retored" << endl; // debug
    linkOrUnlinkStmts(curStmt, nextStmt); // restore link between curStmt and breakstmt 
  }

  // 4. Advance to nextStmt: if there is a next stmt aka not nullptr, then break the graph and advance to the next statement by updating curStmt and returning false
  //struct STMT* nextStmt = get_next_statement(curStmt);
  cout << "nextStmt is set the the next stmt of curstmt" << endl; //debug
  cout << "next stmt: " << nextStmt->line << endl; // DEBUG
  if (nextStmt == nullptr) { // no more stmts program is complete
    cout << "nextStmt was a nullptr. program is complete" << endl; // debug
    this->State = "Completed";
    return true;
  } else {
    // linkOrUnlinkStmts(curStmt, nullptr); // Break the graph between curStmt and nextstmt
    curStmt = nextStmt;
    nextStmt = get_next_statement(curStmt); //IDKKKKKKKKK 
    cout << "curstmt is set to the nextstmt" << endl;
  }

  // // 5. stop at the temporary breakpoint for s
  // if (curStmt == breakStmt) {
  //   cout << "reached temp break for single step" << endl; // DEBUG
  //   return false; // stop at the single step
  // }
  cout << "false is returned for execute_single_step" << endl;
  return false; // continue execution if no breakpoint or end of program
  
}


//
// constructor:
//
Debugger::Debugger(struct STMT* program)
  : State("Loaded"), Program(program), Memory(nullptr), Breakpoints(), curStmt(this->Program), nextStmt(nullptr)
{
  this->Memory = ram_init();
  // fill the map with every line of the program
  //traverse program graph
  //at each statement, add to mapp so that map<line number (cur->line), the stmt (cur)>
  struct STMT* cur = this->Program;
  while (cur != nullptr) {

    int_to_stmt[cur->line] = cur;
    cur = get_next_statement(cur);
  }

  // debug to check if the map was populated correctly
  for (auto pair : int_to_stmt) {
    cout << pair.first<< " : " << pair.second->line<< endl;
  }

}

//
// destructor:
//
Debugger::~Debugger()
{
  ram_destroy(this->Memory);
}


//
// run:
//
// Run the debugger for one execution run of the input program.
//
void Debugger::run()
{
  string cmd;
  
  // //
  // // controls where we start execution from:
  // //
  // struct STMT* curStmt = this->Program;
  
  //
  // if we have a breakpoint, these will be set to the breakpoint,
  // and the statement preceding it:
  //
  struct STMT* prevBreakpoint = nullptr;
  struct STMT* breakpoint = nullptr;
  
  //
  // command loop until quit is entered:
  //
  while (true) {
    cout << endl;
    cout << "Enter a command, type h for help. Type r to run. > ";
    cin >> cmd;
    
    if (cmd == "q") { // consider including something to relink the graph and reset breakpoint data to prevent memory leaks
      break;
    }
    else if (cmd == "h") {
      
      cout << "Available commands:"
      << endl << "r -> Run the program / continue from a breakpoint"
      << endl << "s -> Step to the next stmt by executing current stmt"
      << endl << "b n -> Breakpoint at line n" 
      << endl << "rb n -> Remove breakpoint at line n"
      << endl << "lb -> List all breakpoints"
      << endl << "cb -> Clear all breakpoints"
      << endl << "p varname -> Print variable"
      << endl << "sm -> Show memory contents"
      << endl << "ss -> Show state of debugger"
      << endl << "w -> What line are we on?"
      << endl << "q -> Quit the debugger"
      << endl;
    } else if (cmd == "r" or cmd == "s") {
      if (this->State == "Completed") {
        cout << "program has completed" << endl;
        continue;
      }

      //moved to execute
      // if (this->State == "Loaded") {
      //   this->State = "Running";
      //   cout << "state was changed to " << this->State << endl;
      // }

      execute_steps(cmd);



    // else if (cmd == "r") {
      
    //   //
    //   // run, or continue running, the program:
    //   //
    //   if (this->State == "Completed") {
    //     cout << "program has completed" << endl;
    //     continue; // skip the code below and repeat the loop for next cmd:
    //   }
      
    //   if (this->State == "Loaded")
    //     this->State = "Running";
      
    //   //
    //   // execute via nuPython interpreter:
    //   //
    //   struct ExecuteResult er = execute(curStmt, this->Memory);
      
    //   //
    //   // what happened during execution?
    //   //
    //   if (!er.Success) { // there was an error, we've complete execution:
    //     this->State = "Completed";
    //     continue;  // skip code below and continue loop with next cmd:
    //   }
      
    //   if (er.LastStmt != prevBreakpoint) { // then we ran to completion:
    //     this->State = "Completed";
    //     continue;  // skip code below and continue loop with next cmd:
    //   }
      
    //   //
    //   // if we get here, we've stopped at our breakpoint, so put the 
    //   // program graph back together and setup to continue execution
    //   // from breakpoint:
    //   //
    //   linkOrUnlinkStmts(prevBreakpoint, breakpoint);
     
    //   // start from breakpoint:
    //   curStmt = breakpoint;
      
    //   // and clear breakpoint because it was reached:
    //   prevBreakpoint = nullptr;
    //   breakpoint = nullptr;
    // }
    } else if (cmd == "b") {
      
      int lineNum;
      cin >> lineNum;
      
      //search the set to see if this breakpoint already exists
      if (Breakpoints.find(lineNum) != Breakpoints.end()) { // if it already exists in set
        cout << "breakpoint already set" << endl;
        continue;
      } 

      // if (breakpoint != nullptr) { // sorry, we already have a breakpoint set:
      //   cout << "breakpoint already set" << endl;
      //   continue;  // skip code below and continue with next cmd:
      // }
      
      //
      // Okay, so no breakpoint is set, so we have a valid program
      // graph that we can traverse.
      //
      // first, search the program for the breakpoint, along with
      // the stmt that precedes it:
      //
      prevBreakpoint = nullptr;
      breakpoint = this->Program; // start from first stmt so prev will get set:
      
      bool exists = findBreakpoint(prevBreakpoint, breakpoint, lineNum);
      
      if (!exists) {
        cout << "no such line" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      //
      // okay, the breakpoint line exists, so let's set the breakpoint
      // by severing the program graph between prevStmt and curStmt:
      //
      // linkOrUnlinkStmts(prevBreakpoint, nullptr);


      int i = 1;
      while (i < static_cast<int>(int_to_stmt.size())) {
        bool found = int_to_stmt.find(lineNum - i) != int_to_stmt.end();
        
        if (found) {
          struct STMT* prev = int_to_stmt.find(lineNum-i)->second;
          linkOrUnlinkStmts(prev, nullptr);
          Breakpoints.insert(lineNum); // add line number to breakpoints set if it checked the line exists check
          cout << "breakpoint set" << endl; //DEBUG CHECK
          break;
        }

      i++;
      }
    } 
    else if (cmd == "rb") {
      
      int lineNum;
      cin >> lineNum;
      
      if (breakpoint == nullptr) // we have no breakpoint
        cout << "no such breakpoint" << endl;
      else if (breakpoint->line != lineNum) // not the same breakpoint
        cout << "no such breakpoint" << endl;
      else {  // removing current breakpoint:
        //
        // we remove by linking the program graph back together:
        //
        linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
        prevBreakpoint = nullptr;
        breakpoint = nullptr;
     
        cout << "breakpoint removed" << endl;
      }
    }
    else if (cmd == "lb") {
      
      if (breakpoint == nullptr) {
        cout << "no breakpoints" << endl;
      } else {// we have a breakpoint:
        cout << "breakpoints on lines:"; 
        // for each breakpoint line number in set of breakpoints
        for (auto iter : Breakpoints) { // iter in this case is a line number 
          cout << " " + iter;
        }
        cout << endl;
      }
    }
    else if (cmd == "cb") {
      
      if (breakpoint != nullptr) { // we have a breakpoint to clear:
        //
        // we clear by linking the program graph back together:
        //
        linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
        //reset breakpoint data
        prevBreakpoint = nullptr;
        breakpoint = nullptr;
      }
      
      cout << "breakpoints cleared" << endl;
    }
    else if (cmd == "p") {
      
      string varname;
      cin >> varname;
      
      const char* name = varname.c_str();
      
      struct RAM_VALUE* value = ram_read_cell_by_name(this->Memory, (char*) name);
      
      if (value == nullptr) {
        cout << "no such variable" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      print_value(varname, value);
      ram_free_value(value);
    }
    else if (cmd == "sm") {
      
      ram_print(this->Memory);
    }
    else if (cmd == "ss") {
      
      cout << this->State << endl;
    }
    else if (cmd == "w") {
      
      if (this->State == "Loaded") {
        cout << "line_loaded " << this->Program->line << endl;
        // struct STMT* stop = get_next_statement(curStmt);
        // linkOrUnlinkStmts(curStmt, stop);
        programgraph_print(curStmt);
        
      } else if (this->State == "Completed") {
        cout << "completed execution" << endl;
      } else {// we are running:
        //struct STMT* nextStmt = get_next_statement(curStmt); // create a variable that stores the next stmt
        if (nextStmt != nullptr) {
          cout << "line " << curStmt->line << endl;
          //cout << "next" << nextStmt->line << endl; //DEBUG
          linkOrUnlinkStmts(curStmt, nullptr); // unlink
          //cout << (nullptr == get_next_statement(curStmt)) << endl; //DEBUG 0 means its not unlinked prop and 1 means that its unlinked
          programgraph_print(curStmt); // print one line
          linkOrUnlinkStmts(curStmt, nextStmt); // link again
        } else { // reached the end of the program 
          cout << "completed execution" << endl; 
        } 

      }



    // } else if (cmd == "s") { // this will execute a ingle statement at a time
    //   if (this->State == "Completed") {
    //     cout << "program has completed" << endl;
    //     return;
    //   } else if (this->State == "Loaded") {
    //     this->State = "Running"; // change state to running
    //     curStmt = this->Program; // set the current stmt equal to the beginning of the program
    //   } 
    //   // no else statement for if the state is Running is necessary becausr curStmt is already pointing to the curStmt 
    //   // and there is no need to reset it in the running state when we just did that if it was in the loading state
      
    //   execute_single_step(); // call on helper function
    } else {
      cout << "unknown command" << endl;
    }

  }//while
  
  //
  // at this point execution has completed (or the user quit
  // early). If there's still a breakpoint, we need to repair
  // the program graph so the memory is freed properly when
  // we return to main:
  //
  if (breakpoint != nullptr) { // there's still a breakpoint
    linkOrUnlinkStmts(prevBreakpoint, breakpoint);
    
    prevBreakpoint = nullptr; // not really necessary, but just to be safe
    breakpoint = nullptr;
  }
  
}//run










// having a problem running the last line of the program
// attempting to fix it, but this is the code from before attempting
// getting s s s w to work

/*debugger.cpp*/

//
// Debugger for nuPython, C++ edition! Provides a simple gdb-like
// interface, with support for one breakpoint in straight-line code.
// Uses nuPython interpreter as execution engine.
//
// Authors:
//    Ata Guvendi
//    Prof. Hummel
// 
// Northwestern University
// CS 211
//

#include <iostream>
#include <cassert>

#include "debugger.h"
#include "execute.h"

using namespace std;


//
// print_value
//
// Prints the contents of a RAM cell, both type and value.
//
void Debugger::print_value(string varname, struct RAM_VALUE* value)
{
  cout << varname << " ("; 
  
  switch (value->value_type) {
    
    case RAM_TYPE_INT:
      cout << "int): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_REAL:
      cout << "real): " << value->types.d << endl;
      break;
      
    case RAM_TYPE_STR:
      cout << "str): " << value->types.s << endl;
      break;
      
    case RAM_TYPE_PTR:
      cout << "ptr): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_BOOLEAN:
      cout << "bool): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_NONE:
      cout << "none): " << "None" << endl;
      break;
  }//switch
}


//
// findBreakpoint
//
// starts from the stmt denoted by breakpoint, and searches forward
// for a stmt with the given line number. As it advances, prev is
// updated to always denote the previous statement. The parameters
// are passed-by-reference so they are updated back in the caller.
//
// Note that if breakpoint happens to match immediately, without advancing,
// then prev does not get set. So the caller should probably set prev 
// appropriately before calling (either nullptr, or to the previous stmt).
//
bool Debugger::findBreakpoint(struct STMT*& prev, struct STMT*& breakpoint, int lineNum)
{
  while (breakpoint != nullptr) {
    
    // cout << breakpoint->line << endl;
    
    if (breakpoint->line == lineNum) // found it!
      return true;
      
    prev = breakpoint;
    
    if (breakpoint->stmt_type == STMT_ASSIGNMENT)
      breakpoint = breakpoint->types.assignment->next_stmt;
    else if (breakpoint->stmt_type == STMT_FUNCTION_CALL)
      breakpoint = breakpoint->types.function_call->next_stmt;
    else {
      assert(breakpoint->stmt_type == STMT_PASS);
      breakpoint = breakpoint->types.pass->next_stmt;
    }
  }//while
  
  // if get here, not found
  return false;
}


//
// linkOrUnlinkStmts
//
// updates prev stmt to point to whatever cur is --- this can be 
// nullptr to unlink, or an actual stmt to link them back together.
//
void Debugger::linkOrUnlinkStmts(struct STMT* prev, struct STMT* cur)
{
  if (prev == nullptr) // nothing to link or unlink:
    return;
    
  if (prev->stmt_type == STMT_ASSIGNMENT)
    prev->types.assignment->next_stmt = cur;
  else if (prev->stmt_type == STMT_FUNCTION_CALL)
    prev->types.function_call->next_stmt = cur;
  else {
    assert(prev->stmt_type == STMT_PASS);
    prev->types.pass->next_stmt = cur;
  }
}



/* get_next_statement
when getting the next statement it checks what type it is as every type has a differnet way of getting next statement
Moves to the next statement by checking the type of the current statement (curr).

parameter: stmt
returns: struct
*/
struct STMT* Debugger::get_next_statement(struct STMT* stmt) {
  if (stmt->stmt_type == STMT_ASSIGNMENT) {
      return stmt->types.assignment->next_stmt;
  } else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
      return stmt->types.function_call->next_stmt;
  } else if (stmt->stmt_type == STMT_PASS) {
      return stmt->types.pass->next_stmt;
  } else {
      return nullptr;
  }
} 

/* find_prevStmt
finds the previous statement from a current statement or "target"

parameters: stmt
returns: stmt
*/
struct STMT* Debugger::find_prevStmt(struct STMT* target) {
  struct STMT* prev = nullptr;
  struct STMT* current = this->Program;

  while (current != nullptr && current != target) {
    prev = current;
    current = get_next_statement(current);  // Use get_next_statement to move to the next
  }

  return prev;  // Returns nullptr if target is the first statement
};

/* execute_steps
checks what to do based on if the command is s or r

parameters: string
returns: bool
*/
bool Debugger::execute_steps(string cmd){
  if (nextStmt == nullptr) {
    if (this->State == "Loaded") { // Beginning of program
      nextStmt = get_next_statement(curStmt); // Initialize first step
      this->State = "Running";
      cout << "Program started, state changed to " << this->State << endl;
    }
  }
  
  
  if (cmd == "s") {
    cout << "entered an s command. calling execute_single_step()"<< endl;
    this->State = "Running";
    return execute_single_step(cmd); // Return true if the program completes, otherwise false
    //return false; // exit after a single step if the command is "s"
  }
  
  // for the r command, continue to run until completion or breakpoint is hit
  while (!execute_single_step(cmd)) {
    // the loop will continue until execute_single_step() returns true indicating the program has completed
  } 

  if(nextStmt == nullptr) {
    if (this->State == "Running") { // End of program
      this->State = "Completed";
      cout << "Program completed, state changed to " << this->State << endl;
      return true; // End program
    }
  }
  
  return true; // default return if command is r and the loop should continue
}

/* execute_single_step
executes single step. this is used for our s and r command

parameters: string
returns: bool
*/
bool Debugger::execute_single_step(const string& cmd) {
  // 1. breakpoint check: if the curStmt is a breakpoint then outputs false and stops
  if (Breakpoints.find(curStmt->line) != Breakpoints.end()) {
    cout << "hit breakpoint on line " << curStmt->line << endl;

    if (cmd == "r") {
      nextStmt = get_next_statement(curStmt); // Set up for next step
      cout << "true was returned and execute single step is done because in execute_steps it will run until it single is true" << endl;
      cout << "for if we click r AGAIN: cur stmt = " << curStmt->line<< endl;
      cout << "for if we click r AGAIN: next stmt = " << nextStmt->line<< endl;
      curStmt = nextStmt;
      nextStmt = get_next_statement(curStmt);
      cout << "for if we click r AGAIN: new cur stmt = " << curStmt->line<< endl;
      cout << "for if we click r AGAIN: new next stmt = " << nextStmt->line<< endl;
      return true; // Indicate that we hit a breakpoint and execution should pause
    }

    return false;
  } 

  // 2. set a temp breakpoint for s command
  nextStmt = get_next_statement(curStmt); 
  linkOrUnlinkStmts(curStmt, nullptr); //unlink curStmt from breakStmt temporarily

  // 3. Execution check: if the execution of the curStmt fails, repair the graph and set state to completed and return true
  struct ExecuteResult er = execute(curStmt, this->Memory);
  cout << "execution is run... " << endl; // debug
  
  //step 3.7) if execute failed, signal completion
  //link does not be restored because it was already restored above
  if (!er.Success) { // execution failed aka semantic error, repair the graph
    cout << "execution failed!!!!!!!!!!" << endl; // debug
    this->State = "Completed";
    return true;
  }

  // step 3.5) relink the curstmt from nextstmt then advance
  cout << "curStmt: " << curStmt->line<< endl;
  if (nextStmt == nullptr) { // debug
  cout << "nextStmt is NULLPTR" << endl;
  }
  
  linkOrUnlinkStmts(curStmt, nextStmt); // restore link between curStmt and breakstmt 
  cout << "link restored" << endl; // debug
  // attempt to advance to the next Statement
  if (nextStmt != nullptr) {
    cout << "nextStmt is not a nullptr so enter..." << endl; // debug
    curStmt = nextStmt;
    cout << "curStmt changed to -> " << curStmt->line << endl; // debug
    nextStmt = get_next_statement(curStmt);
    if (nextStmt == nullptr) {
      cout << "nextStmt is A nullptr" << endl; // debug
    } else {
      cout << "nextStmt changed to -> " << nextStmt->line << endl; // debug
    }

  } else { // nextStmt = nullptr
  // If nextStmt is nullptr, check if curStmt is a breakpoint
    // // debug check to see what the breakpoints set looks like
    // for (auto iter : Breakpoints) {
    //   cout << iter << " " << endl;
    // }
    // If nextStmt is nullptr, find the next executable line using int_to_stmt
    auto nextLineIt = int_to_stmt.upper_bound(curStmt->line); // Get next line in map
    
    if (nextLineIt != int_to_stmt.end()) {
      cout << "yay! the line that we broke at exists in the map of program lines" << endl; // debug
      curStmt = nextLineIt->second; // Advance to the next executable statement
      cout << "curStmt is set to " << curStmt->line<< "for look up" << endl; // debug
      nextStmt = get_next_statement(curStmt);
      if (nextStmt == nullptr) {
        cout << "nextStmt is A nullptr" << endl; // debug
      } else {
        cout << "nextStmt changed to -> " << nextStmt->line << endl; // debug
      }
      
      // Check if this new line is a breakpoint
      if (Breakpoints.find(curStmt->line) != Breakpoints.end()) {
        cout << "false is returned and breakpoint found at line " << curStmt->line << endl;
        return false; // Stop at the breakpoint
      } else {
        cout << "state changed to completed because nextStmt is nullptr!!!!!!!!!!" << endl;
        this->State = "Completed";
        return true; // Program end reached
      }
    }
  }
  cout << "false is returned for execute_single_step" << endl;
  return false; // continue if there are more statements
}


//
// constructor:
//
Debugger::Debugger(struct STMT* program)
  : State("Loaded"), Program(program), Memory(nullptr), Breakpoints(), curStmt(this->Program), nextStmt(nullptr)
{
  this->Memory = ram_init();

  // fill the map with every line in the program
  // map<int, struct STMT&> int_to_stmt
  struct STMT* current = this->Program;
  while (current != nullptr) {
    int_to_stmt[current->line] = current;
    current= get_next_statement(current);
  }

  // debug to check if the map was populated correctly
  for (auto pair : int_to_stmt) {
    cout << pair.first<< " : " << pair.second->line<< endl;
  }
}

//
// destructor:
//
Debugger::~Debugger()
{
  ram_destroy(this->Memory);
}


//
// run:
//
// Run the debugger for one execution run of the input program.
//
void Debugger::run()
{
  string cmd;
  
  // //
  // // controls where we start execution from:
  // //
  // struct STMT* curStmt = this->Program;
  
  //
  // if we have a breakpoint, these will be set to the breakpoint,
  // and the statement preceding it:
  //
  struct STMT* prevBreakpoint = nullptr;
  struct STMT* breakpoint = nullptr;
  
  //
  // command loop until quit is entered:
  //
  while (true) {
    cout << endl;
    cout << "Enter a command, type h for help. Type r to run. > ";
    cin >> cmd;
    
    if (cmd == "q") { // consider including something to relink the graph and reset breakpoint data to prevent memory leaks
      break;
    }
    else if (cmd == "h") {
      
      cout << "Available commands:"
      << endl << "r -> Run the program / continue from a breakpoint"
      << endl << "s -> Step to the next stmt by executing current stmt"
      << endl << "b n -> Breakpoint at line n" 
      << endl << "rb n -> Remove breakpoint at line n"
      << endl << "lb -> List all breakpoints"
      << endl << "cb -> Clear all breakpoints"
      << endl << "p varname -> Print variable"
      << endl << "sm -> Show memory contents"
      << endl << "ss -> Show state of debugger"
      << endl << "w -> What line are we on?"
      << endl << "q -> Quit the debugger"
      << endl;
    } else if (cmd == "r" or cmd == "s") {
      if (this->State == "Completed") {
        cout << "program has completed" << endl;
        continue;
      } 

      execute_steps(cmd);



    } else if (cmd == "b") {
      
      int lineNum;
      cin >> lineNum;
      
      //search the set to see if this breakpoint already exists
      if (Breakpoints.find(lineNum) != Breakpoints.end()) { // if it already exists in set
        cout << "breakpoint already set" << endl;
        continue;
      } 

      // if (breakpoint != nullptr) { // sorry, we already have a breakpoint set:
      //   cout << "breakpoint already set" << endl;
      //   continue;  // skip code below and continue with next cmd:
      // }
      
      //
      // Okay, so no breakpoint is set, so we have a valid program
      // graph that we can traverse.
      //
      // first, search the program for the breakpoint, along with
      // the stmt that precedes it:
      //
      prevBreakpoint = nullptr;
      breakpoint = this->Program; // start from first stmt so prev will get set:
      
      bool exists = findBreakpoint(prevBreakpoint, breakpoint, lineNum);
      
      if (!exists) {
        cout << "no such line" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      //
      // okay, the breakpoint line exists, so let's set the breakpoint
      // by severing the program graph between prevStmt and curStmt:
      //
      linkOrUnlinkStmts(prevBreakpoint, nullptr);
      Breakpoints.insert(lineNum); // add line number to breakpoints set if it checked the line exists check
      cout << "breakpoint set" << endl; //DEBUG CHECK
    }
    else if (cmd == "rb") {
      
      int lineNum;
      cin >> lineNum;
      
      if (breakpoint == nullptr) // we have no breakpoint
        cout << "no such breakpoint" << endl;
      else if (breakpoint->line != lineNum) // not the same breakpoint
        cout << "no such breakpoint" << endl;
      else {  // removing current breakpoint:
        //
        // we remove by linking the program graph back together:
        //
        linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
        prevBreakpoint = nullptr;
        breakpoint = nullptr;
     
        cout << "breakpoint removed" << endl;
      }
    }
    else if (cmd == "lb") {
      
      if (breakpoint == nullptr) {
        cout << "no breakpoints" << endl;
      } else {// we have a breakpoint:
        cout << "breakpoints on lines:"; 
        // for each breakpoint line number in set of breakpoints
        for (auto iter : Breakpoints) { // iter in this case is a line number 
          cout << " " + iter;
        }
        cout << endl;
      }
    }
    else if (cmd == "cb") {
      
      if (breakpoint != nullptr) { // we have a breakpoint to clear:
        //
        // we clear by linking the program graph back together:
        //
        linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
        //reset breakpoint data
        prevBreakpoint = nullptr;
        breakpoint = nullptr;
      }
      
      cout << "breakpoints cleared" << endl;
    }
    else if (cmd == "p") {
      
      string varname;
      cin >> varname;
      
      const char* name = varname.c_str();
      
      struct RAM_VALUE* value = ram_read_cell_by_name(this->Memory, (char*) name);
      
      if (value == nullptr) {
        cout << "no such variable" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      print_value(varname, value);
      ram_free_value(value);
    }
    else if (cmd == "sm") {
      
      ram_print(this->Memory);
    }
    else if (cmd == "ss") {
      
      cout << this->State << endl;
    }
    else if (cmd == "w") {
      
      if (this->State == "Loaded") {
        cout << "line_loaded " << this->Program->line << endl;
        // struct STMT* stop = get_next_statement(curStmt);
        // linkOrUnlinkStmts(curStmt, stop);
        programgraph_print(curStmt);
        
      } else if (this->State == "Completed") {
        cout << "completed execution" << endl;
      } else {// we are running:
        //struct STMT* nextStmt = get_next_statement(curStmt); // create a variable that stores the next stmt
        if (nextStmt != nullptr) {
          cout << "line " << curStmt->line << endl;
          //cout << "next" << nextStmt->line << endl; //DEBUG
          linkOrUnlinkStmts(curStmt, nullptr); // unlink
          //cout << (nullptr == get_next_statement(curStmt)) << endl; //DEBUG 0 means its not unlinked prop and 1 means that its unlinked
          programgraph_print(curStmt); // print one line
          linkOrUnlinkStmts(curStmt, nextStmt); // link again
        } else { // reached the end of the program 
          cout << "completed execution" << endl; 
        } 

      }



    // } else if (cmd == "s") { // this will execute a ingle statement at a time
    //   if (this->State == "Completed") {
    //     cout << "program has completed" << endl;
    //     return;
    //   } else if (this->State == "Loaded") {
    //     this->State = "Running"; // change state to running
    //     curStmt = this->Program; // set the current stmt equal to the beginning of the program
    //   } 
    //   // no else statement for if the state is Running is necessary becausr curStmt is already pointing to the curStmt 
    //   // and there is no need to reset it in the running state when we just did that if it was in the loading state
      
    //   execute_single_step(); // call on helper function
    } else {
      cout << "unknown command" << endl;
    }

  }//while
  
  //
  // at this point execution has completed (or the user quit
  // early). If there's still a breakpoint, we need to repair
  // the program graph so the memory is freed properly when
  // we return to main:
  //
  if (breakpoint != nullptr) { // there's still a breakpoint
    linkOrUnlinkStmts(prevBreakpoint, breakpoint);
    
    prevBreakpoint = nullptr; // not really necessary, but just to be safe
    breakpoint = nullptr;
  }
  
}//run



// code before debugs are commented out
// getting s s s w to work

/*debugger.cpp*/

//
// Debugger for nuPython, C++ edition! Provides a simple gdb-like
// interface, with support for one breakpoint in straight-line code.
// Uses nuPython interpreter as execution engine.
//
// Authors:
//    Ata Guvendi
//    Prof. Hummel
// 
// Northwestern University
// CS 211
//

#include <iostream>
#include <cassert>

#include "debugger.h"
#include "execute.h"

using namespace std;


//
// print_value
//
// Prints the contents of a RAM cell, both type and value.
//
void Debugger::print_value(string varname, struct RAM_VALUE* value)
{
  cout << varname << " ("; 
  
  switch (value->value_type) {
    
    case RAM_TYPE_INT:
      cout << "int): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_REAL:
      cout << "real): " << value->types.d << endl;
      break;
      
    case RAM_TYPE_STR:
      cout << "str): " << value->types.s << endl;
      break;
      
    case RAM_TYPE_PTR:
      cout << "ptr): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_BOOLEAN:
      cout << "bool): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_NONE:
      cout << "none): " << "None" << endl;
      break;
  }//switch
}


//
// findBreakpoint
//
// starts from the stmt denoted by breakpoint, and searches forward
// for a stmt with the given line number. As it advances, prev is
// updated to always denote the previous statement. The parameters
// are passed-by-reference so they are updated back in the caller.
//
// Note that if breakpoint happens to match immediately, without advancing,
// then prev does not get set. So the caller should probably set prev 
// appropriately before calling (either nullptr, or to the previous stmt).
//
bool Debugger::findBreakpoint(struct STMT*& prev, struct STMT*& breakpoint, int lineNum)
{
  while (breakpoint != nullptr) {
    
    // cout << breakpoint->line << endl;
    
    if (breakpoint->line == lineNum) // found it!
      return true;
      
    prev = breakpoint;
    
    if (breakpoint->stmt_type == STMT_ASSIGNMENT)
      breakpoint = breakpoint->types.assignment->next_stmt;
    else if (breakpoint->stmt_type == STMT_FUNCTION_CALL)
      breakpoint = breakpoint->types.function_call->next_stmt;
    else {
      assert(breakpoint->stmt_type == STMT_PASS);
      breakpoint = breakpoint->types.pass->next_stmt;
    }
  }//while
  
  // if get here, not found
  return false;
}


//
// linkOrUnlinkStmts
//
// updates prev stmt to point to whatever cur is --- this can be 
// nullptr to unlink, or an actual stmt to link them back together.
//
void Debugger::linkOrUnlinkStmts(struct STMT* prev, struct STMT* cur)
{
  if (prev == nullptr) // nothing to link or unlink:
    return;
    
  if (prev->stmt_type == STMT_ASSIGNMENT)
    prev->types.assignment->next_stmt = cur;
  else if (prev->stmt_type == STMT_FUNCTION_CALL)
    prev->types.function_call->next_stmt = cur;
  else {
    assert(prev->stmt_type == STMT_PASS);
    prev->types.pass->next_stmt = cur;
  }
}



/* get_next_statement
when getting the next statement it checks what type it is as every type has a differnet way of getting next statement
Moves to the next statement by checking the type of the current statement (curr).

parameter: stmt
returns: struct
*/
struct STMT* Debugger::get_next_statement(struct STMT* stmt) {
  if (stmt->stmt_type == STMT_ASSIGNMENT) {
      return stmt->types.assignment->next_stmt;
  } else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
      return stmt->types.function_call->next_stmt;
  } else if (stmt->stmt_type == STMT_PASS) {
      return stmt->types.pass->next_stmt;
  } else {
      return nullptr;
  }
} 

/* find_prevStmt
finds the previous statement from a current statement or "target"

parameters: stmt
returns: stmt
*/
struct STMT* Debugger::find_prevStmt(struct STMT* target) {
  struct STMT* prev = nullptr;
  struct STMT* current = this->Program;

  while (current != nullptr && current != target) {
    prev = current;
    current = get_next_statement(current);  // Use get_next_statement to move to the next
  }

  return prev;  // Returns nullptr if target is the first statement
};

/* execute_steps
checks what to do based on if the command is s or r

parameters: string
returns: bool
*/
bool Debugger::execute_steps(string cmd){
  // initial check for beginning of the program
  if (nextStmt == nullptr) {
    if (this->State == "Loaded") { // Beginning of program
      nextStmt = get_next_statement(curStmt); // Initialize first step
      this->State = "Running";
      cout << "Program started, state changed to " << this->State << endl;
    }
  }
  
  
  if (cmd == "s") {
    cout << "entered an s command. calling execute_single_step()"<< endl;
    this->State = "Running";
    return execute_single_step(cmd); // Return true if the program completes, otherwise false
    //return false; // exit after a single step if the command is "s"
  
    bool completed = execute_single_step(cmd);

    if (nextStmt == nullptr && completed) {
      this->State = "Completed";
      cout << "Program completed, state changed to " << this->State << endl;
      return true;
    }

    return completed; // return true if the program has completed
  }
  
  // for the r command, continue to run until completion or breakpoint is hit
  while (!execute_single_step(cmd)) {
    // the loop will continue until execute_single_step() returns true indicating the program has completed
  } 

  if(nextStmt == nullptr) {
    if (this->State == "Running") { // End of program
      this->State = "Completed";
      cout << "Program completed, state changed to " << this->State << endl;
      return true; // End program
    }
  }
  
  return true; // default return if command is r and the loop should continue
}

/* execute_single_step
executes single step. this is used for our s and r command

parameters: string
returns: bool
*/
bool Debugger::execute_single_step(const string& cmd) {
  // 1. breakpoint check: if the curStmt is a breakpoint then outputs false and stops
  if (Breakpoints.find(curStmt->line) != Breakpoints.end()) {
    cout << "hit breakpoint on line " << curStmt->line << endl;

    if (cmd == "r") {
      nextStmt = get_next_statement(curStmt); // Set up for next step
      cout << "true was returned and execute single step is done because in execute_steps it will run until it single is true" << endl;
      cout << "for if we click r AGAIN: cur stmt = " << curStmt->line<< endl;
      cout << "for if we click r AGAIN: next stmt = " << nextStmt->line<< endl;
      curStmt = nextStmt;
      nextStmt = get_next_statement(curStmt);
      cout << "for if we click r AGAIN: new cur stmt = " << curStmt->line<< endl;
      cout << "for if we click r AGAIN: new next stmt = " << nextStmt->line<< endl;
      return true; // Indicate that we hit a breakpoint and execution should pause
    }

    return false;
  } 

  // 2. set a temp breakpoint for s command
  nextStmt = get_next_statement(curStmt); 
  //only unlink if nextstmt is not nullptr and we are already running (not at the start)
  if (nextStmt != nullptr) {
    linkOrUnlinkStmts(curStmt, nullptr); //unlink curStmt from breakStmt temporarily
    cout << "Temporarily unlinked curStmt from nextStmt" << endl;
  }

  // 3. Execution check: if the execution of the curStmt fails, repair the graph and set state to completed and return true
  struct ExecuteResult er = execute(curStmt, this->Memory);
  cout << "execution is run... " << endl; // debug
  
  //step 3.7) if execute failed, signal completion
  //link does not be restored because it was already restored above
  if (!er.Success) { // execution failed aka semantic error, repair the graph
    cout << "execution failed!!!!!!!!!!" << endl; // debug
    this->State = "Completed";
    return true;
  }

  // step 3.5) relink the curstmt from nextstmt then advance
  cout << "curStmt: " << curStmt->line<< endl;
  if (nextStmt == nullptr) { // debug
  cout << "nextStmt is NULLPTR" << endl;
  }
  
  linkOrUnlinkStmts(curStmt, nextStmt); // restore link between curStmt and breakstmt 
  cout << "link restored" << endl; // debug
  // attempt to advance to the next Statement
  if (nextStmt != nullptr) {
    cout << "nextStmt is not a nullptr so enter..." << endl; // debug
    curStmt = nextStmt;
    cout << "curStmt changed to -> " << curStmt->line << endl; // debug
    nextStmt = get_next_statement(curStmt);
    if (nextStmt == nullptr) {
      cout << "nextStmt is A nullptr" << endl; // debug
    } else {
      cout << "nextStmt changed to -> " << nextStmt->line << endl; // debug
    }

  } else { // nextStmt = nullptr
  // If nextStmt is nullptr, check if curStmt is a breakpoint
    // // debug check to see what the breakpoints set looks like
    // for (auto iter : Breakpoints) {
    //   cout << iter << " " << endl;
    // }
    // If nextStmt is nullptr, find the next executable line using int_to_stmt
    auto nextLineIt = int_to_stmt.upper_bound(curStmt->line); // Get next line in map
    
    if (nextLineIt != int_to_stmt.end()) {
      cout << "yay! the line that we broke at exists in the map of program lines" << endl; // debug
      curStmt = nextLineIt->second; // Advance to the next executable statement
      cout << "curStmt is set to " << curStmt->line<< "for look up" << endl; // debug
      nextStmt = get_next_statement(curStmt);
      if (nextStmt == nullptr) {
        cout << "nextStmt is A nullptr" << endl; // debug
      } else {
        cout << "nextStmt changed to -> " << nextStmt->line << endl; // debug
      }
      
      // Check if this new line is a breakpoint
      if (Breakpoints.find(curStmt->line) != Breakpoints.end()) {
        cout << "false is returned and breakpoint found at line " << curStmt->line << endl;
        return false; // Stop at the breakpoint
      } else {
        cout << "state changed to completed because nextStmt is nullptr!!!!!!!!!!" << endl;
        this->State = "Completed";
        return true; // Program end reached
      }
    }

    if (nextLineIt == int_to_stmt.end()) {
      cout << "No further lines in int_to_stmt; marking program as complete." << endl;
      curStmt = nullptr; // Signal the true end of the program
      return true;
    }
  }
  cout << "false is returned for execute_single_step" << endl;
  return false; // continue if there are more statements
}


//
// constructor:
//
Debugger::Debugger(struct STMT* program)
  : State("Loaded"), Program(program), Memory(nullptr), Breakpoints(), curStmt(this->Program), nextStmt(nullptr)
{
  this->Memory = ram_init();

  // fill the map with every line in the program
  // map<int, struct STMT&> int_to_stmt
  struct STMT* current = this->Program;
  while (current != nullptr) {
    int_to_stmt[current->line] = current;
    current= get_next_statement(current);
  }

  // debug to check if the map was populated correctly
  for (auto pair : int_to_stmt) {
    cout << pair.first<< " : " << pair.second->line<< endl;
  }
}

//
// destructor:
//
Debugger::~Debugger()
{
  ram_destroy(this->Memory);
}


//
// run:
//
// Run the debugger for one execution run of the input program.
//
void Debugger::run()
{
  string cmd;
  
  // //
  // // controls where we start execution from:
  // //
  // struct STMT* curStmt = this->Program;
  
  //
  // if we have a breakpoint, these will be set to the breakpoint,
  // and the statement preceding it:
  //
  struct STMT* prevBreakpoint = nullptr;
  struct STMT* breakpoint = nullptr;
  
  //
  // command loop until quit is entered:
  //
  while (true) {
    cout << endl;
    cout << "Enter a command, type h for help. Type r to run. > ";
    cin >> cmd;
    
    if (cmd == "q") { // consider including something to relink the graph and reset breakpoint data to prevent memory leaks
      break;
    }
    else if (cmd == "h") {
      
      cout << "Available commands:"
      << endl << "r -> Run the program / continue from a breakpoint"
      << endl << "s -> Step to the next stmt by executing current stmt"
      << endl << "b n -> Breakpoint at line n" 
      << endl << "rb n -> Remove breakpoint at line n"
      << endl << "lb -> List all breakpoints"
      << endl << "cb -> Clear all breakpoints"
      << endl << "p varname -> Print variable"
      << endl << "sm -> Show memory contents"
      << endl << "ss -> Show state of debugger"
      << endl << "w -> What line are we on?"
      << endl << "q -> Quit the debugger"
      << endl;
    } else if (cmd == "r" or cmd == "s") {
      if (this->State == "Completed") {
        cout << "program has completed" << endl;
        continue;
      } 

      execute_steps(cmd);



    } else if (cmd == "b") {
      
      int lineNum;
      cin >> lineNum;
      
      //search the set to see if this breakpoint already exists
      if (Breakpoints.find(lineNum) != Breakpoints.end()) { // if it already exists in set
        cout << "breakpoint already set" << endl;
        continue;
      } 

      // if (breakpoint != nullptr) { // sorry, we already have a breakpoint set:
      //   cout << "breakpoint already set" << endl;
      //   continue;  // skip code below and continue with next cmd:
      // }
      
      //
      // Okay, so no breakpoint is set, so we have a valid program
      // graph that we can traverse.
      //
      // first, search the program for the breakpoint, along with
      // the stmt that precedes it:
      //
      prevBreakpoint = nullptr;
      breakpoint = this->Program; // start from first stmt so prev will get set:
      
      bool exists = findBreakpoint(prevBreakpoint, breakpoint, lineNum);
      
      if (!exists) {
        cout << "no such line" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      //
      // okay, the breakpoint line exists, so let's set the breakpoint
      // by severing the program graph between prevStmt and curStmt:
      //
      linkOrUnlinkStmts(prevBreakpoint, nullptr);
      Breakpoints.insert(lineNum); // add line number to breakpoints set if it checked the line exists check
      cout << "breakpoint set" << endl;
    }
    else if (cmd == "rb") {
      
      int lineNum;
      cin >> lineNum;
      
      if (breakpoint == nullptr) // we have no breakpoint
        cout << "no such breakpoint" << endl;
      else if (breakpoint->line != lineNum) // not the same breakpoint
        cout << "no such breakpoint" << endl;
      else {  // removing current breakpoint:
        //
        // we remove by linking the program graph back together:
        //
        linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
        prevBreakpoint = nullptr;
        breakpoint = nullptr;
     
        cout << "breakpoint removed" << endl;
      }
    }
    else if (cmd == "lb") {
      
      if (breakpoint == nullptr) {
        cout << "no breakpoints" << endl;
      } else {// we have a breakpoint:
        cout << "breakpoints on lines:"; 
        // for each breakpoint line number in set of breakpoints
        for (auto iter : Breakpoints) { // iter in this case is a line number 
          cout << " " + iter;
        }
        cout << endl;
      }
    }
    else if (cmd == "cb") {
      
      if (breakpoint != nullptr) { // we have a breakpoint to clear:
        //
        // we clear by linking the program graph back together:
        //
        linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
        //reset breakpoint data
        prevBreakpoint = nullptr;
        breakpoint = nullptr;
      }
      
      cout << "breakpoints cleared" << endl;
    }
    else if (cmd == "p") {
      
      string varname;
      cin >> varname;
      
      const char* name = varname.c_str();
      
      struct RAM_VALUE* value = ram_read_cell_by_name(this->Memory, (char*) name);
      
      if (value == nullptr) {
        cout << "no such variable" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      print_value(varname, value);
      ram_free_value(value);
    }
    else if (cmd == "sm") {
      
      ram_print(this->Memory);
    }
    else if (cmd == "ss") {
      
      cout << this->State << endl;
    }
    else if (cmd == "w") {
      
      if (this->State == "Loaded") {
        cout << "line_loaded " << this->Program->line << endl;
        // struct STMT* stop = get_next_statement(curStmt);
        // linkOrUnlinkStmts(curStmt, stop);
        programgraph_print(curStmt);
        
      } else if (this->State == "Completed") {
        cout << "completed execution" << endl;
      } else {// we are running:
        //struct STMT* nextStmt = get_next_statement(curStmt); // create a variable that stores the next stmt
        if (nextStmt != nullptr) {
          cout << "line " << curStmt->line << endl;
          //cout << "next" << nextStmt->line << endl; //DEBUG
          linkOrUnlinkStmts(curStmt, nullptr); // unlink
          //cout << (nullptr == get_next_statement(curStmt)) << endl; //DEBUG 0 means its not unlinked prop and 1 means that its unlinked
          programgraph_print(curStmt); // print one line
          linkOrUnlinkStmts(curStmt, nextStmt); // link again
        } else { // reached the end of the program 
          cout << "completed execution" << endl; 
        } 

      }



    // } else if (cmd == "s") { // this will execute a ingle statement at a time
    //   if (this->State == "Completed") {
    //     cout << "program has completed" << endl;
    //     return;
    //   } else if (this->State == "Loaded") {
    //     this->State = "Running"; // change state to running
    //     curStmt = this->Program; // set the current stmt equal to the beginning of the program
    //   } 
    //   // no else statement for if the state is Running is necessary becausr curStmt is already pointing to the curStmt 
    //   // and there is no need to reset it in the running state when we just did that if it was in the loading state
      
    //   execute_single_step(); // call on helper function
    } else {
      cout << "unknown command" << endl;
    }

  }//while
  
  //
  // at this point execution has completed (or the user quit
  // early). If there's still a breakpoint, we need to repair
  // the program graph so the memory is freed properly when
  // we return to main:
  //
  if (breakpoint != nullptr) { // there's still a breakpoint
    linkOrUnlinkStmts(prevBreakpoint, breakpoint);
    
    prevBreakpoint = nullptr; // not really necessary, but just to be safe
    breakpoint = nullptr;
  }
  
}//run







// This is code with debugs commented out
// right now the program runs until the last line, but when I attempt to run again it should tell me that it has completed but instead it seg faults
// also when I click w after line 15 has executed it thinks the program has completed
// getting s s s w to work

/*debugger.cpp*/

//
// Debugger for nuPython, C++ edition! Provides a simple gdb-like
// interface, with support for one breakpoint in straight-line code.
// Uses nuPython interpreter as execution engine.
//
// Authors:
//    Ata Guvendi
//    Prof. Hummel
// 
// Northwestern University
// CS 211
//

#include <iostream>
#include <cassert>

#include "debugger.h"
#include "execute.h"

using namespace std;


//
// print_value
//
// Prints the contents of a RAM cell, both type and value.
//
void Debugger::print_value(string varname, struct RAM_VALUE* value)
{
  cout << varname << " ("; 
  
  switch (value->value_type) {
    
    case RAM_TYPE_INT:
      cout << "int): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_REAL:
      cout << "real): " << value->types.d << endl;
      break;
      
    case RAM_TYPE_STR:
      cout << "str): " << value->types.s << endl;
      break;
      
    case RAM_TYPE_PTR:
      cout << "ptr): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_BOOLEAN:
      cout << "bool): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_NONE:
      cout << "none): " << "None" << endl;
      break;
  }//switch
}


//
// findBreakpoint
//
// starts from the stmt denoted by breakpoint, and searches forward
// for a stmt with the given line number. As it advances, prev is
// updated to always denote the previous statement. The parameters
// are passed-by-reference so they are updated back in the caller.
//
// Note that if breakpoint happens to match immediately, without advancing,
// then prev does not get set. So the caller should probably set prev 
// appropriately before calling (either nullptr, or to the previous stmt).
//
bool Debugger::findBreakpoint(struct STMT*& prev, struct STMT*& breakpoint, int lineNum)
{
  while (breakpoint != nullptr) {
    
    // cout << breakpoint->line << endl;
    
    if (breakpoint->line == lineNum) // found it!
      return true;
      
    prev = breakpoint;
    
    if (breakpoint->stmt_type == STMT_ASSIGNMENT)
      breakpoint = breakpoint->types.assignment->next_stmt;
    else if (breakpoint->stmt_type == STMT_FUNCTION_CALL)
      breakpoint = breakpoint->types.function_call->next_stmt;
    else {
      assert(breakpoint->stmt_type == STMT_PASS);
      breakpoint = breakpoint->types.pass->next_stmt;
    }
  }//while
  
  // if get here, not found
  return false;
}


//
// linkOrUnlinkStmts
//
// updates prev stmt to point to whatever cur is --- this can be 
// nullptr to unlink, or an actual stmt to link them back together.
//
void Debugger::linkOrUnlinkStmts(struct STMT* prev, struct STMT* cur)
{
  if (prev == nullptr) // nothing to link or unlink:
    return;
    
  if (prev->stmt_type == STMT_ASSIGNMENT)
    prev->types.assignment->next_stmt = cur;
  else if (prev->stmt_type == STMT_FUNCTION_CALL)
    prev->types.function_call->next_stmt = cur;
  else {
    assert(prev->stmt_type == STMT_PASS);
    prev->types.pass->next_stmt = cur;
  }
}



/* get_next_statement
when getting the next statement it checks what type it is as every type has a differnet way of getting next statement
Moves to the next statement by checking the type of the current statement (curr).

parameter: stmt
returns: struct
*/
struct STMT* Debugger::get_next_statement(struct STMT* stmt) {
  if (stmt->stmt_type == STMT_ASSIGNMENT) {
      return stmt->types.assignment->next_stmt;
  } else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
      return stmt->types.function_call->next_stmt;
  } else if (stmt->stmt_type == STMT_PASS) {
      return stmt->types.pass->next_stmt;
  } else {
      return nullptr;
  }
} 

/* find_prevStmt
finds the previous statement from a current statement or "target"

parameters: stmt
returns: stmt
*/
struct STMT* Debugger::find_prevStmt(struct STMT* target) {
  struct STMT* prev = nullptr;
  struct STMT* current = this->Program;

  while (current != nullptr && current != target) {
    prev = current;
    current = get_next_statement(current);  // Use get_next_statement to move to the next
  }

  return prev;  // Returns nullptr if target is the first statement
};

/* execute_steps
checks what to do based on if the command is s or r

parameters: string
returns: bool
*/
bool Debugger::execute_steps(string cmd){
  // initial check for beginning of the program
  if (nextStmt == nullptr) {
    if (this->State == "Loaded") { // Beginning of program
      nextStmt = get_next_statement(curStmt); // Initialize first step
      this->State = "Running";
      // cout << "Program started, state changed to " << this->State << endl; // debug
    }
  }
  
  
  if (cmd == "s") {
    // cout << "entered an s command. calling execute_single_step()"<< endl; // debug
    this->State = "Running";
    return execute_single_step(cmd); // Return true if the program completes, otherwise false
    //return false; // exit after a single step if the command is "s"
  
    bool completed = execute_single_step(cmd);

    if (nextStmt == nullptr && completed) {
      this->State = "Completed";
      // cout << "Program completed, state changed to " << this->State << endl; // debug
      return true;
    }

    return completed; // return true if the program has completed
  }
  
  // for the r command, continue to run until completion or breakpoint is hit
  while (!execute_single_step(cmd)) {
    // the loop will continue until execute_single_step() returns true indicating the program has completed
  } 

  if(nextStmt == nullptr) {
    if (this->State == "Running") { // End of program
      this->State = "Completed";
      // cout << "Program completed, state changed to " << this->State << endl; // debug
      return true; // End program
    }
  }
  
  return true; // default return if command is r and the loop should continue
}

/* execute_single_step
executes single step. this is used for our s and r command

parameters: string
returns: bool
*/
bool Debugger::execute_single_step(const string& cmd) {
  // 1. breakpoint check: if the curStmt is a breakpoint then outputs false and stops
  if (Breakpoints.find(curStmt->line) != Breakpoints.end()) {
    // cout << "hit breakpoint on line " << curStmt->line << endl; // debug

    if (cmd == "r") {
      nextStmt = get_next_statement(curStmt); // Set up for next step
      // cout << "true was returned and execute single step is done because in execute_steps it will run until it single is true" << endl; // debug
      // cout << "for if we click r AGAIN: cur stmt = " << curStmt->line<< endl; // debug
      // cout << "for if we click r AGAIN: next stmt = " << nextStmt->line<< endl;// debug
      curStmt = nextStmt;
      nextStmt = get_next_statement(curStmt);
      // cout << "for if we click r AGAIN: new cur stmt = " << curStmt->line<< endl;// debug
      // cout << "for if we click r AGAIN: new next stmt = " << nextStmt->line<< endl;// debug
      return true; // Indicate that we hit a breakpoint and execution should pause
    }

    return false;
  } 

  // 2. set a temp breakpoint for s command
  nextStmt = get_next_statement(curStmt); 
  //only unlink if nextstmt is not nullptr and we are already running (not at the start)
  if (nextStmt != nullptr) {
    linkOrUnlinkStmts(curStmt, nullptr); //unlink curStmt from breakStmt temporarily
    // cout << "Temporarily unlinked curStmt from nextStmt" << endl; // debug
  }

  // 3. Execution check: if the execution of the curStmt fails, repair the graph and set state to completed and return true
  struct ExecuteResult er = execute(curStmt, this->Memory);
  // cout << "execution is run... " << endl; // debug
  
  //step 3.7) if execute failed, signal completion
  //link does not be restored because it was already restored above
  if (!er.Success) { // execution failed aka semantic error, repair the graph
    //cout << "execution failed!!!!!!!!!!" << endl; // debug
    this->State = "Completed";
    return true;
  }

  // step 3.5) relink the curstmt from nextstmt then advance
  // cout << "curStmt: " << curStmt->line<< endl; // debug
  // if (nextStmt == nullptr) { // debug
  // cout << "nextStmt is NULLPTR" << endl;
  // }
  
  linkOrUnlinkStmts(curStmt, nextStmt); // restore link between curStmt and breakstmt 
  // cout << "link restored" << endl; // debug
  // attempt to advance to the next Statement
  if (nextStmt != nullptr) {
    // cout << "nextStmt is not a nullptr so enter..." << endl; // debug
    curStmt = nextStmt;
    // cout << "curStmt changed to -> " << curStmt->line << endl; // debug
    nextStmt = get_next_statement(curStmt);
    // if (nextStmt == nullptr) {
    //   cout << "nextStmt is A nullptr" << endl; // debug
    // } else {
    //   cout << "nextStmt changed to -> " << nextStmt->line << endl; // debug
    // }

  } else { // nextStmt = nullptr
  // If nextStmt is nullptr, check if curStmt is a breakpoint
    // // debug check to see what the breakpoints set looks like
    // for (auto iter : Breakpoints) {
    //   cout << iter << " " << endl;
    // }
    // If nextStmt is nullptr, find the next executable line using int_to_stmt
    auto nextLineIt = int_to_stmt.upper_bound(curStmt->line); // Get next line in map
    
    if (nextLineIt != int_to_stmt.end()) {
      // cout << "yay! the line that we broke at exists in the map of program lines" << endl; // debug
      curStmt = nextLineIt->second; // Advance to the next executable statement
      // cout << "curStmt is set to " << curStmt->line<< "for look up" << endl; // debug
      nextStmt = get_next_statement(curStmt);
      // if (nextStmt == nullptr) {
      //   cout << "nextStmt is A nullptr" << endl; // debug
      // } else {
      //   cout << "nextStmt changed to -> " << nextStmt->line << endl; // debug
      // }
      
      // Check if this new line is a breakpoint
      if (Breakpoints.find(curStmt->line) != Breakpoints.end()) {
        // cout << "false is returned and breakpoint found at line " << curStmt->line << endl; // debug
        return false; // Stop at the breakpoint
      } else {
        // cout << "state changed to completed because nextStmt is nullptr!!!!!!!!!!" << endl; // debug
        this->State = "Completed";
        return true; // Program end reached
      }
    }

    if (nextLineIt == int_to_stmt.end()) {
      // cout << "No further lines in int_to_stmt; marking program as complete." << endl; // debug
      curStmt = nullptr; // Signal the true end of the program
      return true;
    }
  }
  // cout << "false is returned for execute_single_step" << endl; // debug
  return false; // continue if there are more statements
}


//
// constructor:
//
Debugger::Debugger(struct STMT* program)
  : State("Loaded"), Program(program), Memory(nullptr), Breakpoints(), curStmt(this->Program), nextStmt(nullptr)
{
  this->Memory = ram_init();

  // fill the map with every line in the program
  // map<int, struct STMT&> int_to_stmt
  struct STMT* current = this->Program;
  while (current != nullptr) {
    int_to_stmt[current->line] = current;
    current= get_next_statement(current);
  }

  // // debug to check if the map was populated correctly
  // for (auto pair : int_to_stmt) {
  //   cout << pair.first<< " : " << pair.second->line<< endl;
  // }
}

//
// destructor:
//
Debugger::~Debugger()
{
  ram_destroy(this->Memory);
}


//
// run:
//
// Run the debugger for one execution run of the input program.
//
void Debugger::run()
{
  string cmd;
  
  // //
  // // controls where we start execution from:
  // //
  // struct STMT* curStmt = this->Program;
  
  //
  // if we have a breakpoint, these will be set to the breakpoint,
  // and the statement preceding it:
  //
  struct STMT* prevBreakpoint = nullptr;
  struct STMT* breakpoint = nullptr;
  
  //
  // command loop until quit is entered:
  //
  while (true) {
    cout << endl;
    cout << "Enter a command, type h for help. Type r to run. > ";
    cin >> cmd;
    
    if (cmd == "q") { // consider including something to relink the graph and reset breakpoint data to prevent memory leaks
      break;
    }
    else if (cmd == "h") {
      
      cout << "Available commands:"
      << endl << "r -> Run the program / continue from a breakpoint"
      << endl << "s -> Step to the next stmt by executing current stmt"
      << endl << "b n -> Breakpoint at line n" 
      << endl << "rb n -> Remove breakpoint at line n"
      << endl << "lb -> List all breakpoints"
      << endl << "cb -> Clear all breakpoints"
      << endl << "p varname -> Print variable"
      << endl << "sm -> Show memory contents"
      << endl << "ss -> Show state of debugger"
      << endl << "w -> What line are we on?"
      << endl << "q -> Quit the debugger"
      << endl;
    } else if (cmd == "r" or cmd == "s") {
      if (this->State == "Completed") {
        cout << "program has completed" << endl;
        continue;
      } 

      execute_steps(cmd);



    } else if (cmd == "b") {
      
      int lineNum;
      cin >> lineNum;
      
      //search the set to see if this breakpoint already exists
      if (Breakpoints.find(lineNum) != Breakpoints.end()) { // if it already exists in set
        cout << "breakpoint already set" << endl;
        continue;
      } 

      // if (breakpoint != nullptr) { // sorry, we already have a breakpoint set:
      //   cout << "breakpoint already set" << endl;
      //   continue;  // skip code below and continue with next cmd:
      // }
      
      //
      // Okay, so no breakpoint is set, so we have a valid program
      // graph that we can traverse.
      //
      // first, search the program for the breakpoint, along with
      // the stmt that precedes it:
      //
      prevBreakpoint = nullptr;
      breakpoint = this->Program; // start from first stmt so prev will get set:
      
      bool exists = findBreakpoint(prevBreakpoint, breakpoint, lineNum);
      
      if (!exists) {
        cout << "no such line" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      //
      // okay, the breakpoint line exists, so let's set the breakpoint
      // by severing the program graph between prevStmt and curStmt:
      //
      linkOrUnlinkStmts(prevBreakpoint, nullptr);
      Breakpoints.insert(lineNum); // add line number to breakpoints set if it checked the line exists check
      cout << "breakpoint set" << endl;
    }
    else if (cmd == "rb") {
      
      int lineNum;
      cin >> lineNum;
      
      if (breakpoint == nullptr) // we have no breakpoint
        cout << "no such breakpoint" << endl;
      else if (breakpoint->line != lineNum) // not the same breakpoint
        cout << "no such breakpoint" << endl;
      else {  // removing current breakpoint:
        //
        // we remove by linking the program graph back together:
        //
        linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
        prevBreakpoint = nullptr;
        breakpoint = nullptr;
     
        cout << "breakpoint removed" << endl;
      }
    }
    else if (cmd == "lb") {
      
      if (breakpoint == nullptr) {
        cout << "no breakpoints" << endl;
      } else {// we have a breakpoint:
        cout << "breakpoints on lines:"; 
        // for each breakpoint line number in set of breakpoints
        for (auto iter : Breakpoints) { // iter in this case is a line number 
          cout << " " + iter;
        }
        cout << endl;
      }
    }
    else if (cmd == "cb") {
      
      if (breakpoint != nullptr) { // we have a breakpoint to clear:
        //
        // we clear by linking the program graph back together:
        //
        linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
        //reset breakpoint data
        prevBreakpoint = nullptr;
        breakpoint = nullptr;
      }
      
      cout << "breakpoints cleared" << endl;
    }
    else if (cmd == "p") {
      
      string varname;
      cin >> varname;
      
      const char* name = varname.c_str();
      
      struct RAM_VALUE* value = ram_read_cell_by_name(this->Memory, (char*) name);
      
      if (value == nullptr) {
        cout << "no such variable" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      print_value(varname, value);
      ram_free_value(value);
    }
    else if (cmd == "sm") {
      
      ram_print(this->Memory);
    }
    else if (cmd == "ss") {
      
      cout << this->State << endl;
    }
    else if (cmd == "w") {
      
      if (this->State == "Loaded") {
        cout << "line " << this->Program->line << endl;
        // struct STMT* stop = get_next_statement(curStmt);
        // linkOrUnlinkStmts(curStmt, stop);
        programgraph_print(curStmt);
        
      } else if (this->State == "Completed") {
        cout << "completed execution" << endl;
      } else {// we are running:
        //struct STMT* nextStmt = get_next_statement(curStmt); // create a variable that stores the next stmt
        if (nextStmt != nullptr) {
          cout << "line " << curStmt->line << endl;
          //cout << "next" << nextStmt->line << endl; //DEBUG
          linkOrUnlinkStmts(curStmt, nullptr); // unlink
          //cout << (nullptr == get_next_statement(curStmt)) << endl; //DEBUG 0 means its not unlinked prop and 1 means that its unlinked
          programgraph_print(curStmt); // print one line
          linkOrUnlinkStmts(curStmt, nextStmt); // link again
        } else { // reached the end of the program 
          cout << "completed execution" << endl; 
        } 

      }



    // } else if (cmd == "s") { // this will execute a ingle statement at a time
    //   if (this->State == "Completed") {
    //     cout << "program has completed" << endl;
    //     return;
    //   } else if (this->State == "Loaded") {
    //     this->State = "Running"; // change state to running
    //     curStmt = this->Program; // set the current stmt equal to the beginning of the program
    //   } 
    //   // no else statement for if the state is Running is necessary becausr curStmt is already pointing to the curStmt 
    //   // and there is no need to reset it in the running state when we just did that if it was in the loading state
      
    //   execute_single_step(); // call on helper function
    } else {
      cout << "unknown command" << endl;
    }

  }//while
  
  //
  // at this point execution has completed (or the user quit
  // early). If there's still a breakpoint, we need to repair
  // the program graph so the memory is freed properly when
  // we return to main:
  //
  if (breakpoint != nullptr) { // there's still a breakpoint
    linkOrUnlinkStmts(prevBreakpoint, breakpoint);
    
    prevBreakpoint = nullptr; // not really necessary, but just to be safe
    breakpoint = nullptr;
  }
  
}//run












// before making changes at oh
// getting s s s w to work

/*debugger.cpp*/

//
// Debugger for nuPython, C++ edition! Provides a simple gdb-like
// interface, with support for one breakpoint in straight-line code.
// Uses nuPython interpreter as execution engine.
//
// Authors:
//    Ata Guvendi
//    Prof. Hummel
// 
// Northwestern University
// CS 211
//

#include <iostream>
#include <cassert>

#include "debugger.h"
#include "execute.h"

using namespace std;


//
// print_value
//
// Prints the contents of a RAM cell, both type and value.
//
void Debugger::print_value(string varname, struct RAM_VALUE* value)
{
  cout << varname << " ("; 
  
  switch (value->value_type) {
    
    case RAM_TYPE_INT:
      cout << "int): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_REAL:
      cout << "real): " << value->types.d << endl;
      break;
      
    case RAM_TYPE_STR:
      cout << "str): " << value->types.s << endl;
      break;
      
    case RAM_TYPE_PTR:
      cout << "ptr): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_BOOLEAN:
      cout << "bool): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_NONE:
      cout << "none): " << "None" << endl;
      break;
  }//switch
}


//
// findBreakpoint
//
// starts from the stmt denoted by breakpoint, and searches forward
// for a stmt with the given line number. As it advances, prev is
// updated to always denote the previous statement. The parameters
// are passed-by-reference so they are updated back in the caller.
//
// Note that if breakpoint happens to match immediately, without advancing,
// then prev does not get set. So the caller should probably set prev 
// appropriately before calling (either nullptr, or to the previous stmt).
//
bool Debugger::findBreakpoint(struct STMT*& prev, struct STMT*& breakpoint, int lineNum)
{
  while (breakpoint != nullptr) {
    
    // cout << breakpoint->line << endl;
    
    if (breakpoint->line == lineNum) // found it!
      return true;
      
    prev = breakpoint;
    
    if (breakpoint->stmt_type == STMT_ASSIGNMENT)
      breakpoint = breakpoint->types.assignment->next_stmt;
    else if (breakpoint->stmt_type == STMT_FUNCTION_CALL)
      breakpoint = breakpoint->types.function_call->next_stmt;
    else {
      assert(breakpoint->stmt_type == STMT_PASS);
      breakpoint = breakpoint->types.pass->next_stmt;
    }
  }//while
  
  // if get here, not found
  return false;
}


//
// linkOrUnlinkStmts
//
// updates prev stmt to point to whatever cur is --- this can be 
// nullptr to unlink, or an actual stmt to link them back together.
//
void Debugger::linkOrUnlinkStmts(struct STMT* prev, struct STMT* cur)
{
  if (prev == nullptr) // nothing to link or unlink:
    return;
    
  if (prev->stmt_type == STMT_ASSIGNMENT)
    prev->types.assignment->next_stmt = cur;
  else if (prev->stmt_type == STMT_FUNCTION_CALL)
    prev->types.function_call->next_stmt = cur;
  else {
    assert(prev->stmt_type == STMT_PASS);
    prev->types.pass->next_stmt = cur;
  }
}



/* get_next_statement
when getting the next statement it checks what type it is as every type has a differnet way of getting next statement
Moves to the next statement by checking the type of the current statement (curr).

parameter: stmt
returns: struct
*/
struct STMT* Debugger::get_next_statement(struct STMT* stmt) {
  if (stmt->stmt_type == STMT_ASSIGNMENT) {
      return stmt->types.assignment->next_stmt;
  } else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
      return stmt->types.function_call->next_stmt;
  } else if (stmt->stmt_type == STMT_PASS) {
      return stmt->types.pass->next_stmt;
  } else {
      return nullptr;
  }
} 

/* find_prevStmt
finds the previous statement from a current statement or "target"

parameters: stmt
returns: stmt
*/
struct STMT* Debugger::find_prevStmt(struct STMT* target) {
  struct STMT* prev = nullptr;
  struct STMT* current = this->Program;

  while (current != nullptr && current != target) {
    prev = current;
    current = get_next_statement(current);  // Use get_next_statement to move to the next
  }

  return prev;  // Returns nullptr if target is the first statement
};

/* execute_steps
checks what to do based on if the command is s or r

parameters: string
returns: bool
*/
bool Debugger::execute_steps(string cmd){
  // initial check for beginning of the program
  if (nextStmt == nullptr) {
    if (this->State == "Loaded") { // Beginning of program
      nextStmt = get_next_statement(curStmt); // Initialize first step
      this->State = "Running";
      // cout << "Program started, state changed to " << this->State << endl; // debug
    }
  }
  
  
  if (cmd == "s") {
    // cout << "entered an s command. calling execute_single_step()"<< endl; // debug
    this->State = "Running";
    //return execute_single_step(cmd); // Return true if the program completes, otherwise false
    //return false; // exit after a single step if the command is "s"
  
    bool completed = execute_single_step(cmd);

    if (curStmt == nullptr && nextStmt == nullptr && completed) {
      this->State = "Completed";
      // cout << "Program completed, state changed to " << this->State << endl; // debug
      return true;
    }

    return completed; // return true if the program has completed
  }
  
  // for the r command, continue to run until completion or breakpoint is hit
  while (!execute_single_step(cmd)) {
    // the loop will continue until execute_single_step() returns true indicating the program has completed
  } 

  if(curStmt == nullptr && nextStmt == nullptr) {
    if (this->State == "Running") { // End of program
      this->State = "Completed";
      // cout << "Program completed, state changed to " << this->State << endl; // debug
      return true; // End program
    }
  }
  
  return true; // default return if command is r and the loop should continue
}

/* execute_single_step
executes single step. this is used for our s and r command

parameters: string
returns: bool
*/
bool Debugger::execute_single_step(const string& cmd) {
  // 1. breakpoint check: if the curStmt is a breakpoint then outputs false and stops
  if (Breakpoints.find(curStmt->line) != Breakpoints.end()) {
    // cout << "hit breakpoint on line " << curStmt->line << endl; // debug

    if (cmd == "r") {
      nextStmt = get_next_statement(curStmt); // Set up for next step
      cout << "true was returned and execute single step is done because in execute_steps it will run until it single is true" << endl; // debug
      cout << "for if we click r AGAIN: cur stmt = " << curStmt->line<< endl; // debug
      cout << "for if we click r AGAIN: next stmt = " << nextStmt->line<< endl;// debug
      // curStmt = nextStmt;
      // nextStmt = get_next_statement(curStmt);
      cout << "for if we click r AGAIN: new cur stmt = " << curStmt->line<< endl;// debug
      cout << "for if we click r AGAIN: new next stmt = " << nextStmt->line<< endl;// debug
      return true; // Indicate that we hit a breakpoint and execution should pause
    }

    return false;
  } 

  // 2. set a temp breakpoint for s command
  nextStmt = get_next_statement(curStmt); 
  //only unlink if nextstmt is not nullptr and we are already running (not at the start)
  if (nextStmt != nullptr) {
    linkOrUnlinkStmts(curStmt, nullptr); //unlink curStmt from breakStmt temporarily
    // cout << "Temporarily unlinked curStmt from nextStmt" << endl; // debug
  }

  // 3. Execution check: if the execution of the curStmt fails, repair the graph and set state to completed and return true
  struct ExecuteResult er = execute(curStmt, this->Memory);
  // cout << "execution is run... " << endl; // debug
  
  //step 3.7) if execute failed, signal completion
  //link does not be restored because it was already restored above
  if (!er.Success) { // execution failed aka semantic error, repair the graph
    //cout << "execution failed!!!!!!!!!!" << endl; // debug
    this->State = "Completed";
    return true;
  }

  // step 3.5) relink the curstmt from nextstmt then advance
  // cout << "curStmt: " << curStmt->line<< endl; // debug
  // if (nextStmt == nullptr) { // debug
  // cout << "nextStmt is NULLPTR" << endl;
  // }
  
  cout << "linking cur" << curStmt->line<<endl;
  //cout <<"linking next" << nextStmt->line <<endl;
  linkOrUnlinkStmts(curStmt, nextStmt); // restore link between curStmt and breakstmt 
  // cout << "link restored" << endl; // debug
  // attempt to advance to the next Statement
  if (nextStmt != nullptr) {
    // cout << "nextStmt is not a nullptr so enter..." << endl; // debug
    curStmt = nextStmt;
    // cout << "curStmt changed to -> " << curStmt->line << endl; // debug
    nextStmt = get_next_statement(curStmt);
    // if (nextStmt == nullptr) {
    //   cout << "nextStmt is A nullptr" << endl; // debug
    // } else {
    //   cout << "nextStmt changed to -> " << nextStmt->line << endl; // debug
    // }

  } else { // nextStmt = nullptr
  // If nextStmt is nullptr, check if curStmt is a breakpoint
    // // debug check to see what the breakpoints set looks like
    // for (auto iter : Breakpoints) {
    //   cout << iter << " " << endl;
    // }
    // If nextStmt is nullptr, find the next executable line using int_to_stmt
    auto nextLineIt = int_to_stmt.upper_bound(curStmt->line); // Get next line in map
    
    if (nextLineIt != int_to_stmt.end()) {
      cout << "yay! the line that we broke at exists in the map of program lines" << endl; // debug
      curStmt = nextLineIt->second; // Advance to the next executable statement
      cout << "curStmt is set to " << curStmt->line<< "for look up" << endl; // debug
      nextStmt = get_next_statement(curStmt);
      if (nextStmt == nullptr) {
        cout << "nextStmt is A nullptr" << endl; // debug
      } else {
        cout << "nextStmt changed to -> " << nextStmt->line << endl; // debug
      }
      
      // Check if this new line is a breakpoint
      if (Breakpoints.find(curStmt->line) != Breakpoints.end()) {
        cout << "curStmt is ..." << curStmt-> line << endl;
        cout << "false is returned and breakpoint found at line " << curStmt->line << endl; // debug
        return false; // Stop at the breakpoint
      } else {
        // cout << "state changed to completed because nextStmt is nullptr!!!!!!!!!!" << endl; // debug
        this->State = "Completed";
        return true; // Program end reached
      }
    }

    if (nextLineIt == int_to_stmt.end()) {
      //cout << "No further lines in int_to_stmt; marking program as complete." << endl; // debug
      curStmt = nullptr; // Signal the true end of the program
      return true;
    }
  }
  // cout << "false is returned for execute_single_step" << endl; // debug
  return false; // continue if there are more statements
}


//
// constructor:
//
Debugger::Debugger(struct STMT* program)
  : State("Loaded"), Program(program), Memory(nullptr), Breakpoints(), curStmt(this->Program), nextStmt(nullptr)
{
  this->Memory = ram_init();

  // fill the map with every line in the program
  // map<int, struct STMT&> int_to_stmt
  struct STMT* current = this->Program;
  while (current != nullptr) {
    int_to_stmt[current->line] = current;
    current= get_next_statement(current);
  }

  // // debug to check if the map was populated correctly
  // for (auto pair : int_to_stmt) {
  //   cout << pair.first<< " : " << pair.second->line<< endl;
  // }
}

//
// destructor:
//
Debugger::~Debugger()
{
  ram_destroy(this->Memory);
}


//
// run:
//
// Run the debugger for one execution run of the input program.
//
void Debugger::run()
{
  string cmd;
  
  // //
  // // controls where we start execution from:
  // //
  // struct STMT* curStmt = this->Program;
  
  //
  // if we have a breakpoint, these will be set to the breakpoint,
  // and the statement preceding it:
  //
  struct STMT* prevBreakpoint = nullptr;
  struct STMT* breakpoint = nullptr;
  
  //
  // command loop until quit is entered:
  //
  while (true) {
    cout << endl;
    cout << "Enter a command, type h for help. Type r to run. > ";
    cin >> cmd;
    
    if (cmd == "q") { // consider including something to relink the graph and reset breakpoint data to prevent memory leaks
      break;
    }
    else if (cmd == "h") {
      
      cout << "Available commands:"
      << endl << "r -> Run the program / continue from a breakpoint"
      << endl << "s -> Step to the next stmt by executing current stmt"
      << endl << "b n -> Breakpoint at line n" 
      << endl << "rb n -> Remove breakpoint at line n"
      << endl << "lb -> List all breakpoints"
      << endl << "cb -> Clear all breakpoints"
      << endl << "p varname -> Print variable"
      << endl << "sm -> Show memory contents"
      << endl << "ss -> Show state of debugger"
      << endl << "w -> What line are we on?"
      << endl << "q -> Quit the debugger"
      << endl;
    } else if (cmd == "r" or cmd == "s") {
      if (this->State == "Completed") {
        cout << "program has completed" << endl;
        continue;
      } 

      execute_steps(cmd);



    } else if (cmd == "b") {
      
      int lineNum;
      cin >> lineNum;
      
      //search the set to see if this breakpoint already exists
      if (Breakpoints.find(lineNum) != Breakpoints.end()) { // if it already exists in set
        cout << "breakpoint already set" << endl;
        continue;
      } 

      // if (breakpoint != nullptr) { // sorry, we already have a breakpoint set:
      //   cout << "breakpoint already set" << endl;
      //   continue;  // skip code below and continue with next cmd:
      // }
      
      //
      // Okay, so no breakpoint is set, so we have a valid program
      // graph that we can traverse.
      //
      // first, search the program for the breakpoint, along with
      // the stmt that precedes it:
      //
      prevBreakpoint = nullptr;
      breakpoint = this->Program; // start from first stmt so prev will get set:
      
      bool exists = findBreakpoint(prevBreakpoint, breakpoint, lineNum);
      
      if (!exists) {
        cout << "no such line" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      //
      // okay, the breakpoint line exists, so let's set the breakpoint
      // by severing the program graph between prevStmt and curStmt:
      //
      cout << "prevbreakpoint" << prevBreakpoint ->line<<endl;
      cout << "breakpoint " << breakpoint->line <<endl;
      linkOrUnlinkStmts(prevBreakpoint, nullptr);
      Breakpoints.insert(lineNum); // add line number to breakpoints set if it checked the line exists check
      cout << "breakpoint set" << endl;
    }
    else if (cmd == "rb") {
      
      int lineNum;
      cin >> lineNum;
      
      if (breakpoint == nullptr) // we have no breakpoint
        cout << "no such breakpoint" << endl;
      else if (breakpoint->line != lineNum) // not the same breakpoint
        cout << "no such breakpoint" << endl;
      else {  // removing current breakpoint:
        //
        // we remove by linking the program graph back together:
        //
        linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
        prevBreakpoint = nullptr;
        breakpoint = nullptr;
     
        cout << "breakpoint removed" << endl;
      }
    }
    else if (cmd == "lb") {
      
      if (breakpoint == nullptr) {
        cout << "no breakpoints" << endl;
      } else {// we have a breakpoint:
        cout << "breakpoints on lines:"; 
        // for each breakpoint line number in set of breakpoints
        for (auto iter : Breakpoints) { // iter in this case is a line number 
          cout << " " + iter;
        }
        cout << endl;
      }
    }
    else if (cmd == "cb") {
      
      if (breakpoint != nullptr) { // we have a breakpoint to clear:
        //
        // we clear by linking the program graph back together:
        //
        linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
        //reset breakpoint data
        prevBreakpoint = nullptr;
        breakpoint = nullptr;
      }
      
      cout << "breakpoints cleared" << endl;
    }
    else if (cmd == "p") {
      
      string varname;
      cin >> varname;
      
      const char* name = varname.c_str();
      
      struct RAM_VALUE* value = ram_read_cell_by_name(this->Memory, (char*) name);
      
      if (value == nullptr) {
        cout << "no such variable" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      print_value(varname, value);
      ram_free_value(value);
    }
    else if (cmd == "sm") {
      
      ram_print(this->Memory);
    }
    else if (cmd == "ss") {
      
      cout << this->State << endl;
    }
    else if (cmd == "w") {
      
      if (this->State == "Loaded") {
        cout << "line " << this->Program->line << endl;
        // struct STMT* stop = get_next_statement(curStmt);
        // linkOrUnlinkStmts(curStmt, stop);
        programgraph_print(curStmt);
        
      } else if (this->State == "Completed") {
        cout << "completed execution" << endl;
      } else {// we are running:
        //struct STMT* nextStmt = get_next_statement(curStmt); // create a variable that stores the next stmt
        if (nextStmt != nullptr) {
          cout << "line " << curStmt->line << endl;
          //cout << "next" << nextStmt->line << endl; //DEBUG
          linkOrUnlinkStmts(curStmt, nullptr); // unlink
          //cout << (nullptr == get_next_statement(curStmt)) << endl; //DEBUG 0 means its not unlinked prop and 1 means that its unlinked
          programgraph_print(curStmt); // print one line
          linkOrUnlinkStmts(curStmt, nextStmt); // link again
        } else { // reached the end of the program 
          cout << "completed execution" << endl; 
        } 

      }



    // } else if (cmd == "s") { // this will execute a ingle statement at a time
    //   if (this->State == "Completed") {
    //     cout << "program has completed" << endl;
    //     return;
    //   } else if (this->State == "Loaded") {
    //     this->State = "Running"; // change state to running
    //     curStmt = this->Program; // set the current stmt equal to the beginning of the program
    //   } 
    //   // no else statement for if the state is Running is necessary becausr curStmt is already pointing to the curStmt 
    //   // and there is no need to reset it in the running state when we just did that if it was in the loading state
      
    //   execute_single_step(); // call on helper function
    } else {
      cout << "unknown command" << endl;
    }

  }//while
  
  //
  // at this point execution has completed (or the user quit
  // early). If there's still a breakpoint, we need to repair
  // the program graph so the memory is freed properly when
  // we return to main:
  //
  if (breakpoint != nullptr) { // there's still a breakpoint
    linkOrUnlinkStmts(prevBreakpoint, breakpoint);
    
    prevBreakpoint = nullptr; // not really necessary, but just to be safe
    breakpoint = nullptr;
  }
  
}//run









// end of oh 
// getting s s s w to work

/*debugger.cpp*/

//
// Debugger for nuPython, C++ edition! Provides a simple gdb-like
// interface, with support for one breakpoint in straight-line code.
// Uses nuPython interpreter as execution engine.
//
// Authors:
//    Ata Guvendi
//    Prof. Hummel
// 
// Northwestern University
// CS 211
//

#include <iostream>
#include <cassert>

#include "debugger.h"
#include "execute.h"

using namespace std;


//
// print_value
//
// Prints the contents of a RAM cell, both type and value.
//
void Debugger::print_value(string varname, struct RAM_VALUE* value)
{
  cout << varname << " ("; 
  
  switch (value->value_type) {
    
    case RAM_TYPE_INT:
      cout << "int): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_REAL:
      cout << "real): " << value->types.d << endl;
      break;
      
    case RAM_TYPE_STR:
      cout << "str): " << value->types.s << endl;
      break;
      
    case RAM_TYPE_PTR:
      cout << "ptr): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_BOOLEAN:
      cout << "bool): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_NONE:
      cout << "none): " << "None" << endl;
      break;
  }//switch
}


//
// findBreakpoint
//
// starts from the stmt denoted by breakpoint, and searches forward
// for a stmt with the given line number. As it advances, prev is
// updated to always denote the previous statement. The parameters
// are passed-by-reference so they are updated back in the caller.
//
// Note that if breakpoint happens to match immediately, without advancing,
// then prev does not get set. So the caller should probably set prev 
// appropriately before calling (either nullptr, or to the previous stmt).
//
bool Debugger::findBreakpoint(struct STMT*& prev, struct STMT*& breakpoint, int lineNum)
{
  while (breakpoint != nullptr) {
    
    // cout << breakpoint->line << endl;
    
    if (breakpoint->line == lineNum) // found it!
      return true;
      
    prev = breakpoint;
    
    if (breakpoint->stmt_type == STMT_ASSIGNMENT)
      breakpoint = breakpoint->types.assignment->next_stmt;
    else if (breakpoint->stmt_type == STMT_FUNCTION_CALL)
      breakpoint = breakpoint->types.function_call->next_stmt;
    else {
      assert(breakpoint->stmt_type == STMT_PASS);
      breakpoint = breakpoint->types.pass->next_stmt;
    }
  }//while
  
  // if get here, not found
  return false;
}


//
// linkOrUnlinkStmts
//
// updates prev stmt to point to whatever cur is --- this can be 
// nullptr to unlink, or an actual stmt to link them back together.
//
void Debugger::linkOrUnlinkStmts(struct STMT* prev, struct STMT* cur)
{
  if (prev == nullptr) // nothing to link or unlink:
    return;
    
  if (prev->stmt_type == STMT_ASSIGNMENT)
    prev->types.assignment->next_stmt = cur;
  else if (prev->stmt_type == STMT_FUNCTION_CALL)
    prev->types.function_call->next_stmt = cur;
  else {
    assert(prev->stmt_type == STMT_PASS);
    prev->types.pass->next_stmt = cur;
  }
}



/* get_next_statement
when getting the next statement it checks what type it is as every type has a differnet way of getting next statement
Moves to the next statement by checking the type of the current statement (curr).

parameter: stmt
returns: struct
*/
struct STMT* Debugger::get_next_statement(struct STMT* stmt) {
  if (stmt->stmt_type == STMT_ASSIGNMENT) {
      return stmt->types.assignment->next_stmt;
  } else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
      return stmt->types.function_call->next_stmt;
  } else if (stmt->stmt_type == STMT_PASS) {
      return stmt->types.pass->next_stmt;
  } else {
      return nullptr;
  }
} 

/* find_prevStmt
finds the previous statement from a current statement or "target"

parameters: stmt
returns: stmt
*/
struct STMT* Debugger::find_prevStmt(struct STMT* target) {
  struct STMT* prev = nullptr;
  struct STMT* current = this->Program;

  while (current != nullptr && current != target) {
    prev = current;
    current = get_next_statement(current);  // Use get_next_statement to move to the next
  }

  return prev;  // Returns nullptr if target is the first statement
};

/* execute_steps
checks what to do based on if the command is s or r

parameters: string
returns: bool
*/
bool Debugger::execute_steps(string cmd){
  // initial check for beginning of the program
  if (nextStmt == nullptr) {
    if (this->State == "Loaded") { // Beginning of program
      nextStmt = get_next_statement(curStmt); // Initialize first step
      this->State = "Running";
      // cout << "Program started, state changed to " << this->State << endl; // debug
    }
  }
  
  
  if (cmd == "s") {
    // cout << "entered an s command. calling execute_single_step()"<< endl; // debug
    this->State = "Running";
    //return execute_single_step(cmd); // Return true if the program completes, otherwise false
    //return false; // exit after a single step if the command is "s"
  
    bool completed = execute_single_step(cmd);

    if (curStmt == nullptr && nextStmt == nullptr && completed) {
      this->State = "Completed";
      // cout << "Program completed, state changed to " << this->State << endl; // debug
      return true;
    }

    return completed; // return true if the program has completed
  }
  
  // for the r command, continue to run until completion or breakpoint is hit
  while (!execute_single_step(cmd)) {
    // the loop will continue until execute_single_step() returns true indicating the program has completed
  } 

  if(curStmt == nullptr && nextStmt == nullptr) {
    if (this->State == "Running") { // End of program
      this->State = "Completed";
      // cout << "Program completed, state changed to " << this->State << endl; // debug
      return true; // End program
    }
  }
  
  return true; // default return if command is r and the loop should continue
}

/* execute_single_step
executes single step. this is used for our s and r command

parameters: string
returns: bool
*/
bool Debugger::execute_single_step(const string& cmd) {
  // 1. breakpoint check: if the curStmt is a breakpoint then outputs false and stops
  if (Breakpoints.find(curStmt->line) != Breakpoints.end()) {
    // cout << "hit breakpoint on line " << curStmt->line << endl; // debug
    auto it = Breakpoints.find(curStmt->line);
    if (it->second == false) {
      cout << "true was returned" << endl; // debug
      return true; // end this while !execute_single_step loop because we reached a bp and its never been hit before
    } else if (it->second == true) {
      cout << "true was returned because we hit the bp and its already been hit before so keep looping through this while" << endl;
      //Breakpoints[curStmt->line] = false;
      //return false;
    }

    // if (cmd == "r") {
    //   nextStmt = get_next_statement(curStmt); // Set up for next step
    //   cout << "true was returned and execute single step is done because in execute_steps it will run until it single is true" << endl; // debug
    //   cout << "for if we click r AGAIN: cur stmt = " << curStmt->line<< endl; // debug
    //   cout << "for if we click r AGAIN: next stmt = " << nextStmt->line<< endl;// debug
    //   // curStmt = nextStmt;
    //   // nextStmt = get_next_statement(curStmt);
    //   cout << "for if we click r AGAIN: new cur stmt = " << curStmt->line<< endl;// debug
    //   cout << "for if we click r AGAIN: new next stmt = " << nextStmt->line<< endl;// debug
    //   return true; // Indicate that we hit a breakpoint and execution should pause
    // }

    // return false;
  } 

  // 2. set a temp breakpoint for s command
  nextStmt = get_next_statement(curStmt); 
  //only unlink if nextstmt is not nullptr and we are already running (not at the start)
  if (nextStmt != nullptr) {
    linkOrUnlinkStmts(curStmt, nullptr); //unlink curStmt from breakStmt temporarily
    // cout << "Temporarily unlinked curStmt from nextStmt" << endl; // debug
  }

  // 3. Execution check: if the execution of the curStmt fails, repair the graph and set state to completed and return true
  struct ExecuteResult er = execute(curStmt, this->Memory);
  // cout << "execution is run... " << endl; // debug
  
  //step 3.7) if execute failed, signal completion
  //link does not be restored because it was already restored above
  if (!er.Success) { // execution failed aka semantic error, repair the graph
    //cout << "execution failed!!!!!!!!!!" << endl; // debug
    this->State = "Completed";
    return true;
  }

  // step 3.5) relink the curstmt from nextstmt then advance
  // cout << "curStmt: " << curStmt->line<< endl; // debug
  // if (nextStmt == nullptr) { // debug
  // cout << "nextStmt is NULLPTR" << endl;
  // }
  
  cout << "linking cur" << curStmt->line<<endl;
  //cout <<"linking next" << nextStmt->line <<endl;
  linkOrUnlinkStmts(curStmt, nextStmt); // restore link between curStmt and breakstmt 
  // cout << "link restored" << endl; // debug
  
  
  // attempt to advance to the next Statement
  if (nextStmt != nullptr) {
    // cout << "nextStmt is not a nullptr so enter..." << endl; // debug
    curStmt = nextStmt;
    // cout << "curStmt changed to -> " << curStmt->line << endl; // debug
    nextStmt = get_next_statement(curStmt);
    // if (nextStmt == nullptr) {
    //   cout << "nextStmt is A nullptr" << endl; // debug
    // } else {
    //   cout << "nextStmt changed to -> " << nextStmt->line << endl; // debug
    // }

  } else { // nextStmt = nullptr
  // If nextStmt is nullptr, check if curStmt is a breakpoint
    // // debug check to see what the breakpoints set looks like
    // for (auto iter : Breakpoints) {
    //   cout << iter << " " << endl;
    // }
    // If nextStmt is nullptr, find the next executable line using int_to_stmt
    auto nextLineIt = int_to_stmt.upper_bound(curStmt->line); // Get next line in map
    
    if (nextLineIt != int_to_stmt.end()) {
      cout << "yay! the line that we broke at exists in the map of program lines" << endl; // debug
      curStmt = nextLineIt->second; // Advance to the next executable statement
      cout << "curStmt is set to " << curStmt->line<< "for look up" << endl; // debug
      nextStmt = get_next_statement(curStmt);
      //debug
      if (nextStmt == nullptr) {
        cout << "nextStmt is A nullptr" << endl; // debug
      } else {
        cout << "nextStmt changed to -> " << nextStmt->line << endl; // debug
      }
      
      // Check if this new line is a breakpoint
      if (Breakpoints.find(curStmt->line) != Breakpoints.end()) {
        cout << "curStmt is ..." << curStmt-> line << endl;
        cout << "false is returned and breakpoint found at line " << curStmt->line << endl; // debug
        cout << "hit breakpoint on line " << curStmt->line << endl;
        programgraph_print(curStmt);
        Breakpoints[curStmt->line] = true;
        return true; // Stop at the breakpoint
      } else {
        // cout << "state changed to completed because nextStmt is nullptr!!!!!!!!!!" << endl; // debug
        this->State = "Completed";
        return true; // Program end reached
      }
    }

    if (nextLineIt == int_to_stmt.end()) {
      //cout << "No further lines in int_to_stmt; marking program as complete." << endl; // debug
      curStmt = nullptr; // Signal the true end of the program
      return true;
    }
  }
  // cout << "false is returned for execute_single_step" << endl; // debug
  return false; // continue if there are more statements
}


//
// constructor:
//
Debugger::Debugger(struct STMT* program)
  : State("Loaded"), Program(program), Memory(nullptr), Breakpoints(), curStmt(this->Program), nextStmt(nullptr)
{
  this->Memory = ram_init();

  // fill the map with every line in the program
  // map<int, struct STMT&> int_to_stmt
  struct STMT* current = this->Program;
  while (current != nullptr) {
    int_to_stmt[current->line] = current;
    current= get_next_statement(current);
  }

  // // debug to check if the map was populated correctly
  // for (auto pair : int_to_stmt) {
  //   cout << pair.first<< " : " << pair.second->line<< endl;
  // }
}

//
// destructor:
//
Debugger::~Debugger()
{
  ram_destroy(this->Memory);
}


//
// run:
//
// Run the debugger for one execution run of the input program.
//
void Debugger::run()
{
  string cmd;
  
  // //
  // // controls where we start execution from:
  // //
  // struct STMT* curStmt = this->Program;
  
  //
  // if we have a breakpoint, these will be set to the breakpoint,
  // and the statement preceding it:
  //
  struct STMT* prevBreakpoint = nullptr;
  struct STMT* breakpoint = nullptr;
  
  //
  // command loop until quit is entered:
  //
  while (true) {
    cout << endl;
    cout << "Enter a command, type h for help. Type r to run. > ";
    cin >> cmd;
    
    if (cmd == "q") { // consider including something to relink the graph and reset breakpoint data to prevent memory leaks
      break;
    }
    else if (cmd == "h") {
      
      cout << "Available commands:"
      << endl << "r -> Run the program / continue from a breakpoint"
      << endl << "s -> Step to the next stmt by executing current stmt"
      << endl << "b n -> Breakpoint at line n" 
      << endl << "rb n -> Remove breakpoint at line n"
      << endl << "lb -> List all breakpoints"
      << endl << "cb -> Clear all breakpoints"
      << endl << "p varname -> Print variable"
      << endl << "sm -> Show memory contents"
      << endl << "ss -> Show state of debugger"
      << endl << "w -> What line are we on?"
      << endl << "q -> Quit the debugger"
      << endl;
    } else if (cmd == "r" or cmd == "s") {
      if (this->State == "Completed") {
        cout << "program has completed" << endl;
        continue;
      } 

      execute_steps(cmd);



    } else if (cmd == "b") {
      
      int lineNum;
      cin >> lineNum;
      
      //search the set to see if this breakpoint already exists
      if (Breakpoints.find(lineNum) != Breakpoints.end()) { // if it already exists in set
        cout << "breakpoint already set" << endl;
        continue;
      } 

      // if (breakpoint != nullptr) { // sorry, we already have a breakpoint set:
      //   cout << "breakpoint already set" << endl;
      //   continue;  // skip code below and continue with next cmd:
      // }
      
      //
      // Okay, so no breakpoint is set, so we have a valid program
      // graph that we can traverse.
      //
      // first, search the program for the breakpoint, along with
      // the stmt that precedes it:
      //
      prevBreakpoint = nullptr;
      breakpoint = this->Program; // start from first stmt so prev will get set:
      
      bool exists = findBreakpoint(prevBreakpoint, breakpoint, lineNum);
      
      if (!exists) {
        cout << "no such line" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      //
      // okay, the breakpoint line exists, so let's set the breakpoint
      // by severing the program graph between prevStmt and curStmt:
      //
      cout << "prevbreakpoint" << prevBreakpoint ->line<<endl;
      cout << "breakpoint " << breakpoint->line <<endl;
      linkOrUnlinkStmts(prevBreakpoint, nullptr);
      Breakpoints[lineNum] = false; // add the bp to the map and set it to false aka never hit before
      cout << "breakpoint set" << endl;
    }
    else if (cmd == "rb") {
      
      int lineNum;
      cin >> lineNum;
      
      if (breakpoint == nullptr) // we have no breakpoint
        cout << "no such breakpoint" << endl;
      else if (breakpoint->line != lineNum) // not the same breakpoint
        cout << "no such breakpoint" << endl;
      else {  // removing current breakpoint:
        //
        // we remove by linking the program graph back together:
        //
        linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
        prevBreakpoint = nullptr;
        breakpoint = nullptr;
     
        cout << "breakpoint removed" << endl;
      }
    }
    else if (cmd == "lb") {
      
      if (breakpoint == nullptr) {
        cout << "no breakpoints" << endl;
      } else {// we have a breakpoint:
        cout << "breakpoints on lines:"; 
        // for each breakpoint line number in set of breakpoints
        for (auto iter : Breakpoints) { // iter in this case is a line number 
          cout << " " << iter.first << endl;
        }
        cout << endl;
      }
    }
    else if (cmd == "cb") {
      
      if (breakpoint != nullptr) { // we have a breakpoint to clear:
        //
        // we clear by linking the program graph back together:
        //
        linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
        //reset breakpoint data
        prevBreakpoint = nullptr;
        breakpoint = nullptr;
      }
      
      cout << "breakpoints cleared" << endl;
    }
    else if (cmd == "p") {
      
      string varname;
      cin >> varname;
      
      const char* name = varname.c_str();
      
      struct RAM_VALUE* value = ram_read_cell_by_name(this->Memory, (char*) name);
      
      if (value == nullptr) {
        cout << "no such variable" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      print_value(varname, value);
      ram_free_value(value);
    }
    else if (cmd == "sm") {
      
      ram_print(this->Memory);
    }
    else if (cmd == "ss") {
      
      cout << this->State << endl;
    }
    else if (cmd == "w") {
      
      if (this->State == "Loaded") {
        cout << "line " << this->Program->line << endl;
        // struct STMT* stop = get_next_statement(curStmt);
        // linkOrUnlinkStmts(curStmt, stop);
        programgraph_print(curStmt);
        
      } else if (this->State == "Completed") {
        cout << "completed execution" << endl;
      } else {// we are running:
        //struct STMT* nextStmt = get_next_statement(curStmt); // create a variable that stores the next stmt
        if (nextStmt != nullptr) {
          cout << "line " << curStmt->line << endl;
          //cout << "next" << nextStmt->line << endl; //DEBUG
          linkOrUnlinkStmts(curStmt, nullptr); // unlink
          //cout << (nullptr == get_next_statement(curStmt)) << endl; //DEBUG 0 means its not unlinked prop and 1 means that its unlinked
          programgraph_print(curStmt); // print one line
          linkOrUnlinkStmts(curStmt, nextStmt); // link again
        } else { // reached the end of the program 
          cout << "completed execution" << endl; 
        } 

      }



    // } else if (cmd == "s") { // this will execute a ingle statement at a time
    //   if (this->State == "Completed") {
    //     cout << "program has completed" << endl;
    //     return;
    //   } else if (this->State == "Loaded") {
    //     this->State = "Running"; // change state to running
    //     curStmt = this->Program; // set the current stmt equal to the beginning of the program
    //   } 
    //   // no else statement for if the state is Running is necessary becausr curStmt is already pointing to the curStmt 
    //   // and there is no need to reset it in the running state when we just did that if it was in the loading state
      
    //   execute_single_step(); // call on helper function
    } else {
      cout << "unknown command" << endl;
    }

  }//while
  
  //
  // at this point execution has completed (or the user quit
  // early). If there's still a breakpoint, we need to repair
  // the program graph so the memory is freed properly when
  // we return to main:
  //
  if (breakpoint != nullptr) { // there's still a breakpoint
    linkOrUnlinkStmts(prevBreakpoint, breakpoint);
    
    prevBreakpoint = nullptr; // not really necessary, but just to be safe
    breakpoint = nullptr;
  }
  
}//run





// working during class
// no such line problem fixed kind of 
// getting s s s w to work

/*debugger.cpp*/

//
// Debugger for nuPython, C++ edition! Provides a simple gdb-like
// interface, with support for one breakpoint in straight-line code.
// Uses nuPython interpreter as execution engine.
//
// Authors:
//    Ata Guvendi
//    Prof. Hummel
// 
// Northwestern University
// CS 211
//

#include <iostream>
#include <cassert>

#include "debugger.h"
#include "execute.h"

using namespace std;


//
// print_value
//
// Prints the contents of a RAM cell, both type and value.
//
void Debugger::print_value(string varname, struct RAM_VALUE* value)
{
  cout << varname << " ("; 
  
  switch (value->value_type) {
    
    case RAM_TYPE_INT:
      cout << "int): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_REAL:
      cout << "real): " << value->types.d << endl;
      break;
      
    case RAM_TYPE_STR:
      cout << "str): " << value->types.s << endl;
      break;
      
    case RAM_TYPE_PTR:
      cout << "ptr): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_BOOLEAN:
      cout << "bool): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_NONE:
      cout << "none): " << "None" << endl;
      break;
  }//switch
}


//
// findBreakpoint
//
// starts from the stmt denoted by breakpoint, and searches forward
// for a stmt with the given line number. As it advances, prev is
// updated to always denote the previous statement. The parameters
// are passed-by-reference so they are updated back in the caller.
//
// Note that if breakpoint happens to match immediately, without advancing,
// then prev does not get set. So the caller should probably set prev 
// appropriately before calling (either nullptr, or to the previous stmt).
//
bool Debugger::findBreakpoint(struct STMT*& prev, struct STMT*& breakpoint, int lineNum)
{
  while (breakpoint != nullptr) {
    
    // cout << breakpoint->line << endl;
    
    if (breakpoint->line == lineNum) // found it!
      return true;
      
    prev = breakpoint;
    
    if (breakpoint->stmt_type == STMT_ASSIGNMENT)
      breakpoint = breakpoint->types.assignment->next_stmt;
    else if (breakpoint->stmt_type == STMT_FUNCTION_CALL)
      breakpoint = breakpoint->types.function_call->next_stmt;
    else {
      assert(breakpoint->stmt_type == STMT_PASS);
      breakpoint = breakpoint->types.pass->next_stmt;
    }
  }//while
  
  // if get here, not found
  return false;
}


//
// linkOrUnlinkStmts
//
// updates prev stmt to point to whatever cur is --- this can be 
// nullptr to unlink, or an actual stmt to link them back together.
//
void Debugger::linkOrUnlinkStmts(struct STMT* prev, struct STMT* cur)
{
  if (prev == nullptr) // nothing to link or unlink:
    return;
    
  if (prev->stmt_type == STMT_ASSIGNMENT)
    prev->types.assignment->next_stmt = cur;
  else if (prev->stmt_type == STMT_FUNCTION_CALL)
    prev->types.function_call->next_stmt = cur;
  else {
    assert(prev->stmt_type == STMT_PASS);
    prev->types.pass->next_stmt = cur;
  }
}



/* get_next_statement
when getting the next statement it checks what type it is as every type has a differnet way of getting next statement
Moves to the next statement by checking the type of the current statement (curr).

parameter: stmt
returns: struct
*/
struct STMT* Debugger::get_next_statement(struct STMT* stmt) {
  if (stmt->stmt_type == STMT_ASSIGNMENT) {
      return stmt->types.assignment->next_stmt;
  } else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
      return stmt->types.function_call->next_stmt;
  } else if (stmt->stmt_type == STMT_PASS) {
      return stmt->types.pass->next_stmt;
  } else {
      return nullptr;
  }
} 

/* find_prevStmt
finds the previous statement from a current statement or "target"

parameters: stmt
returns: stmt
*/
struct STMT* Debugger::find_prevStmt(struct STMT* target) {
  struct STMT* prev = nullptr;
  struct STMT* current = this->Program;

  while (current != nullptr && current != target) {
    prev = current;
    current = get_next_statement(current);  // Use get_next_statement to move to the next
  }

  return prev;  // Returns nullptr if target is the first statement
};

/* execute_steps
checks what to do based on if the command is s or r

parameters: string
returns: bool
*/
bool Debugger::execute_steps(string cmd){
  // initial check for beginning of the program
  if (nextStmt == nullptr) {
    if (this->State == "Loaded") { // Beginning of program
      nextStmt = get_next_statement(curStmt); // Initialize first step
      this->State = "Running";
      // cout << "Program started, state changed to " << this->State << endl; // debug
    }
  }
  
  
  if (cmd == "s") {
    // cout << "entered an s command. calling execute_single_step()"<< endl; // debug
    this->State = "Running";
    //return execute_single_step(cmd); // Return true if the program completes, otherwise false
    //return false; // exit after a single step if the command is "s"
  
    bool completed = execute_single_step(cmd);

    if (curStmt == nullptr && nextStmt == nullptr && completed) {
      this->State = "Completed";
      // cout << "Program completed, state changed to " << this->State << endl; // debug
      return true;
    }

    return completed; // return true if the program has completed
  }
  
  // for the r command, continue to run until completion or breakpoint is hit
  while (!execute_single_step(cmd)) {
    // the loop will continue until execute_single_step() returns true indicating the program has completed
  } 

  if(curStmt == nullptr && nextStmt == nullptr) {
    if (this->State == "Running") { // End of program
      this->State = "Completed";
      // cout << "Program completed, state changed to " << this->State << endl; // debug
      return true; // End program
    }
  }
  
  return true; // default return if command is r and the loop should continue
}

/* execute_single_step
executes single step. this is used for our s and r command

parameters: string
returns: bool
*/
bool Debugger::execute_single_step(const string& cmd) {
  // 1. breakpoint check: if the curStmt is a breakpoint then outputs false and stops
  if (Breakpoints.find(curStmt->line) != Breakpoints.end()) {
    // cout << "hit breakpoint on line " << curStmt->line << endl; // debug
    auto it = Breakpoints.find(curStmt->line);
    if (it->second == false) {
      cout << "true was returned" << endl; // debug
      return true; // end this while !execute_single_step loop because we reached a bp and its never been hit before
    } else if (it->second == true) {
      cout << "it right now after searching current stmt is... " << it->first << endl;
      cout << "true was returned because we hit the bp and its already been hit before so keep looping through this while" << endl;
      //Breakpoints[curStmt->line] = false;
      //return false;
    }

    // if (cmd == "r") {
    //   nextStmt = get_next_statement(curStmt); // Set up for next step
    //   cout << "true was returned and execute single step is done because in execute_steps it will run until it single is true" << endl; // debug
    //   cout << "for if we click r AGAIN: cur stmt = " << curStmt->line<< endl; // debug
    //   cout << "for if we click r AGAIN: next stmt = " << nextStmt->line<< endl;// debug
    //   // curStmt = nextStmt;
    //   // nextStmt = get_next_statement(curStmt);
    //   cout << "for if we click r AGAIN: new cur stmt = " << curStmt->line<< endl;// debug
    //   cout << "for if we click r AGAIN: new next stmt = " << nextStmt->line<< endl;// debug
    //   return true; // Indicate that we hit a breakpoint and execution should pause
    // }

    // return false;
  } 

  // 2. set a temp breakpoint for s command
  nextStmt = get_next_statement(curStmt); 
  //only unlink if nextstmt is not nullptr and we are already running (not at the start)
  if (nextStmt != nullptr) {
    linkOrUnlinkStmts(curStmt, nullptr); //unlink curStmt from breakStmt temporarily
    // cout << "Temporarily unlinked curStmt from nextStmt" << endl; // debug
  }

  // 3. Execution check: if the execution of the curStmt fails, repair the graph and set state to completed and return true
  struct ExecuteResult er = execute(curStmt, this->Memory);
  // cout << "execution is run... " << endl; // debug
  
  //step 3.7) if execute failed, signal completion
  //link does not be restored because it was already restored above
  if (!er.Success) { // execution failed aka semantic error, repair the graph
    //cout << "execution failed!!!!!!!!!!" << endl; // debug
    this->State = "Completed";
    return true;
  }

  // step 3.5) relink the curstmt from nextstmt then advance
  // cout << "curStmt: " << curStmt->line<< endl; // debug
  // if (nextStmt == nullptr) { // debug
  // cout << "nextStmt is NULLPTR" << endl;
  // }
  
  cout << "linking cur" << curStmt->line<<endl;
  //cout <<"linking next" << nextStmt->line <<endl;
  linkOrUnlinkStmts(curStmt, nextStmt); // restore link between curStmt and breakstmt 
  // cout << "link restored" << endl; // debug
  // attempt to advance to the next Statement
  if (nextStmt != nullptr) {
    // cout << "nextStmt is not a nullptr so enter..." << endl; // debug
    curStmt = nextStmt;
    // cout << "curStmt changed to -> " << curStmt->line << endl; // debug
    nextStmt = get_next_statement(curStmt);
    // if (nextStmt == nullptr) {
    //   cout << "nextStmt is A nullptr" << endl; // debug
    // } else {
    //   cout << "nextStmt changed to -> " << nextStmt->line << endl; // debug
    // }

  } else { // nextStmt = nullptr
  // If nextStmt is nullptr, check if curStmt is a breakpoint
    // // debug check to see what the breakpoints set looks like
    // for (auto iter : Breakpoints) {
    //   cout << iter << " " << endl;
    // }
    // If nextStmt is nullptr, find the next executable line using int_to_stmt
    auto nextLineIt = int_to_stmt.upper_bound(curStmt->line); // Get next line in map
    
    if (nextLineIt != int_to_stmt.end()) {
      cout << "yay! the line that we broke at exists in the map of program lines" << endl; // debug
      curStmt = nextLineIt->second; // Advance to the next executable statement
      cout << "curStmt is set to " << curStmt->line<< "for look up" << endl; // debug
      nextStmt = get_next_statement(curStmt);
      //debug
      if (nextStmt == nullptr) {
        cout << "nextStmt is A nullptr" << endl; // debug
      } else {
        cout << "nextStmt changed to -> " << nextStmt->line << endl; // debug
      }
      
      // Check if this new line is a breakpoint
      if (Breakpoints.find(curStmt->line) != Breakpoints.end()) {
        cout << "curStmt is ..." << curStmt-> line << endl;
        cout << "false is returned and breakpoint found at line " << curStmt->line << endl; // debug
        cout << "hit breakpoint on line " << curStmt->line << endl;
        programgraph_print(curStmt);
        Breakpoints[curStmt->line] = true;
        
        return true; // Stop at the breakpoint
      } else {
        // cout << "state changed to completed because nextStmt is nullptr!!!!!!!!!!" << endl; // debug
        this->State = "Completed";
        return true; // Program end reached
      }
    }

    if (nextLineIt == int_to_stmt.end()) {
      //cout << "No further lines in int_to_stmt; marking program as complete." << endl; // debug
      curStmt = nullptr; // Signal the true end of the program
      return true;
    }
  }
  // cout << "false is returned for execute_single_step" << endl; // debug
  return false; // continue if there are more statements
}


//
// constructor:
//
Debugger::Debugger(struct STMT* program)
  : State("Loaded"), Program(program), Memory(nullptr), Breakpoints(), curStmt(this->Program), nextStmt(nullptr)
{
  this->Memory = ram_init();

  // fill the map with every line in the program
  // map<int, struct STMT&> int_to_stmt
  struct STMT* current = this->Program;
  while (current != nullptr) {
    int_to_stmt[current->line] = current;
    current= get_next_statement(current);
  }

  // // debug to check if the map was populated correctly
  // for (auto pair : int_to_stmt) {
  //   cout << pair.first<< " : " << pair.second->line<< endl;
  // }
}

//
// destructor:
//
Debugger::~Debugger()
{
  ram_destroy(this->Memory);
}


//
// run:
//
// Run the debugger for one execution run of the input program.
//
void Debugger::run()
{
  string cmd;
  
  // //
  // // controls where we start execution from:
  // //
  // struct STMT* curStmt = this->Program;
  
  //
  // if we have a breakpoint, these will be set to the breakpoint,
  // and the statement preceding it:
  //
  // struct STMT* prevBreakpoint = nullptr;
  // struct STMT* breakpoint = nullptr;
  
  //
  // command loop until quit is entered:
  //
  while (true) {
    cout << endl;
    cout << "Enter a command, type h for help. Type r to run. > ";
    cin >> cmd;
    
    if (cmd == "q") { // consider including something to relink the graph and reset breakpoint data to prevent memory leaks
      break;
    }
    else if (cmd == "h") {
      
      cout << "Available commands:"
      << endl << "r -> Run the program / continue from a breakpoint"
      << endl << "s -> Step to the next stmt by executing current stmt"
      << endl << "b n -> Breakpoint at line n" 
      << endl << "rb n -> Remove breakpoint at line n"
      << endl << "lb -> List all breakpoints"
      << endl << "cb -> Clear all breakpoints"
      << endl << "p varname -> Print variable"
      << endl << "sm -> Show memory contents"
      << endl << "ss -> Show state of debugger"
      << endl << "w -> What line are we on?"
      << endl << "q -> Quit the debugger"
      << endl;
    } else if (cmd == "r" or cmd == "s") {
      if (this->State == "Completed") {
        cout << "program has completed" << endl;
        continue;
      } 

      execute_steps(cmd);



    } else if (cmd == "b") {
      
      int lineNum;
      cin >> lineNum;
      
      //search the set to see if this breakpoint already exists
      if (Breakpoints.find(lineNum) != Breakpoints.end()) { // if it already exists in set
        cout << "breakpoint already set" << endl;
        continue;
      } 

      // if (breakpoint != nullptr) { // sorry, we already have a breakpoint set:
      //   cout << "breakpoint already set" << endl;
      //   continue;  // skip code below and continue with next cmd:
      // }
      
      //
      // Okay, so no breakpoint is set, so we have a valid program
      // graph that we can traverse.
      //
      // first, search the program for the breakpoint, along with
      // the stmt that precedes it:
      //

      // prevBreakpoint = nullptr;
      // breakpoint = this->Program; // start from first stmt so prev will get set:
      
      // bool exists = findBreakpoint(prevBreakpoint, breakpoint, lineNum);
      
      // // if (!exists) {
      //   cout << "prevbreakpoint ..." <<prevBreakpoint << endl;
      //   cout << "breakpoint ..." << breakpoint << endl;
      //   cout << "no such line" << endl;
      //   continue;  // skip code below and continue with next cmd:
      // }
      
      //
      // okay, the breakpoint line exists, so let's set the breakpoint
      // by severing the program graph between prevStmt and curStmt:
      //
      auto existsYES = int_to_stmt.find(lineNum); // exists will give us an iterator
      if (existsYES == int_to_stmt.end()) {
        cout << "no such line" << endl;
        continue;
      }

      struct STMT* breakpoint = existsYES->second;
      struct STMT* prevBreakpoint = nullptr;

      if (existsYES != int_to_stmt.begin()) {  // Ensure it's not the first line
        prevBreakpoint = prev(existsYES)->second;  // Get the previous statement
      }

      if (prevBreakpoint != nullptr) {
        linkOrUnlinkStmts(prevBreakpoint, nullptr);
      }

      Breakpoints[lineNum] = false; // add the bp to the map and set it to false aka never hit before
      cout << "breakpoint set" << endl;


      // //
      // // okay, the breakpoint line exists, so let's set the breakpoint
      // // by severing the program graph between prevStmt and curStmt:
      // //
      // cout << "prevbreakpoint" << prevBreakpoint ->line<<endl;
      // cout << "breakpoint " << breakpoint->line <<endl;
      // linkOrUnlinkStmts(prevBreakpoint, nullptr);
      // Breakpoints[lineNum] = false; // add the bp to the map and set it to false aka never hit before
      // cout << "breakpoint set" << endl;
    }
    // else if (cmd == "rb") {
      
    //   int lineNum;
    //   cin >> lineNum;
      
    //   if (breakpoint == nullptr) // we have no breakpoint
    //     cout << "no such breakpoint" << endl;
    //   else if (breakpoint->line != lineNum) // not the same breakpoint
    //     cout << "no such breakpoint" << endl;
    //   else {  // removing current breakpoint:
    //     //
    //     // we remove by linking the program graph back together:
    //     //
    //     linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
    //     prevBreakpoint = nullptr;
    //     breakpoint = nullptr;
     
    //     cout << "breakpoint removed" << endl;
    //   }
    // }
    // else if (cmd == "lb") {
      
    //   if (breakpoint == nullptr) {
    //     cout << "no breakpoints" << endl;
    //   } else {// we have a breakpoint:
    //     cout << "breakpoints on lines:"; 
    //     // for each breakpoint line number in set of breakpoints
    //     for (auto iter : Breakpoints) { // iter in this case is a line number 
    //       cout << " " << iter.first << endl;
    //     }
    //     cout << endl;
    //   }
    // }
    // else if (cmd == "cb") {
      
    //   if (breakpoint != nullptr) { // we have a breakpoint to clear:
    //     //
    //     // we clear by linking the program graph back together:
    //     //
    //     linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
    //     //reset breakpoint data
    //     prevBreakpoint = nullptr;
    //     breakpoint = nullptr;
    //   }
      
    //   cout << "breakpoints cleared" << endl;
    // }
    else if (cmd == "p") {
      
      string varname;
      cin >> varname;
      
      const char* name = varname.c_str();
      
      struct RAM_VALUE* value = ram_read_cell_by_name(this->Memory, (char*) name);
      
      if (value == nullptr) {
        cout << "no such variable" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      print_value(varname, value);
      ram_free_value(value);
    }
    else if (cmd == "sm") {
      
      ram_print(this->Memory);
    }
    else if (cmd == "ss") {
      
      cout << this->State << endl;
    }
    else if (cmd == "w") {
      
      if (this->State == "Loaded") {
        cout << "line " << this->Program->line << endl;
        // struct STMT* stop = get_next_statement(curStmt);
        // linkOrUnlinkStmts(curStmt, stop);
        programgraph_print(curStmt);
        
      } else if (this->State == "Completed") {
        cout << "completed execution" << endl;
      } else {// we are running:
        //struct STMT* nextStmt = get_next_statement(curStmt); // create a variable that stores the next stmt
        if (nextStmt != nullptr) {
          cout << "line " << curStmt->line << endl;
          //cout << "next" << nextStmt->line << endl; //DEBUG
          linkOrUnlinkStmts(curStmt, nullptr); // unlink
          //cout << (nullptr == get_next_statement(curStmt)) << endl; //DEBUG 0 means its not unlinked prop and 1 means that its unlinked
          programgraph_print(curStmt); // print one line
          linkOrUnlinkStmts(curStmt, nextStmt); // link again
        } else { // reached the end of the program 
          cout << "completed execution" << endl; 
        } 

      }



    // } else if (cmd == "s") { // this will execute a ingle statement at a time
    //   if (this->State == "Completed") {
    //     cout << "program has completed" << endl;
    //     return;
    //   } else if (this->State == "Loaded") {
    //     this->State = "Running"; // change state to running
    //     curStmt = this->Program; // set the current stmt equal to the beginning of the program
    //   } 
    //   // no else statement for if the state is Running is necessary becausr curStmt is already pointing to the curStmt 
    //   // and there is no need to reset it in the running state when we just did that if it was in the loading state
      
    //   execute_single_step(); // call on helper function
    } else {
      cout << "unknown command" << endl;
    }

  }//while
  
  //
  // at this point execution has completed (or the user quit
  // early). If there's still a breakpoint, we need to repair
  // the program graph so the memory is freed properly when
  // we return to main:
  //
  // if (breakpoint != nullptr) { // there's still a breakpoint
  //   linkOrUnlinkStmts(prevBreakpoint, breakpoint);
    
  //   prevBreakpoint = nullptr; // not really necessary, but just to be safe
  //   breakpoint = nullptr;
  // }
  
}//run