#include <switch.h>
#include <string>

void _Assert(std::string conditionTxt, std::string file, int line);
void ErrorMsg(Result res, std::string shortMsg, std::string bigDesc = "");

#define Assert(condition) if (!condition) _Assert(#condition, __FILE__, __LINE__)