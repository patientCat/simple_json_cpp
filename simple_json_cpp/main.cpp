#include "boost/test/minimal.hpp"
#include "JsonParse.hh"


#define TEST_PARSE_ERROR(err, str)\
    do { \
        JsonParse jp; \
        BOOST_CHECK( jp.parse( str, strlen( str ) ).second == err ); \
    } while( 0 )


#define TEST_NUMBER(x, str ) \
    do{\
        JsonParse jp; \
        auto [json_value, json_err] = jp.parse(str); \
        BOOST_CHECK(json_err == JParseError::JSON_PARSE_OK); \
        BOOST_CHECK(json_value.get_number() == ( x ) &&json_value.get_type() == EJsonType::JSON_NUMBER); \
    } while( 0 )

#define TEST_STRING(res_str, str  ) \
    do{\
        JsonParse jp; \
        auto [json_value, json_err] = jp.parse(str); \
        BOOST_CHECK(json_err == JParseError::JSON_PARSE_OK); \
        BOOST_CHECK(json_value.get_string() == ( res_str ) &&json_value.get_type() == EJsonType::JSON_STRING); \
    } while( 0 )

using namespace boost;
using namespace std;

#define TEST_CHECK(err, type, str) \
do{\
  JsonParse jp;\
  auto [json_value, json_err] = jp.parse(str);\
  BOOST_CHECK(json_err == err);\
  BOOST_CHECK(json_value.get_type() == type);\
}while(0)

static void test_parse_null() {
  TEST_CHECK(JSON_PARSE_OK, EJsonType::JSON_NULL, "null");
  TEST_CHECK(JSON_PARSE_OK, EJsonType::JSON_NULL, "null ");
}

static void test_parse_bool() {
  TEST_CHECK(JSON_PARSE_OK, EJsonType::JSON_TRUE, "true");
  TEST_CHECK(JSON_PARSE_OK, EJsonType::JSON_FALSE, "false");
}

static void test_root_not_singular() {
  JsonParse jp;
  BOOST_CHECK( jp.parse( "", 0 ).second == JParseError::JSON_PARSE_EXPECT_VALUE); // should error
  BOOST_CHECK( jp.parse( "   ", 3 ).second == JParseError::JSON_PARSE_EXPECT_VALUE); // should error
}
static void test_invalid_value() {
  JsonParse jp;
  BOOST_CHECK( jp.parse( "  a", 3 ).second == JParseError::JSON_PARSE_INVALID_VALUE); // should error
  BOOST_CHECK( jp.parse( "a a", 3 ).second == JParseError::JSON_PARSE_INVALID_VALUE); // should error
  BOOST_CHECK( jp.parse( "a ", 2 ).second == JParseError::JSON_PARSE_INVALID_VALUE); // should error
}



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


static void test_parse_invalid_number() {
  /* ... */
  /* invalid number */
  TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "+0" );
  TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "0.a3" );
  TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "+1" );
  TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, ".123" ); /* at least one digit before '.' */
  TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "1." ); /* at least one digit after '.' */
  TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "INF" );
  TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "inf" );
  TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "NAN" );
  TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "nan" );
}
static void test_parse_string(){
  TEST_STRING("", "\"\"");
  TEST_STRING("Hello", "\"Hello\"");
  TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
  TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}
static void test_parse_object(){
  JsonParse jp;
  // (jp.parse("{}", 2) == JParseError::JSON_PARSE_OK);
  string str1 = " { "
                "\"n\" : null , "
                "\"f\" : false , "
                "\"t\" : true , "
                "\"i\" : 123 ,"
                "\"num1\" : -321,"
                "\"num2\" : -123"
                " } ";
  auto [json_value, json_err] = jp.parse(str1);

  BOOST_CHECK(json_err == JParseError::JSON_PARSE_OK);
//  BOOST_CHECK(json_value->get_object_element_by<void *>("n") == nullptr);
//  BOOST_CHECK(json_value->get_object_element_by<bool>("f") == false);
//  BOOST_CHECK(json_value->get_object_element_by<bool>("t") == true);
//  BOOST_CHECK(json_value->get_object_element_by<double>("i") == 123);
//  BOOST_CHECK(json_value->get_object_element_by<double>("num1") == -321);
//  BOOST_CHECK(json_value->get_object_element_by<double>("num2") == -123);

  // 解决嵌套对象
  string str2 =
      " { "
      "\"n\" : null , "
      "\"f\" : false , "
      "\"t\" : true , "
      "\"i\" : 123 , "
      "\"s\" : \"abc\", "
      "\"a\" : [ 1, 2, 3 ],"
      "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
      " } ";
  jp.parse(str2);


}
static void test_parse_object_miss_key()
{
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_KEY, "{:1,");
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_KEY, "{1:1,");
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_KEY, "{true:1,");
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_KEY, "{false:1,");
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_KEY, "{null:1,");
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_KEY, "{{}:1,");
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_KEY, "{\"a\":1,");
}

static void test_parse_object_miss_member()
{
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_MEMBER, "{\"a\":");
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_MEMBER, "{\"a\":,");
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_MEMBER, "{\"a\": abc,");
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_MEMBER, "{\"a\": true, \"b\":");
}
static void test_parse_object_miss_colon()
{
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_COLON, "{\"a\"");
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_COLON, "{\"a\", abc,");
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_COLON, "{\"a\"a abc,");
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_COLON, "{\"a\"} abc,");
}
static void test_parse_object_miss_comma()
{
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_COMMA, "{\"a\":true \"");
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_COMMA, "{\"a\":false \"");
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_COMMA, "{\"a\":null \"");
}
static void test_parse_object_miss_bracket()
{
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_RIGHT_BRACKET, "{\"a\":true");
  TEST_PARSE_ERROR(JSON_PARSE_OBJECT_MISS_RIGHT_BRACKET, "{\"a\":true, \"b\":123");
}

static void test_parse_array()
{
  JsonParse jp;
  TEST_CHECK(JSON_PARSE_OK, EJsonType::JSON_ARRAY, "[]");

  auto &&[json_value, json_err] = jp.parse("[null, 12, true, 12.3, -5.6, -1.5e-3, false, \"null\"]");
  BOOST_CHECK(json_err == JSON_PARSE_OK);
  BOOST_CHECK(json_value.get_type() == EJsonType::JSON_ARRAY);
  BOOST_CHECK(json_value.get_array_element_by(0).get_null() == nullptr);
  BOOST_CHECK(json_value.get_array_element_by(1).get_number() == 12);
  BOOST_CHECK(json_value.get_array_element_by(2).get_boolean() == true);
  BOOST_CHECK(json_value.get_array_element_by(3).get_number() == 12.3);
  BOOST_CHECK(json_value.get_array_element_by(4).get_number() == -5.6);
  BOOST_CHECK(json_value.get_array_element_by(5).get_number() == -1.5e-3);
  BOOST_CHECK(json_value.get_array_element_by(6).get_boolean() == false);
  BOOST_CHECK(json_value.get_array_element_by(7).get_string() == "null");

    BOOST_CHECK(json_value[0].get_null() == nullptr);
    BOOST_CHECK(json_value[1].get_number() == 12);
    BOOST_CHECK(json_value[2].get_boolean() == true);
    BOOST_CHECK(json_value[3].get_number() == 12.3);
    BOOST_CHECK(json_value[4].get_number() == -5.6);
    BOOST_CHECK(json_value[5].get_number() == -1.5e-3);
    BOOST_CHECK(json_value[6].get_boolean() == false);
    BOOST_CHECK(json_value[7].get_string() == "null");

  // 数组嵌套
  auto &&[json_value1, json_err1] = jp.parse("[null, [null]]");
  BOOST_CHECK(json_err1 == JSON_PARSE_OK);
  BOOST_CHECK(json_value1.get_array_element_by(1).get_array_element_by(0).get_null() == nullptr);
    BOOST_CHECK(json_value1[1][0].get_null() == nullptr);
  auto &&[json_value2, json_err2] = jp.parse("[null, [null, [null]]]");
  BOOST_CHECK(json_err2 == JSON_PARSE_OK);
}
static void test_array_error()
{
  TEST_PARSE_ERROR(JSON_PARSE_ARRAY_MISS_VALUE, "[a]") ;
  TEST_PARSE_ERROR(JSON_PARSE_ARRAY_MISS_COMMA, "[123, null false]") ;
  TEST_PARSE_ERROR(JSON_PARSE_ARRAY_MISS_COMMA, "[null, null false]") ;
  TEST_PARSE_ERROR(JSON_PARSE_ARRAY_MISS_COMMA, "[null, 123 false]") ;
  TEST_PARSE_ERROR(JSON_PARSE_ARRAY_MISS_COMMA, "[null, 123 10.5]") ;
  TEST_PARSE_ERROR(JSON_PARSE_ARRAY_MISS_RIGHT_BRACKET, "[123") ;
  TEST_PARSE_ERROR(JSON_PARSE_ARRAY_MISS_RIGHT_BRACKET, "[123, null") ;
  TEST_PARSE_ERROR(JSON_PARSE_ARRAY_LAST_MUST_NOT_COMMA, "[123, false,]") ;
}
static void test_parse()
{
#if 0
  test_parse_null();
  test_parse_bool();
  test_parse_number();
  test_root_not_singular();
  test_invalid_value();
  test_parse_invalid_number();
  test_parse_string();
  test_parse_object_miss_key();
  test_parse_object_miss_member();
  test_parse_object_miss_colon();
  test_parse_object_miss_comma();
  test_parse_object_miss_bracket();
#endif
#if 0
    test_parse_array();
  test_parse_object();
  test_array_error();
#endif
}
static void test_stringfy()
{
}
static void test_all() {
    test_parse();
    test_stringfy();
}

int test_main( int argc, char *argv[] ) {

  test_all();
  return 0;
}