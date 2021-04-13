#include "Precompiled.hpp"

#include "DocTypeParser.hpp"
#include "MacroDatabase.hpp"

namespace Zero
{
////////////////////
//The Macro Toolbox (tm)
////////////////////
#define Make_Node(NodeType) UniquePointer<NodeType> node = new NodeType;

#define Get_Statements while (node->mStatements.PushBack(this->Statement()));

#define Accept_Rule node.Release()

////////////////////
//Call to get parsed block
////////////////////
UniquePointer<BlockNode> ParseBlock(TypeTokens* tokens)
{
  DocTypeParser parser(*tokens);

  UniquePointer<BlockNode> retVal;
  try
  {
    retVal = parser.Block();
  }
  catch (ParsingException &e)
  {
    // for now do nothing  but break
    Zero::DoNotifyError("Parsing error", e.what());
  }

  return retVal.Release();
}


////////////////////////////////////////////////////////////////////////
// DocTypeParser (helpers)
////////////////////////////////////////////////////////////////////////
bool DocTypeParser::accept(DocTokenType::Enum type)
{
  // automatically return false if we are at the end of the token stream
  if (mIndex >= mTokens.Size())
  {
    return false;
  }

  // since we are not at the end, check if the token matches, if it does move index forward
  if (type == mTokens[mIndex].mEnumTokenType)
  {

    ++mIndex;
    return true;
  }

  return false;
}

bool DocTypeParser::accept(DocTokenType::Enum type, DocToken*& token)
{
  // automatically return false if we are at the end of the token stream
  if (mIndex >= mTokens.Size())
  {
    return false;
  }

  // since we are not at the end, check if the token matches, if it does move index forward
  if (type == mTokens[mIndex].mEnumTokenType)
  {
    // PrintRule::AcceptedToken(mTokens[mIndex]);

     // HERE is the new addition with this accept, we copy the token over
    token = &mTokens[mIndex];

    ++mIndex;
    return true;
  }

  return false;
}

////
//Bool expects
////
bool DocTypeParser::expect(DocTokenType::Enum type)
{
  if (accept(type))
  {
    return true;
  }

  throw ParsingException();
}


bool DocTypeParser::expect(DocTokenType::Enum type, String errorMsg)
{
  if (accept(type))
  {
    return true;
  }

  throw ParsingException(errorMsg);
}

bool DocTypeParser::expect(bool expected)
{
  if (expected)
    return true;

  throw ParsingException();
}

bool DocTypeParser::expect(bool expected, String errorMsg)
{
  if (expected)
  {
    return true;
  }

  throw ParsingException(errorMsg);
}

////
//Token out expects
////
void DocTypeParser::expect(DocTokenType::Enum type, DocToken*& output)
{
  if (accept(type, output))
  {
    return;
  }

  throw ParsingException();
}

void DocTypeParser::expect(DocTokenType::Enum type, String errorMsg, DocToken*& output)
{
  if (accept(type, output))
  {
    return;
  }

  throw ParsingException(errorMsg);
}

////
//Template type expects
////
template < typename T >
T DocTypeParser::expect(T expectedValue)
{
  if (expectedValue)
    return expectedValue;

  throw ParsingException();
}

template < typename T >
T DocTypeParser::expect(T expectedValue, String errorMsg)
{
  if (expectedValue)
    return expectedValue;

  throw ParsingException(errorMsg);
}


////////////////////////////////////////////////////////////////////////
// DocTypeParser (rules)
////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------Block
// Block = (Function|BlockNode|CallNode)*
UniquePointer<BlockNode> DocTypeParser::Block(void)
{
  Make_Node(BlockNode);

  for (;;)
  {
    UniquePointer<AbstractNode> newNode = Function();

    if (!newNode)
    {
      newNode = Call();
    }


    if (newNode != nullptr)
    {
      node->mGlobals.PushBack(newNode.Release());
      continue;
    }
    break;
  }

  return Accept_Rule;
}

// check for macro expansion, create a new code block if one is found
void BlockNode::AddToClassDoc(RawClassDoc* doc)
{

  forRange(AbstractNode *node, mGlobals.All())
  {
    node->AddToClassDoc(doc);
  }

}

//--------------------------------------------------------------------------------Call
//callNode = <Comment>? <Identifier> <OpenParentheses> 
//(Parameter (<Comma> Parameter)*)? <CloseParentheses>
CallNode* DocTypeParser::Call(void)
{
  DocToken *comment = nullptr;

  accept(DocTokenType::Comment, comment);

  DocToken *name;

  accept(DocTokenType::Identifier, name);

  if (!name)
    return nullptr;

  Make_Node(CallNode);

  if (comment)
    node->mComment = comment->mText;

  node->mName = name;


  expect(DocTokenType::OpenParen, "Expected OpenParentheses for call");

  DocToken *arg;


  if (accept(DocTokenType::Identifier, arg) || accept(DocTokenType::StringLiteral, arg))
  {
    node->mArguments.PushBack(arg);

    while (accept(DocTokenType::Comma))
    {
      arg = nullptr;
      expect(accept(DocTokenType::Identifier, arg) || accept(DocTokenType::StringLiteral, arg)
        , "Expected parameter after comma, did you leave a trailing comma?");
      node->mArguments.PushBack(arg);
    }
  }

  expect(DocTokenType::CloseParen, "Expected CloseParentheses after parameter list");

  // just eat the semicolon because macros are gonna macro sometimes
  accept(DocTokenType::Semicolon);

  return Accept_Rule;
}

// check for macro expansion, create a new code block if one is found
void CallNode::AddToClassDoc(RawClassDoc* doc)
{
  if (mComment.Empty())
    return;

  TypeTokens commentTokens;

  AppendTokensFromString(DocLangDfa::Get(), mComment, &commentTokens);

  // if it is blank it does not even contain the MacroComment keyword
  if (commentTokens.Size() < 1)
    return;

  // check for a macro comment directive
  if (commentTokens[0].mText != "MacroComment")
    return;

  MacroCall call;

  call.mClass = doc;

  // parse any options in the comment really quick
  call.ParseOptions(commentTokens);

  //call.DoOptionExpansion();


  // copy over any arguments that were passed to the macro call
  forRange(DocToken* token, mArguments.All())
  {
    call.mMacroArgs.PushBack(token->mText);
  }

  MacroDatabase* database = MacroDatabase::GetInstance();

  // just return out if we do not load the macro
  if (!call.LoadMacroWithName(mName->mText))
    return;

  // expand our new call
  call.ExpandCall();

  UniquePointer<BlockNode> parsedMacro = ParseBlock(&call.mExpandedMacro);

  // Will recurse again if more macros are present
  if (parsedMacro)
    parsedMacro->AddToClassDoc(doc);

  MacroDatabase::GetInstance()->mMacroExpandStack.PopBack();
}

void ExpandCommentVariables(String* comment)
{
  // if we have no comment just return 
  if (!comment)
    return;

  TypeTokens commentTokens;

  AppendTokensFromString(DocLangDfa::Get(), *comment, &commentTokens);

  StringBuilder newComment;

  // note: the "minus one" is so we do not go out of bounds when grabbing token after '$'
  for (uint i = 0; i < commentTokens.Size(); ++i)
  {
    DocToken *commentToken = &commentTokens[i];

    if (commentToken->mEnumTokenType == DocTokenType::DollarSign)
    {
      // move to the next comment token
      ++i;

      if (i >= commentTokens.Size())
      {
        DocLogger::Get()->Write("ingnoring unnamed comment option");
        break;
      }

      commentToken = &commentTokens[i];

      // check if this token is actually a comment variable/option
      StringRef value = MacroDatabase::GetInstance()->SearchMacroExpandStackForOption(commentToken->mText);

      // make sure we actually found the option
      if (value != "")
      {
        newComment << value << " ";
      }
      // if it was empty it was invalid option so print the error and move on
      else
      {
        DocLogger::Get()->Write("unable to find comment option '%s'", commentToken->mText.c_str());
      }
    }
    else
    {
      newComment << commentToken->mText << " ";
    }
  }

  *comment = newComment.ToString();
}

//--------------------------------------------------------------------------------Function
// FunctionDec = <Comment>? Type <Identifier> 
//  <OpenParentheses> (Parameter (<Comma> Parameter)*)?  <CloseParentheses> <keyword>? <SemiColon>?
FunctionNode* DocTypeParser::Function(void)
{
  unsigned startIndex = mIndex;

  DocToken *comment = nullptr;

  accept(DocTokenType::Comment, comment);

  UniquePointer<TypeNode> retType = Type();

  if (!retType)
    return nullptr;

  Make_Node(FunctionNode);

  if (comment)
    node->mComment = comment->mText;

  node->mReturnType = retType.Release();

  // we are going to reset the index to before
  if (!accept(DocTokenType::Identifier, node->mName))
  {
    mIndex = startIndex;
    return nullptr;
  }

  ExpandCommentVariables(&(node->mComment));

  expect(DocTokenType::OpenParen, "Expected OpenParentheses for parameter specification");

  UniquePointer<ParameterNode> param = Parameter();
  if (param)
  {
    node->mParameters.PushBack(param.Release());

    while (accept(DocTokenType::Comma))
    {
      param = Parameter();
      expect(param != nullptr, "Expected parameter after comma, did you leave a trailing comma?");
      node->mParameters.PushBack(param.Release());
    }
  }

  expect(DocTokenType::CloseParen, "Expected CloseParentheses after parameter list");

  // eat any trailing identifiers, they would have been thrown away anyway
  while (accept(DocTokenType::Identifier) || accept(DocTokenType::ConstQualifier)) {}

  // because macros you can totally not have a semicolon if you are a jerk
  accept(DocTokenType::Semicolon);

  return Accept_Rule;
}

void FunctionNode::AddToClassDoc(RawClassDoc *doc)
{
  RawMethodDoc* newMethod = new RawMethodDoc();

  newMethod->mDescription = mComment;

  newMethod->mName = mName->mText;

  CopyArrayOfTokenPtrToTypeTokens(mReturnType->mTokens, newMethod->mReturnTokens);

  // since param needs to save to the method doc we will implement it here
  forRange(ParameterNode *param, mParameters.All())
  {
    RawMethodDoc::Parameter *newParam = new RawMethodDoc::Parameter;

    CopyArrayOfTokenPtrToTypeTokens(param->mType->mTokens, newParam->mTokens);

    newParam->mName = param->mName->mText;

    newMethod->mParsedParameters.PushBack(newParam);
  }

  doc->mMethods.PushBack(newMethod);

  doc->mMethodMap[newMethod->mName].Append(newMethod);
}

//--------------------------------------------------------------------------------Parameter
// Parameter = Type <Identifier>
UniquePointer<ParameterNode> DocTypeParser::Parameter(void)
{
  UniquePointer<TypeNode> type = Type();

  if (!type)
    return nullptr;

  Make_Node(ParameterNode);

  expect(DocTokenType::Identifier, "Expected identifier for parameter", node->mName);

  node->mType = type.Release();

  return Accept_Rule;
}

void ParameterNode::AddToClassDoc(RawClassDoc* doc)
{
  Error("ParameterNodes should never directly be added to doc, \
ld be automatically added by FunctionNode instead");
}

//--------------------------------------------------------------------------------TypeNode
/// Type = NamedType | FunctionType
UniquePointer<TypeNode> DocTypeParser::Type(void)
{
  UniquePointer<TypeNode> node = NamedType();

  if (node == nullptr)
  {
    node = FunctionType();
  }

  return Accept_Rule;
}

void TypeNode::AddToClassDoc(RawClassDoc* doc)
{
  Error("TypeNodes should never directly be added to doc");
}

// NOTE: Ignoring namespace for now
//NamedType = Namespace*<Identifier> <Asterisk>* <Ampersand>?
UniquePointer<TypeNode> DocTypeParser::NamedType(void)
{
  DocToken *currToken = nullptr;

  accept(DocTokenType::StaticQualifier);

  Make_Node(TypeNode);

  if (accept(DocTokenType::ConstQualifier, currToken))
  {
    node->mTokens.PushBack(currToken);
  }

  if (!accept(DocTokenType::Identifier, currToken)
    && !accept(DocTokenType::Void, currToken))
  {
    if (!node->mTokens.Empty())
      throw ParsingException("Invalid Const Qualifier Found");

    return nullptr;
  }


  node->mTokens.PushBack(currToken);

  while (accept(DocTokenType::Pointer, currToken))
  {
    node->mTokens.PushBack(currToken);
  }

  if (accept(DocTokenType::Reference, currToken))
  {
    node->mTokens.PushBack(currToken);
  }

  return Accept_Rule;
}

// FunctionType = Type <Asterisk>+ <Ampersand>? 
//   <OpenParentheses> Type (<Comma> Type)* <CloseParentheses>
UniquePointer<TypeNode> DocTypeParser::FunctionType(void)
{
  // not supporting this for now, has not come up and seems annoying
  return nullptr;
}


////////////////////////////////////////////////////////////////////////
// ParsingException
////////////////////////////////////////////////////////////////////////
ParsingException::ParsingException() : mError("ParsingException Occured") {}

ParsingException::ParsingException(StringParam error) : mError(error) {}

const char* ParsingException::what() const
{
  return mError.c_str();
}

}