#include "utils/string_to_uuid.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <userver/utest/utest.hpp>

UTEST(UuidFromStringTest, ValidUuidString) {
    const std::string valid_uuid_str = "123e4567-e89b-42d3-a456-556642440000";
    boost::uuids::uuid result = Utils::StringToUuid(valid_uuid_str);

    EXPECT_NE(result, boost::uuids::uuid{}); // Не пустой UUID
    EXPECT_EQ(boost::uuids::to_string(result), valid_uuid_str);
}

UTEST(UuidFromStringTest, UppercaseUuidString) {
    const std::string uppercase_uuid_str = "123E4567-E89B-42D3-A456-556642440000";
    boost::uuids::uuid result = Utils::StringToUuid(uppercase_uuid_str);

    EXPECT_NE(result, boost::uuids::uuid{});
    EXPECT_EQ(boost::uuids::to_string(result), "123e4567-e89b-42d3-a456-556642440000");
}

UTEST(UuidFromStringTest, InvalidUuidStringReturnsEmpty) {
    const std::string invalid_uuid_str = "invalid-uuid-format";
    boost::uuids::uuid result = Utils::StringToUuid(invalid_uuid_str);

    EXPECT_EQ(result, boost::uuids::uuid{});
}

UTEST(UuidFromStringTest, EmptyStringReturnsEmpty) {
    const std::string empty_str = "";
    boost::uuids::uuid result = Utils::StringToUuid(empty_str);

    EXPECT_EQ(result, boost::uuids::uuid{});
}

UTEST(UuidFromStringTest, PartialUuidStringReturnsEmpty) {
    const std::string partial_uuid_str = "123e4567-e89b";
    boost::uuids::uuid result = Utils::StringToUuid(partial_uuid_str);

    EXPECT_EQ(result, boost::uuids::uuid{});
}

UTEST(UuidFromStringTest, RandomStringReturnsEmpty) {
    const std::string random_str = "just-some-random-string";
    boost::uuids::uuid result = Utils::StringToUuid(random_str);

    EXPECT_EQ(result, boost::uuids::uuid{});
}

UTEST(UuidFromStringTest, AnotherValidUuid) {
    const std::string valid_uuid_str = "550e8400-e29b-41d4-a716-446655440000";
    boost::uuids::uuid result = Utils::StringToUuid(valid_uuid_str);

    EXPECT_NE(result, boost::uuids::uuid{});
    EXPECT_EQ(boost::uuids::to_string(result), valid_uuid_str);
}
