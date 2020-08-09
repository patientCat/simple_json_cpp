//
// Created by Pawn on 2020/8/10.
//
#include "JsonParse.hh"


thread_local const char *JsonParse::context_{};
thread_local size_t JsonParse::size_{};
thread_local size_t JsonParse::curr_index_{};

JsonType &JsonType::operator[](size_t index) {
     return impl_->get_array_element_by(index);
}

EJsonType JsonType::get_type() const {
    return impl_->get_type();
}

std::string JsonType::get_string() {

    return impl_->get_string();

}

JsonType &JsonType::get_array_element_by(size_t index) {
    return impl_->get_array_element_by(index);
}

JsonType &JsonType::get_object_element_by(const std::string &key) {
    return impl_->get_object_element_by(key);
}

bool JsonType::get_boolean() const {
    return impl_->get_boolean();
}

void *JsonType::get_null() const {
    return impl_->get_null();
}

double JsonType::get_number() const {
    return impl_->get_number();
}
