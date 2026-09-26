#ifndef _RSTL_RED_BLACK_TREE
#define _RSTL_RED_BLACK_TREE

#include "types.h"

#include "rstl/functional.hpp"
#include "rstl/iterator.hpp"
#include "rstl/pair.hpp"
#include "rstl/rmemory_allocator.hpp"

namespace rstl {

enum node_color {
  kNC_Black,
  kNC_Red,
};

void rbtree_rebalance(void*, void*);
void* rbtree_traverse_forward(const void*, void*);
void* rbtree_rebalance_for_erase(void* header_void, void* node_void);

template < typename T, typename P, int U, typename S = select1st< P >, typename Cmp = less< T >,
           typename Alloc = rmemory_allocator >
class red_black_tree {
private:
  struct node {
    node* mLeft;
    node* mRight;
    node* mParent;
    node_color mColor;
    uchar mValue[sizeof(P)];

    node(node* left, node* right, node* parent, node_color color, const P& value)
    : mLeft(left), mRight(right), mParent(parent), mColor(color) {
      construct(get_value(), value);
    }
    ~node() { reinterpret_cast< P* >(mValue)->~P(); }

    P* get_value() { return reinterpret_cast< P* >(&mValue); }
    const P* get_value() const { return reinterpret_cast< const P* >(&mValue); }

    node* get_left() { return mLeft; }
    void set_left(node* n) { mLeft = n; }
    node* get_right() { return mRight; }
    void set_right(node* n) { mRight = n; }
  };
  class header {
  public:
    header() : mLeftmost(nullptr), mRightmost(nullptr), mRootNode(nullptr) {}
    void set_root(node* n) { mRootNode = n; }
    void set_leftmost(node* n) { mLeftmost = n; }
    void set_rightmost(node* n) { mRightmost = n; }

    node* get_root() const { return mRootNode; }
    node* get_leftmost() const { return mLeftmost; }
    node* get_rightmost() const { return mRightmost; }

  private:
    node* mLeftmost;
    node* mRightmost;
    node* mRootNode;
  };

public:
  struct const_iterator {
    typedef int difference_type;
    typedef forward_iterator_tag iterator_category;
    typedef P* value_type;

    node* mNode;
    const header* mHeader;
    // bool x8_;

    // TODO why is this bool here?
    const_iterator(node* node, const header* header, bool b)
    : mNode(node), mHeader(header) /*, x8_(b)*/ {}

    const P* operator->() const { return mNode->get_value(); }
    const P* operator*() const { return mNode->get_value(); }
    bool operator==(const const_iterator& other) const {
      return mNode == other.mNode && mHeader == other.mHeader;
    }
    bool operator!=(const const_iterator& other) const {
      // return !(*this == other);
      return mNode != other.mNode || mHeader != other.mHeader;
    }

    const_iterator& operator++() {
      mNode = static_cast< node* >(rbtree_traverse_forward(static_cast< const void* >(mHeader),
                                                           static_cast< void* >(mNode)));
      return *this;
    }

    const_iterator operator++(int) {
      const_iterator result = *this;
      mNode = static_cast< node* >(rbtree_traverse_forward(static_cast< const void* >(mHeader),
                                                           static_cast< void* >(mNode)));
      return result;
    }
  };
  struct iterator : public const_iterator {
    iterator(node* node, const header* header, bool b) : const_iterator(node, header, b) {}

    P* operator->() { return const_iterator::mNode->get_value(); }
    P* operator*() { return const_iterator::mNode->get_value(); }
    node* get_node() { return const_iterator::mNode; }
  };

  red_black_tree(const S& selector = S(), const Cmp& cmp = Cmp(), const Alloc& alloc = Alloc())
  : mSelector(selector), mCmp(cmp), mAllocator(alloc), mCount(0) {}
  red_black_tree(CInputStream& in, const S& selector = S(), const Cmp& cmp = Cmp(),
                 const Alloc& alloc = Alloc());
  ~red_black_tree() { clear(); }

  pair< iterator, bool > insert_into(node* n, const P& item);
  pair< iterator, bool > insert(const P& item) { return insert_into(mHeader.get_root(), item); }

  const_iterator begin() const {
    // TODO
    return const_iterator(mHeader.get_leftmost(), &mHeader, false);
  }
  const_iterator end() const {
    // TODO
    return const_iterator(nullptr, &mHeader, false);
  }

  iterator begin() {
    // TODO
    return iterator(mHeader.get_leftmost(), &mHeader, false);
  }
  iterator end() {
    // TODO
    return iterator(nullptr, &mHeader, false);
  }

  const_iterator find(const T& key) const {
    return const_iterator(find_node(key), &mHeader, false);
  }

  iterator find(const T& key) { return iterator(find_node(key), &mHeader, false); }

  node* find_node(const T& key) const {
    node* n = mHeader.get_root();
    node* needle = nullptr;
    while (n != nullptr) {
      if (!mCmp(mSelector(*n->get_value()), key)) {
        needle = n;
        n = n->get_left();
      } else {
        n = n->get_right();
      }
    }
    bool noResult = false;
    if (needle == nullptr || mCmp(key, mSelector(*needle->get_value()))) {
      noResult = true;
    }
    return noResult ? nullptr : needle;
  }

  iterator erase(iterator it) {
    node* node = it.get_node();
    ++it;
    free_node(rebalance_for_erase(node));
    mCount--;
    return it;
  }

  void clear() {
    node* root = mHeader.get_root();
    if (root != nullptr) {
      free_node_and_sub_nodes(root);
    }
    mHeader.set_root(nullptr);
    mHeader.set_leftmost(nullptr);
    mHeader.set_rightmost(nullptr);
    mCount = 0;
  }

  int size() const { return mCount; }

private:
  S mSelector;
  Cmp mCmp;
  Alloc mAllocator;
  int mCount;
  header mHeader;

  node* create_node(node* left, node* right, node* parent, node_color color, const P& value) {
    node* n;
    Alloc::allocate(n, 1);
    new (n) node(left, right, parent, color, value);
    return n;
  }

  void free_node_and_sub_nodes(node* n);

  void free_node(node* n) {
    n->~node();
    Alloc::deallocate(n);
  }

  void rebalance(node* n) { rbtree_rebalance(&mHeader, n); }

  node* rebalance_for_erase(node* n) {
    return static_cast< node* >(rbtree_rebalance_for_erase(&mHeader, n));
  }
};

template < typename T, typename P, int U, typename S, typename Cmp, typename Alloc >
void red_black_tree< T, P, U, S, Cmp, Alloc >::free_node_and_sub_nodes(node* n) {
  if (node* left = n->get_left()) {
    free_node_and_sub_nodes(left);
  }
  if (node* right = n->get_right()) {
    free_node_and_sub_nodes(right);
  }
  free_node(n);
}

template < typename T, typename P, int U, typename S, typename Cmp, typename Alloc >
pair< typename red_black_tree< T, P, U, S, Cmp, Alloc >::iterator, bool >
red_black_tree< T, P, U, S, Cmp, Alloc >::insert_into(node* start, const P& item) {
  if (start == nullptr) {
    mHeader.set_root(create_node(nullptr, nullptr, nullptr, kNC_Black, item));
    mCount += 1;
    mHeader.set_leftmost(mHeader.get_root());
    mHeader.set_rightmost(mHeader.get_root());
    return pair< iterator, bool >(iterator(mHeader.get_root(), &mHeader, false), true);

  } else {
    node* n = start;
    node* newNode = nullptr;
    while (newNode == nullptr) {
      bool firstComp = mCmp(mSelector(item), mSelector(*n->get_value()));
      if (!U && !firstComp && !mCmp(mSelector(*n->get_value()), mSelector(item))) {
        return pair< iterator, bool >(iterator(n, &mHeader, false), false);
      }
      if (firstComp) {
        if (n->get_left() == nullptr) {
          newNode = create_node(nullptr, nullptr, n, kNC_Red, item);
          n->set_left(newNode);
          if (n == mHeader.get_leftmost()) {
            mHeader.set_leftmost(newNode);
          }
        } else {
          n = n->get_left();
        }
      } else {
        if (n->get_right() == nullptr) {
          newNode = create_node(nullptr, nullptr, n, kNC_Red, item);
          n->set_right(newNode);
          if (n == mHeader.get_rightmost()) {
            mHeader.set_rightmost(newNode);
          }
        } else {
          n = n->get_right();
        }
      }
    }
    mCount += 1;
    rebalance(newNode);
    return pair< iterator, bool >(iterator(newNode, &mHeader, false), true);
  }
}

}; // namespace rstl

#endif // _RSTL_RED_BLACK_TREE
