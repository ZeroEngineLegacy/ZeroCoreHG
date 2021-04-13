///////////////////////////////////////////////////////////////////////////////////////
//
//	FilePath.hpp
//	Simple wrapper for dealing with file paths, extensions, root name, etc.
//	
//	Authors: Chris Peters
//	Copyright 2009, Digipen Institute of Technology
//
///////////////////////////////////////////////////////////////////////////////////////
#pragma once

typedef std::string String;
typedef const String& StringRef;
typedef const char* cstr;

class StringTokenizer
{
public:
	StringTokenizer(StringRef stringToTok, char delim);
	~StringTokenizer();
	void Tokenize(cstr buffer, int length, char delim);
	std::vector<char*> Tokens;
private:
	char* Buffer;
};

class FilePath
{
public:
	FilePath();
	FilePath(StringRef file);
	FilePath& operator=(StringRef file);
	void SetFilePath(StringRef file);
	String GetFilePathWithNewExtension(StringRef newExtension);

	//Extension of file which may be empty includes the period
	//such as ".png", ".txt"
	String Extension;
	//The root filename of the file without the extension or path
	//and in lower case For "C:\Data\FileName.txt" "filename"
	String FileName;
	//The full directory path of the file's location
	//For"C:\Data\FileName.txt" "c:\data\"
	String FullDirectory;
	//The full path including the filename
	//For "C:\Data\FileName.txt" "c:\data\filename.txt"
	String FullPath;
};