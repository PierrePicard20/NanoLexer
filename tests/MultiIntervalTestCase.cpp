#include "../NanoLexer/LexerTreeNode.h"

#include <gtest\gtest.h>

// Intersection on MultiInterval

TEST(SetTests, MultiIntervalIntersection1) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 15));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::Interval i2(10, 25);

    RegularExpression::MultiInterval res;
    res.addInterval(RegularExpression::Interval(10, 15));
    res.addInterval(RegularExpression::Interval(20, 25));

    auto intersect = i1.intersectWith(i2);
    ASSERT_EQ(*intersect.get(), res);
}

TEST(SetTests, MultiIntervalIntersection2) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    i1.addInterval(RegularExpression::Interval(40, 50));
    RegularExpression::Interval i2(15, 35);

    RegularExpression::Interval res(20, 30);

    auto intersect = i1.intersectWith(i2);
    ASSERT_EQ(*intersect.get(), res);
}

TEST(SetTests, MultiIntervalIntersection3) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::Interval i2(15, 25);

    RegularExpression::Interval res(20, 25);

    auto intersect = i1.intersectWith(i2);
    ASSERT_EQ(*intersect.get(), res);
}

TEST(SetTests, MultiIntervalIntersection4) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::MultiInterval i2;
    i2.addInterval(RegularExpression::Interval(1, 4));
    i2.addInterval(RegularExpression::Interval(25, 35));

    RegularExpression::Interval res(25, 30);

    auto intersect = i1.intersectWith(std::make_shared<RegularExpression::MultiInterval>(i2));
    ASSERT_EQ(*intersect.get(), res);
}

TEST(SetTests, MultiIntervalIntersection5) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::MultiInterval i2;
    i2.addInterval(RegularExpression::Interval(5, 15));
    i2.addInterval(RegularExpression::Interval(25, 35));

    RegularExpression::MultiInterval res;
    res.addInterval(RegularExpression::Interval(5, 10));
    res.addInterval(RegularExpression::Interval(25, 30));

    auto intersect = i1.intersectWith(std::make_shared<RegularExpression::MultiInterval>(i2));
    ASSERT_EQ(*intersect.get(), res);
}

TEST(SetTests, MultiIntervalIntersection6) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::MultiAntiInterval i2;
    i2.addInterval(RegularExpression::Interval(16, 18));
    i2.addInterval(RegularExpression::Interval(40, 50));

    auto intersect = i1.intersectWith(std::make_shared<RegularExpression::MultiAntiInterval>(i2));
    ASSERT_EQ(*intersect.get(), i1);
}

TEST(SetTests, MultiIntervalIntersection7) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::MultiAntiInterval i2;
    i2.addInterval(RegularExpression::Interval(8, 18));
    i2.addInterval(RegularExpression::Interval(25, 50));

    RegularExpression::MultiInterval res;
    res.addInterval(RegularExpression::Interval(5, 7));
    res.addInterval(RegularExpression::Interval(20, 24));

    auto intersect = i1.intersectWith(std::make_shared<RegularExpression::MultiAntiInterval>(i2));
    ASSERT_EQ(*intersect.get(), res);
}

// Union on MultiInterval

TEST(SetTests, MultiIntervalUnion1) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::Interval i2(40, 50);

    RegularExpression::MultiInterval res;
    res.addInterval(RegularExpression::Interval(5, 10));
    res.addInterval(RegularExpression::Interval(20, 30));
    res.addInterval(RegularExpression::Interval(40, 50));

    auto uni = i1.unionWith(i2);
    ASSERT_EQ(*uni.get(), res);
}

TEST(SetTests, MultiIntervalUnion2) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::Interval i2(15, 25);

    RegularExpression::MultiInterval res;
    res.addInterval(RegularExpression::Interval(5, 10));
    res.addInterval(RegularExpression::Interval(15, 30));

    auto uni = i1.unionWith(i2);
    ASSERT_EQ(*uni.get(), res);
}

TEST(SetTests, MultiIntervalUnion3) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::Interval i2(10, 25);

    RegularExpression::Interval res(5, 30);

    auto uni = i1.unionWith(i2);
    ASSERT_EQ(*uni.get(), res);
}

TEST(SetTests, MultiIntervalUnion4) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::MultiInterval i2;
    i2.addInterval(RegularExpression::Interval(11, 15));
    i2.addInterval(RegularExpression::Interval(25, 40));

    RegularExpression::MultiInterval res;
    res.addInterval(RegularExpression::Interval(5, 15));
    res.addInterval(RegularExpression::Interval(20, 40));

    auto uni = i1.unionWith(std::make_shared<RegularExpression::MultiInterval>(i2));
    ASSERT_EQ(*uni.get(), res);
}

TEST(SetTests, MultiIntervalUnion5) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::MultiInterval i2;
    i2.addInterval(RegularExpression::Interval(11, 20));
    i2.addInterval(RegularExpression::Interval(25, 40));

    RegularExpression::Interval res(5, 40);

    auto uni = i1.unionWith(std::make_shared<RegularExpression::MultiInterval>(i2));
    ASSERT_EQ(*uni.get(), res);
}

TEST(SetTests, MultiIntervalUnion6) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::MultiAntiInterval i2;
    i2.addInterval(RegularExpression::Interval(10, 25));

    RegularExpression::MultiAntiInterval res;
    res.addInterval(RegularExpression::Interval(11, 19));

    auto uni = i1.unionWith(std::make_shared<RegularExpression::MultiAntiInterval>(i2));
    ASSERT_EQ(*uni.get(), res);
}

// Substract

TEST(SetTests, MultiIntervalSubstract1) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::Interval i2(15, 45);

    RegularExpression::Interval res(5, 10);

    auto uni = i1.substract(std::make_shared<RegularExpression::Interval>(i2));
    ASSERT_EQ(*uni.get(), res);
}

TEST(SetTests, MultiIntervalSubstract2) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::Interval i2(15, 25);

    RegularExpression::MultiInterval res;
    res.addInterval(RegularExpression::Interval(5, 10));
    res.addInterval(RegularExpression::Interval(26, 30));

    auto uni = i1.substract(std::make_shared<RegularExpression::Interval>(i2));
    ASSERT_EQ(*uni.get(), res);
}

TEST(SetTests, MultiIntervalSubstract3) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::MultiInterval i2;
    i2.addInterval(RegularExpression::Interval(10, 20));
    i2.addInterval(RegularExpression::Interval(30, 40));

    RegularExpression::MultiInterval res;
    res.addInterval(RegularExpression::Interval(5, 9));
    res.addInterval(RegularExpression::Interval(21, 29));

    auto uni = i1.substract(std::make_shared<RegularExpression::MultiInterval>(i2));
    ASSERT_EQ(*uni.get(), res);
}

TEST(SetTests, MultiIntervalSubstract4) {
    RegularExpression::MultiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 10));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::MultiAntiInterval i2;
    i2.addInterval(RegularExpression::Interval(8, 25));

    RegularExpression::MultiInterval res;
    res.addInterval(RegularExpression::Interval(8, 10));
    res.addInterval(RegularExpression::Interval(20, 25));

    auto uni = i1.substract(std::make_shared<RegularExpression::MultiAntiInterval>(i2));
    ASSERT_EQ(*uni.get(), res);
}
