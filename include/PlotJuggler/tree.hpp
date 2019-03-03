/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright 2016-2017 Davide Faconti
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of Willow Garage, Inc. nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
* *******************************************************************/


#ifndef STRING_TREE_H
#define STRING_TREE_H

#include <vector>
#include <iostream>
#include <memory>


/**
 * @brief Element of the tree. it has a single parent and N >= 0 children.
 */
class StringNode
{

public:

  typedef std::vector<StringNode*> ChildrenVector;

  StringNode(const StringNode* parent );

  const StringNode* parent() const  { return _parent; }

  const std::string& value() const          { return _value; }
  void setValue( const std::string& value)  { _value = value; }

  const ChildrenVector& children()const   { return _children; }
  ChildrenVector& children()              { return _children; }

  const StringNode* child(size_t index) const { return (_children[index]); }
  StringNode* child(size_t index) { return (_children[index]); }

  StringNode *addChild(const std::string& child );

  bool contains(const std::string& val);

  bool isLeaf() const { return _children.empty(); }

  std::string toString(char delimiter = '/') const;

private:
  const StringNode*   _parent;
  std::string         _value;
  ChildrenVector    _children;
};

//-----------------------------------------


inline void PrintTree(std::ostream &os, const StringNode* node, int indent)
{
  for (int i=0; i<indent; i++) os << " ";
  os << node->value() << std::endl;

  for (const auto& child: node->children() )
  {
    PrintTree(os, child, indent+3);
  }
}

inline StringNode::StringNode(const StringNode *parent):
  _parent(parent)
{

}

inline bool StringNode::contains(const std::string &val)
{
    for(const auto& v: _children)
    {
        if( v->value() == val ) return true;
    }
    return false;
}

inline std::string StringNode::toString(char delimiter) const
{
    std::string out;
    const StringNode* array[16];
    int index = 0;
    size_t out_size = 0;
    const StringNode* node = this;
    while(node != nullptr)
    {
        array[index++] = node;
        out_size += node->value().size();
        node = node->parent();
    }
    const size_t array_size(index);
    index--;
    out.reserve(out_size + array_size);
    while( index >= 0)
    {
        out.append( array[index]->value() );
        if( index != 0)
        {
            out.push_back( delimiter );
        }
    }
    return out;
}

inline StringNode *StringNode::addChild(const std::string& value)
{
  for(const auto& v: _children)
  {
      if( v->value() == value ) return v;
  }
  _children.push_back( new StringNode(this) );
  _children.back()->setValue( value );
  return _children.back();
}



#endif // STRING_TREE_H
