#pragma once
#include "JsonType.hpp"
#include <boost/assert.hpp>
#include <array>
#include <map>
#include <unordered_map>


struct JsonTree {

};

enum JParseError {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_INVALID_STRING_ESCAPE,
    LEPT_PARSE_INVALID_STRING_CHAR,
};


struct {

    JsonType type;
};

class JsonParse {
  public:
    // ��Json�ı�������Json��
    // ��ò�Ҫ�����ַ�����'\0'�ַ�������ʹ��size����
    void skip_space() {
        while ( curr_index_ != size_ && isspace( context_[curr_index_] ) ) {
            curr_index_++;
        }
    }

    // ������﷨�� ws value ws  , wsָ�հ׷�
    JParseError parse( const char *context, int size ) {
        context_ = context;
        size_ = size;
        curr_index_ = 0;
        assert( context );
        // strip space
        skip_space();

        type_ = JsonType::JSON_INVALID;
        JParseError ret;
        if ( ( ret = parse_value() ) == JParseError::LEPT_PARSE_OK ) {
            skip_space();
            if ( curr_index_ != size_ )
                return JParseError::LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
        return ret;
    }

    JParseError parse_value() {
        if ( curr_index_ == size_ )
            return LEPT_PARSE_EXPECT_VALUE;
        switch ( context_[curr_index_] ) {
        case 'n':
            return parse_value_compare_with( "null", 4, JsonType::JSON_NULL );
        case 't':
            return parse_value_compare_with( "true", 4, JsonType::JSON_TRUE );
        case 'f':
            return parse_value_compare_with( "false", 5, JsonType::JSON_FALSE );
        case '\0':
            return LEPT_PARSE_EXPECT_VALUE;
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
            return parse_number();
        default:
            return LEPT_PARSE_INVALID_VALUE;
        }
    }
    // ����״̬ת��ͼ����������number�Ľ���
    //
    enum NumberState {
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
        EXPECTED,
        END,
    };
    static int get_state_index( char x ) {
        if ( x > '0' && x <= '9' )
            return 3;
        switch ( x ) {
        case '-' :
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
            break;
        }
    }
    using TableType = std::map<NumberState, std::array< NumberState, 8>>;
    static TableType get_status_table() {
        static TableType table = {
            /*-*/		/*+*/		/*0*/   /*1-9*/		/*digit*/	/*e*/		/*.*/    /*E_A_N*/
            {START,	   {NEGATIVE,	INVALID,	ZERO,	DIGIT1_9,	INVALID,	INVALID,	INVALID, INVALID }},
            {NEGATIVE, {INVALID,	INVALID,	ZERO,	DIGIT1_9,	INVALID,	INVALID,	INVALID, INVALID }},
            {POSITIVE, {INVALID,	INVALID,	DIGIT,	DIGIT,		DIGIT,		INVALID,	INVALID, INVALID }},
            {ZERO,	   {INVALID,	INVALID,	DIGIT,	DIGIT,		DIGIT,		STATUS_E,	POINT, INVALID }},
            {DIGIT1_9, {INVALID,	INVALID,	DIGIT,	DIGIT,		DIGIT,		STATUS_E,	POINT, INVALID }},
            {DIGIT,    {INVALID,	INVALID,	DIGIT,	DIGIT,		DIGIT,		STATUS_E,	POINT, INVALID }},
            {STATUS_E, {E_AFTER_N,	POSITIVE,	DIGIT,	DIGIT,		DIGIT,		INVALID,	INVALID, INVALID }},
            {POINT,	   {INVALID,	INVALID,	DIGIT,	DIGIT,		DIGIT,		INVALID,	INVALID, INVALID }},
            {E_AFTER_N,{INVALID,	INVALID,	DIGIT,	DIGIT,		DIGIT,		INVALID,	INVALID, INVALID }},
        };
        return table;
    }
    JParseError parse_number() {
        NumberState prev_state = START;
        double K = 0.1;
        // ״̬������ͬ��ֵ������ͬ�ı仯
        while ( curr_index_ != size_ ) {
            char curr_char = context_[curr_index_];
            if ( isspace( curr_char ) ) {
                type_ = JsonType::JSON_NUMBER;
                return JParseError::LEPT_PARSE_OK;
            }

            auto index_of_curr_char = get_state_index( curr_char );
            if ( index_of_curr_char == -1 ) /*�����յ���Ч��״̬*/
                break;
            auto curr_state = get_status_table().at( prev_state ).at( index_of_curr_char );
            prev_state = curr_state;
            // process prev_state
            if ( prev_state == INVALID )
                return JParseError::LEPT_PARSE_INVALID_VALUE;
            if ( prev_state == POSITIVE ) {
                continue;
            }
            if ( prev_state == NEGATIVE ) {
                integral_part_flag_ = -1;
            } else if ( prev_state ==  ZERO || prev_state == DIGIT1_9|| prev_state == DIGIT ) {
                // ����Ҫ���������ֵĶ���
                int curr_num = curr_char - '0';
                switch ( res_num_status_ ) {
                case JsonParse::INTEGRAL:
                    integral_part_ = integral_part_ * 10 + curr_num;
                    break;
                case JsonParse::FRACTIONAL:
                    fractional_part_ = fractional_part_ + ( curr_num ) * K;
                    K = K * 0.1;
                    break;
                case JsonParse::EXPONENTIAL:
                    exponential_part_ = exponential_part_ * 10 + curr_num;
                    break;
                default:
                    break;
                }
            } else if ( prev_state == POINT ) {
                res_num_status_ = FRACTIONAL;
            } else if ( prev_state == STATUS_E ) {
                res_num_status_ = EXPONENTIAL;
            } else if ( prev_state == E_AFTER_N ) {
                exponential_part_flag_ = -1;
            }

            if ( curr_index_ == size_ && ( prev_state == DIGIT || prev_state == DIGIT1_9 || prev_state == ZERO ) ) {
                prev_state = EXPECTED;
                type_ = JsonType::JSON_NUMBER;
                return JParseError::LEPT_PARSE_OK;
            }
        }
        return LEPT_PARSE_INVALID_VALUE;
    }
    // �����ַ�����ע��ת���ַ��Ĵ���
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

    bool parse_zhuanyi_string( char next_char ) {
        switch ( next_char ) {
        case 0x22 /*	"	*/:
        case 0x5c /*	\	*/:
        case 0x2f /*	/	*/:
            str_.push_back( next_char );
            break;
        case 0x62 /*	b	*/:
            str_.push_back( 0x08 );
            break;
        case 0x66 /*	f	*/:
            str_.push_back( 0x0c );
            break;
        case 0x6e /*	n	*/:
            str_.push_back( 0x0A );
            break;
        case 0x72 /*	r	*/:
            str_.push_back( 0x0d );
            break;
        case 0x74 /*	t	*/:
            str_.push_back( 0x09 );
            break;
        default:
            return false;
        }
        return true;
    }
    // ����Json����
    JParseError parse_object() {

        // expect
        assert( context_[curr_index_++] == '{' );
        skip_space();
        if ( context_[curr_index_++] == '}' ) {
            type_ = JsonType::JSON_OBJECT;
            return LEPT_PARSE_OK;
        }
        JParseError err;
        for ( ;; ) {
            // ����key
            if ( ( err = parse_string() ) != LEPT_PARSE_OK )
                break;
            std::string str = str_;
            str_.clear();

            skip_space();

            // �����ֺ�
            if ( context_[curr_index_++] != ':' )
                break;

            skip_space();
            // ����value
            if ( ( err = parse_value() ) != LEPT_PARSE_OK )
                break;

            skip_space();

            // ����',' ע�����һ�в�������,
            if ( context_[curr_index_++] != ',' )
                break;

            skip_space();
            // ��key-value ����map
            object_;
        }
    }
    JParseError parse_string() {
        curr_index_++; // ������ʼ��\"
        char curr_char = context_[curr_index_];
        char next_char = context_[curr_index_ + 1];
        bool is_ok;
        while ( curr_index_ != size_ ) {
            switch ( curr_char ) {
            case '\"' :
                break;
            case '\\':
                if ( curr_index_ + 1 == size_ )
                    return JParseError::LEPT_PARSE_INVALID_VALUE;

                is_ok = parse_zhuanyi_string( next_char );
                if ( !is_ok )
                    return JParseError::LEPT_PARSE_INVALID_STRING_ESCAPE;
            default:
                if ( curr_char < 0x20 ) {
                    return LEPT_PARSE_INVALID_STRING_CHAR;
                }
                str_.push_back( curr_char );
                break;
            }
        }
        type_ = JsonType::JSON_STRING;
        return JParseError::LEPT_PARSE_OK;
    }
    JParseError parse_value_compare_with( const char *str, size_t n, JsonType type ) {
        int index = curr_index_;
        if( strncmp( &context_[curr_index_], str, n ) != 0 )
            return LEPT_PARSE_INVALID_VALUE;
        curr_index_ += n;
        type_ = type;
        return LEPT_PARSE_OK;
    }
    // ����key���������
    JsonType get_type() const {
        return type_;
    }
    void *get_null() const {

    }
    bool get_boolean() const {

        if ( type_ == JsonType::JSON_FALSE || type_ == JsonType::JSON_TRUE )
            throw std::logic_error( "��������: ����bool����" );
        return type_ == JsonType::JSON_FALSE ? false : true;
    }
    double get_num() const {
        throw std::logic_error( "��������: ����number����" );
        double res =  integral_part_flag_ * ( integral_part_ + fractional_part_ ) * pow( 10, exponential_part_flag_ * exponential_part_ );
        return res;
    }
  private:
    size_t size_{};
    const char *context_{};
    size_t curr_index_{};
    JsonType type_{ JsonType::JSON_INVALID };

    std::string str_;
    class MemberType;
    std::unordered_map<std::string, MemberType> object_;

    int integral_part_flag_{1};
    int exponential_part_flag_{1};
    double integral_part_{};
    double fractional_part_{};
    double exponential_part_{};
    enum ResNumStatus {
        INTEGRAL,
        FRACTIONAL,
        EXPONENTIAL,
    } res_num_status_;
};
