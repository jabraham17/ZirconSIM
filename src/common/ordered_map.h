
#ifndef ZIRCON_COMMON_ORDERED_MAP_H_
#define ZIRCON_COMMON_ORDERED_MAP_H_

#include <iterator>
#include <unordered_map>
#include <vector>

namespace common {

// emulates a map that maintains ionsertaion order
template <typename Key, typename T> class ordered_map {

    // member types
  private:
    using value_type = std::pair<const Key, T>;
    using size_type = std::size_t;

    std::unordered_map<Key, T> map_;
    std::vector<Key> insertion_order_;

    // iterators
  public:
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = value_type;
        using pointer = value_type*;
        using reference = value_type&;

        Iterator(
            std::unordered_map<Key, T>* map,
            typename std::vector<Key>::iterator it)
            : map_(map), it_(it) {}
        Iterator(
            std::unordered_map<Key, T>& map,
            typename std::vector<Key>::iterator it)
            : Iterator(&map, it) {}

        reference operator*() const { return *(map_->find(*it_)); }
        pointer operator->() { return map_->find(*it_); }
        Iterator& operator++() {
            it_++;
            return *this;
        }
        // Postfix increment
        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        friend bool operator==(const Iterator& a, const Iterator& b) {
            return a.it_ == b.it_;
        };
        friend bool operator!=(const Iterator& a, const Iterator& b) {
            return a.it_ != b.it_;
        };

      private:
        std::unordered_map<Key, T>* map_;
        typename std::vector<Key>::iterator it_;
    };

    Iterator begin() { return Iterator(map_, insertion_order_.begin()); }
    Iterator end() { return Iterator(map_, insertion_order_.end()); }

    // capacity
    bool empty() { return map_.empty(); }
    size_type size() { return map_.size(); }

    // modifers
    void clear() {
        map_.clear();
        insertion_order_.clear();
    }
    std::pair<Iterator, bool> insert(const value_type& value) {
        auto [internal_it, succ] = map_.insert(value);
        if(succ) {
            insertion_order_.push_back(value.first);
            return {Iterator(map_, insertion_order_.end() - 1), true};
        } else {
            auto insert_it = std::find(
                insertion_order_.begin(),
                insertion_order_.end(),
                value.first);
            return {Iterator(map_, insert_it), false};
        }
    }
    template <class M>
    std::pair<Iterator, bool> insert_or_assign(const Key& k, M&& obj) {
        if(map_.count(k)) {
            map_[k] = std::forward<M>(obj);
            auto insert_it =
                std::find(insertion_order_.begin(), insertion_order_.end(), k);
            return {Iterator(map_, insert_it), false};
        } else {
            return insert(value_type(k, std::forward<M>(obj)));
        }
    }
    size_type erase(const Key& key) {
        auto ret = map_.erase(key);
        auto insert_it =
            std::find(insertion_order_.begin(), insertion_order_.end(), key);
        insertion_order_.erase(insert_it);
        return ret;
    }

    // lookup
    // disabled for now as making these track insertion order is a pain in the ass
    // T& at(const Key& key) { return map_.at(key); }
    // const T& at(const Key& key) const { return map_.at(key); }
    // T& operator[](const Key& key) { return map_[key]; }
    size_type count(const Key& key) const { return map_.count(key); }
    Iterator find(const Key& key) {
        if(map_.count(key)) {
            auto insert_it = std::find(
                insertion_order_.begin(),
                insertion_order_.end(),
                key);
            return Iterator(map_, insert_it);
        } else return end();
    }
};

} // namespace common

#endif
