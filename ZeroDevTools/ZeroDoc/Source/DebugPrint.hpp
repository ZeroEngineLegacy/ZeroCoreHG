#pragma once

#ifdef _MSC_VER

//In Visual Studio output to the visual studio
//debug ouput window
void DebugPrint2(const char *str,...)
{
  va_list vaList;
  char buff[2048];
  va_start(vaList,str);
  vsprintf_s(buff,_countof(buff),str,vaList);
  va_end(vaList);
  OutputDebugStringA(buff);
}

#else

//For other compilers just use printf
#define DebugPrint printf

#endif
