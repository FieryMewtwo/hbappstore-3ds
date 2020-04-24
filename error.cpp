#include "error.hpp"
#include <iostream>
#include <sstream>

void ErrorMsg(Result res, std::string shortMsg, std::string bigDesc) {
#ifdef SWITCH
    ErrorApplicationConfig con;
    errorApplicationCreate(&con, shortMsg.c_str(), (bigDesc == "" ? NULL : bigDesc.c_str()));
    errorApplicationSetNumber(&con, res);
    errorApplicationShow(&con);
#endif
}

void _Assert(std::string conditionTxt, std::string file, int line) {
#ifdef SWITCH
    std::stringstream msg;
    msg << "Assert fail: " << conditionTxt << " at:" << std::endl << file << ":" << line;
    ErrorMsg(0, msg.str());
#endif
}