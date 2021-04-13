#include "Precompiled.hpp"

#include <Engine/EngineStandard.hpp>

#include "MacroDatabase.hpp"
#include "DocTypeParser.hpp"
#include "DocTypeTokens.hpp"
#include "RawDocumentation.hpp"

namespace Zero
{


/// super basic test that checks if we can expand trivial macros properly
bool doTest0(void)
{
  // For Test0
  String simpleArgPassingTestString =
    "#define TestsSimpleArgPassing(A, B)\\\
      void ExampleFn(A B);";

  String simpleArgPassingCallTestString = "TestsSimpleArgPassing(unsigned, potato)";

  String exampleOutputString = "void ExampleFn(unsigned potato);";

  TypeTokens exampleOutputTokens;
  AppendTokensFromString(DocLangDfa::Get(), exampleOutputString, &exampleOutputTokens);


  // init the Macro
  MacroData testMacro(simpleArgPassingTestString);

  testMacro.mParameters.PushBack("A");
  testMacro.mParameters.PushBack("B");


  // init the MacroCall (we are cheating a bunch since parsing relies to heavy on xml
  MacroCall testCall;
  testCall.mClass = nullptr;
  testCall.mMacroArgs.PushBack("unsigned");
  testCall.mMacroArgs.PushBack("potato");
  testCall.mMacro = &testMacro;

  testCall.ExpandCall();

  return exampleOutputTokens == testCall.mExpandedMacro;
}

/// this not only is a more complex Test0, it also tests MacroComment option extraction
bool doTest1(void)
{
  MacroDatabase::GetInstance()->mMacroExpandStack.Clear();
  // For Test1
  String AnchorAccessorsMacroTestString =
    "#define DeclareAnchorAccessors(ConstraintType, anchor)                                  \\\
      /* The local point of the anchor on object A.*/                                        \\\
      Vec3 GetLocalPointA() const;                                                           \\\
      void SetLocalPointA(Vec3Param localPoint);                                             \\\
      /* The local point of the anchor on object B.*/                                        \\\
      Vec3 GetLocalPointB() const;                                                           \\\
      void SetLocalPointB(Vec3Param localPoint);                                             \\\
      /* The position of the anchor on object A given a position in world space*/            \\\
      Vec3 GetWorldPointA();                                                                 \\\
      void SetWorldPointA(Vec3Param worldPoint);                                             \\\
      /* The position of the anchor on object B given a position in world space*/            \\\
      Vec3 GetWorldPointB();                                                                 \\\
      void SetWorldPointB(Vec3Param worldPoint);                                             \\\
      /* Sets the position of the anchor on object A and B given a position in world space*/ \\\
      void SetWorldPoints(Vec3Param point);                                                  \\\
      /* Virtual function for when an object link point changes*/                            \\\
      void ObjectLinkPointUpdated(size_t edgeIndex, Vec3Param localPoint) override;";

  String AnchorAccessorsMacroCallTestString = "DeclareAnchorAccessors(PositionJoint, mAnchors);";

  String AnchorAccessorsMacroCallCommentTestString
    = "MacroComment Location : \"Systems/Physics/Joints/ConstraintAtomDefines.hpp\"";

  UnsortedMap<String, String> testOptions;
  static const String location = "location";
  testOptions[location] = "Systems/Physics/Joints/ConstraintAtomDefines.hpp";

  String exampleOutputString = "\
      /* The local point of the anchor on object A.*/                                        \
      Vec3 GetLocalPointA() const;                                                           \
      void SetLocalPointA(Vec3Param localPoint);                                             \
      /* The local point of the anchor on object B.*/                                        \
      Vec3 GetLocalPointB() const;                                                           \
      void SetLocalPointB(Vec3Param localPoint);                                             \
      /* The position of the anchor on object A given a position in world space*/            \
      Vec3 GetWorldPointA();                                                                 \
      void SetWorldPointA(Vec3Param worldPoint);                                             \
      /* The position of the anchor on object B given a position in world space*/            \
      Vec3 GetWorldPointB();                                                                 \
      void SetWorldPointB(Vec3Param worldPoint);                                             \
      /* Sets the position of the anchor on object A and B given a position in world space*/ \
      void SetWorldPoints(Vec3Param point);                                                  \
      /* Virtual function for when an object link point changes*/                            \
      void ObjectLinkPointUpdated(size_t edgeIndex, Vec3Param localPoint) override;";

  TypeTokens exampleOutputTokens;
  AppendTokensFromString(DocLangDfa::Get(), exampleOutputString, &exampleOutputTokens);

  // init the Macro
  MacroData testMacro(AnchorAccessorsMacroTestString);
  testMacro.mParameters.PushBack("ConstraintType");
  testMacro.mParameters.PushBack("anchor");


  // init the MacroCall (we are cheating a bunch since parsing relies to heavy on xml)
  MacroCall testCall;
  testCall.mClass = nullptr;
  testCall.mMacroArgs.PushBack("PositionJoint");
  testCall.mMacroArgs.PushBack("mAnchors");
  testCall.mMacro = &testMacro;

  // this part tests mostly just what we tested in test0
  testCall.ExpandCall();

  // now test the option extraction
  TypeTokens commentToken;
  AppendTokensFromString(DocLangDfa::Get(), AnchorAccessorsMacroCallCommentTestString, &commentToken);

  testCall.ParseOptions(commentToken);

  bool testStatus = testCall.GetOption(location) != "";
  testStatus &= testCall.GetOption(location) == testOptions[location];
  testStatus &= exampleOutputTokens == testCall.mExpandedMacro;

  return testStatus;
}


/// Test2 checks if we handle the "strigify" macro operator correctly
bool doTest2(void)
{
  MacroDatabase::GetInstance()->mMacroExpandStack.Clear();

  // For Test2
  String stringifyTestString =
    "#define TestStringify(Str)\\\
      void ExampleFnWithStrDefault(String example = #Str);";

  String stringifyCallTestString = "TestStringify(defaultValue)";

  String exampleOutputString = "void ExampleFnWithStrDefault(String example = \"defaultValue\");";

  TypeTokens exampleOutputTokens;
  AppendTokensFromString(DocLangDfa::Get(), exampleOutputString, &exampleOutputTokens);

  // init the Macro
  MacroData testMacro(stringifyTestString);
  testMacro.mParameters.PushBack("Str");

  // init the MacroCall (we are cheating a bunch since parsing relies to heavy on xml)
  MacroCall testCall;
  testCall.mMacroArgs.PushBack("defaultValue");
  testCall.mMacro = &testMacro;

  testCall.ExpandCall();

  return testCall.mExpandedMacro == exampleOutputTokens;
}


/// Test3 tests if we do macro concatination correctly
bool doTest3(void)
{
  MacroDatabase::GetInstance()->mMacroExpandStack.Clear();

  // For Test3
  String concatTestString =
    "#define TestConcat(A,B)\\\
      void A##B##Fn(A a, B b);";

  String concatCallTestString = "TestConcat(Super, Rad);";

  String exampleOutputString = "void SuperRadFn(Super a, Rad b);";

  TypeTokens exampleOutputTokens;
  AppendTokensFromString(DocLangDfa::Get(), exampleOutputString, &exampleOutputTokens);

  // init the macro
  MacroData testMacro(concatTestString);
  testMacro.mParameters.PushBack("A");
  testMacro.mParameters.PushBack("B");

  // init the MacroCall (we are cheating a bunch since parsing relies to heavy on xml)
  MacroCall testCall;
  testCall.mMacroArgs.PushBack("Super");
  testCall.mMacroArgs.PushBack("Rad");
  testCall.mMacro = &testMacro;

  testCall.ExpandCall();

  return testCall.mExpandedMacro == exampleOutputTokens;
}

/// Test4 uses actual code to see if simple macro expand works on larger example (codename: Davis Testcase)
bool doTest4(void)
{
  MacroDatabase::GetInstance()->mMacroExpandStack.Clear();

  // From Test1
  String AnchorAccessorsMacroTestString =
    "#define DeclareAnchorAccessors(ConstraintType, anchor)                                  \\\
      /* The local point of the anchor on object A.*/                                        \\\
      Vec3 GetLocalPointA() const;                                                           \\\
      void SetLocalPointA(Vec3Param localPoint);                                             \\\
      /* The local point of the anchor on object B.*/                                        \\\
      Vec3 GetLocalPointB() const;                                                           \\\
      void SetLocalPointB(Vec3Param localPoint);                                             \\\
      /* The position of the anchor on object A given a position in world space*/            \\\
      Vec3 GetWorldPointA();                                                                 \\\
      void SetWorldPointA(Vec3Param worldPoint);                                             \\\
      /* The position of the anchor on object B given a position in world space*/            \\\
      Vec3 GetWorldPointB();                                                                 \\\
      void SetWorldPointB(Vec3Param worldPoint);                                             \\\
      /* Sets the position of the anchor on object A and B given a position in world space*/ \\\
      void SetWorldPoints(Vec3Param point);                                                  \\\
      /* Virtual function for when an object link point changes*/                            \\\
      void ObjectLinkPointUpdated(size_t edgeIndex, Vec3Param localPoint) override;";

  String AnchorAccessorsMacroCallTestString = "DeclareAnchorAccessors(PositionJoint, mAnchors);";

  String AnchorAccessorsMacroCallCommentTestString
    = "///MacroComment Location : \"Systems/Physics/Joints/ConstraintAtomDefines.hpp\"";

  UnsortedMap<String, String> testOptions;
  static const String location = "location";
  testOptions[location] = "\"Systems/Physics/Joints/ConstraintAtomDefines.hpp\"";

  String exampleOutputString =
    "/* The local point of the anchor on object A.*/                                        \\\
      Vec3 GetLocalPointA() const;                                                           \\\
      void SetLocalPointA(Vec3Param localPoint);                                             \\\
      /* The local point of the anchor on object B.*/                                        \\\
      Vec3 GetLocalPointB() const;                                                           \\\
      void SetLocalPointB(Vec3Param localPoint);                                             \\\
      /* The position of the anchor on object A given a position in world space*/            \\\
      Vec3 GetWorldPointA();                                                                 \\\
      void SetWorldPointA(Vec3Param worldPoint);                                             \\\
      /* The position of the anchor on object B given a position in world space*/            \\\
      Vec3 GetWorldPointB();                                                                 \\\
      void SetWorldPointB(Vec3Param worldPoint);                                             \\\
      /* Sets the position of the anchor on object A and B given a position in world space*/ \\\
      void SetWorldPoints(Vec3Param point);                                                  \\\
      /* Virtual function for when an object link point changes*/                            \\\
      void ObjectLinkPointUpdated(size_t edgeIndex, Vec3Param localPoint) override;";

  TypeTokens exampleOutputTokens;
  AppendTokensFromString(DocLangDfa::Get(), exampleOutputString, &exampleOutputTokens);

  // init the Macro
  MacroData testMacro(AnchorAccessorsMacroTestString);
  testMacro.mParameters.PushBack("ConstraintType");
  testMacro.mParameters.PushBack("anchor");

  // init the MacroCall (we are cheating a bunch since parsing relies to heavy on xml)
  MacroCall testCall;
  RawClassDoc *testClass = new RawClassDoc("testClass");
  testCall.mClass = testClass;
  testCall.mMacroArgs.PushBack("PositionJoint");
  testCall.mMacroArgs.PushBack("mAnchors");
  testCall.mMacro = &testMacro;

  // this part tests mostly just what we tested in test0
  testCall.ExpandCall();

  testCall.AddExpandedMacroDocToRawClass();

  //TODO: write the rest of the test so it compares with the known correct output

  bool retVal = true;

  // check if the class has all of the functions it should
  retVal &= testClass->mMethodMap.ContainsKey("GetLocalPointA")
    && testClass->mMethodMap.ContainsKey("SetLocalPointA")
    && testClass->mMethodMap.ContainsKey("GetLocalPointB")
    && testClass->mMethodMap.ContainsKey("SetLocalPointB")
    && testClass->mMethodMap.ContainsKey("GetWorldPointA")
    && testClass->mMethodMap.ContainsKey("SetWorldPointA")
    && testClass->mMethodMap.ContainsKey("GetWorldPointB")
    && testClass->mMethodMap.ContainsKey("SetWorldPointB")
    && testClass->mMethodMap.ContainsKey("SetWorldPoints")
    && testClass->mMethodMap.ContainsKey("ObjectLinkPointUpdated");

  return retVal;
}

// Tests if macro expand and concat work on a non-trivial code example(codename: Andrew Testcase)
bool doTest5(void)
{
  MacroDatabase::GetInstance()->mMacroExpandStack.Clear();

  String AndrewMacroTestString0 = "\
#define DeclareVariantGetSetForArithmeticTypes(property)       \\\
/* MacroComment*/                                              \\\
DeclareVariantGetSetForType(property, Integer,       int);     \\\
/* MacroComment*/                                              \\\
DeclareVariantGetSetForType(property, DoubleInteger, s64);     \\\
/* MacroComment*/                                              \\\
DeclareVariantGetSetForType(property, Integer2,      IntVec2); \\\
/* MacroComment*/                                              \\\
DeclareVariantGetSetForType(property, Integer3,      IntVec3); \\\
/* MacroComment*/                                              \\\
DeclareVariantGetSetForType(property, Integer4,      IntVec4); \\\
/* MacroComment*/                                              \\\
DeclareVariantGetSetForType(property, Real,          float);   \\\
/* MacroComment*/                                              \\\
DeclareVariantGetSetForType(property, DoubleReal,    double);  \\\
/* MacroComment*/                                              \\\
DeclareVariantGetSetForType(property, Real2,         Vec2);    \\\
/* MacroComment*/                                              \\\
DeclareVariantGetSetForType(property, Real3,         Vec3);    \\\
/* MacroComment*/                                              \\\
DeclareVariantGetSetForType(property, Real4,         Vec4);    \\\
/* MacroComment*/                                              \\\
DeclareVariantGetSetForType(property, Quaternion,    Quat)";

  String AndrewMacroTestString1 = "\
#define DeclareVariantGetSetForType(property, typeName, type)  \\\
void Set##property##typeName(type value);                      \\\
/* Andrew's getter test description*/                          \\\
type Get##property##typeName() const;";

  String AndrewMacroCallTestString = "DeclareVariantGetSetForArithmeticTypes(DeltaThreshold);";

  String AndrewMacroCallCommentTestString = "MacroComment comment : \"Controls the delta \
threshold at which a net property's primitive-components \
are considered changed during change detection\"";


  // so we have to add both macro fns to the database
  // after we do that we can go ahead and test the call
  // init the Macro


  MacroData *testMacro0 = new MacroData(AndrewMacroTestString0);
  testMacro0->mParameters.PushBack("property");
  testMacro0->mName = "DeclareVariantGetSetForArithmeticTypes";

  MacroData *testMacro1 = new MacroData(AndrewMacroTestString1);
  testMacro1->mParameters.PushBack("property");
  testMacro1->mParameters.PushBack("typeName");
  testMacro1->mParameters.PushBack("type");
  testMacro1->mName = "DeclareVariantGetSetForType";

  MacroDatabase &database = *MacroDatabase::GetInstance();

  database.mMacrosByName[testMacro0->mName] = testMacro0;
  database.mMacrosByName[testMacro1->mName] = testMacro1;

  // init the MacroCall (we are cheating a bunch since parsing relies to heavy on xml)
  MacroCall testCall;
  RawClassDoc *testClass = new RawClassDoc("testClass");
  testCall.mClass = testClass;

  testCall.mMacroArgs.PushBack("DeltaThreshold");
  testCall.mMacro = testMacro0;

  testCall.ExpandCall();

  testCall.AddExpandedMacroDocToRawClass();

  database.mMacrosByName.Clear();
  delete testMacro0;
  delete testMacro1;
  // since the next test really tests the macro comments, checking we got all methods is fine
  return testClass->mMethods.Size() == 22;
}


/// Test6 is just like Test5 except it also tests MacroComment option passing two layers deep
bool doTest6(void)
{
  MacroDatabase::GetInstance()->mMacroExpandStack.Clear();

  // bringing back the andrew case to test variable passing

  String AndrewMacroTestString0 =
    "#define DeclareVariantGetSetForArithmeticTypes(property)       \\\
/* MacroComment*/\\\
DeclareVariantGetSetForType(property, Integer,       int);     \\\
/* MacroComment*/\\\
DeclareVariantGetSetForType(property, DoubleInteger, s64);     \\\
/* MacroComment*/\\\
DeclareVariantGetSetForType(property, Integer2,      IntVec2); \\\
/* MacroComment*/\\\
DeclareVariantGetSetForType(property, Integer3,      IntVec3); \\\
/* MacroComment*/\\\
DeclareVariantGetSetForType(property, Integer4,      IntVec4); \\\
/* MacroComment*/\\\
DeclareVariantGetSetForType(property, Real,          float);   \\\
/* MacroComment*/\\\
DeclareVariantGetSetForType(property, DoubleReal,    double);  \\\
/* MacroComment*/\\\
DeclareVariantGetSetForType(property, Real2,         Vec2);    \\\
/* MacroComment*/\\\
DeclareVariantGetSetForType(property, Real3,         Vec3);    \\\
/* MacroComment*/\\\
DeclareVariantGetSetForType(property, Real4,         Vec4);    \\\
/* MacroComment*/\\\
DeclareVariantGetSetForType(property, Quaternion,    Quat)";

  String AndrewMacroTestString1 =
    "#define DeclareVariantGetSetForType(property, typeName, type)         \\\
/*Allows for $verb of $typename property: $property of type: $typeName. $comment.*/\\\
void Set##property##typeName(type value);                              \\\
type Get##property##typeName() const";

  String AndrewMacroCallTestString0 = "DeclareVariantGetSetForArithmeticTypes(DeltaThreshold);";
  String AndrewMacroCallCommentTestString0 = "/// MacroComment\
/// comment : \"DeltaThreshold probably does stuff\",\
/// verb : \"Tweeking\",";

  String AndrewMacroCallTestString1 = "DeclareVariantGetSetForArithmeticTypes(QuantizationRangeMin);";
  String AndrewMacroCallCommentTestString1 = "/// MacroComment\
/// comment : \"QuantizationRangeMin is gibberish to me\",\
/// verb : \"Eating\",";

  String AndrewMacroCallTestString2 = "DeclareVariantGetSetForArithmeticTypes(QuantizationRangeMax);";
  String AndrewMacroCallCommentTestString2 = "/// MacroComment\
/// comment : \"I think I am trying way to hard at writting this test case\",\
/// verb : \"Dueling\",";

  MacroData *testMacro0 = new MacroData(AndrewMacroTestString0);
  testMacro0->mParameters.PushBack("property");
  testMacro0->mName = "DeclareVariantGetSetForArithmeticTypes";

  MacroData *testMacro1 = new MacroData(AndrewMacroTestString1);
  testMacro1->mParameters.PushBack("property");
  testMacro1->mParameters.PushBack("typeName");
  testMacro1->mParameters.PushBack("type");
  testMacro1->mName = "DeclareVariantGetSetForType";

  MacroDatabase &database = *MacroDatabase::GetInstance();

  database.mMacrosByName[testMacro0->mName] = testMacro0;
  database.mMacrosByName[testMacro1->mName] = testMacro1;

  RawClassDoc *testClass = new RawClassDoc("testClass");
  MacroCall testCall0;
  testCall0.mClass = testClass;
  testCall0.mMacro = testMacro0;
  testCall0.mMacroArgs.PushBack("DeltaThreshold");
  testCall0.AddOption("comment", "DeltaThreshold probably does stuff");
  testCall0.AddOption("verb", "TWEEKING");

  MacroCall testCall1;
  testCall1.mClass = testClass;
  testCall1.mMacro = testMacro0;
  testCall1.mMacroArgs.PushBack("QuantizationRangeMin");
  testCall1.AddOption("comment", "QuantizationRangeMin is gibberish to me");
  testCall1.AddOption("verb", "EATING");

  MacroCall testCall2;
  testCall2.mClass = testClass;
  testCall2.mMacro = testMacro0;
  testCall2.mMacroArgs.PushBack("QuantizationRangeMax");
  testCall2.AddOption("comment", "I think I am trying way to hard at writing this test case");
  testCall2.AddOption("verb", "DUELING");

  testCall0.ExpandCall();
  testCall0.AddExpandedMacroDocToRawClass();

  testCall1.ExpandCall();
  testCall1.AddExpandedMacroDocToRawClass();

  testCall2.ExpandCall();
  testCall2.AddExpandedMacroDocToRawClass();


  static const int methodCount = 66;

  if (testClass->mMethods.Size() != methodCount)
    return false;

  // if we had all the methods, make sure the comments are correct (oh god there are 66)
  // change of plans, make sure the first 10 are correct
  bool retVal = true;

  retVal &= testClass->mMethods[0]->mDescription == "Allows for TWEEKING of Integer property : DeltaThreshold of type : Integer . DeltaThreshold probably does stuff . ";
  retVal &= testClass->mMethods[2]->mDescription == "Allows for TWEEKING of DoubleInteger property : DeltaThreshold of type : DoubleInteger . DeltaThreshold probably does stuff . ";
  retVal &= testClass->mMethods[52]->mDescription == "Allows for DUELING of Integer4 property : QuantizationRangeMax of type : Integer4 . I think I am trying way to hard at writing this test case . ";

  return retVal;
}

bool doAllTests(void)
{
  bool retVal = doTest0();

  retVal &= doTest1();
  retVal &= doTest2();
  retVal &= doTest3();
  retVal &= doTest4();
  retVal &= doTest5();
  retVal &= doTest6();

  return retVal;
}

bool RunMacroTests(int test)
{
  switch (test)
  {
  case 0:
    return doTest0();
  case 1:
    return doTest1();
  case 2:
    return doTest2();
  case 3:
    return doTest3();
  case 4:
    return doTest4();
  case 5:
    return doTest5();
  case 6:
    return doTest6();
  default:
    return doAllTests();
  }
}

}