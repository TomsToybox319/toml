#include <algorithm>
#include <cctype>
#include <expected>
#include <map>
#include <string>
#include <variant>

#include "unittest.h"

namespace {
// Whitespace means tab (0x09) or space (0x20).
// Newline means LF (0x0A) or CRLF (0x0D 0x0A).
// I guess this handling is technically invalid? But good enough
constexpr auto WHITESPACE_CHARS = " \r\n\t";

constexpr auto Trim(const std::string& Str) noexcept -> std::string {
    const auto StartIndex = Str.find_first_not_of(WHITESPACE_CHARS);
    const auto EndIndex = Str.find_last_not_of(WHITESPACE_CHARS);
    if (StartIndex == std::string::npos || EndIndex == std::string::npos)
        return "";
    return Str.substr(StartIndex, EndIndex - StartIndex + 1);
}

constexpr auto ParseString(const std::string& Str) noexcept
    -> std::expected<std::string, std::string> {
    if (Str.empty()) {
        return std::unexpected("Invalid value");
    }
    if (Str[0] != '"') return std::unexpected("Invalid value");
    if (Str[Str.size() - 1] != '"')
        return std::unexpected("Unterminated string");
    return Str.substr(1, Str.size() - 2);
}

constexpr auto IsValidKey(const std::string& Key) noexcept -> bool {
    return std::all_of(Key.begin(), Key.end(),
                       [](char c) { return std::isalnum(c) || c == '_'; });
}

constexpr auto ParseKey(const std::string& Str) noexcept
    -> std::expected<std::pair<std::string, std::variant<std::string>>,
                     std::string> {
    const auto EqIndex = Str.find('=');
    if (EqIndex == std::string::npos)
        return std::unexpected("Expected '=' after a key in key-value pair");
    const auto Key = Trim(Str.substr(0, EqIndex));
    if (!IsValidKey(Key)) return std::unexpected(Key + " is not a valid key");
    const auto Value = ParseString(Trim(Str.substr(EqIndex + 1)));
    if (Value.has_value())
        return std::pair<std::string, std::variant<std::string>>{Key, *Value};
    return std::unexpected(Value.error());
}

auto Parse(const std::string& Str) noexcept
    -> std::expected<std::map<std::string, std::variant<std::string>>,
                     std::string> {
    const size_t StartIndex = Str.find_first_not_of(WHITESPACE_CHARS);
    if (StartIndex == std::string::npos) return {};
    const auto Pair = ParseKey(Str.substr(StartIndex));
    if (Pair.has_value())
        return std::map<std::string, std::variant<std::string>>{*Pair};
    return std::unexpected(Pair.error());
};
}  // namespace

TEST(Reporting_error_messages) {
    ASSERT_EQ(Parse("key").error(),
              "Expected '=' after a key in key-value pair");
    ASSERT_EQ(Parse("key = ").error(), "Invalid value");
    ASSERT_EQ(Parse("key =").error(), "Invalid value");
    ASSERT_EQ(Parse("key =val").error(), "Invalid value");
    ASSERT_EQ(Parse("key = \"val").error(), "Unterminated string");
    ASSERT_EQ(Parse("k@ey = \"val\"\n").error(), "k@ey is not a valid key");
}

TEST(Empty_string_parses_to_empty_container) {
    ASSERT(Parse("")->empty());
    ASSERT(Parse(" ")->empty());
    ASSERT(Parse("\n")->empty());
    ASSERT(Parse("\t")->empty());
}

TEST(Can_parse_single_kv_pair) {
    {
        constexpr auto TomlSnippet = R"(title = "TOML example")";
        const auto Result = Parse(TomlSnippet);
        ASSERT(Result.has_value());
        ASSERT_EQ(Result->count("title"), size_t(1));
        const auto ResultStr = std::get<std::string>(Result->at("title"));
        ASSERT_EQ(ResultStr, "TOML example");
    }
    {
        constexpr auto TomlSnippet = R"(subject = "another string")";
        const auto Result = Parse(TomlSnippet);
        ASSERT(Result.has_value());
        EXPECT_EQ(Result->count("title"), size_t(0));
        ASSERT_EQ(Result->count("subject"), size_t(1));
        const auto ResultStr = std::get<std::string>(Result->at("subject"));
        EXPECT_EQ(ResultStr, "another string");
    }
}

UNIT_TEST_INIT
