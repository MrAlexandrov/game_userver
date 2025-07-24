#include <utils/string_to_uuid.hpp>
#include <userver/utest/utest.hpp>

// Тесты для true значений
UTEST(FromStringBoolTest, TrueStringLowercase) {
    EXPECT_TRUE(NUtils::StringToBool("true"));
}

UTEST(FromStringBoolTest, TrueStringUppercase) {
    EXPECT_TRUE(NUtils::StringToBool("TRUE"));
}

UTEST(FromStringBoolTest, TrueStringMixedCase) {
    EXPECT_TRUE(NUtils::StringToBool("True"));
    EXPECT_TRUE(NUtils::StringToBool("TrUe"));
}

UTEST(FromStringBoolTest, TrueNumeric) {
    EXPECT_TRUE(NUtils::StringToBool("1"));
}

UTEST(FromStringBoolTest, YesString) {
    EXPECT_TRUE(NUtils::StringToBool("yes"));
    EXPECT_TRUE(NUtils::StringToBool("YES"));
    EXPECT_TRUE(NUtils::StringToBool("Yes"));
}

UTEST(FromStringBoolTest, OnString) {
    EXPECT_TRUE(NUtils::StringToBool("on"));
    EXPECT_TRUE(NUtils::StringToBool("ON"));
    EXPECT_TRUE(NUtils::StringToBool("On"));
}

// Тесты для false значений
UTEST(FromStringBoolTest, FalseStringLowercase) {
    EXPECT_FALSE(NUtils::StringToBool("false"));
}

UTEST(FromStringBoolTest, FalseStringUppercase) {
    EXPECT_FALSE(NUtils::StringToBool("FALSE"));
}

UTEST(FromStringBoolTest, FalseStringMixedCase) {
    EXPECT_FALSE(NUtils::StringToBool("False"));
    EXPECT_FALSE(NUtils::StringToBool("FaLsE"));
}

UTEST(FromStringBoolTest, FalseNumeric) {
    EXPECT_FALSE(NUtils::StringToBool("0"));
}

UTEST(FromStringBoolTest, NoString) {
    EXPECT_FALSE(NUtils::StringToBool("no"));
    EXPECT_FALSE(NUtils::StringToBool("NO"));
    EXPECT_FALSE(NUtils::StringToBool("No"));
}

UTEST(FromStringBoolTest, OffString) {
    EXPECT_FALSE(NUtils::StringToBool("off"));
    EXPECT_FALSE(NUtils::StringToBool("OFF"));
    EXPECT_FALSE(NUtils::StringToBool("Off"));
}

// Тесты для ошибок
UTEST(FromStringBoolTest, EmptyStringThrowsException) {
    EXPECT_FALSE(NUtils::StringToBool(""));
}

UTEST(FromStringBoolTest, InvalidStringThrowsException) {
    EXPECT_THROW({
        NUtils::StringToBool("invalid");
    }, std::invalid_argument);
    
    EXPECT_THROW({
        NUtils::StringToBool("maybe");
    }, std::invalid_argument);
    
    EXPECT_THROW({
        NUtils::StringToBool("2");
    }, std::invalid_argument);
}

UTEST(FromStringBoolTest, WhitespaceStringsThrowException) {
    EXPECT_THROW({
        NUtils::StringToBool(" ");
    }, std::invalid_argument);
    
    EXPECT_THROW({
        NUtils::StringToBool(" true ");
    }, std::invalid_argument);
}

// Граничные случаи
UTEST(FromStringBoolTest, SingleCharacters) {
    EXPECT_TRUE(NUtils::StringToBool("1"));
    EXPECT_FALSE(NUtils::StringToBool("0"));
    
    EXPECT_THROW({
        NUtils::StringToBool("2");
    }, std::invalid_argument);
}
