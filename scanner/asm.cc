#include <iostream>
#include <string>
#include <vector>
#include "scanner.h"
#include <algorithm>
#include <map>


void write_byte(int num) {
  unsigned char c = num >> 24;
  std::cout << c;
  c = num >> 16;
  std::cout << c;
  c = num >> 8;
  std::cout << c;
  c = num;
  std::cout << c;
}

int main() {
  std::string line;
  std::vector<int> WORD_INT{ 2, 6};
  std::vector<int> WORD_HEX{ 2, 7};
  std::vector<int> WORD_LABEL{ 2, 0};
  std::vector<int> analysis;
  std::vector<std::vector<int>> parsing;
  std::vector<std::vector<Token>> tokens;
  std::vector<int> JR_JALR{ 0, 8};
  std::vector<int> ADD_SUB_SLT_SLTU{ 0, 8, 3, 8, 3, 8};
  std::vector<int> BEQ_BNE_INT{ 0, 8, 3, 8, 3, 6};
  std::vector<int> BEQ_BNE_HEX{ 0, 8, 3, 8, 3, 7};
  std::vector<int> BEQ_BNE_LABEL{ 0, 8, 3, 8, 3, 0};
  std::vector<int> MULT_MULTU_DIV{ 0, 8, 3, 8};
  std::vector<int> SW_LW_INT{ 0, 8, 3, 6, 4, 8, 5};
  std::vector<int> SW_LW_HEX{ 0, 8, 3, 7, 4, 8, 5};

  std::map<std::string, int> sym_table;

  int line_number = 0;

  try {
    while (getline(std::cin, line)) {
      analysis.clear();
      std::vector<Token> tokenLine = scan(line);
      int size = tokenLine.size();
      // Getting Kinds
      for(int i = 0; i <= size - 1; i++) {
        auto &token = tokenLine[i];
        analysis.push_back(token.getKind());
      }
  
      // make sym table and remove from front
      while(true) {
        if(analysis.size() > 0 && analysis[0] == 1) {
          std::string label = tokenLine[0].getLexeme();
          label = label.substr(0, label.size() - 1);
          if (!((tokenLine[0].getLexeme()[0] >= 'a' && tokenLine[0].getLexeme()[0] <= 'z') ||
          (tokenLine[0].getLexeme()[0] >= 'A' && tokenLine[0].getLexeme()[0] <= 'Z'))) {
            std::cerr << "ERROR: INVALID LABEL NAME\n";
            return 0;
          }
          if(sym_table.find(label) != sym_table.end()) {
            std::cerr << "ERROR: CANT HAVE TWO LABELS WITH SAME NAME\n";
            return 0;
          }
          sym_table[label] = line_number;
          analysis.erase(analysis.begin() + 0);
          tokenLine.erase(tokenLine.begin() + 0);
        } else {
          break;
        }
      }

      if(analysis.size() > 0) {
        line_number += 4;
      }

      parsing.push_back(analysis);
      tokens.push_back(tokenLine);
    } 
  } catch (ScanningFailure &f) {
    std::cerr << f.what() << std::endl;

    return 1;
  }


  // Printing the label table
  
  for (std::map<std::string, int>::iterator it = sym_table.begin(); it != sym_table.end(); ++it) {
    std::cerr << it->first << " " << it->second << std::endl;
  }

  for(int i = 0; i < parsing.size(); i++) {
    analysis = parsing[i];
    std::vector<Token> tokenLine = tokens[i];
    if (analysis == WORD_INT || analysis == WORD_HEX || analysis == WORD_LABEL) { // .word command
      int64_t num = tokenLine[1].toLong();
      if(analysis == WORD_LABEL) {
        std::string l = tokenLine[1].getLexeme();
        if (sym_table.find(l) == sym_table.end()) {
          std::cerr << "ERROR: LABEL NOT FOUND\n";
          return 0;
        }
        num = sym_table[l];
        write_byte(num);
      } else if (analysis == WORD_HEX) {
        if (num <= 0xffffffff) {
          write_byte(num);
        } else {
          std::cerr << "ERROR: NUMBER OUT OF LIMIT\n";
          return 0;
        }
      } else if (analysis == WORD_INT) {
        if (num <= 4294967295 && num >= -2147483648) {
          write_byte(num);
        } else {
          std::cerr << "ERROR: NUMBER OUT OF LIMIT\n";
          return 0;
        }
      }
    } else if (analysis == JR_JALR) {
      std::string cmd = tokenLine[0].getLexeme();
      int reg = tokenLine[1].toLong();
      if(reg >= 0 && reg <= 31) {
        if(cmd == "jr") {
          int func = 8;
          int op = 0;
          int s = reg;
          int num = op | (s << 21) | func;
          write_byte(num);
        } else if(cmd == "jalr") {
          int func = 9;
          int op = 0;
          int s = reg;
          int num = op | (s << 21) | func;
          write_byte(num);
        } else if(cmd == "lis") {
          int func = 20;
          int op = 0;
          int s = reg;
          int num = op | (s << 11) | func;
          write_byte(num);
        } else if(cmd == "mflo") {
          int func = 18;
          int op = 0;
          int s = reg;
          int num = op | (s << 11) | func;
          write_byte(num);
        } else if(cmd == "mfhi") {
          int func = 16;
          int op = 0;
          int s = reg;
          int num = op | (s << 11) | func;
          write_byte(num);
        } else {
          std::cerr << "ERROR: INVALID COMMAND\n";
          return 0;
        }
      } else {
        std::cerr << "ERROR: INVALID REGISTER\n";
        return 0;
      }
    } else if (analysis == ADD_SUB_SLT_SLTU) {
      std::string cmd = tokenLine[0].getLexeme();
      int reg1 = tokenLine[1].toLong();
      int reg2 = tokenLine[3].toLong();
      int reg3 = tokenLine[5].toLong();
      if (reg1 >= 0 && reg1 <= 31 && reg2 >= 0 && reg2 <= 31 && reg3 >= 0 && reg3 <= 31) {
        if (cmd == "add") {
          int func = 32;
          int op = 0;
          int d = reg1;
          int s = reg2;
          int t = reg3;
          int num = op | (s << 21) | (t << 16) | (d << 11) | func;
          write_byte(num);
        } else if (cmd == "sub") {
          int func = 34;
          int op = 0;
          int d = reg1;
          int s = reg2;
          int t = reg3;
          int num = op | (s << 21) | (t << 16) | (d << 11) | func;
          write_byte(num);
        } else if (cmd == "slt") {
          int func = 42;
          int op = 0;
          int d = reg1;
          int s = reg2;
          int t = reg3;
          int num = op | (s << 21) | (t << 16) | (d << 11) | func;
          write_byte(num);
        } else if (cmd == "sltu") {
          int func = 43;
          int op = 0;
          int d = reg1;
          int s = reg2;
          int t = reg3;
          int num = op | (s << 21) | (t << 16) | (d << 11) | func;
          write_byte(num);
        } else {
          std::cerr << "ERROR: INVALID COMMAND\n";
          return 0;
        }
      } else {
        std::cerr << "ERROR: INVALID REGISTER\n";
        return 0;
      }
    } else if (analysis == BEQ_BNE_HEX || analysis == BEQ_BNE_INT || analysis == BEQ_BNE_LABEL) {
      std::string cmd = tokenLine[0].getLexeme();
      int reg1 = tokenLine[1].toLong();
      int reg2 = tokenLine[3].toLong();
      if (reg1 >= 0 && reg1 <= 31 && reg2 >= 0 && reg2 <= 31) {
        if (analysis == BEQ_BNE_HEX) {
          int64_t steps = tokenLine[5].toLong();
          if(!(steps <= 0xffff)) {
            std::cerr << "ERROR: VALUE OUT OF RANGE\n";
            return 0;
          }
          if (cmd == "beq") {
            int op = 4;
            int s = reg1;
            int t = reg2;
            int offset = steps;
            int num = (op << 26) | (s << 21) | (t << 16) | (offset & 0xffff);
            write_byte(num);
          } else if (cmd == "bne") {
            int op = 5;
            int s = reg1;
            int t = reg2;
            int offset = steps;
            int num = (op << 26) | (s << 21) | (t << 16) | (offset & 0xffff);
            write_byte(num);
          } else {
            std::cerr << "ERROR: INVALID COMMAND\n";
            return 0;
          }
        } else if (analysis == BEQ_BNE_INT) {
          int64_t steps = tokenLine[5].toLong();
          if(!(steps >= -32768 && steps <= 32767)) {
            std::cerr << "ERROR: VALUE OUT OF RANGE\n";
            return 0;
          }
          if (cmd == "beq") {
            int op = 4;
            int s = reg1;
            int t = reg2;
            int offset = steps;
            int num = (op << 26) | (s << 21) | (t << 16) | (offset & 0xffff);
            write_byte(num);
          } else if (cmd == "bne") {
            int op = 5;
            int s = reg1;
            int t = reg2;
            int offset = steps;
            int num = (op << 26) | (s << 21) | (t << 16) | (offset & 0xffff);
            write_byte(num);
          } else {
            std::cerr << "ERROR: INVALID COMMAND\n";
            return 0;
          }
        } else if (analysis == BEQ_BNE_LABEL) {
          std::string step = tokenLine[5].getLexeme();
          if(sym_table.find(step) == sym_table.end()) {
            std::cerr << "ERROR: LABEL NOT FOUND\n";
            return 0;
          }
          int steps = (sym_table[step] - (i * 4) - 4) / 4;
          std::cerr << i <<std::endl;
          if(!(steps >= -32768 && steps <= 32767)) {
            std::cerr << "ERROR: VALUE OUT OF RANGE\n";
            return 0;
          }
          if (cmd == "beq") {
            int op = 4;
            int s = reg1;
            int t = reg2;
            int offset = steps;
            int num = (op << 26) | (s << 21) | (t << 16) | (offset & 0xffff);
            write_byte(num);
          } else if (cmd == "bne") {
            int op = 5;
            int s = reg1;
            int t = reg2;
            int offset = steps;
            int num = (op << 26) | (s << 21) | (t << 16) | (offset & 0xffff);
            write_byte(num);
          } else {
            std::cerr << "ERROR: INVALID COMMAND\n";
            return 0;
          }
        }
      } else {
        std::cerr << "ERROR: INVALID REGISTER\n";
        return 0;
      }
    } else if (analysis == MULT_MULTU_DIV) {
      std::string cmd = tokenLine[0].getLexeme();
      int reg1 = tokenLine[1].toLong();
      int reg2 = tokenLine[3].toLong();
      if (reg1 >= 0 && reg1 <= 31 && reg2 >= 0 && reg2 <= 31) {
        if (cmd == "mult") {
          int op = 24;
          int s = reg1;
          int t = reg2;
          int num = op | (s << 21) | (t << 16);
          write_byte(num);
        } else if (cmd == "multu") {
          int op = 25;
          int s = reg1;
          int t = reg2;
          int num = op | (s << 21) | (t << 16);
          write_byte(num);
        } else if (cmd == "div") {
          int op = 26;
          int s = reg1;
          int t = reg2;
          int num = op | (s << 21) | (t << 16);
          write_byte(num);  
        } else {
          std::cerr << "ERROR: INVALID COMMAND\n";
          return 0;
        }
      } else {
        std::cerr << "ERROR: INVALID REGISTER\n";
        return 0;
      }
    } else if (analysis == SW_LW_HEX || analysis == SW_LW_INT) {
      std::string cmd = tokenLine[0].getLexeme();
      int reg2 = tokenLine[1].toLong();
      int reg1 = tokenLine[5].toLong();
      int64_t steps = tokenLine[3].toLong();
      if (reg1 >= 0 && reg1 <= 31 && reg2 >= 0 && reg2 <= 31) {
        if (analysis == SW_LW_INT) {
          if(!(steps >= -32768 && steps <= 32767)) {
            std::cerr << "ERROR: VALUE OUT OF RANGE\n";
            return 0;
          }
        } else if (analysis == SW_LW_HEX) {
          if(!(steps <= 0xffff)) {
            std::cerr << "ERROR: VALUE OUT OF RANGE\n";
            return 0;
          }
        }
        if (cmd  == "lw") {
          int op = 35;
          int s = reg1;
          int t = reg2;
          int offset = steps;
          int num = (op << 26) | (s << 21) | (t << 16) | (offset & 0xffff);
          write_byte(num);  
        } else if (cmd  == "sw") {
          int op = 43;
          int s = reg1;
          int t = reg2;
          int offset = steps;
          int num = (op << 26) | (s << 21) | (t << 16) | (offset & 0xffff);
          write_byte(num);  
        } else {
          std::cerr << "ERROR: INVALID COMMAND\n";
          return 0;
        }
      } else {
        std::cerr << "ERROR: INVALID REGISTER\n";
        return 0;
      }
    } else if (analysis.size() == 0) {
      continue;
    } else {
      std::cerr << "ERROR: INVALID COMMAND\n";
      return 0;
    }
  }

  return 0;
}
