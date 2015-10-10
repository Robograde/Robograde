#include <gtest/gtest.h>

TEST(first, firsttest)
{
	ASSERT_EQ(1, 2) << "Nope nope nope";
}

TEST(first, secondtest)
{
	ASSERT_EQ(1, 1) << "yes!";
}

TEST(first, thirdtest)
{
	ASSERT_EQ(1, 3) << "nope!";
}
