#include "../NanoLexer/LexerTreeNode.h"

#include <gtest\gtest.h>

// Intersection on Interval

TEST(SetTests, IntervalIntersection1) {
    RegularExpression::Interval i1(10, 20);
    RegularExpression::Interval i2(15, 25);
    RegularExpression::Interval res(15, 20);

    auto intersect = i1.intersectWith(i2);
    ASSERT_EQ(*intersect.get(), res);

    intersect = i2.intersectWith(i1);
    ASSERT_EQ(*intersect.get(), res);
}

TEST(SetTests, IntervalIntersection2) {
    RegularExpression::Interval i1(10, 20);
    RegularExpression::Interval i2(30, 40);

    auto intersect = i1.intersectWith(i2);
    ASSERT_EQ(intersect, nullptr);

    intersect = i2.intersectWith(i1);
    ASSERT_EQ(intersect, nullptr);
}

TEST(SetTests, IntervalIntersection3) {
    RegularExpression::Interval i1(10, 20);
    RegularExpression::Interval i2(5, 40);

    auto intersect = i1.intersectWith(i2);
    ASSERT_EQ(*intersect.get(), i1);

    intersect = i2.intersectWith(i1);
    ASSERT_EQ(*intersect.get(), i1);
}

TEST(SetTests, IntervalIntersection4) {
    RegularExpression::Interval i1(10, 20);
    RegularExpression::Interval i2(21, 40);

    auto intersect = i1.intersectWith(i2);
    ASSERT_EQ(intersect, nullptr);

    intersect = i2.intersectWith(i1);
    ASSERT_EQ(intersect, nullptr);
}

TEST(SetTests, IntervalIntersection5) {
    RegularExpression::Interval i1(10, 20);
    RegularExpression::Interval i2(20, 40);
    RegularExpression::Interval res(20, 20);

    auto intersect = i1.intersectWith(i2);
    ASSERT_EQ(*intersect.get(), res);

    intersect = i2.intersectWith(i1);
    ASSERT_EQ(*intersect.get(), res);
}

// Union on Interval

TEST(SetTests, IntervalUnion1) {
    RegularExpression::Interval i1(10, 20);
    RegularExpression::Interval i2(20, 40);
    RegularExpression::Interval res(10, 40);

    auto uni = i1.unionWith(i2);
    ASSERT_EQ(*uni.get(), res);
}

TEST(SetTests, IntervalUnion2) {
    RegularExpression::Interval i1(10, 20);
    RegularExpression::Interval i2(30, 40);

    std::set<RegularExpression::Interval>  set;
    set.insert(i1);
    set.insert(i2);
    RegularExpression::MultiInterval res(std::move(set));

    auto uni = i1.unionWith(i2);
    ASSERT_EQ(*uni.get(), res);
}

TEST(SetTests, IntervalUnion3) {
    RegularExpression::Interval i1(10, 20);
    RegularExpression::Interval i2(15, 30);
    RegularExpression::Interval res(10, 30);

    auto uni = i1.unionWith(i2);
    ASSERT_EQ(*uni.get(), res);
}

// Substract

TEST(SetTests, IntervalSubstract1) {
    RegularExpression::Interval i1(5, 15);
    RegularExpression::Interval i2(10, 25);

    RegularExpression::Interval res(5, 9);

    auto sub = i1.substract(i2);
    ASSERT_EQ(*sub.get(), res);
}

TEST(SetTests, IntervalSubstract2) {
    RegularExpression::Interval i1(5, 15);
    RegularExpression::Interval i2(20, 25);

    auto sub = i1.substract(i2);
    ASSERT_EQ(*sub.get(), i1);
}

TEST(SetTests, IntervalSubstract3) {
    RegularExpression::Interval i1(5, 15);
    RegularExpression::Interval i2(5, 25);

    auto sub = i1.substract(i2);
    ASSERT_EQ(sub, RegularExpression::EmptySet::get());
}

TEST(SetTests, IntervalSubstract4) {
    RegularExpression::Interval i1(5, 15);
    RegularExpression::Interval i2(15, 25);

    RegularExpression::Interval res(5, 14);

    auto sub = i1.substract(i2);
    ASSERT_EQ(*sub.get(), res);
}

TEST(SetTests, IntervalSubstract5) {
    RegularExpression::Interval i1(5, 15);
    RegularExpression::Interval i2(15, 25);

    RegularExpression::Interval res(16, 25);

    auto sub = i2.substract(i1);
    ASSERT_EQ(*sub.get(), res);
}
