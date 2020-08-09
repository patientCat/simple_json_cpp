#pragma once
#include "boost/assert.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

enum JParseError {
  JSON_PARSE_OK = 0,
  JSON_PARSE_EXPECT_VALUE,
  JSON_PARSE_INVALID_VALUE,
  JSON_PARSE_ROOT_NOT_SINGULAR,
  JSON_PARSE_INVALID_STRING_ESCAPE,
  JSON_PARSE_INVALID_STRING_CHAR,
  JSON_PARSE_STRING_MISS_DOUBLE_QUATION,  // 没有找到"
  // JSON_OBJECT
  JSON_PARSE_OBJECT_MISS_KEY,
  JSON_PARSE_OBJECT_MISS_COLON,
  JSON_PARSE_OBJECT_MISS_MEMBER,
  JSON_PARSE_OBJECT_MISS_COMMA,
  JSON_PARSE_OBJECT_MISS_RIGHT_BRACKET,
  JSON_PARSE_OBJECT_LAST_MUST_NOT_COMMA,
  // JSON_ARRAY
  JSON_PARSE_ARRAY_MISS_VALUE,
  JSON_PARSE_ARRAY_MISS_COMMA,
  JSON_PARSE_ARRAY_LAST_MUST_NOT_COMMA,
  JSON_PARSE_ARRAY_MISS_RIGHT_BRACKET,
};

enum class EJsonType : char {
  JSON_INVALID,
  JSON_NULL,
  JSON_TRUE,
  JSON_FALSE,
  JSON_NUMBER,
  JSON_STRING,
  JSON_ARRAY,
  JSON_OBJECT,
};

class ParseNumHelper {
  enum class ResNumStatus {
    INTEGRAL,
    FRACTIONAL,
    EXPONENTIAL,
  };

 public:
  ParseNumHelper() { reset(); }
  void reset() {
    integral_part_ = 0;
    fractional_part_ = 0;
    exponential_part_ = 0;
    integral_part_flag_ = 1;
    exponential_part_flag_ = 1;
    K = 0.1;

    set_integral_status();
  }
  double get_num() const {
    double res = integral_part_flag_ * (integral_part_ + fractional_part_) *
                 pow(10, exponential_part_flag_ * exponential_part_);
    return res;
  }
  void calc_number(int curr_num) {
    switch (num_status_) {
      case ResNumStatus::INTEGRAL:
        integral_part_ = integral_part_ * 10 + curr_num;
        break;
      case ResNumStatus::FRACTIONAL:
        fractional_part_ = fractional_part_ + (curr_num)*K;
        K = K * 0.1;
        break;
      case ResNumStatus::EXPONENTIAL:
        exponential_part_ = exponential_part_ * 10 + curr_num;
        break;
      default:
        break;
    }
  }
  void set_exponential_flag() { exponential_part_flag_ = -1; }
  void set_integral_flag() { integral_part_flag_ = -1; }
  void set_fractional_status() { num_status_ = ResNumStatus::FRACTIONAL; }
  void set_exponential_status() { num_status_ = ResNumStatus::EXPONENTIAL; }

 private:
  double K = 0.1;
  void set_integral_status() { num_status_ = ResNumStatus::INTEGRAL; }
  int integral_part_flag_{1};
  int exponential_part_flag_{1};
  double integral_part_{};
  double fractional_part_{};
  double exponential_part_{};
  ResNumStatus num_status_{ResNumStatus::INTEGRAL};
};


class JsonParse;
class JsonImpl;
class JsonType{
    friend  JsonParse;
public:
    explicit JsonType(JsonImpl *impl = nullptr)
    {
        impl_.reset(impl);
    }
    JsonType(const JsonType&) = delete;
    void operator=(const JsonType&) = delete;

    JsonType(JsonType&& rhs)
    {
        std::swap(impl_, rhs.impl_);
    }
    JsonType & operator=(JsonType&& rhs)
    {
        if(this != &rhs)
        {
            std::swap(impl_, rhs.impl_);
        }
        return *this;
    }
    void reset(JsonImpl *impl)
    {
        impl_.reset(impl);
    }

    JsonType& operator[](size_t index);
    // 根据key来获得数据
    EJsonType get_type() const ;

    std::string get_string();

    JsonType& get_array_element_by(size_t index);

    JsonType& get_object_element_by(const std::string &key);

    [[nodiscard]] void *get_null() const;

    [[nodiscard]] bool get_boolean() const;

    [[nodiscard]] double get_number() const;
private:
    std::unique_ptr<JsonImpl> impl_;
};

using JsonNullType = void *;
using JsonBoolType = bool;
using JsonNumberType = double;
using JsonStringType = std::string;

using JsonArrayType = std::vector<JsonType>;
using JsonObjectType =
    std::unordered_map<JsonStringType, JsonType>;


class JsonImpl {
 public:
  friend class JsonParse;

  using ObjectType =
      std::variant<JsonNullType, JsonBoolType, JsonNumberType, JsonStringType,
                   JsonArrayType, JsonObjectType>;

 private:
  ObjectType obj{};
  EJsonType type{EJsonType::JSON_INVALID};

 public:
public:

  explicit JsonImpl(EJsonType atype = EJsonType::JSON_INVALID) : type(atype) {}
  JsonImpl(const JsonImpl &) = delete;
  void operator=(const JsonImpl &) = delete;
  JsonImpl(JsonImpl &&rhs) {
    std::swap(obj, rhs.obj);
    std::swap(type, rhs.type);
  }
  JsonImpl &operator=(JsonImpl &&rhs) {
    swap(obj, rhs.obj);
    type = rhs.type;
    rhs.type = EJsonType ::JSON_INVALID;
    return *this;
  }

 public:
  // 根据key来获得数据
  EJsonType get_type() const { return type; }

  std::string get_string() {
    BOOST_ASSERT_MSG(type == EJsonType::JSON_STRING, "type is not json_string");
    return std::get<std::string>(obj);
  }

  JsonType& get_array_element_by(size_t index) {
    BOOST_ASSERT_MSG(type == EJsonType::JSON_ARRAY, "type is not json_array");
    auto &json_array = std::get<JsonArrayType>(obj);
    BOOST_ASSERT_MSG(index < json_array.size(), "index out of json_array");
    auto &value = json_array[index];
    return value;
  }

  JsonType& get_object_element_by(const std::string &key) {
    BOOST_ASSERT_MSG(type == EJsonType::JSON_OBJECT, "type is not json_object");
    auto &json_object = std::get<JsonObjectType>(obj);
    BOOST_ASSERT_MSG(json_object.count(key) > 0,
                     "key not exist, please check key spelling");
    auto &value = json_object[key];
    return value;
  }

  [[nodiscard]] void *get_null() const {
    BOOST_ASSERT_MSG(type == EJsonType::JSON_NULL, "type is not json_null");
    return std::get<void *>(obj);
  }

  [[nodiscard]] bool get_boolean() const {
    BOOST_ASSERT_MSG(
        type == EJsonType::JSON_TRUE || type == EJsonType::JSON_FALSE,
        "type is not json_bool");
    return std::get<bool>(obj);
  }

  [[nodiscard]] double get_number() const {
    BOOST_ASSERT_MSG(type == EJsonType::JSON_NUMBER, "type is not json_number");
    return std::get<double>(obj);
  }
};


class JsonParse {
 public:
  // 将Json文本解析成Json树

  static std::pair<JsonType, JParseError> parse(std::string_view str) {
    return parse((const char *)str.data(), str.size());
  }
  // 这里的语法是 ws value ws  , ws指空白符
  static std::pair<JsonType, JParseError> parse(const char *context,
                                                   int size) {
    context_ = context;
    size_ = size;
    curr_index_ = 0;
    assert(context);
    // strip space
    skip_space();

    std::unique_ptr<JsonImpl> curr_value = std::make_unique<JsonImpl>();
    JParseError ret;
    if ((ret = parse_value(*curr_value)) == JParseError::JSON_PARSE_OK) {
      skip_space();
      if (curr_index_ != size_)
        return {JsonType{}, JParseError::JSON_PARSE_ROOT_NOT_SINGULAR};
    }
    std::pair<JsonType, JParseError> res;
    res.first.reset(curr_value.release());
    res.second = ret;
    return res;
  }

  // 生成器
    static std::string stringfy(const JsonType& json_t)
    {
        switch (json_t.get_type()) {
            case EJsonType::JSON_NULL:
                return "null";
                break;
            case EJsonType::JSON_TRUE:
                return "true";
                break;
            case EJsonType::JSON_FALSE:
                return "false";
                break;
            case EJsonType::JSON_NUMBER:
                break;
            case EJsonType::JSON_ARRAY:
                break;
            case EJsonType::JSON_OBJECT:
                break;
            default:
                break;
        }
    }

private:
    static std::string stringfy_array(JsonArrayType & json_array)
    {

    }
    static std::string stringfy_object(JsonObjectType & json_object)
    {

    }
public:
private:
    // 跳过空格，到一个非空格字符
  static void skip_space() {
    while (curr_index_ != size_ && isspace(context_[curr_index_])) {
      curr_index_++;
    }
  }
  static JParseError parse_value(JsonImpl &value) {
    static const int NULL_LENGTH = 4;
    static const int TRUE_LENGTH = 4;
    static const int FALSE_LENGTH = 5;
    if (curr_index_ == size_) return JSON_PARSE_EXPECT_VALUE;
    JParseError err;
    switch (context_[curr_index_]) {
      case 'n':
        err = parse_value_compare_with("null", NULL_LENGTH);
        value.type = EJsonType::JSON_NULL;
        value.obj = nullptr;
        return err;

      case 't':
        err = parse_value_compare_with("true", TRUE_LENGTH);
        value.type = EJsonType::JSON_TRUE;
        value.obj = true;
        return err;
      case 'f':
        err = parse_value_compare_with("false", FALSE_LENGTH);
        value.type = EJsonType::JSON_FALSE;
        value.obj = false;
        return err;
      case '\0':
        return JSON_PARSE_EXPECT_VALUE;
      case '-':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        return parse_number(value);
      case '\"':
        return parse_string(value);
      case '{':
        return parse_object(value);
      case '[':
        return parse_array(value);
      default:
        return JSON_PARSE_INVALID_VALUE;
    }
  }
  // 根据状态转移图，很容易做number的解析
  //
  enum class NumberState {
    NEGATIVE,
    POSITIVE,
    ZERO,
    DIGIT1_9,
    DIGIT,
    STATUS_E,
    POINT,
    E_AFTER_N,
    START,
    INVALID,
  };
  static int get_state_index(char x) {
    if (x > '0' && x <= '9') return 3;
    switch (x) {
      case '-':
        return 0;
      case '+':
        return 1;
      case '0':
        return 2;
      case 'e':
      case 'E':
        return 5;
      case '.':
        return 6;
      default:
        return -1;
    }
  }
  using TableType = std::map<NumberState, std::array<NumberState, 8>>;
  static TableType get_status_table() {
    static TableType table = {
        /*0 stands for NEGATIVE */
        /*1 stands for POSITIVE */
        /*2 stands for ZERO */
        /*3 stands for DIGIT1_9*/
        /*4 stands for DIGIT*/
        /*5 stands for STATUS_E*/
        /*6 stands for POINT*/
        /*7 stands for E_A_N*/
        {NumberState::START,
         {NumberState::NEGATIVE, NumberState::INVALID, NumberState::ZERO,
          NumberState::DIGIT1_9, NumberState::INVALID, NumberState::INVALID,
          NumberState::INVALID, NumberState::INVALID}},
        {NumberState::NEGATIVE,
         {NumberState::INVALID, NumberState::INVALID, NumberState::ZERO,
          NumberState::DIGIT1_9, NumberState::INVALID, NumberState::INVALID,
          NumberState::INVALID, NumberState::INVALID}},
        {NumberState::POSITIVE,
         {NumberState::INVALID, NumberState::INVALID, NumberState::DIGIT,
          NumberState::DIGIT, NumberState::DIGIT, NumberState::INVALID,
          NumberState::INVALID, NumberState::INVALID}},
        {NumberState::ZERO,
         {NumberState::INVALID, NumberState::INVALID, NumberState::DIGIT,
          NumberState::DIGIT, NumberState::DIGIT, NumberState::STATUS_E,
          NumberState::POINT, NumberState::INVALID}},
        {NumberState::DIGIT1_9,
         {NumberState::INVALID, NumberState::INVALID, NumberState::DIGIT,
          NumberState::DIGIT, NumberState::DIGIT, NumberState::STATUS_E,
          NumberState::POINT, NumberState::INVALID}},
        {NumberState::DIGIT,
         {NumberState::INVALID, NumberState::INVALID, NumberState::DIGIT,
          NumberState::DIGIT, NumberState::DIGIT, NumberState::STATUS_E,
          NumberState::POINT, NumberState::INVALID}},
        {NumberState::STATUS_E,
         {NumberState::E_AFTER_N, NumberState::POSITIVE, NumberState::DIGIT,
          NumberState::DIGIT, NumberState::DIGIT, NumberState::INVALID,
          NumberState::INVALID, NumberState::INVALID}},
        {NumberState::POINT,
         {NumberState::INVALID, NumberState::INVALID, NumberState::DIGIT,
          NumberState::DIGIT, NumberState::DIGIT, NumberState::INVALID,
          NumberState::INVALID, NumberState::INVALID}},
        {NumberState::E_AFTER_N,
         {NumberState::INVALID, NumberState::INVALID, NumberState::DIGIT,
          NumberState::DIGIT, NumberState::DIGIT, NumberState::INVALID,
          NumberState::INVALID, NumberState::INVALID}},
    };
    return table;
  }
  // 解析数组
  static JParseError parse_array(JsonImpl &value) {
    BOOST_ASSERT(context_[curr_index_] == '[');
    curr_index_++;
    skip_space();
    value.obj.emplace<JsonArrayType>(JsonArrayType{});
    if (context_[curr_index_] == ']') {
      if (context_[curr_index_] == '}') {
        curr_index_++;
        return JSON_PARSE_ARRAY_LAST_MUST_NOT_COMMA;
      }
      value.type = EJsonType::JSON_ARRAY;
      curr_index_++;
      return JParseError::JSON_PARSE_OK;
    }

    enum class JParseArrayStatus {
      EXPECTED_VALUE,
      EXPECTED_COMMA,
      EXPECTED_BRAKCET,
    } status;

    auto &json_array = std::get<JsonArrayType>(value.obj);
    status = JParseArrayStatus::EXPECTED_VALUE;
    JParseError err;

    std::unique_ptr array_obj(std::make_unique<JsonImpl>());
    int array_obj_num = 0;

    // 为什么要用一个队列？因为如果解析失败的话，会发生内存泄漏
    std::queue<std::unique_ptr<JsonImpl>> local_queue;
    for (; curr_index_ != size_;) {
      switch (status) {
        case JParseArrayStatus::EXPECTED_VALUE:
          if (context_[curr_index_] == ']') {
            curr_index_++;
            return JSON_PARSE_ARRAY_LAST_MUST_NOT_COMMA;
          }
          if ((err = parse_value(*array_obj)) != JSON_PARSE_OK) {
            return JSON_PARSE_ARRAY_MISS_VALUE;
          }
          skip_space();
          status = JParseArrayStatus::EXPECTED_COMMA;
          break;
        case JParseArrayStatus::EXPECTED_COMMA:
          if (context_[curr_index_] == ',') {
            // 将值插入到array中
            array_obj_num++;
            local_queue.push(std::move(array_obj));
            status = JParseArrayStatus::EXPECTED_VALUE;

            // 重置array_obj
            array_obj = std::move(std::make_unique<JsonImpl>());

            curr_index_++;
            skip_space();
          } else if (context_[curr_index_] == ']') {
            array_obj_num++;
            local_queue.push(std::move(array_obj));
            status = JParseArrayStatus::EXPECTED_BRAKCET;
          } else {
            return JSON_PARSE_ARRAY_MISS_COMMA;
          }
          break;
        case JParseArrayStatus::EXPECTED_BRAKCET:
          if (context_[curr_index_] == ']') {
            assert(array_obj_num == local_queue.size());
            curr_index_++;

            while (!local_queue.empty()) {
              json_array.emplace_back(local_queue.front().release());
              local_queue.pop();
            }

            value.type = EJsonType::JSON_ARRAY;
            return JSON_PARSE_OK;
          } else {
            return JSON_PARSE_ARRAY_MISS_RIGHT_BRACKET;
          }
        default:
          BOOST_ASSERT(false);  // 不可能情况
      }
    }
    switch (status) {
      case JParseArrayStatus::EXPECTED_VALUE:
        return JSON_PARSE_ARRAY_MISS_VALUE;
      case JParseArrayStatus::EXPECTED_COMMA:
        [[fallthrough]];
      case JParseArrayStatus::EXPECTED_BRAKCET:
        return JSON_PARSE_ARRAY_MISS_RIGHT_BRACKET;
    }
  }

  static JParseError parse_number(JsonImpl &value) {
    NumberState prev_state = NumberState::START;
    NumberState curr_state;
    ParseNumHelper parseNumHelper;
    bool is_end = false;
    // 状态遇到不同的值发生不同的变化
    // 结束条件为，curr_index_走到头， 或者get_state_index收到不合理的字符
    for (; curr_index_ != size_; curr_index_++) {
      char curr_char = context_[curr_index_];
      auto index_of_curr_char = get_state_index(curr_char);
      if (index_of_curr_char == -1) /*代表收到不在状态图的字符*/
      {
        break;
      }
      curr_state = get_status_table().at(prev_state).at(index_of_curr_char);
      prev_state = curr_state;
      switch (curr_state) {
        case NumberState::INVALID:
          return JParseError::JSON_PARSE_INVALID_VALUE;
        case NumberState::POSITIVE:
          continue;
        case NumberState::NEGATIVE:
          parseNumHelper.set_integral_flag();
          break;
        case NumberState::E_AFTER_N:
          parseNumHelper.set_exponential_flag();
          break;
        case NumberState::ZERO:
          [[fallthrough]];
        case NumberState::DIGIT1_9:
          [[fallthrough]];
        case NumberState::DIGIT:
          // 这里要计算三部分的东西
          parseNumHelper.calc_number(curr_char - '0');
          break;
        case NumberState::POINT:
          parseNumHelper.set_fractional_status();
          break;
        case NumberState::STATUS_E:
          parseNumHelper.set_exponential_status();
          break;
        case NumberState::START:
          break;
      }  // end switch
    }    // end for
    if (curr_state == NumberState::DIGIT1_9 ||
        curr_state == NumberState::ZERO || curr_state == NumberState::DIGIT) {
      value.obj = parseNumHelper.get_num();
      value.type = EJsonType::JSON_NUMBER;
      return JSON_PARSE_OK;
    }
    return JSON_PARSE_INVALID_VALUE;
  }
  // 解析字符串，注意转义字符的处理
  /*
     %x22 /          ; "    quotation mark  U+0022
     %x5C /          ; \    reverse solidus U+005C
     %x2F /          ; /    solidus         U+002F
     %x62 /          ; b    backspace       U+0008
     %x66 /          ; f    form feed       U+000C
     %x6E /          ; n    line feed       U+000A
     %x72 /          ; r    carriage return U+000D
     %x74 /          ; t    tab             U+0009
  */

  static bool parse_zhuanyi_string(std::string &str, char next_char) {
    switch (next_char) {
      case 0x22 /*	"	*/:
      case 0x5c /*	\	*/:
      case 0x2f /*	/	*/:
        str.push_back(next_char);
        break;
      case 0x62 /*	b	*/:
        str.push_back(0x08);
        break;
      case 0x66 /*	f	*/:
        str.push_back(0x0c);
        break;
      case 0x6e /*	n	*/:
        str.push_back(0x0A);
        break;
      case 0x72 /*	r	*/:
        str.push_back(0x0d);
        break;
      case 0x74 /*	t	*/:
        str.push_back(0x09);
        break;
      default:
        return false;
    }
    return true;
  }
  // 解析Json对象
  static JParseError parse_object(JsonImpl &value) {
    // expect
    assert(context_[curr_index_] == '{');
    curr_index_++;
    skip_space();
    value.obj.emplace<JsonObjectType>(JsonObjectType{});
    if (context_[curr_index_] == '}') {
      curr_index_++;
      value.type = EJsonType::JSON_OBJECT;
      return JSON_PARSE_OK;
    }
    JParseError err;
    enum class JParseObjectStatus : char {
      EXPECTED_KEY = 1,
      EXPECTED_COLON = 2,
      EXPECTED_MEMBER = 4,
      EXPECTED_COMMA = 8,
      EXPECTED_BRAKCET = 16,
      ESTABLISHED = 32,
    } status;
    status = JParseObjectStatus::EXPECTED_KEY;

    JsonStringType key;
    std::unique_ptr<JsonImpl> member;

    int object_num = 0;

    std::queue<std::pair<JsonStringType, std::unique_ptr<JsonImpl>>> local_queue;
    // 获得json_object
    auto &json_object = std::get<JsonObjectType>(value.obj);

    for (; curr_index_ != size_;) {
      switch (status) {
        case JParseObjectStatus::EXPECTED_KEY: {
          if (context_[curr_index_] == '}') {
            curr_index_++;
            return JSON_PARSE_OBJECT_LAST_MUST_NOT_COMMA;
          }
          // 解析key
          JsonImpl tmp;
          if ((err = parse_string(tmp)) != JSON_PARSE_OK) {
            return JSON_PARSE_OBJECT_MISS_KEY;
          }
          key = std::get<JsonStringType>(tmp.obj);
          status = JParseObjectStatus::EXPECTED_COLON;
        } break;
        case JParseObjectStatus::EXPECTED_COLON:
          // 解析分号
          if (context_[curr_index_++] != ':') {
            return JSON_PARSE_OBJECT_MISS_COLON;
          }
          status = JParseObjectStatus::EXPECTED_MEMBER;
          break;
        case JParseObjectStatus::EXPECTED_MEMBER:
          // 解析member
          if ((err = parse_value(*member)) != JSON_PARSE_OK) {
            return JSON_PARSE_OBJECT_MISS_MEMBER;
          }
          status = JParseObjectStatus::EXPECTED_COMMA;
          break;
        case JParseObjectStatus::EXPECTED_COMMA:
          if (context_[curr_index_] == ',') {
            object_num++;
            local_queue.push(std::make_pair<JsonStringType, std::unique_ptr<JsonImpl>>(
                std::move(key), std::move(member)));

            // reset key and member
            key.clear();
            member = std::move(std::make_unique<JsonImpl>());
            curr_index_++;

            status = JParseObjectStatus::EXPECTED_KEY;
          } else if (context_[curr_index_] == '}') {
            object_num++;
            local_queue.push(std::make_pair<JsonStringType, std::unique_ptr<JsonImpl>>(
                std::move(key), std::move(member)));

            status = JParseObjectStatus::EXPECTED_BRAKCET;
          } else {
            return JSON_PARSE_OBJECT_MISS_COMMA;
          }
          break;
        case JParseObjectStatus::EXPECTED_BRAKCET:
          if (context_[curr_index_] == '}') {
            curr_index_++;
            value.type = EJsonType::JSON_OBJECT;

            while (!local_queue.empty()) {
              auto &front_elem = local_queue.front();
              json_object[front_elem.first].reset(front_elem.second.release());
              local_queue.pop();
            }

            return JSON_PARSE_OK;
          } else {
            return JSON_PARSE_OBJECT_MISS_RIGHT_BRACKET;
          }
      }
      skip_space();
    }

    assert(curr_index_ == size_);
    switch (status) {
      case JParseObjectStatus::EXPECTED_KEY:
        return JSON_PARSE_OBJECT_MISS_KEY;
      case JParseObjectStatus::EXPECTED_COLON:
        return JSON_PARSE_OBJECT_MISS_COLON;
      case JParseObjectStatus::EXPECTED_MEMBER:
        return JSON_PARSE_OBJECT_MISS_MEMBER;
      case JParseObjectStatus::EXPECTED_COMMA:  // 仔细理解这种情况
        [[fallthrough]];                        // not code error, e.g {"abc": 1
                          // 这种情况返回的错误应该是缺少'}'
      case JParseObjectStatus::EXPECTED_BRAKCET:
        return JSON_PARSE_OBJECT_MISS_RIGHT_BRACKET;
      default:
        return JSON_PARSE_INVALID_VALUE;
    }
  }

  static JParseError parse_string(JsonImpl &value) {
    value.obj = std::string("");
    auto &str = std::get<std::string>(value.obj);
    if (context_[curr_index_] != '\"')
      return JSON_PARSE_STRING_MISS_DOUBLE_QUATION;
    curr_index_++;  // 跳过起始的\"
    bool is_ok;
    char curr_char;
    char next_char;
    bool is_string = false;
    while (curr_index_ != size_) {
      curr_char = context_[curr_index_];
      switch (curr_char) {
        case '\"':
          is_string = true;
          break;
        case '\\':
          curr_index_++;
          if (curr_index_ == size_)
            return JParseError::JSON_PARSE_INVALID_VALUE;
          next_char = context_[curr_index_];

          is_ok = parse_zhuanyi_string(str, next_char);
          if (!is_ok) return JParseError::JSON_PARSE_INVALID_STRING_ESCAPE;
          break;
        default:
          if (curr_char < 0x20) {
            return JSON_PARSE_INVALID_STRING_CHAR;
          }
          str.push_back(curr_char);
          break;
      }
      curr_index_++;
      if (is_string) break;
    }
  LABEL_STRING_END:
    value.type = EJsonType::JSON_STRING;
    return JParseError::JSON_PARSE_OK;
  }
  static JParseError parse_value_compare_with(const char *str, size_t n) {
    int index = curr_index_;
    if (strncmp(&context_[curr_index_], str, n) != 0)
      return JSON_PARSE_INVALID_VALUE;
    curr_index_ += n;
    return JSON_PARSE_OK;
  }

 private:
    // 为了保证线程安全
  thread_local static const char *context_;
  thread_local static size_t size_;
  thread_local static size_t curr_index_;
};

