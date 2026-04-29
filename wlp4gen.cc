#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib>

std::pair<std::vector<std::string>, std::map<std::string, std::string>> active_table;
std::string fun_name;
int is_mips_array = -1;

struct Node {
  std::vector<Node*> children;
  std::vector<std::string> tokens;
  std::string rule;
  std::string lexme;
  std::string type;
};

std::string typecheck(Node*, std::map<std::string, 
                              std::pair<std::vector<std::string>, std::map<std::string, std::string>>>, 
                              Node*);

// freeing memory
void free_memory(Node *node) {
  int size = node->children.size();
  for(int i = 0; i <= size - 1; i++) {
    free_memory(node->children[i]);
  }
  delete node;
  node = nullptr;
}

// build parse tree
void build_tree(Node *node) {
  std::string token, rule;
  if (std::getline(std::cin, rule)) {
    std::string str;
    node->rule = rule;
    std::istringstream iss(rule);
    iss >> str;
    if(rule[0] >= 'A' && rule[0] <= 'Z') {
      iss >> str;
      node->lexme = str;
    } else {
      while(iss >> str) {
        node->tokens.push_back(str);
      }
      int size = node->tokens.size();
      for(int i = 0; i <= size - 1; i++) {
        node->children.emplace_back(new Node);
        build_tree(node->children[i]);
      }
    }
  }
}

void push(std::string reg) {
  std::cout << "sw " << reg << ", -4($30)\n";
  std::cout << "sub $30, $30, $4\n";
}

void pop(std::string reg) {
  std::cout << "add $30, $30, $4\n";
  std::cout << "lw " << reg << ", -4($30)\n";
}

// Epilogue
void epilogue(int num) {
  std::cout << "; begin epilogue\n";
  std::cout << "jr $31\n";
}

// Prologue
void prologue(std::map<std::string, std::pair<std::vector<std::string>, std::map<std::string, std::string>>> tables) {
  std::cout << "; begin prologue\n";
  std::cout << ".import init\n";
  std::cout << ".import new\n";
  std::cout << ".import delete\n";
  std::cout << ".import print\n"; 
  std::cout << "lis $4\n";
  std::cout << ".word 4\n";
  std::cout << "lis $10\n";
  std::cout << ".word print\n";
  std::cout << "lis $11\n";
  std::cout << ".word 1\n";
  std::cout << "sub $29 , $30 , $4  ; setup frame pointer\n";
  push("$2");
  if (tables["wain"].first[0] == "int") {
    std::cout << "add $2, $0, $0" << std::endl;
  }
  push("$31");
  std::cout << "lis $5" << std::endl;
  std::cout << ".word init" << std::endl;
  std::cout << "jalr $5" << std::endl;
  pop("$31");
  pop("$2");
  std::cout << "; end Prologue and begin Body\n";
}

// get vector of arguments passed to a function
void getArglist(Node* node, std::vector<std::string> &actual, 
std::map<std::string, std::pair<std::vector<std::string>, 
                      std::map<std::string, std::string>>> tables,
Node *root) {
  if (node->rule == "arglist expr") {
    actual.push_back(typecheck(node->children[0], tables, root));
  } else if (node->rule == "arglist expr COMMA arglist") {
    actual.push_back(typecheck(node->children[0], tables, root));
    getArglist(node->children[2], actual, tables, root);
  }
}

// process the parameters of a function
void get_paramlist(Node *node, Node *root) {
  if (node->children[0]->children[0]->rule == "type INT") {
    if(active_table.second[node->children[0]->children[1]->lexme] == "") {
      active_table.second[node->children[0]->children[1]->lexme] = "int";
      active_table.first.push_back("int");
    } else {
      std::cerr << "ERROR: Redeclaration of indentifier: ";
      std::cerr << node->children[0]->children[1]->lexme << "\n";
    }
  } else if (node->children[0]->children[0]->rule == "type INT STAR") {
    if(active_table.second[node->children[0]->children[1]->lexme] == "") {
      active_table.second[node->children[0]->children[1]->lexme] = "int*";
      active_table.first.push_back("int*");
    } else {
      std::cerr << "ERROR: Redeclaration of identifier: ";
      std::cerr << node->children[0]->children[1]->lexme << "\n";
    }
  }
  if (node->rule == "paramlist dcl COMMA paramlist") {
    get_paramlist(node->children[2], root);
  }
}

// build the symbol table
void build_symbol_table(Node *node, std::map<std::string, 
                          std::pair<std::vector<std::string>, 
                            std::map<std::string, std::string>>> &tables, 
                        Node *root) {
  if(node->rule == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
    
    if (node->children[3]->children[0]->rule == "type INT") {
      if(active_table.second[node->children[3]->children[1]->lexme] == "") {
        active_table.second[node->children[3]->children[1]->lexme] = "int";
        active_table.first.push_back("int");
      } else {
        std::cerr << "ERROR: Redeclaration of identifier: ";
        std::cerr << node->children[3]->children[1]->lexme << "\n";
      }
    } else if (node->children[3]->children[0]->rule == "type INT STAR") {
      if(active_table.second[node->children[3]->children[1]->lexme] == "") {
        active_table.second[node->children[3]->children[1]->lexme] = "int*";
        active_table.first.push_back("int*");
      } else {
        std::cerr << "ERROR: Redeclaration of identifier: ";
        std::cerr << node->children[3]->children[1]->lexme << "\n";
      }
    }
    if (node->children[5]->children[0]->rule == "type INT") {
      if(active_table.second[node->children[5]->children[1]->lexme] == "") {
        active_table.second[node->children[5]->children[1]->lexme] = "int";
        active_table.first.push_back("int");
      } else {
        std::cerr << "ERROR: Redeclaration of identifier: ";
        std::cerr << node->children[5]->children[1]->lexme << "\n";
      }
    } else if (node->children[5]->children[0]->rule == "type INT STAR") {
      if(active_table.second[node->children[5]->children[1]->lexme] == "") {
        active_table.second[node->children[5]->children[1]->lexme] = "int*";
        active_table.first.push_back("int*");
      } else {
        std::cerr << "ERROR: Redeclaration of identifier: ";
        std::cerr << node->children[5]->children[1]->lexme << "\n";
      }
    }

    int size = node->children.size();
    for(int i = 8; i <= size - 1; i++) {
      build_symbol_table(node->children[i], tables, root);
    }

    tables["wain"] = active_table;
    active_table.first.clear(); 
    active_table.second.clear();
  
  } else if (node->rule == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
    fun_name = node->children[1]->lexme;
    if(tables.count(fun_name) != 0) {
      std::cerr << "ERROR: Redeclaration of function: " << fun_name << "\n";
    } else {
      if(node->children[3]->children.size() != 0) {
        get_paramlist(node->children[3]->children[0], root);
      }

      int size = node->children.size();
      for(int i = 6; i <= size - 1; i++) {
        build_symbol_table(node->children[i], tables, root);
      }

      tables[fun_name] = active_table;
      active_table.first.clear(); 
      active_table.second.clear();
    }
    
  } else if (node->rule == "dcl type ID") {
    if (node->children[0]->rule == "type INT") {
      if(active_table.second[node->children[1]->lexme] == "") {
        active_table.second[node->children[1]->lexme] = "int";
      } else {
        std::cerr << "ERROR: Redeclaration of identifier: ";
        std::cerr << node->children[1]->lexme << "\n";
      }
    } else if (node->children[0]->rule == "type INT STAR") {
      if(active_table.second[node->children[1]->lexme] == "") {
        active_table.second[node->children[1]->lexme] = "int*";
      } else {
        std::cerr << "ERROR: Redeclaration of identifier: ";
        std::cerr << node->children[1]->lexme << "\n";
      }
    }
  } else if (node->rule == "factor ID LPAREN RPAREN" || 
            node->rule == "factor ID LPAREN arglist RPAREN") {
    if(active_table.second.count(node->children[0]->lexme) != 0) {
      std::cerr << "ERROR: Function not found: ";
      std::cerr << node->children[0]->lexme << "\n";
    }
    if (tables.count(node->children[0]->lexme) == 0 && 
        fun_name != node->children[0]->lexme) {
      std::cerr << "ERROR: Function not found: ";
      std::cerr << node->children[0]->lexme << "\n";
    }
    int size = node->children.size();
    for(int i = 1; i <= size - 1; i++) {
      build_symbol_table(node->children[i], tables, root);
    }
  } else if (node->rule[0] == 'I' && node->rule[1] == 'D') {
    if (active_table.second.count(node->lexme) == 0) {
      std::cerr << "ERROR: Unidentified identifier: " << node->lexme << "\n";
    }
  } else {     
    int size = node->children.size();
    for(int i = 0; i <= size - 1; i++) {
      build_symbol_table(node->children[i], tables, root);
    }
  }
}

// check welltype() and type correctness
std::string typecheck(Node *node, std::map<std::string, std::pair<std::vector<std::string>, 
                      std::map<std::string, std::string>>> tables, Node *root) {
  if (node->rule[0] == 'N' && node->rule[1] == 'U' && node->rule[2] == 'M') {
    return "int";
  } else if (node->rule[0] == 'N' && node->rule[1] == 'U' && 
            node->rule[2] == 'L' && node->rule[3] == 'L' ) {
    return "int*";
  } else if (node->rule[0] == 'I' && node->rule[1] == 'D') {
    return tables[fun_name].second[node->lexme];
  } else if (node->rule == "procedures procedure procedures") {
    std::string ltype = typecheck(node->children[0], tables, root);
    std::string rtype = typecheck(node->children[1], tables, root);
    if (ltype != "correct procedure" || rtype == "correct procedure") {
      std::cerr << "ERROR: error in procedure\n";
      return "ERROR";
    }
    return "valid program";
  } else if (node->rule == "procedures main") {
    std::string ltype = typecheck(node->children[0], tables, root);
    if (ltype != "correct wain") {
      std::cerr << "ERROR: error in wain\n";
      return "ERROR";
    }
    return "valid program";
  } else if (node->rule == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
    fun_name = node->children[1]->lexme;
    std::string ltype = typecheck(node->children[9], tables, root);
    std::string dclstype = typecheck(node->children[6], tables, root);
    std::string stype = typecheck(node->children[7], tables, root);
    if (ltype != "int") {
      std::cerr << "ERROR: return expression should be int\n";
      return "ERROR";
    }
    if (dclstype != "correct") {
      std::cerr << "ERROR: dcls expression should be int\n";
      return "ERROR";
    }
    if (stype != "success") {
      std::cerr << "ERROR: statement expression should be int\n";
      return "ERROR";
    }
    return "correct procedure";
  } else if (node->rule == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
    fun_name = "wain";
    if (tables["wain"].second[node->children[5]->children[1]->lexme] != "int") {
      std::cerr << "ERROR: second parameter of wain should be int\n";
      return "ERROR";
    }
    std::string ltype = typecheck(node->children[11], tables, root);
    std::string dclstype = typecheck(node->children[8], tables, root);
    std::string stype = typecheck(node->children[9], tables, root);
    if (ltype != "int") {
      std::cerr << "ERROR (wain): return expression should be int\n";
      return "ERROR";
    }
    if (dclstype != "correct") {
      std::cerr << "ERROR (wain): dcls expression should be int\n";
      return "ERROR";
    }
    if (stype != "success") {
      std::cerr << "ERROR (wain): statement expression should be int\n";
      return "ERROR";
    }
    return "correct wain";    
  } else if (node->rule == "factor ID") {
    node->type = typecheck(node->children[0], tables, root);
    return node->type;
  } else if (node->rule == "factor NUM") {
    node->type = "int";
    return "int";
  } else if (node->rule == "factor NULL") {
    node->type = "int*";
    return "int*";
  } else if (node->rule == "factor LPAREN expr RPAREN") {
    node->type = typecheck(node->children[1], tables, root);
    return node->type;
  } else if (node->rule == "factor AMP lvalue") {
    std::string ltype = typecheck(node->children[1], tables, root);
    if (ltype != "int") {
      std::cerr << "ERROR (factor AMP lvalue): expected int but got int*\n";
      return "ERROR";
    }
    node->type = "int*";
    return "int*";
  } else if (node->rule == "factor STAR factor") {
    std::string ltype = typecheck(node->children[1], tables, root);
    if (ltype != "int*") {
      std::cerr << "ERROR (factor STAR factor): expected int* but got int\n";
      return "ERROR";
    }
    node->type = "int";
    return "int";
  } else if (node->rule == "factor NEW INT LBRACK expr RBRACK") {
    std::string ltype = typecheck(node->children[3], tables, root);
    if (ltype != "int") {
      std::cerr << "ERROR (factor NEW INT LBRACK expr RBRACK): expected int but got int*\n";
      return "ERROR";
    }
    node->type = "int*";
    return "int*";
  } else if (node->rule == "factor ID LPAREN RPAREN") {
    int num_args = tables[node->children[0]->lexme].first.size();
    if (num_args != 0) {
      std::cerr << "ERROR (factor ID LPAREN RPAREN): funtion signature does not match: " << node->children[0]->lexme << "\n";
      return "ERROR";
    }
    node->type = "int";
    return "int";
  } else if (node->rule == "factor ID LPAREN arglist RPAREN") {
    std::vector<std::string> expected = tables[node->children[0]->lexme].first;
    std::vector<std::string> actual;
    getArglist(node->children[2], actual, tables, root);
    if (expected != actual) {
      std::cerr << "ERROR (factor ID LPAREN arglist RPAREN): function signature does not match: " << node->children[0]->lexme << "\n";
      return "ERROR";
    }
    node->type = "int";
    return "int";
  } else if (node->rule == "lvalue ID") {
    node->type = typecheck(node->children[0], tables, root);
    return node->type;
  } else if (node->rule == "lvalue STAR factor") {
    std::string ltype = typecheck(node->children[1], tables, root);
    if (ltype != "int*") {
      std::cerr << "ERROR (lvalue STAR factor): expected int* but got int\n";
      return "ERROR";
    }
    node->type = "int";
    return "int";
  } else if (node->rule == "lvalue LPAREN lvalue RPAREN") {
    node->type = typecheck(node->children[1], tables, root);
    return node->type;
  } else if (node->rule == "term factor") {
    node->type = typecheck(node->children[0], tables, root);
    return node->type;
  } else if (node->rule == "term term STAR factor") {
    std::string ltype = typecheck(node->children[0], tables, root);
    std::string rtype = typecheck(node->children[2], tables, root);
    if (ltype != "int" || rtype != "int") {
      std::cerr << "ERROR (term term STAR factor): both should be int\n";
      return "ERROR";
    }
    node->type = "int";
    return "int";
  } else if (node->rule == "term term SLASH factor") {
    std::string ltype = typecheck(node->children[0], tables, root);
    std::string rtype = typecheck(node->children[2], tables, root);
    if (ltype != "int" || rtype != "int") {
      std::cerr << "ERROR (term term SLASH factor): both should be int\n";
      return "ERROR";
    }
    node->type = "int";
    return "int";
  } else if (node->rule == "term term PCT factor") {
    std::string ltype = typecheck(node->children[0], tables, root);
    std::string rtype = typecheck(node->children[2], tables, root);
    if (ltype != "int" || rtype != "int") {
      std::cerr << "ERROR (term term PCT factor): both should be int\n";
      return "ERROR";
    }
    node->type = "int";
    return "int";
  } else if (node->rule == "expr term") {
    node->type = typecheck(node->children[0], tables, root);;
    return node->type;
  } else if (node->rule == "expr expr PLUS term") {
    std::string ltype = typecheck(node->children[0], tables, root);
    std::string rtype = typecheck(node->children[2], tables, root);
    if (ltype == "int" && rtype == "int") {
      node->type = "int";
      return "int";
    } else if (ltype == "int*" && rtype == "int") {
      node->type = "int*";
      return "int*";
    } else if (ltype == "int" && rtype == "int*") {
      node->type = "int*";
      return "int*";
    } else {
      std::cerr << "ERROR (expr expr PLUS term): both int*\n";
      return "ERROR";
    }
  } else if (node->rule == "expr expr MINUS term") {
    std::string ltype = typecheck(node->children[0], tables, root);
    std::string rtype = typecheck(node->children[2], tables, root);
    if (ltype == "int" && rtype == "int") {
      node->type = "int";
      return "int";
    } else if (ltype == "int*" && rtype == "int") {
      node->type = "int*";
      return "int*";
    } else if (ltype == "int*" && rtype == "int*") {
      node->type = "int";
      return "int";
    } else {
      std::cerr << "ERROR (expr expr MINUS term): expr int and term int*\n";
      return "ERROR";
    }
  } else if (node->rule == "statement lvalue BECOMES expr SEMI") {
    std::string ltype = typecheck(node->children[0], tables, root);
    std::string rtype = typecheck(node->children[2], tables, root);
    if (ltype != rtype) {
      std::cerr << "ERROR (statement lvalue BECOMES expr SEMI): lhs and rhs should be same type\n";
      return "ERROR";
    }
    return "success";
  } else if (node->rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
    std::string validtest = typecheck(node->children[2], tables, root);
    std::string ltype = typecheck(node->children[5], tables, root);
    std::string rtype = typecheck(node->children[9], tables, root);
    if (ltype != "success" || rtype != "success" || validtest != "valid") {
      std::cerr << "ERROR (statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE): not well typed\n";
      return "ERROR";
    }
    return "success";
  } else if (node->rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
    std::string validtest = typecheck(node->children[2], tables, root);
    std::string ltype = typecheck(node->children[5], tables, root);
    if (ltype != "success" || validtest != "valid") {
      std::cerr << "ERROR (statement WHILE LPAREN test RPAREN LBRACE statements RBRACE): not well typed\n";
      return "ERROR";
    }
    return "success";
  } else if (node->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
    std::string ltype = typecheck(node->children[2], tables, root);
    if (ltype != "int") {
      std::cerr << "ERROR (statement PRINTLN LPAREN expr RPAREN SEMI): expression should be int\n";
      return "ERROR";
    }
    return "success";
  } else if (node->rule == "statement DELETE LBRACK RBRACK expr SEMI") {
    std::string ltype = typecheck(node->children[3], tables, root);
    if (ltype != "int*") {
      std::cerr << "ERROR (statement DELETE LBRACK RBRACK expr SEMI): expression should be int*\n";
      return "ERROR";
    }
    return "success";
  } else if (node->rule == "statements statements statement") {
    std::string ltype = typecheck(node->children[0], tables, root);
    std::string rtype = typecheck(node->children[1], tables, root);
    if (ltype != "success" || rtype != "success") {
      std::cerr << "ERROR (statements statements statement): expression should be int*\n";
      return "ERROR";
    }
    return "success";
  } else if (node->rule == "statements") {
    return "success";
  } else if (node->rule[0] == 't' && node->rule[1] == 'e' && node->rule[2] == 's' && node->rule[3] == 't') {
    std::string ltype = typecheck(node->children[0], tables, root);
    std::string rtype = typecheck(node->children[2], tables, root);
    if (ltype != rtype) {
      std::cerr << "ERROR (test): lhs and rhs should be same type\n";
      return "ERROR";
    }
    return "valid";
  } else if (node->rule == "dcls dcls dcl BECOMES NUM SEMI") {
    if(tables[fun_name].second[node->children[1]->children[1]->lexme] != "int") {
      std::cerr << "ERROR (dcls dcls dcl BECOMES NUM SEMI): dcl should be int\n";
      return "ERROR";
    }
    if (typecheck(node->children[0], tables, root) != "correct") {
      std::cerr << "ERROR (dcls dcls dcl BECOMES NUM SEMI): dcls not correct\n";
      return "ERROR";
    }
    return "correct";
  } else if (node->rule == "dcls dcls dcl BECOMES NULL SEMI") {
    if(tables[fun_name].second[node->children[1]->children[1]->lexme] != "int*") {
      std::cerr << "ERROR (dcls dcls dcl BECOMES NULL SEMI): dcl should be int*\n";
      return "ERROR";
    }
    if (typecheck(node->children[0], tables, root) != "correct") {
      std::cerr << "ERROR (dcls dcls dcl BECOMES NULL SEMI): dcls not correct\n";
      return "ERROR";
    }
    return "correct";
  } else if (node->rule == "dcls") {
    return "correct";
  }
}

// generate mips code
void generate(Node *node, std::map<std::string, std::pair<std::vector<std::string>, 
                      std::map<std::string, std::string>>> tables, int &lables,
                      std::map<std::string, std::map<std::string, int>> &offset_table, int &num) {
  if (node->rule[0] == 'N' && node->rule[1] == 'U' && node->rule[2] == 'M') {
    std::cout << "lis $3\n";
    std::cout << ".word " << node->lexme << "\n";
  } else if (node->rule == "procedures procedure procedures") {
    generate(node->children[1], tables, lables, offset_table, num);
    generate(node->children[0], tables, lables, offset_table, num);
  } else if (node->rule == "procedures main") {
    generate(node->children[0], tables, lables, offset_table, num);
  } else if (node->rule == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
    num = 0;
    fun_name = node->children[1]->lexme;
    generate(node->children[3], tables, lables, offset_table, num);
    std::cout << "F" << node->children[1]->lexme << ":\n";
    std::cout << "sub $29, $30, $4\n";
    generate(node->children[6], tables, lables, offset_table, num);
    generate(node->children[7], tables, lables, offset_table, num);
    generate(node->children[9], tables, lables, offset_table, num);
    std::cout << "add $30, $29, $4\n";
    std::cout << "jr $31\n";
  } else if (node->rule == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
    prologue(tables);
    offset_table[fun_name][node->children[3]->children[1]->lexme] = num * -4;
    num++;
    push("$1");
    offset_table[fun_name][node->children[5]->children[1]->lexme] = num * -4;
    num++;
    push("$2");
    generate(node->children[8], tables, lables, offset_table, num);
    generate(node->children[9], tables, lables, offset_table, num);
    generate(node->children[11], tables, lables, offset_table, num);
    pop("$2");
    pop("$1");
    epilogue(num);
  } else if (node->rule == "params paramlist") {
    generate(node->children[0], tables, lables, offset_table, num);
  } else if (node->rule == "paramlist dcl") {
    generate(node->children[0], tables, lables, offset_table, num);
  } else if (node->rule == "paramlist dcl COMMA paramlist") {
    generate(node->children[0], tables, lables, offset_table, num);
    generate(node->children[2], tables, lables, offset_table, num);
  } else if (node->rule == "dcl type ID") {
    int num_param = tables[fun_name].first.size();
    offset_table[fun_name][node->children[1]->lexme] = 4 * (num_param - num);
    num++;
  } else if (node->rule == "factor ID") {
    std::cout << "lw $3, " << offset_table[fun_name][node->children[0]->lexme] << "($29)  ; $3 <- " << node->children[0]->lexme << "\n";
  } else if (node->rule == "factor NUM") {
    generate(node->children[0], tables, lables, offset_table, num);
  } else if (node->rule == "factor NULL") {
    std::cout << "add $3, $0, $11\n";
  } else if (node->rule == "factor LPAREN expr RPAREN") {
    generate(node->children[1], tables, lables, offset_table, num);
  } else if (node->rule == "factor AMP lvalue") {
    if (node->children[1]->rule == "lvalue ID") {
      std::cout << "lis $3\n";
      std::cout << ".word " << offset_table[fun_name][node->children[1]->children[0]->lexme] << "\n";
      std::cout << "add $3, $3, $29\n";
    } else if (node->children[1]->rule == "lvalue STAR factor") {
      generate(node->children[1]->children[1], tables, lables, offset_table, num);
    }
  } else if (node->rule == "factor STAR factor") {
    generate(node->children[1], tables, lables, offset_table, num);
    std::cout << "lw $3, 0($3)\n";
  } else if (node->rule == "factor NEW INT LBRACK expr RBRACK") {
    generate(node->children[3], tables, lables, offset_table, num);
    std::cout << "add $1, $3, $0\n";
    push("$31");
    std::cout << "lis $5\n";
    std::cout << ".word new\n";
    std::cout << "jalr $5\n";
    pop("$31");
    std::cout << "bne $3, $0, 1\n";
    std::cout << "add $3, $11, $0\n";
  } else if (node->rule == "factor ID LPAREN RPAREN") {
    push("$29");
    push("$31");
    std::cout << "lis $5\n";
    std::cout << ".word F" << node->children[0]->lexme << "\n";
    std::cout << "jalr $5\n";
    pop("$31");
    pop("$29");
  } else if (node->rule == "factor ID LPAREN arglist RPAREN") {
    push("$29");
    push("$31");
    generate(node->children[2], tables, lables, offset_table, num);
    std::cout << "lis $5\n";
    std::cout << ".word F" << node->children[0]->lexme << "\n";
    std::cout << "jalr $5\n";
    int num_param = tables[node->children[0]->lexme].first.size();
    for (int i = 0; i <= num_param - 1; i++) {
      pop("$31");
    }
    pop("$31");
    pop("$29");
  } else if (node->rule == "arglist expr") {
    generate(node->children[0], tables, lables, offset_table, num);
    push("$3");
  } else if (node->rule == "arglist expr COMMA arglist") {
    generate(node->children[0], tables, lables, offset_table, num);
    push("$3");
    generate(node->children[2], tables, lables, offset_table, num);
  } else if (node->rule == "lvalue ID") {
    std::cout << "sw $3, " << offset_table[fun_name][node->children[0]->lexme] << "($29)\n";
  } else if (node->rule == "lvalue STAR factor") {
    generate(node->children[1], tables, lables, offset_table, num);
  } else if (node->rule == "lvalue LPAREN lvalue RPAREN") {
    generate(node->children[1], tables, lables, offset_table, num);
  } else if (node->rule == "term factor") {
    generate(node->children[0], tables, lables, offset_table, num);
  } else if (node->rule == "term term STAR factor") {
    generate(node->children[0], tables, lables, offset_table, num);
    push("$3");
    generate(node->children[2], tables, lables, offset_table, num);
    pop("$5");
    std::cout << "mult $5, $3\n";
    std::cout << "mflo $3\n";
  } else if (node->rule == "term term SLASH factor") {
    generate(node->children[0], tables, lables, offset_table, num);
    push("$3");
    generate(node->children[2], tables, lables, offset_table, num);
    pop("$5");
    std::cout << "div $5, $3\n";
    std::cout << "mflo $3\n";
  } else if (node->rule == "term term PCT factor") {
    generate(node->children[0], tables, lables, offset_table, num);
    push("$3");
    generate(node->children[2], tables, lables, offset_table, num);
    pop("$5");
    std::cout << "div $5, $3\n";
    std::cout << "mfhi $3\n";
  } else if (node->rule == "expr term") {
    generate(node->children[0], tables, lables, offset_table, num);
  } else if (node->rule == "expr expr PLUS term") {
    if (node->children[0]->type == "int*" && node->children[2]->type == "int") {
      generate(node->children[0], tables, lables, offset_table, num);
      push("$3");
      generate(node->children[2], tables, lables, offset_table, num);
      std::cout << "mult $3, $4\n";
      std::cout << "mflo $3\n";
      pop("$5");
      std::cout << "add $3, $5, $3\n";
    } else if (node->children[0]->type == "int" && node->children[2]->type == "int*") {
      generate(node->children[2], tables, lables, offset_table, num);
      push("$3");
      generate(node->children[0], tables, lables, offset_table, num);
      std::cout << "mult $3, $4\n";
      std::cout << "mflo $3\n";
      pop("$5");
      std::cout << "add $3, $5, $3\n";
    } else {
      generate(node->children[0], tables, lables, offset_table, num);
      push("$3");
      generate(node->children[2], tables, lables, offset_table, num);
      pop("$5");
      std::cout << "add $3, $5, $3\n";
    }
  } else if (node->rule == "expr expr MINUS term") {
    if (node->children[0]->type == "int*" && node->children[2]->type == "int*") {
      generate(node->children[0], tables, lables, offset_table, num);
      push("$3");
      generate(node->children[2], tables, lables, offset_table, num);
      pop("$5");
      std::cout << "sub $3, $5, $3\n";
      std::cout << "div $3, $4\n";
      std::cout << "mflo $3\n";
    } else if (node->children[0]->type == "int*" && node->children[2]->type == "int") {
      generate(node->children[0], tables, lables, offset_table, num);
      push("$3");
      generate(node->children[2], tables, lables, offset_table, num);
      std::cout << "mult $3, $4\n";
      std::cout << "mflo $3\n";
      pop("$5");
      std::cout << "sub $3, $5, $3\n";
    } else {
      generate(node->children[0], tables, lables, offset_table, num);
      push("$3");
      generate(node->children[2], tables, lables, offset_table, num);
      pop("$5");
      std::cout << "sub $3, $5, $3\n";
    }
  } else if (node->rule == "statement lvalue BECOMES expr SEMI") {
    if(node->children[0]->rule == "lvalue ID") {
      generate(node->children[2], tables, lables, offset_table, num);
      generate(node->children[0], tables, lables, offset_table, num);
    } else if (node->children[0]->rule == "lvalue STAR factor") {
      generate(node->children[2], tables, lables, offset_table, num);
      push("$3");
      generate(node->children[0], tables, lables, offset_table, num);
      pop("$5");
      std::cout << "sw $5, 0($3)\n";
    }
  } else if (node->rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
    generate(node->children[2], tables, lables, offset_table, num);
    lables++;
    int lable_index = lables;
    std::cout << "beq $3 , $0 , else" << lable_index << "\n";
    generate(node->children[5], tables, lables, offset_table, num);
    std::cout << "beq $0 , $0 , endif" << lable_index << "\n";
    std::cout << "else" << lable_index << ":\n";
    generate(node->children[9], tables, lables, offset_table, num);
    std::cout << "endif" << lable_index << ":\n";
  } else if (node->rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
    lables++;
    int lable_index = lables;
    std::cout << "loop" << lable_index << ":\n";
    generate(node->children[2], tables, lables, offset_table, num);
    std::cout << "beq $3 , $0 , endWhile" << lable_index << "\n";
    generate(node->children[5], tables, lables, offset_table, num);
    std::cout << "beq $0 , $0 , loop" << lable_index << "\n";
    std::cout << "endWhile" << lable_index << ":\n";
  } else if (node->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
    push("$1");
    generate(node->children[2], tables, lables, offset_table, num);
    std::cout << "add $1, $3, $0\n";
    push("$31");
    std::cout << "lis $5\n";
    std::cout << ".word print\n";
    std::cout << "jalr $5\n";
    pop("$31");
    pop("$1");
  } else if (node->rule == "statement DELETE LBRACK RBRACK expr SEMI") {
    lables++;
    int lable_index = lables;
    generate(node->children[3], tables, lables, offset_table, num);
    std::cout << "beq $3, $11, skipDelete" << lable_index << "\n";
    std::cout << "add $1, $3, $0\n";
    push("$31");
    std::cout << "lis $5\n";
    std::cout << ".word delete\n";
    std::cout << "jalr $5\n";
    pop("$31");
    std::cout << "skipDelete" << lable_index << ":\n";
  } else if (node->rule == "statements statements statement") {
    generate(node->children[0], tables, lables, offset_table, num);
    generate(node->children[1], tables, lables, offset_table, num);
  } else if (node->rule == "test expr LT expr") {
    generate(node->children[0], tables, lables, offset_table, num);
    push("$3");
    generate(node->children[2], tables, lables, offset_table, num);
    pop("$5");
    if(node->children[0]->type == "int") {
      std::cout << "slt $3, $5, $3\n";
    } else if (node->children[0]->type == "int*") {
      std::cout << "sltu $3, $5, $3\n";
    }    
  } else if (node->rule == "test expr GT expr") {
    generate(node->children[0], tables, lables, offset_table, num);
    push("$3");
    generate(node->children[2], tables, lables, offset_table, num);
    pop("$5");
    if(node->children[0]->type == "int") {
      std::cout << "slt $3, $3, $5\n";
    } else if (node->children[0]->type == "int*") {
      std::cout << "sltu $3, $3, $5\n";
    }
  } else if (node->rule == "test expr EQ expr") {
    generate(node->children[0], tables, lables, offset_table, num);
    push("$3");
    generate(node->children[2], tables, lables, offset_table, num);
    pop("$5");
    if(node->children[0]->type == "int") {
      std::cout << "slt $6, $3, $5\n";
      std::cout << "slt $7, $5, $3\n";
    } else if (node->children[0]->type == "int*") {
      std::cout << "sltu $6, $3, $5\n";
      std::cout << "sltu $7, $5, $3\n";
    }
    std::cout << "add $3, $6, $7\n";
    std::cout << "sub $3, $11, $3\n";
  } else if (node->rule == "test expr NE expr") {
    generate(node->children[0], tables, lables, offset_table, num);
    push("$3");
    generate(node->children[2], tables, lables, offset_table, num);
    pop("$5");
    if(node->children[0]->type == "int") {
      std::cout << "slt $6, $3, $5\n";
      std::cout << "slt $7, $5, $3\n";
    } else if (node->children[0]->type == "int*") {
      std::cout << "sltu $6, $3, $5\n";
      std::cout << "sltu $7, $5, $3\n";
    }    
    std::cout << "add $3, $6, $7\n";
  } else if (node->rule == "test expr LE expr") {
    generate(node->children[0], tables, lables, offset_table, num);
    push("$3");
    generate(node->children[2], tables, lables, offset_table, num);
    pop("$5");
    if(node->children[0]->type == "int") {
      std::cout << "slt $3, $3, $5\n";
    } else if (node->children[0]->type == "int*") {
      std::cout << "sltu $3, $3, $5\n";
    }
    std::cout << "sub $3, $11, $3\n";
  } else if (node->rule == "test expr GE expr") {
    generate(node->children[0], tables, lables, offset_table, num);
    push("$3");
    generate(node->children[2], tables, lables, offset_table, num);
    pop("$5");
    if(node->children[0]->type == "int") {
      std::cout << "slt $3, $5, $3\n";
    } else if (node->children[0]->type == "int*") {
      std::cout << "sltu $3, $5, $3\n";;
    }
    std::cout << "sub $3, $11, $3\n";
  } else if (node->rule == "dcls dcls dcl BECOMES NUM SEMI") {
    generate(node->children[0], tables, lables, offset_table, num);
    if (fun_name != "wain") {
      int num_param = tables[fun_name].first.size();
      offset_table[fun_name][node->children[1]->children[1]->lexme] = 4 * (num_param - num);
      num++;
    } else {
      offset_table[fun_name][node->children[1]->children[1]->lexme] = num * -4;
      num++;
    }
    std::cout << "lis $5\n";
    std::cout << ".word " << node->children[3]->lexme << "\n";
    push("$5");
  } else if (node->rule == "dcls dcls dcl BECOMES NULL SEMI") {
    generate(node->children[0], tables, lables, offset_table, num);
    if (fun_name != "wain") {
      int num_param = tables[fun_name].first.size();
      offset_table[fun_name][node->children[1]->children[1]->lexme] = 4 * (num_param - num);
      num++;
    } else {
      offset_table[fun_name][node->children[1]->children[1]->lexme] = num * -4;
      num++;
    }
    std::cout << "lis $5\n";
    std::cout << ".word 1\n";
    push("$5");
  }
}

int main() {
  Node *root = new Node;
  build_tree(root);
  std::map<std::string, std::pair<std::vector<std::string>, std::map<std::string, std::string>>> tables;
  std::map<std::string, std::map<std::string, int>> offset_table;
  int num = 0;
  int lables = 0;
  fun_name = "wain";
  build_symbol_table(root, tables, root);
  fun_name = "wain";
  typecheck(root->children[1], tables, root);
  fun_name = "wain";
  generate(root->children[1], tables, lables, offset_table, num);
  free_memory(root);
}