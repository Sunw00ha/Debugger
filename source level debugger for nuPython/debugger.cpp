/*debugger.cpp*/
//
// hold implementation of debugger class
// 
// Irene Ha
// Northwestern University
// CS 211

// Original author: Prof. Joe Hummel

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
  if (stmt == nullptr) {
    return nullptr;
  }
  
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


/* execute_single_step
executes single step. this is used for our s and r command

parameters: string
returns: bool
*/
bool Debugger::execute_single_step(const string& cmd) {
  // initial check for beginning of the program
  if (State == "Completed") {
    cout << "program has completed" << endl; // i think this part needs to he changed to program has completed
    return true;
  } else { // state right now is loaded
    State = "Running";
  }
  
  while (curStmt != nullptr) {
    // 1. breakpoint check: if the curStmt is a breakpoint then outputs false and stops
    //cout << "cur stmt ..." << curStmt->line << endl;
    if (Breakpoints.find(curStmt->line) != Breakpoints.end()) {
      

      
      auto it = Breakpoints.find(curStmt->line);
      if (it->second == false) {
        cout << "hit breakpoint at line " << curStmt->line << endl;
        linkOrUnlinkStmts(curStmt, nullptr);
        programgraph_print(curStmt);
        linkOrUnlinkStmts(curStmt, nextStmt);
        Breakpoints[curStmt->line] = true;
        //cout << "HIT HERE" << endl;
        return true; // end this while !execute_single_step loop because we reached a bp and its never been hit before
      } else if (it->second == true) {
        Breakpoints[curStmt->line] = false;
        //cout << "true was returned because we hit the bp and its already been hit before so keep looping through this while" << endl;
        //Breakpoints[curStmt->line] = false;
        //return false;
      }

    } 
    //cout << "broke out of here" << endl; // debug

    // assuming already unlinked
    // 3. Execution check: if the execution of the curStmt fails, repair the graph and set state to completed and return true
    // cout << "** execute is running from curStmt: " << curStmt->line<< " and was broken here so cur stmt is pointing to null" << endl; //debug
    linkOrUnlinkStmts(curStmt, nullptr); // break
    struct ExecuteResult er = execute(curStmt, this->Memory);
    // cout << "execution was run" << endl; // debug
    
    //step 3.7) if execute failed, signal completion
    //link does not be restored because it was already restored above
    if (!er.Success) { // execution failed aka semantic error, repair the graph
      //cout << "execution failed!!!!!!!!!!" << endl; // debug
      this->State = "Completed";
      // cout << "this happened and state changed to COMPLETED" <<endl;
      linkOrUnlinkStmts(curStmt, nextStmt);
      return true;
    }
    linkOrUnlinkStmts(curStmt, nextStmt); // relink
    // cout << "cur stmt  " << curStmt->line << "  and nextStmt  " << "nextStmt->line" << "  have been relinked"<< endl;

    //* advance cur stmt
    curStmt = nextStmt; 
    // //debug 
    // if (curStmt == nullptr) {
    //   cout << "curStmt is nullptr" << endl;
    // } else {
    //   cout << "cur stmt now is ..." << curStmt->line <<endl; //debug
    // }
    
    // advance next stmt
    nextStmt = get_next_statement(curStmt);
    // //debug
    // if (nextStmt == nullptr) {
    //   cout << "next stmt is nullptr" << endl;
    // } else {
    //   cout << "next stmt now is ..." << nextStmt->line <<endl; //debug
    // }
    
    // if ((curStmt != nullptr) && (nextStmt != nullptr)) {
    // linkOrUnlinkStmts(curStmt, nullptr);
    // cout << "unlinked     " << curStmt->line<< "    from rest of graph" <<endl; //debug
    // } 
    // //debug
    // else if ((curStmt == nullptr) && (nextStmt == nullptr)){
    //   cout << "we didnt unlink because we ran to the end where cur is null and next is null " << endl;
      
    // }

    if (cmd == "s") {
      // cout << "command s so broken out of while" << endl; // debug 
      break;
    }
  }
  
  // change state to completed if this condition is true because then we ACTUALLY know it ended
  if (curStmt == nullptr && nextStmt == nullptr) {
    //cout << "hello!!!!!!!!!!!!!!!!!!!!" << endl; // debug
    State = "Completed";
  }
  return true;
}


//
// constructor:
//
Debugger::Debugger(struct STMT* program)
  : State("Loaded"), Program(program), Memory(nullptr), Breakpoints()
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
  // struct STMT* current = this->Program;
  //   while(current != nullptr) {
  //     struct STMT* nextStmt = get_next_statement(current);
  //     linkOrUnlinkStmts(current, nextStmt);
  //     current = nextStmt;
  //   }

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
  curStmt = this->Program; 
  nextStmt = get_next_statement(curStmt);
  linkOrUnlinkStmts(curStmt, nullptr); // break the first and second line to start
  
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
    cout << "Enter a command, type h for help. Type r to run. > " << endl;
    cin >> cmd;
    
    if (cmd == "q") { // consider including something to relink the graph and reset breakpoint data to prevent memory leaks
      linkOrUnlinkStmts(curStmt, nextStmt);
      break;
    }
    else if (cmd == "h") {
      
      cout << "Available commands:"
      << endl << "r -> Run the program / continue from a breakpoint"
      << endl << "s -> Step to next stmt by executing current stmt"
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
      execute_single_step(cmd); // use helper

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
      linkOrUnlinkStmts(curStmt, nextStmt);
      bool exists = findBreakpoint(prevBreakpoint, breakpoint, lineNum);
      linkOrUnlinkStmts(curStmt, nullptr);
      
      if (!exists) {
        cout << "no such line" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      //
      // okay, the breakpoint line exists, so let's set the breakpoint
      // by severing the program graph between prevStmt and curStmt:
      //
      // cout << "prevbreakpoint" << prevBreakpoint ->line<<endl;
      // cout << "breakpoint " << breakpoint->line <<endl;
      //linkOrUnlinkStmts(prevBreakpoint, nullptr);
      Breakpoints[lineNum] = false; // add the bp to the map and set it to false aka never hit before
      cout << "breakpoint set" << endl;
    }
    else if (cmd == "rb") {
      
      int lineNum;
      cin >> lineNum;

      if (Breakpoints.find(lineNum) != Breakpoints.end()) {
        Breakpoints.erase(lineNum);
        cout << "breakpoint removed" << endl;
      } else {
        cout << "no such breakpoint" << endl;
      }
      
      // if (breakpoint == nullptr) // we have no breakpoint
      //   cout << "no such breakpoint" << endl;
      // else if (breakpoint->line != lineNum) // not the same breakpoint
      //   cout << "no such breakpoint" << endl;
      // else {  // removing current breakpoint:
      //   //
      //   // we remove by linking the program graph back together:
      //   //
      //   linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
      //   prevBreakpoint = nullptr;
      //   breakpoint = nullptr;
     
      //   cout << "breakpoint removed" << endl;
      }
    
    else if (cmd == "lb") {
      if (Breakpoints.size() == 0) {
        cout << "no breakpoints" << endl;
      } else {// we have breakpoints:
        cout << "breakpoints on lines: ";
        for (auto iter : Breakpoints) { // iter in this case is a line number 
          cout << iter.first << " ";
        }
        cout << endl;
      }
    }
    else if (cmd == "cb") {
      
      // if (breakpoint != nullptr) { // we have a breakpoint to clear:
      //   //
      //   // we clear by linking the program graph back together:
      //   //
      //   linkOrUnlinkStmts(prevBreakpoint, breakpoint);
        
      //   //reset breakpoint data
      //   prevBreakpoint = nullptr;
      //   breakpoint = nullptr;
      // }

      Breakpoints.clear();
      
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
        // linkOrUnlinkStmts(curStmt, nextStmt);
        
      } else if (this->State == "Completed") {
        cout << "completed execution" << endl;
      } else if (this->State == "Running") {// we are running:
        // cout << "entered the w if running condition" << endl; // debug
        
        // ////// DEBUGGGGGG
        // if (curStmt == nullptr) {
        //   cout << "cur is a nullptr" << endl;
        // } else {
        //   cout << "cur is line " << curStmt->line << endl;
        // }

        // if (nextStmt == nullptr) {
        //   cout << "next is a nullptr" << endl;
        // } else {
        //   cout << "next is line " << nextStmt->line << endl;
        // }
        // ////// DEBUGGGGGG


        if ((nextStmt != nullptr) or (curStmt != nullptr)) { // print out the line number and unlink before print and relink after print
          cout << "line " << curStmt->line << endl;
          //cout << "next" << nextStmt->line << endl; //DEBUG
          
          linkOrUnlinkStmts(curStmt, nullptr); // unlink
          
          //cout << (nullptr == get_next_statement(curStmt)) << endl; //DEBUG 0 means its not unlinked prop and 1 means that its unlinked
          programgraph_print(curStmt); // print one line
          linkOrUnlinkStmts(curStmt, nextStmt); // link again
        } 
        else if ((nextStmt == nullptr) && (curStmt == nullptr)) { // reached the end of the program 
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
    //cout << "HITS HERE" << endl;
    prevBreakpoint = nullptr; // not really necessary, but just to be safe
    breakpoint = nullptr;
  }
  
}//run