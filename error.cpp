#include "error.hpp"
#include <iostream>
#include <sstream>

void ErrorMsg(Result res, std::string shortMsg, std::string bigDesc) {
    ErrorApplicationConfig con;
    errorApplicationCreate(&con, shortMsg.c_str(), (bigDesc == "" ? NULL : bigDesc.c_str()));
    errorApplicationSetNumber(&con, res);
    errorApplicationShow(&con);
}

void _Assert(std::string conditionTxt, std::string file, int line) {
    std::stringstream msg;
    msg << "Assert fail: " << conditionTxt << " at:" << std::endl << file << ":" << line;
    ErrorMsg(0, msg.str());
}