#pragma once

#include "Engine/EngineContainers.hpp"



namespace DocTokenType
{
  enum Enum
  {
#define TOKEN(Name, Value) Name,
#include "DocTypeTokens.inl"
#undef TOKEN
    EnumCount
  }; 
}

static const char *DocTokenTypes[] = {
#define TOKEN(Name, Value) #Name,
#include "DocTypeTokens.inl"
#undef TOKEN
  ""
};

static const char *DocTokens[] = {
#define TOKEN(Name, Value) Value,
#include "DocTypeTokens.inl"
#undef TOKEN
  ""
};

static const char *DocKeywords[DocTokenType::EnumCount - DocTokenType::KeywordStart] = {
#define TOKEN(Name, Value) Value,
#include "DocTypeKeywords.inl"
#undef TOKEN
  "INVALID_KEYWORD"
};


namespace Zero
{
  class DocToken;
  typedef Array<DocToken> TypeTokens;

  /// Builds a string from a list of tokens
  String ConvertTokenListToString(const TypeTokens& tokens);

  /// allocates new typetokens array in out
  void CopyArrayOfTokenPtrToTypeTokens(const Array<DocToken*>& in, TypeTokens*& out);

  /// Hacky function to fix bad formating or extra quotes left in a string token
  void CleanupStringToken(DocToken *String);

  /// Hacky function to fix terrible formatting or extra comment tokens left in comment
  void CleanupCommentToken(DocToken *Comment);

  /// Sets the verbose flag for all classes in the file which will cause them to output skipped tokens
  void SetVerboseFlag(void);

  // instead of pointing to text stream directly we copy 
  // to a string since we want to be able to load these
  class DocToken
  {
  public:
    String mText;
    DocTokenType::Enum mEnumTokenType;

    /// defaults to an invalid type
    DocToken();

    /// takes text and assumes Identifier token if no type was passed
    DocToken(StringParam text, DocTokenType::Enum type = DocTokenType::Identifier);

    /// copies a DocToken
    DocToken(const DocToken &cpy);

    /// Serializes a DocToken
    void Serialize(Serializer& stream);

    /// Checks both if the type and if the actual text of the two tokens are the same
    bool operator==(const DocToken& right);
  };

  class DocDfaState;

  struct DocDfaEdge
  {
    DocDfaState *mChild;

    char mEdgeID;

    /// checks if both edges are goint to the same node from the same node
    bool operator==(const DocDfaEdge& rhs) const;
  };

  class DocDfaState
  {
  public:
    UnsortedMap<char, DocDfaEdge> mEdges;

    DocDfaState *mDefault;

    DocTokenType::Enum mTokenTypeID;
  };

  /// Sets up enum map for token types
  void InitializeTokens(void);

  /// Creates Dfa starting from a root token
  DocDfaState* CreateLangDfa(void);

  /// Read token starting at startingState and using stream with Dfa to create outToken
  void ReadToken(DocDfaState* startingState, const char* stream, DocToken& outToken);

  /// Appends tokens from str to the end of the TypeTokens in output
  void AppendTokensFromString(DocDfaState* startingState, StringRef str, TypeTokens *output);
}