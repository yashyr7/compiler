#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <map>

struct Node {
  std::vector<Node*> v;
  std::string lex;
  std::string val;
};

void free_memory(Node *node) {
  int size = node->v.size();
  for(int i = 0; i <= size - 1; i++) {
    free_memory(node->v[i]);
  }
  delete node;
}

void print_preorder(Node *node) {
  std::cout << node->val;
  if(node->lex != "") {
    std::cout << " " << node->lex << " ";
  }
  for(auto i : node->v) {
    std::cout << " " << i->val;
  }
  std::cout << "\n";
  int size = node->v.size();
  for(int i = 0; i <= size - 1; i++) {
    print_preorder(node->v[i]);
  }
}

int search(std::string state, std::string sym, std::string type, std::vector<std::vector<std::string>> actions) {
  for(int i = 0; i < actions.size(); i++) {
    if(actions[i][0] == state && actions[i][1] == sym && actions[i][2] == type) {
      return i;
    }
  }
  return -1;
}

int main() {
  int terminal, non_terminal, num_rules, num_states, num_actions;
  std::vector<Node*> tree_stack;
  std::vector<std::vector<std::string>> rules;
  std::vector<std::vector<std::string>> actions;
  //std::vector<std::string> symbol_stack;
  std::vector<std::string> state_stack;
  std::vector<std::string> expression;
  std::map<int, std::string> lexme;
  int size = 0;

  std::ifstream fin;
  fin.open("wlp4.lr1");

  fin >> terminal;
  for(int i = 0; i <= terminal - 1; i++) {
    std::string temp;
    fin >> temp;
  }
  //std::cerr << sym_terminal[0] << " " << sym_terminal[terminal - 1] << "\n";

  fin >> non_terminal;
  for(int i = 0; i <= non_terminal - 1; i++) {
    std::string temp;
    fin >> temp;
  }

  //std::cerr << sym_nterminal[0] << " " << sym_nterminal[non_terminal - 1] << "\n";

  std::string start;
  fin >> start;

  fin >> num_rules;
  std::string trash;
  std::getline(fin, trash);
  for(int i = 0; i <= num_rules -1; i++) {
    std::string temp;
    std::getline(fin, temp);
    //std::cerr << temp << "\n";
    std::istringstream iss(temp);
    std::string str;
    std::vector<std::string> t;
    while(iss >> str) {
      t.push_back(str);
    }
    rules.push_back(t);
  }

  fin >> num_states >> num_actions;
  //std::cerr << num_actions << "\n";

  for(int i = 0; i <= num_actions - 1; i++) {
    std::string temp;
    std::vector<std::string> t;
    for(int j = 0; j <= 3; j++) {
      fin >> temp;
      t.push_back(temp);
    }
    actions.push_back(t);
  }

  std::string str, l;
  expression.push_back("BOF");
  lexme[size] = "BOF";
  size++;
  while(std::cin >> str >> l) {
    //std::cerr << "entered2\n";
    expression.push_back(str);
    lexme[size] = l;
    size++;
  }
  expression.push_back("EOF");
  lexme[size] = "EOF";
  size++;
  
  //std::cerr << lexme[expression[13]] << "\n";
  
  state_stack.push_back("0");

  for(int i = 0; i <= size - 1; i++) {
    while(1) {
      //std::cerr << "entered\n";
      int k = search(state_stack.back(), expression[i], "reduce", actions);
      
      if(k == -1) {
        break;
      }
      
      int rule_index = std::stoi(actions[k][3]);  // rule number
      int s = rules[rule_index].size(); // number of elements to pop + 1

      std::vector<Node*> popped;
      //std::cerr << "entered1\n";
      
      // popping elements
      for(int j = 0; j <= s - 2; j++) {
        int tree_size = tree_stack.size();
        popped.push_back(tree_stack[tree_size - 1]);
        //std::cerr << tree_stack[tree_size - 1]->val << " pushed\n";
        tree_stack[tree_size - 1] = NULL;
        tree_stack.pop_back();
        state_stack.pop_back();
      }
      //std::cerr << "exited\n";
            
      // push the reduced symbol
      tree_stack.push_back(new Node);
      tree_stack[tree_stack.size() - 1]->val = rules[std::stoi(actions[k][3])][0];
      
      //std::cerr << "entered2\n";
      int popped_size = popped.size();
      for(int l = 0; l <= popped_size - 1; l++) {
        int tree_size = tree_stack.size();
        tree_stack[tree_size- 1]->v.push_back(popped[popped_size - 1 - l]);
      }
      //std::cerr << "entered3\n";
      // push the state after reduction
      state_stack.push_back(actions[search(state_stack.back(), tree_stack.back()->val, "shift", actions)][3]);
    }
    //std::cerr << "reached2\n";
    
    tree_stack.push_back(new Node);
    tree_stack[tree_stack.size() - 1]->val = expression[i];
    if(lexme[i] != "") {
      tree_stack[tree_stack.size() - 1]->lex = lexme[i];
    }

    int k = search(state_stack.back(), expression[i], "shift", actions);
    if(k == -1) {
      std::cerr << "ERROR at " << i << "\n";
      for(auto i : tree_stack){
        free_memory(i);
      }
      return 0;
    }
    state_stack.push_back(actions[k][3]);
  }


  //std::cerr << tree_stack[1]->v.size() << "\n";
  std::cout << "start BOF procedures EOF\nBOF BOF\n";
  print_preorder(tree_stack[1]);
  std::cout << "EOF EOF\n";
  free_memory(tree_stack[0]);
  free_memory(tree_stack[1]);
  free_memory(tree_stack[2]);
  
}