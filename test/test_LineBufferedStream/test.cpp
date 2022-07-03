#include "serial/impl.cpp"
#include <unity.h>


void test_buffer_char_no_line()
{
    std::string buffer;
    TEST_ASSERT_FALSE(bufferChar(buffer, 'a'));
    TEST_ASSERT_FALSE(bufferChar(buffer, 's'));
    TEST_ASSERT_FALSE(bufferChar(buffer, 'd'));
    TEST_ASSERT_FALSE(bufferChar(buffer, 'f'));

    TEST_ASSERT_TRUE(buffer.compare("asdf") == 0);
}


void test_buffer_char_filter_carriage_return()
{
    std::string buffer;
    TEST_ASSERT_FALSE(bufferChar(buffer, 'a'));
    TEST_ASSERT_FALSE(bufferChar(buffer, 's'));
    TEST_ASSERT_FALSE(bufferChar(buffer, '\r'));
    TEST_ASSERT_FALSE(bufferChar(buffer, 'd'));
    TEST_ASSERT_FALSE(bufferChar(buffer, '\r'));
    TEST_ASSERT_FALSE(bufferChar(buffer, 'f'));

    TEST_ASSERT_TRUE(buffer.compare("asdf") == 0);
}


void test_buffer_char_one_line()
{
    std::string buffer;
    TEST_ASSERT_FALSE(bufferChar(buffer, 'a'));
    TEST_ASSERT_FALSE(bufferChar(buffer, 's'));
    TEST_ASSERT_FALSE(bufferChar(buffer, 'd'));
    TEST_ASSERT_FALSE(bufferChar(buffer, 'f'));
    TEST_ASSERT_TRUE(bufferChar(buffer, '\n'));

    TEST_ASSERT_TRUE(buffer.compare("asdf\n") == 0);
}


void test_buffer_char_one_line_with_cr()
{
    std::string buffer;
    TEST_ASSERT_FALSE(bufferChar(buffer, 'a'));
    TEST_ASSERT_FALSE(bufferChar(buffer, 's'));
    TEST_ASSERT_FALSE(bufferChar(buffer, 'd'));
    TEST_ASSERT_FALSE(bufferChar(buffer, 'f'));
    TEST_ASSERT_FALSE(bufferChar(buffer, '\r'));
    TEST_ASSERT_TRUE(bufferChar(buffer, '\n'));

    TEST_ASSERT_TRUE(buffer.compare("asdf\n") == 0);
}


void test_take_line_no_data()
{
    std::string buffer{ "" };

    std::string line{ getAndRemoveLine(buffer, true) };
    TEST_ASSERT_TRUE(line.empty());

    TEST_ASSERT_TRUE(buffer.compare("") == 0);
}


void test_take_line_no_line()
{
    std::string buffer{ "asdf" };

    std::string line{ getAndRemoveLine(buffer, true) };
    TEST_ASSERT_TRUE(line.empty());

    TEST_ASSERT_TRUE(buffer.compare("asdf") == 0);
}


void test_take_line_one_line_with_no_pending()
{
    std::string buffer{ "asdf\n" };

    std::string line{ getAndRemoveLine(buffer, true) };
    TEST_ASSERT_TRUE(!line.empty());

    TEST_ASSERT_TRUE(buffer.compare("") == 0);
}


void test_take_line_one_line_with_pending_line()
{
    std::string buffer{ "asdf\nghjk" };

    std::string line1{ getAndRemoveLine(buffer, true) };
    TEST_ASSERT_TRUE(!line1.empty());

    TEST_ASSERT_TRUE(buffer.compare("ghjk") == 0);

    std::string line2{ getAndRemoveLine(buffer, true) };
    TEST_ASSERT_TRUE(line2.empty());

    TEST_ASSERT_TRUE(buffer.compare("ghjk") == 0);
}

void test_take_line_two_lines_with_no_pending_line()
{
    std::string buffer{ "asdf\nghjk\n" };

    std::string line1{ getAndRemoveLine(buffer, true) };
    TEST_ASSERT_TRUE(!line1.empty());

    TEST_ASSERT_TRUE(buffer.compare("ghjk\n") == 0);

    std::string line2{ getAndRemoveLine(buffer, true) };
    TEST_ASSERT_TRUE(!line2.empty());

    TEST_ASSERT_TRUE(buffer.compare("") == 0);
}

int tests()
{
    UNITY_BEGIN();
    RUN_TEST(test_buffer_char_no_line);
    RUN_TEST(test_buffer_char_filter_carriage_return);
    RUN_TEST(test_buffer_char_one_line);
    RUN_TEST(test_buffer_char_one_line_with_cr);

    RUN_TEST(test_take_line_no_data);
    RUN_TEST(test_take_line_no_line);
    RUN_TEST(test_take_line_one_line_with_no_pending);
    RUN_TEST(test_take_line_one_line_with_pending_line);
    RUN_TEST(test_take_line_two_lines_with_no_pending_line);
    return UNITY_END();
}

void setUp() {}

void tearDown() {}

#include "../helpers/run-tests.h"
