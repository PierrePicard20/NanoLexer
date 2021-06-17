#include "../NanoLexer/LexerTreeNode.h"

#include <gtest\gtest.h>


// Intersection on MultiAntiInterval

TEST(SetTests, MultiAntiIntervalIntersection1) {
    RegularExpression::MultiAntiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 15));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::Interval i2(10, 25);

    RegularExpression::Interval res(16, 19);

    auto intersect = i1.intersectWith(i2);
    ASSERT_EQ(*intersect.get(), res);
}

TEST(SetTests, MultiAntiIntervalIntersection2) {
    RegularExpression::MultiAntiInterval i1;
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::Interval i2(20, 30);

    auto intersect = i1.intersectWith(i2);
    ASSERT_EQ(intersect, RegularExpression::EmptySet::get());
}

// Union

TEST(SetTests, MultiAntiIntervalUnion1) {
    RegularExpression::MultiAntiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 15));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::Interval i2(10, 25);

    RegularExpression::MultiAntiInterval res;
    res.addInterval(RegularExpression::Interval(5, 9));
    res.addInterval(RegularExpression::Interval(26, 30));

    auto un = i1.unionWith(i2);
    ASSERT_EQ(*un.get(), res);
}

// Substract

TEST(SetTests, MultiAntiIntervalSubstract1) {
    RegularExpression::MultiAntiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 15));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::Interval i2(10, 25);

    RegularExpression::MultiAntiInterval res;
    res.addInterval(RegularExpression::Interval(5, 30));

    auto sub = i1.substract(std::make_shared<RegularExpression::Interval>(i2));
    ASSERT_EQ(*sub.get(), res);
}

TEST(SetTests, MultiAntiIntervalSubstract2) {
    RegularExpression::MultiAntiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 15));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::Interval i2(10, 18);

    RegularExpression::MultiAntiInterval res;
    res.addInterval(RegularExpression::Interval(5, 18));
    res.addInterval(RegularExpression::Interval(20, 30));

    auto sub = i1.substract(std::make_shared<RegularExpression::Interval>(i2));
    ASSERT_EQ(*sub.get(), res);
}

TEST(SetTests, MultiAntiIntervalSubstract3) {
    RegularExpression::MultiAntiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 15));
    i1.addInterval(RegularExpression::Interval(20, 30));
    RegularExpression::MultiInterval i2;
    i2.addInterval(RegularExpression::Interval(10, 25));
    i2.addInterval(RegularExpression::Interval(40, 50));

    RegularExpression::MultiAntiInterval res;
    res.addInterval(RegularExpression::Interval(5, 30));
    res.addInterval(RegularExpression::Interval(40, 50));

    auto sub = i1.substract(std::make_shared<RegularExpression::MultiInterval>(i2));
    ASSERT_EQ(*sub.get(), res);
}

TEST(SetTests, MultiAntiIntervalSubstract4) {
    RegularExpression::MultiAntiInterval i1;
    i1.addInterval(RegularExpression::Interval(5, 15));
    i1.addInterval(RegularExpression::Interval(30, 40));
    RegularExpression::MultiAntiInterval i2;
    i2.addInterval(RegularExpression::Interval(10, 20));
    i2.addInterval(RegularExpression::Interval(35, 50));

    RegularExpression::MultiInterval res;
    res.addInterval(RegularExpression::Interval(16, 20));
    res.addInterval(RegularExpression::Interval(41, 50));

    auto sub = i1.substract(std::make_shared<RegularExpression::MultiAntiInterval>(i2));
    ASSERT_EQ(*sub.get(), res);
}
