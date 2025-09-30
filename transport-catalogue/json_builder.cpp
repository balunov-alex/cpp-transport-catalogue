#include "json_builder.h"

namespace json {
    
Builder::BaseContext Builder::Value(Node::Value value) {
    if (object_is_complete_) {
        throw std::logic_error("Calling Value(Node::Value) with the finished object");
    }
    if (!key_.has_value() && nodes_stack_.size() && !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Calling Value(Node::Value) in wrong place");
    }
    if (nodes_stack_.empty()) {
        root_.GetValue() = std::move(value);
        object_is_complete_ = true;
    } else {
        Node& last_complex_node = *nodes_stack_.back();
        if (last_complex_node.IsArray()) {
            last_complex_node.AsArray().emplace_back(MakeNodeFromValue(std::move(value)));
        }
        if (last_complex_node.IsDict()) {
            last_complex_node.AsDict().emplace(key_.value(), MakeNodeFromValue(std::move(value)));
            key_ = std::nullopt;
        } 
    }
    return *this;
}

Builder::DictItemContext Builder::StartDict() {
    if (object_is_complete_) {
        throw std::logic_error("Calling StartDict() with the finished object");
    }    
    if (!key_.has_value() && nodes_stack_.size() && !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Calling StartDict() in wrong place");
    }
    AddComplexNode(Dict());
    return BaseContext{*this};
}

Builder::ArrayItemContext Builder::StartArray() {
    if (object_is_complete_) {
        throw std::logic_error("Calling StartArray() with the finished object");
    }        
    if (!key_.has_value() && nodes_stack_.size() && !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Calling StartArray() in wrong place");
    }
    AddComplexNode(Array());       
    return BaseContext{*this};
}

Builder::DictKeyContext Builder::Key(std::string str) {
    if (object_is_complete_) {
        throw std::logic_error("Calling Key(std::string) with the finished object");
    }         
    if (!nodes_stack_.back()->IsDict() || key_) {
        throw std::logic_error("Calling Key(std::string) from outside the Dict or after another Key(std::string)");
    }
    key_ = std::move(str);
    return BaseContext{*this};
}
    
Builder::BaseContext Builder::EndDict() {
    if (object_is_complete_) {
        throw std::logic_error("Calling EndDict() with the finished object");
    }     
    if (!nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Calling EndDict() in the context of another container");
    }
    nodes_stack_.pop_back();
    if (!nodes_stack_.size()) {
        object_is_complete_ = true;
    }
    return *this;
}
    
Builder::BaseContext Builder::EndArray() {
    if (object_is_complete_) {
        throw std::logic_error("Calling EndArray() with the finished object");
    }         
    if (!nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Calling EndArray() in the context of another container");
    }    
    nodes_stack_.pop_back();
    if (!nodes_stack_.size()) {
        object_is_complete_ = true;
    }    
    return *this;
}
    
Node Builder::Build() const {
    if (!object_is_complete_) {
        throw std::logic_error("Calling Build() when the described object is not ready");
    }
    return std::move(root_);
}
    
Node Builder::MakeNodeFromValue(Node::Value value) const {
    if (std::holds_alternative<int>(value)) {
        return std::get<int>(value);
    }
    if (std::holds_alternative<double>(value)) {
        return std::get<double>(value);
    }
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value);
    }
    if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(std::move(value));
    }
    if (std::holds_alternative<Array>(value)) {
        return std::get<Array>(std::move(value));
    }
    if (std::holds_alternative<Dict>(value)) {
        return std::get<Dict>(std::move(value));
    }
    return {};
}
    
void Builder::AddComplexNode(Node::Value value) {
    if (nodes_stack_.empty()) {
        root_.GetValue() = std::move(value);
        nodes_stack_.push_back(&root_);
    } else {    
        Node& last_complex_node = *nodes_stack_.back();
        if (last_complex_node.IsArray()) {
            Array& last_node_array = last_complex_node.AsArray();
            last_node_array.emplace_back(MakeNodeFromValue(std::move(value)));
            nodes_stack_.push_back(&last_node_array.back());
        }
        if (last_complex_node.IsDict()) {
            Dict& last_node_dict = last_complex_node.AsDict();
            last_node_dict.emplace(key_.value(), MakeNodeFromValue(std::move(value)));
            nodes_stack_.push_back(&last_node_dict.at(key_.value()));
            key_ = std::nullopt;
        }        
    }    
}    
    
}
