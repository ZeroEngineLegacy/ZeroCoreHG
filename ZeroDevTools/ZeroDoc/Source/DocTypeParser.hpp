#pragma once

#include "RawDocumentation.hpp"
#include "DocTypeTokens.hpp"

namespace Zero
{
    class Visitor;

    class Type;
    class Variable;
    class Function;

    class BlockNode;
    class FunctionNode;
    class CallNode;
    class ParameterNode;
    class TypeNode;
    class StatementNode;

    /// Parses block of code pointed to by tokens and returns the initial block for parsed code
    UniquePointer<BlockNode> ParseBlock(TypeTokens* tokens);

    /// Replaces any MacroComment options used in comment if it exists in the macro expansion scope
    void DoCommentVariableReplacements(TypeTokens &comment);

    /// contains the grammar for interpreting tokens into useful language constructs 
    class DocTypeParser
    {
    public:
      /// Parser requires being constructed with a token list and optionally an index into tokens
      explicit DocTypeParser(TypeTokens& tokens, int index = 0)
        : mTokens(tokens), mIndex(index) {}


      ////////////////////
      //Rules
      ////////////////////

      /// Creates and returns a block node if a block of code exists
      UniquePointer<BlockNode> Block(void);

      /// Creates and returns a FunctionNode if a function call exists at index
      FunctionNode* Function(void);

      /// Creates and Returns a CallNode if there is a macro call
      CallNode* Call(void);

      /// Used by Function And Call to get any parameters that exists in Function or Macro call
      UniquePointer<ParameterNode> Parameter(void);

      /// Returns a TypeNode if a NamedType or a FunctionType node exists
      UniquePointer<TypeNode> Type(void);

      /// Returns a TypeNode if we have an identifier that can have a namespace and pointer/ref
      UniquePointer<TypeNode> NamedType(void);

      /// TODO: currently not implemented because no documented macros seem to have need yet
      UniquePointer<TypeNode> FunctionType(void);

    private:
      ////////////////////
      //Helpers
      ////////////////////

      /// Will iterate past a token of type 'type' and return true if it exists
      bool accept(DocTokenType::Enum type);
      /// Will iterate past a token of type 'type' and return true if it exists at token
      bool accept(DocTokenType::Enum type, DocToken*& token);

      /// Will iterate past a token with expectedValue if it exists and throw if it does not
      template <typename T>
      T expect(T expectedValue);

      /// Will iterate past token with expectedValue if it exists and throw errorMsg if it does not
      template <typename T>
      T expect(T expectedValue, String errorMsg);

      // bool output

      /// Will iterate past a token of type if it exists and throw if it does not
      bool expect(DocTokenType::Enum type);
      /// Will iterate past a token of type if it exists and throw errorMsg if it does not
      bool expect(DocTokenType::Enum type, String errorMsg);
      /// Will iterate past a token of type if it exists and throw if it does not
      bool expect(bool expected);
      /// Will iterate past a token of type if it exists and throw errorMsg if it does not
      bool expect(bool expected, String errorMsg);

      // token name output

      /// Same as bool return expect FNs except returns token in 'output'
      void expect(DocTokenType::Enum type, DocToken*& output);
      /// Same as bool return expect FNs except returns token in 'output'.
      void expect(DocTokenType::Enum type, String errorMsg, DocToken*& output);

      ////////////////////
      //Data
      ////////////////////

      TypeTokens& mTokens;

      unsigned mIndex;
    };

    class AbstractNode
    {
    public:
      AbstractNode() {};

      /// documentation that should never be written to classDoc should override this to throw
      virtual void AddToClassDoc(RawClassDoc *doc) = 0;
    };

    class BlockNode : public AbstractNode
    {
    public:
      /// throws error since BlockNodes have no proper way to be added to class doc
      virtual void AddToClassDoc(RawClassDoc *doc) override;

      Array<UniquePointer<AbstractNode> > mGlobals;
    };

    class StatementNode : public AbstractNode
    {
    public:
      /// remains pure virtual because if you are calling add to class on this you are wrong
      virtual void AddToClassDoc(RawClassDoc *doc) = 0;

    };

    class TypeNode : public AbstractNode
    {
    public:
      TypeNode() {};

      /// remains pure virtual because if you are calling add to class on this you are wrong
      virtual void AddToClassDoc(RawClassDoc *doc) override;

      // for ease of extracting info we are going to maintain a list of our tokens
      Array<DocToken *> mTokens;
    };

    class VariableNode : public StatementNode
    {
    public:
      VariableNode() {};

      /// remains pure virtual because if you are calling add to class on this you are wrong
      virtual void AddToClassDoc(RawClassDoc *doc) = 0;

      DocToken *mName;

      UniquePointer<TypeNode> mType;

      // The variable node should create the following symbol
      Variable* mSymbol;
    };

    class ParameterNode : public VariableNode
    {
    public:
      /// Throws error becuase this should not be called but can be by mistake easily
      virtual void AddToClassDoc(RawClassDoc *doc) override;

    };

    class FunctionNode : public AbstractNode
    {
    public:
      FunctionNode() {};

      /// Adds function call to ClassDoc by filling a "RawMethodDoc" class
      virtual void AddToClassDoc(RawClassDoc *doc) override;

      DocToken *mName;

      Array<UniquePointer<ParameterNode> > mParameters;

      // Can be null
      UniquePointer<TypeNode> mReturnType;

      String mComment;
    };

    class CallNode : public StatementNode
    {
    public:

      /// Will throw error because if this is called it means there is an unexpanded macro in block
      virtual void AddToClassDoc(RawClassDoc *doc) override;

      Array<DocToken *> mArguments;

      DocToken *mName;

      String mComment;
    };

    // Throw this exception type when a parsing error occurrs
    class ParsingException : public std::exception
    {
    public:
      ParsingException();
      ParsingException(StringParam error);
      const char* what() const override;
      String mError;
    };
}