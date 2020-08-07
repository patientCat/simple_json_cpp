#include <boost/test/minimal.hpp>
#include "JsonParse.hpp"

using namespace boost;

static void test_parse_null() {
    JsonParse jp;
    BOOST_CHECK( jp.parse( "null", 4 ) == JParseError::LEPT_PARSE_OK );
    BOOST_CHECK( jp.get_type() == JsonType::JSON_NULL );
    BOOST_CHECK( jp.parse( "null ", 5 ) == JParseError::LEPT_PARSE_OK );
    BOOST_CHECK( jp.get_type() == JsonType::JSON_NULL );
}

static void test_parse_bool() {
    JsonParse jp;
    BOOST_CHECK( jp.parse( "true", 4 ) == JParseError::LEPT_PARSE_OK );
    BOOST_CHECK( jp.get_type() == JsonType::JSON_TRUE );
    BOOST_CHECK( jp.parse( "false", 5 ) == JParseError::LEPT_PARSE_OK );
    BOOST_CHECK( jp.get_type() == JsonType::JSON_FALSE );
}

static void test_root_not_singular() {
    JsonParse jp;
    BOOST_CHECK( jp.parse( "", 0 ) == JParseError::LEPT_PARSE_EXPECT_VALUE ); // should error
    BOOST_CHECK( jp.parse( "   ", 3 ) == JParseError::LEPT_PARSE_EXPECT_VALUE ); // should error
}
static void test_invalid_value() {
    JsonParse jp;
    BOOST_CHECK( jp.parse( "  a", 3 ) == JParseError::LEPT_PARSE_INVALID_VALUE ); // should error
    BOOST_CHECK( jp.parse( "a a", 3 ) == JParseError::LEPT_PARSE_INVALID_VALUE ); // should error
    BOOST_CHECK( jp.parse( "a ", 2 ) == JParseError::LEPT_PARSE_INVALID_VALUE ); // should error
}


#define TEST_NUMBER(x, str ) \
    do{\
        JsonParse jp; \
        BOOST_CHECK(jp.parse(str, strlen(str)) == JParseError::LEPT_PARSE_OK); \
        BOOST_CHECK(jp.get_num() == ( x ) &&jp.get_type() == JsonType::JSON_NUMBER); \
    } while( 0 )
static void test_parse_number() {
    TEST_NUMBER( 0.0, "0" );
    TEST_NUMBER( 0.0, "-0" );
    TEST_NUMBER( 0.0, "-0.0" );
    TEST_NUMBER( 1.0, "1" );
    TEST_NUMBER( -1.0, "-1" );
    TEST_NUMBER( 1.5, "1.5" );
    TEST_NUMBER( -1.5, "-1.5" );
    TEST_NUMBER( 3.1416, "3.1416" );
    TEST_NUMBER( 1E10, "1E10" );
    TEST_NUMBER( 1e10, "1e10" );
    TEST_NUMBER( 1E+10, "1E+10" );
    TEST_NUMBER( 1E-10, "1E-10" );
    TEST_NUMBER( -1E10, "-1E10" );
    TEST_NUMBER( -1e10, "-1e10" );
    TEST_NUMBER( -1E+10, "-1E+10" );
    TEST_NUMBER( -1E-10, "-1E-10" );
    TEST_NUMBER( 1.234E+10, "1.234E+10" );
    TEST_NUMBER( 1.234E-10, "1.234E-10" );
    TEST_NUMBER( 0.0, "1e-10000" ); /* must underflow */
}
#define TEST_ERROR(err, str)\
    do { \
        JsonParse jp; \
        BOOST_CHECK( jp.parse( str, strlen( str ) ) == err ); \
    } while( 0 )

static void test_parse_invalid_number() {
    /* ... */
    /* invalid number */
    TEST_ERROR( LEPT_PARSE_INVALID_VALUE, "+0" );
    TEST_ERROR( LEPT_PARSE_INVALID_VALUE, "0.a3" );
    TEST_ERROR( LEPT_PARSE_INVALID_VALUE, "+1" );
    TEST_ERROR( LEPT_PARSE_INVALID_VALUE, ".123" ); /* at least one digit before '.' */
    TEST_ERROR( LEPT_PARSE_INVALID_VALUE, "1." ); /* at least one digit after '.' */
    TEST_ERROR( LEPT_PARSE_INVALID_VALUE, "INF" );
    TEST_ERROR( LEPT_PARSE_INVALID_VALUE, "inf" );
    TEST_ERROR( LEPT_PARSE_INVALID_VALUE, "NAN" );
    TEST_ERROR( LEPT_PARSE_INVALID_VALUE, "nan" );
}

static void test_all() {
    test_parse_null();
    test_parse_bool();
    test_parse_number();
    test_root_not_singular();
    test_invalid_value();
    test_parse_number();
    test_parse_invalid_number();
}

int test_main( int argc, char *argv[] ) {

    test_all();
    system( "pause" );
    return 0;
}