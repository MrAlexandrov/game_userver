#include <utils/string_to_uuid.hpp>
#include <userver/utest/utest.hpp>

// Тесты для true значений
UTEST(FromStringBoolTest, TrueStringLowercase) {
    EXPECT_TRUE(Utils::StringToBool("true"));
}

UTEST(FromStringBoolTest, TrueStringUppercase) {
    EXPECT_TRUE(Utils::StringToBool("TRUE"));
}

UTEST(FromStringBoolTest, TrueStringMixedCase) {
    EXPECT_TRUE(Utils::StringToBool("True"));
    EXPECT_TRUE(Utils::StringToBool("TrUe"));
}

UTEST(FromStringBoolTest, TrueNumeric) {
    EXPECT_TRUE(Utils::StringToBool("1"));
}

UTEST(FromStringBoolTest, YesString) {
    EXPECT_TRUE(Utils::StringToBool("yes"));
    EXPECT_TRUE(Utils::StringToBool("YES"));
    EXPECT_TRUE(Utils::StringToBool("Yes"));
}

UTEST(FromStringBoolTest, OnString) {
    EXPECT_TRUE(Utils::StringToBool("on"));
    EXPECT_TRUE(Utils::StringToBool("ON"));
    EXPECT_TRUE(Utils::StringToBool("On"));
}

// Тесты для false значений
UTEST(FromStringBoolTest, FalseStringLowercase) {
    EXPECT_FALSE(Utils::StringToBool("false"));
}

UTEST(FromStringBoolTest, FalseStringUppercase) {
    EXPECT_FALSE(Utils::StringToBool("FALSE"));
}

UTEST(FromStringBoolTest, FalseStringMixedCase) {
    EXPECT_FALSE(Utils::StringToBool("False"));
    EXPECT_FALSE(Utils::StringToBool("FaLsE"));
}

UTEST(FromStringBoolTest, FalseNumeric) {
    EXPECT_FALSE(Utils::StringToBool("0"));
}

UTEST(FromStringBoolTest, NoString) {
    EXPECT_FALSE(Utils::StringToBool("no"));
    EXPECT_FALSE(Utils::StringToBool("NO"));
    EXPECT_FALSE(Utils::StringToBool("No"));
}

UTEST(FromStringBoolTest, OffString) {
    EXPECT_FALSE(Utils::StringToBool("off"));
    EXPECT_FALSE(Utils::StringToBool("OFF"));
    EXPECT_FALSE(Utils::StringToBool("Off"));
}

// Тесты для ошибок
UTEST(FromStringBoolTest, EmptyStringThrowsException) {
    EXPECT_FALSE(Utils::StringToBool(""));
}

UTEST(FromStringBoolTest, InvalidStringThrowsException) {
    EXPECT_THROW({
        Utils::StringToBool("invalid");
    }, std::invalid_argument);

    EXPECT_THROW({
        Utils::StringToBool("maybe");
    }, std::invalid_argument);

    EXPECT_THROW({
        Utils::StringToBool("2");
    }, std::invalid_argument);
}

UTEST(FromStringBoolTest, WhitespaceStringsThrowException) {
    EXPECT_THROW({
        Utils::StringToBool(" ");
    }, std::invalid_argument);

    EXPECT_THROW({
        Utils::StringToBool(" true ");
    }, std::invalid_argument);
}

// Граничные случаи
UTEST(FromStringBoolTest, SingleCharacters) {
    EXPECT_TRUE(Utils::StringToBool("1"));
    EXPECT_FALSE(Utils::StringToBool("0"));

    EXPECT_THROW({
        Utils::StringToBool("2");
    }, std::invalid_argument);
}
