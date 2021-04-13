///////////////////////////////////////////////////////////////////////////////////////
//
//	FilePath.cpp
//	Simple wrapper for dealing with file paths, extensions, root name, etc.
//	
//	Authors: Chris Peters
//	Copyright 2009, Digipen Institute of Technology
//
///////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "FilePath.hpp"

#include <algorithm>

StringTokenizer::StringTokenizer(StringRef stringToTok, char delim)
{
	Tokenize(stringToTok.c_str(), stringToTok.size(), delim);
}

void StringTokenizer::Tokenize(cstr buffer, int length, char delim)
{
	//Copy the string and chop it up
	Buffer = new char[length+1];
	strcpy(Buffer, buffer);
	Buffer[length] = '\0'; 

	int curTok = 0;
	char* strToTok = Buffer;
	char* curTokBeg = strToTok;
	while(*strToTok != '\0')
	{
		if(*strToTok == delim)
		{
			Tokens.push_back(curTokBeg);
			++curTok;

			//Null terminate the string
			*strToTok = '\0';

			//Move to next segment
			curTokBeg = strToTok+1;

			//Eat all delims
			//while(*strToTok != '\0' && *strToTok==tok)
			//	++strToTok;

		}
		++strToTok;
	}

	//End of the string
	if(curTokBeg!=strToTok)
	{			
		Tokens.push_back(curTokBeg);
		++curTok;
	}
}

StringTokenizer::~StringTokenizer()
{
	delete [] Buffer;
}


char convertSlash(char d)
{
	if( d == '/' )
		return '\\';
	else
		return tolower(d);
}

inline void toLower(std::string &s)
{
	std::transform(s.begin(), s.end(), s.begin(), convertSlash); 
}


FilePath::FilePath()
{

}

FilePath::FilePath(StringRef file)
{
	SetFilePath(file);
}

FilePath& FilePath::operator=(StringRef file)
{
	SetFilePath(file);
	return *this;
}

void FilePath::SetFilePath(StringRef file)
{
	FullPath = file;
	
	toLower(FullPath);

	std::size_t dirEnd = FullPath.find_last_of( "\\/");
	dirEnd = (dirEnd == std::string::npos) ? 0 : dirEnd+1;

	std::size_t fileEnd = FullPath.find_last_of( ".");
	fileEnd = (fileEnd == std::string::npos) ? file.size() : fileEnd;

	//Extension may be nothing
	Extension = FullPath.substr(fileEnd+1);
	FileName = FullPath.substr(dirEnd, fileEnd - dirEnd);
	FullDirectory = FullPath.substr(0,dirEnd);
}

String FilePath::GetFilePathWithNewExtension(StringRef newExtension)
{
	return FullDirectory + FileName + newExtension;
}