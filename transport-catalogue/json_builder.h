#pragma once

#include "json.h"

#include <optional>

namespace json { 
    
class Builder { 
private:
    class BaseContext;
    class DictItemContext;
    class ArrayItemContext;
    class DictKeyContext;
    
public:
    BaseContext Value(Node::Value value);

    DictItemContext StartDict();
    
    ArrayItemContext StartArray();
    
    DictKeyContext Key(std::string str);
    
    BaseContext EndDict();
    
    BaseContext EndArray();
    
    Node Build() const;
    
private:
    Node MakeNodeFromValue(Node::Value value) const;
    void AddComplexNode(Node::Value value);
    
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> key_;
    
    mutable bool object_is_complete_ = false;      

    class BaseContext {
    public:
        BaseContext(Builder& builder) 
            : builder_(builder) {
        }
        
        Node Build() {
            return builder_.Build();
        }
        
        DictKeyContext Key(std::string key) {
            return builder_.Key(std::move(key));
        }
        
        BaseContext Value(Node::Value value) {
            return builder_.Value(std::move(value));
        }
        
        DictItemContext StartDict() {
            return builder_.StartDict();
        }
        
        ArrayItemContext StartArray() {
            return builder_.StartArray();
        }
        
        BaseContext EndDict() {
            return builder_.EndDict();
        }
        
        BaseContext EndArray() {
            return builder_.EndArray();
        }
        
    private:
        Builder& builder_;
    };    
 
    class DictItemContext : public BaseContext {
    public:
        DictItemContext(BaseContext base)
            : BaseContext(base) {            
        }
        
        BaseContext Value(Node::Value value) = delete;
        DictItemContext StartDict() = delete;    
        ArrayItemContext StartArray() = delete;
        BaseContext EndArray() = delete;
        Node Build() = delete;      
    };    
    
    class ArrayItemContext : public BaseContext {
    public:
        ArrayItemContext(BaseContext base)
            : BaseContext(base) {            
        }
        
        ArrayItemContext Value(Node::Value value) { 
            return BaseContext::Value(std::move(value)); 
        }
        
        DictKeyContext Key(std::string str) = delete;
        Builder& EndDict() = delete;
        Node Build() = delete;   
    };
        
    class DictKeyContext : public BaseContext {
    public:    
        DictKeyContext(BaseContext base)
            : BaseContext(base) {            
        }    
    
        DictItemContext Value(Node::Value value) { 
            return BaseContext::Value(std::move(value)); 
        }
        
        DictKeyContext Key(std::string str) = delete;    
        BaseContext EndDict() = delete;    
        BaseContext EndArray() = delete;
        Node Build() const = delete;             
    };
};      
       
}
