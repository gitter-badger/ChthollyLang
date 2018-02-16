#define GTEST_HAS_STD_TUPLE_ 1
#define GTEST_HAS_TR1_TUPLE 0

#include <gtest/gtest.h>
#include <chtholly.h>

using namespace Chtholly;

std::string ToString(const ParseTree::Observer& v)
{
	std::string out;

	const auto isTerm = v.value().type == ParseUnit::Type::term;

	if (isTerm) out += '(';

	if (isTerm)
	{
		out += v.value().name + ' ';
	}
	else
	{
		out += v.value().name + '[' + std::string(v.value().value) + "] ";
	}

	for (auto i = v.childrenBegin(); i != v.childrenEnd(); ++i)
	{
		out += ToString(i);
	}

	if (isTerm) out += ')';

	return out;
}

std::ostream& operator<<(std::ostream& out, const ParseTree& tree)
{
	out << ToString(tree.observer());
	return out;
}

ParseTree parseString(const std::string_view& input_string)
{
	ParseTree tree;
	Parser::Expression(Parser::MakeInfo(input_string, tree.modifier()));
	return tree;
}

TEST(Token, IntegerLiteral) {
	EXPECT_EQ(parseString("1"),ParseTree(Token("IntLiteral","1")));
	EXPECT_EQ(parseString("1234567890"), ParseTree(Token("IntLiteral", "1234567890")));
	EXPECT_EQ(parseString("78294385928147234687253"), ParseTree(Token("IntLiteral", "78294385928147234687253")));
}

TEST(Token, FloatLiteral)
{
	EXPECT_EQ(parseString("1."), ParseTree(Token("FloatLiteral", "1.")));
	EXPECT_EQ(parseString("01.e0222"), ParseTree(Token("FloatLiteral", "01.e0222")));
	EXPECT_EQ(parseString("2.345"), ParseTree(Token("FloatLiteral", "2.345")));
	EXPECT_EQ(parseString("6.78e-90"), ParseTree(Token("FloatLiteral", "6.78e-90")));
	EXPECT_EQ(parseString("234235.345323333333e+3435756756782"), ParseTree(Token("FloatLiteral", "234235.345323333333e+3435756756782")));
}

TEST(Token, StringLiteral)
{
	EXPECT_EQ(parseString(R"("hello")"), ParseTree(Token("StringLiteral", R"("hello")")));

	//EXPECT_EQ(parseString(R"("\thello\n\"")"), ParseTree(Token("StringLiteral", R"("\thello\n\"")")));
	EXPECT_EQ(parseString("\"\\thello\\n\\\"\""), ParseTree(Token("StringLiteral", "\"\\thello\\n\\\"\"")));

	auto tested_string_literal = R"("
		Dear user:
			Hello!
			Enjoy it.
	")";

	EXPECT_EQ(parseString(tested_string_literal), ParseTree(Token("StringLiteral", tested_string_literal)));
}

TEST(Token, IdentifierLiteral)
{
	EXPECT_EQ(parseString("null"), ParseTree(Token("NullLiteral", "null")));
	EXPECT_EQ(parseString("undef"), ParseTree(Token("UndefinedLiteral", "undef")));
	EXPECT_EQ(parseString("true"), ParseTree(Token("TrueLiteral", "true")));
	EXPECT_EQ(parseString("false"), ParseTree(Token("FalseLiteral", "false")));
}

TEST(Token, Identifier)
{
	EXPECT_EQ(parseString("a"), ParseTree(Token("Identifier", "a")));
	EXPECT_EQ(parseString("integer_container"), ParseTree(Token("Identifier", "integer_container")));
	EXPECT_EQ(parseString("_Static_assert"), ParseTree(Token("Identifier", "_Static_assert")));
	EXPECT_EQ(parseString("iHave100AppleForYou"), ParseTree(Token("Identifier", "iHave100AppleForYou")));
}

TEST(Expression, DefineExpression)
{
	EXPECT_EQ(parseString("var x"), ParseTree(Term("VarDefineExpression", Token("Identifier", "x"))));
	EXPECT_EQ(parseString("const hello"), ParseTree(Term("ConstDefineExpression", Token("Identifier", "hello"))));
	EXPECT_EQ(parseString("var y: int"), ParseTree(Term("VarDefineExpression", Token("Identifier", "y"), Token("Identifier", "int"))));
}
