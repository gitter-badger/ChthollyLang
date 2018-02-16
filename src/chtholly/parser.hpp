/*
* Copyright 2017 PragmaTwice
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*		http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once

#include <string_view>
#include <tuple>

#include "parserc.hpp"
#include "chartype.hpp"

#define G(StringView,stringLiteral) \
	std::get<StringView>(std::make_tuple(stringLiteral##sv, L##stringLiteral##sv))


namespace Chtholly
{
	using namespace std::literals;

	template <typename StringView>
	class BasicParser : public BasicParserCombinator<StringView>
	{

	public:

		BasicParser() = delete;

		/*
		// Space = '\t' | '\n' | '\v' | '\f' | '\r' | ' '
		inline static Process Space = Match(isspace);

		// Digit = '0' ... '9'
		inline static Process Digit = Match(isdigit);

		// UpperCaseLetter = 'A' ... 'Z'
		inline static Process UpperCaseLetter = Match(isupper);

		// LowerCaseLetter = 'a' ... 'z'
		inline static Process LowerCaseLetter = Match(islower);

		// Letter = UpperCaseLetter | LowerCaseLetter
		inline static Process Letter = Match(isalpha);

		// DigitOrLetter = Digit | Letter
		inline static Process DigitOrLetter = Match(isalnum);
		*/

		#define GL(stringLiteral) G(Lang,stringLiteral)

		using CType = CharType<Char>;


		// MatchKey(A) = A ^ (DigitOrLetter | '_')
		static Process MatchKey(LangRef word)
		{
			return Match(word) ^ (Match(CType::isAlphaOrNum) | Match('_'));
		}

		static Process MatchKey(InitListRef<Lang> wordList)
		{
			return Match(wordList) ^ (Match(CType::isAlphaOrNum) | Match('_'));
		}

		// IntLiteral = Digit+
		inline static Process IntLiteral =
			Catch(+Match(CType::isDigit), "IntLiteral");

		// FloatLiteral = Digit+ '.' Digit* (('E'|'e') ('+'|'-')? Digit+)?
		inline static Process FloatLiteral =
			Catch(
				(
					+Match(CType::isDigit),
					Match('.'),
					*Match(CType::isDigit),
					~(
						Match({ 'E','e' }),
						~Match({ '+','-' }),
						+Match(CType::isDigit)
					)
				),
				"FloatLiteral");

		// UnescapedCharacter = not ('"' | '\\')
		inline static Process UnescapedCharacter =
			Match([=](Char c)
			{
				return c != '"' && c != '\\';
			});

		// EscapedCharacter = '\\' ('"' | '\\' | 'b' | 'f' | 'n' | 'r' | 't' | 'v')
		inline static Process EscapedCharacter =
			(
				Match('\\'),
				Match({ '"','\\','b','f','n','r','t','v' })
			);

		// StringLiteral = '"' (EscapedCharacter|UnescapedCharacter)* '"'
		inline static Process StringLiteral =
			Catch(
				(
					Match('"'),
					*(
						EscapedCharacter | UnescapedCharacter
					),
					Match('"')
				), 
			"StringLiteral");

		// Identifier = (Letter | '_') (DigitOrLetter | '_')*
		inline static Process Identifier =
			Catch(
				(
					(
						Match(CType::isAlpha) | Match('_')
					),
					*(
						Match(CType::isAlphaOrNum) | Match('_')
					)
				), 
			"Identifier");

		// NullLiteral = "null"
		inline static Process NullLiteral =
			Catch(MatchKey(GL("null")), "NullLiteral");

		// UndefinedLiteral = "undef"
		inline static Process UndefinedLiteral =
			Catch(MatchKey(GL( "undef")), "UndefinedLiteral");

		// TrueLiteral = "true"
		inline static Process TrueLiteral =
			Catch(MatchKey(GL( "true")), "TrueLiteral");

		// FalseLiteral = "false"
		inline static Process FalseLiteral =
			Catch(MatchKey(GL( "false")), "FalseLiteral");

		// Literal = FloatLiteral | IntLiteral | StringLiteral | NullLiteral | UndefinedLiteral | TrueLiteral | FalseLiteral
		inline static Process Literal =
			(
				FloatLiteral     |
				IntLiteral       |
				StringLiteral    |
				NullLiteral      |
				UndefinedLiteral |
				TrueLiteral      |
				FalseLiteral
			);

		// MultiLineComment = "/*" (not "*/")* "*/"
		inline static Process MultiLineComment =
			(
				Match(GL("/*")),
				AnyCharUntil(Match(GL( "*/")))
			);

		// SingleLineComment = "//" (not '\n')*
		inline static Process SingleLineComment =
			(
				Match(GL("//")),
				AnyCharUntil(Match('\n'))
			);

		// Comment = SingleLineComment | MultiLineComment
		inline static Process Comment =
			(
				SingleLineComment | 
				MultiLineComment
			);

		// Term(A) = Space* A
		static Process Term(ProcessRef lhs)
		{
			return *(*Match(CType::isSpace), Comment), *Match(CType::isSpace), lhs;
		}

		// BinaryOperator(A,B) = A (B A)*
		static Process BinaryOperator(ProcessRef operatorUponIt, ProcessRef operatorMatcher)
		{
			return operatorUponIt, *(Term(Catch(operatorMatcher,"BinaryOperator")), operatorUponIt);
		}

		// ExpressionList = '(' Expression ')'
		inline static Process ExpressionList =
			(
				Term(Match('(')),
				Expression,
				Term(Match(')'))
			);

		// ArrayList = '[' (SigleExpression (',' SigleExpression)*)? ']'
		inline static Process ArrayList =
			(
				Term(Match('[')),
				ChangeIn("ArrayList"),
				~(
					SigleExpression,
					*(
						Term(Match(',')),
						SigleExpression
					)
				),
				ChangeOut(),
				Term(Match(']'))
			);

		// DictList = '{' (SigleExpression (',' SigleExpression)*)? '}'
		inline static Process DictList =
			(
				Term(Match('{')),
				ChangeIn("DictList"),
				~(
					SigleExpression,
					*(
						Term(Match(',')),
						SigleExpression
					)
				),
				ChangeOut(),
				Term(Match('}'))
			);

		// UndefExpression = '(' ')'
		inline static Process UndefExpression =
			(
				Term(Match('(')),
				Term(Match(')')),
				ChangeIn("UndefExpression"),
				ChangeOut()
			);

		// List = UndefExpression | ExpressionList | ArrayList | DictList
		inline static Process List =
			(
				UndefExpression|
				ExpressionList |
				ArrayList      |
				DictList
			);

		// PrimaryExpression = Literal | Identifier | List
		inline static Process PrimaryExpression =
			(
				Term(Literal)	 |
				Term(Identifier) |
				List
			);

		inline static Process ConstraintPartAtConstraintExperssion =
			(
				Term(Match(':')),
				SigleExpression
			);

		// ConstraintExperssion = Identifier (':' PrimaryExpression)?
		inline static Process ConstraintExperssion =
			(
				Term(Identifier),
				~ConstraintPartAtConstraintExperssion
			);

		// ConstraintExperssionAtPatternExperssion = Identifier "..."? (':' PrimaryExpression)?
		inline static Process ConstraintExperssionAtPatternExperssion =
			(
				ChangeIn("ConstraintExperssionAtPatternExperssion"),
				(
					Term(Identifier),
					~Term(Catch(Match(GL("...")), "Separator"))
				),
				~ConstraintPartAtConstraintExperssion,
				ChangeOut()
			);

		// MultiExpressionPackage(exp) = exp ((';'|',') exp)* (';'|',')?
		static Process MultiExpressionPackage(ProcessRef exp)
		{
			return
				(
					exp,
					*(
						Term(Catch(Match({ ',',';' }), "Separator")) ^ Term(Match(')')),
						exp
					),
					~Term(Catch(Match({ ',',';' }), "Separator"))
				);
		}

		// PatternExperssion = '(' ( ConstraintExperssionAtPatternExperssion ((','|';') ConstraintExperssionAtPatternExperssion)* (','|';')? | Atom ) ')' (':' PrimaryExpression)?
		inline static Process PatternExperssion =
			(
				Term(Match('(')),
				ChangeIn("PatternExperssion"),
				(
					Term(Match(')')) |
					(
						MultiExpressionPackage(ConstraintExperssionAtPatternExperssion),
						Term(Match(')')),
						~ConstraintPartAtConstraintExperssion
					)
				),
				ChangeOut()
			);

		// VarDefineExpression = "var" ConstraintExperssion
		inline static Process VarDefineExpression =
			(
				Term(MatchKey(GL( "var"))),
				ChangeIn("VarDefineExpression"),
				ConstraintExperssion | PatternExperssion,
				ChangeOut()
			);
		
		// ConstDefineExpression = "const" ConstraintExperssion
		inline static Process ConstDefineExpression =
			(
				Term(MatchKey(GL( "const"))),
				ChangeIn("ConstDefineExpression"),
				ConstraintExperssion | PatternExperssion,
				ChangeOut()
			);

		// DefineExpression = PrimaryExpression | VarDefineExpression | ConstDefineExpression
		inline static Process DefineExpression =
			(
				VarDefineExpression |
				ConstDefineExpression |
				PrimaryExpression
			);

		// LambdaExperssion = DefineExpression | "fn" PatternExperssion SigleExpression
		inline static Process LambdaExperssion =
			(
				(
					Term(MatchKey(GL("fn"))),
					ChangeIn("LambdaExpression"),
					PatternExperssion,
					SigleExpression,
					ChangeOut()
				) |
				DefineExpression
			);

		// ConditionExpression = LambdaExperssion | "if" '(' Expression ')' SigleExpression ("else" SigleExpression)?
		inline static Process ConditionExpression =
			(
				(
					Term(MatchKey(GL( "if"))),
					ChangeIn("ConditionExpression"),
					Term(Match('(')),
					Expression,
					Term(Match(')')),
					SigleExpression,
					~(
						Term(MatchKey(GL( "else"))),
						SigleExpression
					),
					ChangeOut()
				) |
				LambdaExperssion
			);

		// ReturnExpression = ConditionExpression | "return" SigleExpression?
		inline static Process ReturnExpression =
			(
				(
					Term(MatchKey(GL( "return"))),
					ChangeIn("ReturnExpression"),
					~Process(SigleExpression),
					ChangeOut()
				) |
				ConditionExpression
			);

		// LoopControlExpression = ReturnExpression | ("break"|"continue") SigleExpression?
		inline static Process LoopControlExpression =
			(
				(
					Term(MatchKey({ GL( "break"), GL("continue") })),
					ChangeIn("LoopControlExpression"),
					~Process(SigleExpression),
					ChangeOut()
				) |
				ReturnExpression
			);

		// WhileLoopExpression = LoopControlExpression | "while" '(' Expression ')' SigleExpression
		inline static Process WhileLoopExpression =
			(
				(
					Term(MatchKey(GL( "while"))),
					ChangeIn("WhileLoopExpression"),
					Term(Match('(')),
					Expression,
					Term(Match(')')),
					SigleExpression, 
					~(
						Term(MatchKey(GL( "else"))),
						SigleExpression
					),
					ChangeOut()
				) |
				LoopControlExpression
			);

		// DoWhileLoopExpression = WhileLoopExpression | "do" SigleExpression "while" '(' Expression ')'
		inline static Process DoWhileLoopExpression =
			(
				(
					Term(MatchKey(GL( "do"))),
					ChangeIn("DoWhileLoopExpression"),
					SigleExpression,
					Term(MatchKey(GL( "while"))),
					Term(Match('(')),
					Expression,
					Term(Match(')')),
					~(
						Term(MatchKey(GL( "else"))),
						SigleExpression
					),
					ChangeOut()
				) |
				WhileLoopExpression
			);

		// FunctionExpression = DoWhileLoopExpression List*
		inline static Process FunctionExpression =
			(
				ChangeIn("FunctionExpression"),
				DoWhileLoopExpression,
				*List,
				ChangeOut(true)
			);

		// PointExpression = FunctionExpression | PointExpression '->' FunctionExpression
		inline static Process PointExpression =
			(
				ChangeIn("PointExpression"),
				BinaryOperator(FunctionExpression, Match(GL( "->"))),
				ChangeOut(true)
			);

		// FoldExperssion = PointExpression "..."*
		inline static Process FoldExperssion =
			(
				ChangeIn("FoldExperssion"),
				PointExpression,
				*Term(Catch(Match(GL("...")),"Separator")),
				ChangeOut(true)
			);

		// UnaryExpression = FoldExperssion | (('+' | '-') | "not") UnaryExpression
		inline static Process UnaryExpression =
			(
				ChangeIn("UnaryExpression"),
				*(
					Term(Catch(Match({ '+','-' }), "UnaryOperator")) |
					Term(Catch(MatchKey(GL("not")), "UnaryOperator"))
				),
				FoldExperssion,
				ChangeOut(true)
			);

		// MultiplicativeExpression = UnaryExpression | MultiplicativeExpression ('*'|'/'|'%') UnaryExpression
		inline static Process MultiplicativeExpression =
			(
				ChangeIn("MultiplicativeExpression"),
				BinaryOperator(UnaryExpression, Match({ '*','/','%' }) ^Match('=')),
				ChangeOut(true)
			);

		// AdditiveExpression = MultiplicativeExpression | AdditiveExpression ('+'|'-') MultiplicativeExpression
		inline static Process AdditiveExpression =
			(
				ChangeIn("AdditiveExpression"),
				BinaryOperator(MultiplicativeExpression, Match({ '+','-' }) ^Match('=')),
				ChangeOut(true)
			);

		// RelationalExpression = AdditiveExpression | RelationalExpression ("<="|">="|'<'|'>') AdditiveExpression
		inline static Process RelationalExpression =
			(
				ChangeIn("RelationalExpression"),
				BinaryOperator(AdditiveExpression, Match({ GL("<="), GL(">="), GL("<"), GL(">") })),
				ChangeOut(true)
			);

		// EqualityExpression = RelationalExpression | EqualityExpression ("=="|"<>") RelationalExpression
		inline static Process EqualityExpression =
			(
				ChangeIn("EqualityExpression"),
				BinaryOperator(RelationalExpression, Match({ GL("=="), GL("<>") })),
				ChangeOut(true)
			);

		// LogicalAndExpression = EqualityExpression | LogicalAndExpression "and" EqualityExpression
		inline static Process LogicalAndExpression =
			(
				ChangeIn("LogicalAndExpression"),
				BinaryOperator(EqualityExpression, MatchKey(GL( "and"))),
				ChangeOut(true)
			);

		// LogicalOrExpression = LogicalAndExpression | LogicalOrExpression "or" LogicalAndExpression
		inline static Process LogicalOrExpression =
			(
				ChangeIn("LogicalOrExpression"),
				BinaryOperator(LogicalAndExpression, MatchKey(GL( "or"))),
				ChangeOut(true)
			);

		// AssignmentOperator = '=' | '*=' | '/=' | '%=' | '+=' | '-='
		inline static Process AssignmentOperator =
			Match({ GL("="), GL("*="), GL("/="), GL("%="), GL("+="), GL("-=") });

		// AssignmentExpression = LogicalOrExpression | LogicalOrExpression AssignmentOperator SigleExpression
		inline static Process AssignmentExpression =
			(
				ChangeIn("AssignmentExpression"),
				LogicalOrExpression, *(Term(Catch(AssignmentOperator,"BinaryOperator")), SigleExpression),
				ChangeOut(true)
			);

		// PairExperssion = AssignmentExpression | PairExperssion ':' SigleExpression
		inline static Process PairExperssion =
			(
				ChangeIn("PairExperssion"),
				AssignmentExpression, *(Term(Match(':')), SigleExpression),
				ChangeOut(true)
			);

		// Cannot be inline static Process
		// SigleExpression = DoWhileLoopExpression
		static Info SigleExpression(Info info)
		{
			return PairExperssion(info);
		}

		// Cannot be inline static Process
		// Expression = SigleExpression ((';'|',') SigleExpression)* (';'|',')?
		static Info Expression(Info info)
		{
			return 
			(
				ChangeIn("Expression"),
				MultiExpressionPackage(SigleExpression),
				ChangeOut(true)
			)(info);
			
		}

		#undef GL
	};

	using Parser = BasicParser<ParserCombinator::Lang>;

}

#undef G