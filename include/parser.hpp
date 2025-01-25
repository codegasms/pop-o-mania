#pragma once
#include <stdio.h>
#include <string.h>

#include <fstream>
#include <gamefile.hpp>
#include <iostream>
#include <map>
#include <vector>

class Parser {
   public:
    Parser();
    GameFile parse(std::string file);
    GameFile parseMetadata(std::string file);

   private:
    std::pair<std::string, std::string> parseKeyValue(std::string text, bool hasSpaceBefore,
                                                      bool hasSpaceAfter);
    std::vector<std::string> parseSeperatedLists(std::string, char);
    std::string DecimalToBinaryString(int);
};
