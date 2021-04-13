#include "Precompiled.hpp"

#include "DocTypeTokens.hpp"

#define LETTERS 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W',\
                 'X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t',\
                 'u','v','w','x','y','z'
#define NUMBERS '0','1','2','3','4','5','6','7','8','9'
#define WHITESPACE ' ','\r','\n','\t'

#define INVALID_TOKEN 0

namespace Zero
{
  static Zero::UnsortedMap<Zero::String, DocTokenType::Enum> *DocTypeStringEnumMap;

  static bool verboseFlag = false;

  ////////////////////////////////////////////////////////////
  //////////Helpers
  ////////////////////////////////////////////////////////////
  void CleanupStringToken(DocToken *str)
  {
    if (str->mText.SizeInBytes() < 2 || str->mText.c_str()[0] != '"')
      return;

    str->mText = str->mText.SubStringFromByteIndices(1, str->mText.SizeInBytes() - 2);
  }
  void CleanupCommentToken(DocToken *comment)
  {
    String& text = comment->mText;

    // single line comment
    if (text.c_str()[1] == '/')
    {
      if (text.SizeInBytes() < 2)
        return;

      text = text.SubStringFromByteIndices(2, text.SizeInBytes());
    }
    // block comment
    else
    {
      if (text.SizeInBytes() < 4 || text.c_str()[1] != '*')
        return;
      
      text = text.SubStringFromByteIndices(2, text.SizeInBytes() - 4);
    }
  }

  void SetVerboseFlag(void)
  {
    verboseFlag = true;
  }

  //Builds a string from a list of tokens
  String ConvertTokenListToString(const TypeTokens& tokens)
  {
    StringBuilder builder;

    for (uint i = 0; i < tokens.Size(); ++i)
    {
      builder.Append(tokens[i].mText);
      builder.Append(" ");
    }
    return builder.ToString();
  }

  void CopyArrayOfTokenPtrToTypeTokens(const Array<DocToken*>& in, TypeTokens*& out)
  {
    out = new TypeTokens();
    forRange(DocToken *tokenPtr, in.All())
    {
      out->PushBack(*tokenPtr);
    }
  }

  void AppendTokensFromString(DocDfaState* startingState, StringRef str, TypeTokens *output)
  {
    const char *stream = str.c_str();

    // Read until we exhaust the stream
    while (*stream != '\0')
    {
      DocToken token;
      ReadToken(startingState, stream, token);

      stream += token.mText.SizeInBytes();

      if (token.mText.SizeInBytes() == 0)
      {
        if (verboseFlag)
          printf("Skipping one character of input: '%s'\n", stream);
        ++stream;
      }
      else if (token.mEnumTokenType == DocTokenType::Whitespace)
      {
        continue;
      }
      else if (output != nullptr)
      {
        if (token.mEnumTokenType == DocTokenType::Comment)
        {
          CleanupCommentToken(&token);
        }
        else if (token.mEnumTokenType == DocTokenType::StringLiteral)
        {
          CleanupStringToken(&token);
        }

        output->PushBack(token);
      }
    }

  }

  void InitializeTokens(void)
  {
    DocTypeStringEnumMap = new Zero::UnsortedMap<Zero::String, DocTokenType::Enum>();

    for (uint i = 0; i < DocTokenType::EnumCount; ++i)
    {
      (*DocTypeStringEnumMap)[DocTokenTypes[i]] = (DocTokenType::Enum)i;
    }
  }

////////////////////////////////////////////////////////////
//////////DFA
////////////////////////////////////////////////////////////

bool Zero::DocDfaEdge::operator==(const DocDfaEdge& rhs) const
{
  return mChild == rhs.mChild && mEdgeID == rhs.mEdgeID;
}

DocDfaState* AddState(int acceptingToken)
{
  DocDfaState *newState = new DocDfaState();

  newState->mTokenTypeID = static_cast<DocTokenType::Enum>(acceptingToken);

  newState->mDefault = nullptr;

  return newState;
}

void AddEdge(DocDfaState* from, DocDfaState* to, char c)
{
  //DocDfaEdge *newEdge = new DocDfaEdge();

  // add edge to the parent
  from->mEdges[c].mChild = to;
  from->mEdges[c].mEdgeID = c;
}

//void AddListOfEdges(DocDfaState* from, DocDfaState* to) {}

template <typename First = char>
void AddListOfEdges(DocDfaState* from, DocDfaState* to, const First& first)
{
  AddEdge(from, to, first);
}

template <typename First, typename... Rest>
void AddListOfEdges(DocDfaState* from, DocDfaState* to, const First& first, const Rest&... rest)
{
  // recursive call using pack expansion syntax
  AddEdge(from, to, first);
  AddListOfEdges<Rest...>(from, to, rest...);
}


void AddDefaultEdge(DocDfaState* from, DocDfaState* to)
{
  from->mDefault = to;
}

DocDfaState* CreateLangDfa(void)
{
  //first create the starting node
  DocDfaState* root = AddState(0);

  /////Whitespace = [ \r\n\t]+                          /////

  DocDfaState *whitespace = AddState(DocTokenType::Whitespace);

  AddListOfEdges(root, whitespace, WHITESPACE);
  AddListOfEdges(whitespace, whitespace, WHITESPACE);

  /////Identifier = [a-zA-Z_][a-zA-Z0-9_]*              /////

  DocDfaState *identifier1 = AddState(DocTokenType::Identifier);
  DocDfaState *identifier2 = AddState(DocTokenType::Identifier);

  AddListOfEdges(root, identifier1, LETTERS, '_');
  AddListOfEdges(identifier1, identifier2, LETTERS, NUMBERS, '_');
  AddListOfEdges(identifier2, identifier2, LETTERS, NUMBERS, '_');

  /////Number = [0-9]+(\.[0-9]+f?)?              /////


  /////Pointer/Reference = [&*]*                        /////
  DocDfaState *pointer = AddState(DocTokenType::Pointer);
  DocDfaState *reference = AddState(DocTokenType::Reference);

  AddEdge(root, pointer, '*');
  AddEdge(root, reference, '&');

  /////Pound = #                                        /////
  DocDfaState *pound = AddState(DocTokenType::Pound);
  DocDfaState *concat = AddState(DocTokenType::Concatenate);

  AddEdge(root, pound, '#');
  AddEdge(pound, concat, '#');

  /////DollarSign = $                                   /////
  DocDfaState *dollarSign = AddState(DocTokenType::DollarSign);

  AddEdge(root, dollarSign, '$');

  /////GreaterThan/LessThan = (<|>)                     /////

  DocDfaState *LessThan = AddState(DocTokenType::LessThan);
  DocDfaState *GreaterThan = AddState(DocTokenType::GreaterThan);

  AddEdge(root, LessThan, '<');
  AddEdge(root, GreaterThan, '>');

  /////Scope Resolution                                 /////
  DocDfaState *Invalid = AddState(DocTokenType::Invalid);
  DocDfaState *Colon = AddState(DocTokenType::Colon);
  DocDfaState *ScopeRes = AddState(DocTokenType::ScopeResolution);

  AddEdge(root, Colon, ':');
  AddEdge(Colon, ScopeRes, ':');

  /////Parenthesis                                     /////
  DocDfaState *openParen = AddState(DocTokenType::OpenParen);
  DocDfaState *closeParen = AddState(DocTokenType::CloseParen);

  AddEdge(root, openParen, '(');
  AddEdge(root, closeParen, ')');

  /////Comma                                           /////
  DocDfaState *comma = AddState(DocTokenType::Comma);

  AddEdge(root, comma, ',');

  /////Dot                                             /////
  DocDfaState *period = AddState(DocTokenType::Dot);

  AddEdge(root, period, '.');

  /////Semicolon                                       /////
  DocDfaState *semicolon = AddState(DocTokenType::Semicolon);

  AddEdge(root, semicolon, ';');

  /////Brackets                                        /////
  DocDfaState *bracketO = AddState(DocTokenType::OpenBracket);
  DocDfaState *bracketC = AddState(DocTokenType::CloseBracket);

  AddEdge(root, bracketO, '[');
  AddEdge(root, bracketC, ']');

  DocDfaState *curlOpen = AddState(DocTokenType::OpenCurly);
  DocDfaState *curlClose = AddState(DocTokenType::CloseCurly);

  AddEdge(root, curlOpen, '{');
  AddEdge(root, curlClose, '}');

  /////Comments                                       /////
  DocDfaState *forwardSlash = AddState(DocTokenType::ForwardSlash);
  DocDfaState *singleLineComment = AddState(DocTokenType::Comment);
  DocDfaState *blockComment = AddState(DocTokenType::Comment);
  DocDfaState *blockCommentMightEnd = AddState(DocTokenType::Comment);
  DocDfaState *blockCommentEnd = AddState(DocTokenType::Comment);


  AddEdge(root, forwardSlash, '/');
  AddEdge(forwardSlash, singleLineComment, '/');
  AddDefaultEdge(singleLineComment, singleLineComment);

  AddEdge(forwardSlash, blockComment, '*');
  AddDefaultEdge(blockComment, blockComment);
  AddEdge(blockComment, blockCommentMightEnd, '*');
  AddDefaultEdge(blockCommentMightEnd, blockComment);
  AddEdge(blockCommentMightEnd, blockCommentMightEnd, '*');
  AddEdge(blockCommentMightEnd, blockCommentEnd, '/');

  /////Assignment / Equivalence                       /////
  DocDfaState *assignment = AddState(DocTokenType::Assignment);
  DocDfaState *equiv = AddState(DocTokenType::Equivalence);

  AddEdge(root, assignment, '=');
  AddEdge(assignment, equiv, '=');

  /////Backslash                                      /////
  DocDfaState *backslash = AddState(DocTokenType::BackwardSlash);

  AddEdge(root, backslash, '\\');

  /////Quotes                                         /////
  DocDfaState *stringLiteralStart = AddState(DocTokenType::Invalid);
  DocDfaState *stringLiteral1 = AddState(DocTokenType::Invalid);
  DocDfaState *stringEscapeCharacter = AddState(DocTokenType::Invalid);
  DocDfaState *stringLiteralEnd = AddState(DocTokenType::StringLiteral);

  // possible edges from open quote
  AddEdge(root, stringLiteralStart, '\"');
  stringLiteralStart->mDefault = stringLiteral1;
  AddEdge(stringLiteralStart, stringLiteralEnd, '\"');
  AddEdge(stringLiteralStart, stringEscapeCharacter, '\\');

  // possible edges for the rest of the string
  AddEdge(stringLiteral1, stringEscapeCharacter, '\\');
  AddListOfEdges(stringEscapeCharacter, stringLiteral1, 'n', 'r', 't', '\"');
  stringLiteral1->mDefault = stringLiteral1;
  AddEdge(stringLiteral1, stringLiteralEnd, '\"');

  return root;
}

////////////////////////////////////////////////////////////
//////////DocToken
////////////////////////////////////////////////////////////

DocToken::DocToken() : mEnumTokenType(DocTokenType::EnumCount) {}

DocToken::DocToken(StringParam text, DocTokenType::Enum type)
  : mText(text)
  , mEnumTokenType(type)
{
}

DocToken::DocToken(const DocToken &cpy) 
  : mText(cpy.mText)
  , mEnumTokenType(cpy.mEnumTokenType)
{

}

void DocToken::Serialize(Serializer& stream)
{
  SerializeName(mText);
  String Type = DocTokenTypes[mEnumTokenType];
  SerializeName(Type);
  mEnumTokenType = (*DocTypeStringEnumMap)[Type.c_str()];
}

bool DocToken::operator==(const DocToken& right)
{
  return mText == right.mText;
}

////////////////////////////////////////////////////////////
//////////ParsingFunctions
////////////////////////////////////////////////////////////

int checkForKeyword(const char*stream, int tokenLen)
{
  char token[50] = { '\0' };
  int index = 0;
  for (; stream[index] != '\0'; ++index)
  {
    if (stream[index] == ' ')
    {
      break;
    }
  }

  strncpy(token, stream, tokenLen);

  index = 0;
  for (const char *keyword : DocKeywords)
  {
    if (std::strcmp(keyword, token) == 0)
      return DocTokenType::KeywordStart + 1 + index;

    ++index;
  }
  return DocTokenType::Identifier;
}

DocDfaEdge *FindEdge(DocDfaState *state, char c)
{
  if (!state->mEdges.ContainsKey(c))
    return NULL;

  return &state->mEdges[c];
}

void CheckForKeyword(DocToken& outToken)
{
  for (uint i = 0; i < DocTokenType::EnumCount - DocTokenType::KeywordStart; ++i)
  {
    if (outToken.mText == DocKeywords[i])
    {
      outToken.mEnumTokenType = static_cast<DocTokenType::Enum>(DocTokenType::KeywordStart + 1 + i);
    }
  }
}


void ReadToken(DocDfaState* startingState, const char* stream, DocToken& outToken)
{
  DocDfaState *currState = startingState;
  DocDfaState *acceptingState = nullptr;

  int tokenLen = 0;
  int acceptedTokenLen = 0;
  for (const char *nextChar = stream; *nextChar != '\0'; ++nextChar, ++tokenLen)
  {
    if (currState->mTokenTypeID != 0)
    {
      acceptingState = currState;
      acceptedTokenLen = tokenLen;
    }

    DocDfaEdge *foundEdge = FindEdge(currState, *nextChar);

    if (foundEdge == nullptr)
    {
      // first, check for a mDefault edge
      if (currState->mDefault != nullptr)
      {
        currState = currState->mDefault;

        continue;
      }

      // if no mDefault, check for last accepting state
      if (acceptingState != nullptr)
      {
        // just return the accepting token
        outToken.mEnumTokenType = acceptingState->mTokenTypeID;
       // outToken.id = outToken.mEnumTokenType;
        outToken.mText = String(stream, acceptedTokenLen);

        CheckForKeyword(outToken);
        return;
      }

      // if both are false, return invalid token
      outToken.mEnumTokenType = DocTokenType::Invalid;
      //outToken.id = outToken.mEnumTokenType;
      outToken.mText = String(stream, tokenLen);
      return;

    } //otherwise, we found an edge

      // traverse to that node and get the next character
    currState = foundEdge->mChild;
  }
  if (currState->mTokenTypeID != 0)
  {
    acceptingState = currState;
    acceptedTokenLen = tokenLen;
  }

  if (acceptingState != nullptr)
  {
    // just return the accepting token
    outToken.mEnumTokenType = acceptingState->mTokenTypeID;
    //outToken.id = outToken.mEnumTokenType;
    outToken.mText = String(stream, acceptedTokenLen);

    CheckForKeyword(outToken);
  }
  else
  {
    // if both are false, return invalid token
    outToken.mEnumTokenType = DocTokenType::Invalid;
    //outToken.id = outToken.mEnumTokenType;
    outToken.mText = String(stream, tokenLen);
  }
}

}