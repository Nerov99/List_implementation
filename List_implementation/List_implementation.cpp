// List_implementation.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include <iostream>
#include <memory>
#include <functional>

template<typename T>
class List {
private:
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = size_t;
    using Split_predicate = std::function<bool(const_reference value)>;
    struct Node {
        using node_ptr = std::unique_ptr<Node>;
        value_type data;
        node_ptr next;
        Node() : data{}, next{ nullptr } {}
        Node(const value_type& data) : data{ data }, next{ nullptr } {}
        Node(value_type&& data) : data{ std::forward<value_type>(data) }, next{ nullptr } {}
    };
    using raw_pointer = Node*;
    using node_ptr = std::unique_ptr<Node>;

    template<typename value_type>
    class forward_iterator: public std::iterator<std::forward_iterator_tag, value_type> {
    private:
        raw_pointer ptr;
    public: 
        forward_iterator() : ptr{ nullptr } {}
        forward_iterator(const node_ptr& p) : ptr{ p.get() } {}
        forward_iterator& operator++() {
            ptr = ptr->next.get();
            return *this;
        }
        forward_iterator operator++(int) {
            forward_iterator tmp = *this;
            ++*this;
            return tmp;
        }
        reference operator*() const {
            return ptr->data;
        }
        raw_pointer operator->() const {
            return ptr;
        }
        friend bool operator==(const forward_iterator& a, const forward_iterator& b) {
            return a.ptr == b.ptr;
        }
        friend bool operator!=(const forward_iterator& a, const forward_iterator& b) {
            return !(a == b);
        }
    };
    using iterator = typename forward_iterator<value_type>;
    using const_iterator = typename forward_iterator<const value_type>;

    node_ptr head;
    size_type m_size;
public:
    List() : head{ std::make_unique<Node>() }, m_size{ 0 } {}
    List(raw_pointer node, size_type size) : List() {
        head->next = std::move(node->next);
        m_size = size;
    }
    List(const List& obj): List() {
        if (obj.head->next == nullptr)
            return;

        raw_pointer temp = obj.head->next.get();
        while (temp != nullptr) {
            this->push_front(temp->data);
            temp = temp->next.get();
        }
    }
    List(List&& obj): List() {
        if(m_size != 0)
            this->clear();

        head = std::move(obj.head);
        m_size = obj.size();

        obj.head = std::make_unique<Node>();
        obj.m_size = 0;
    }
    List(std::initializer_list<T> il): List() {
        for (const auto& x : il) {
            this->push_front(x);
        }
    }
    
    List& operator=(const List& obj) {
        if (this == &obj)
            return *this;

        if (obj.size() != this->size())
            this->resize(obj.size());

        raw_pointer temp = head.get();
        raw_pointer obj_head = obj.head.get();
        while (obj_head != nullptr) {
            temp->data = obj_head->data;
            temp = temp->next.get();
            obj_head = obj_head->next.get();
        }

        return *this;
    }
    List& operator=(List&& obj) {
        if (this == &obj)
            return *this;

        if (m_size != 0)
            this->clear();

        head = std::move(obj.head);
        m_size = obj.size();
        obj.head = std::make_unique<Node>();
        obj.m_size = 0;

        return *this;

    }
    List& operator=(std::initializer_list<value_type> il) {

        if (il.size() != this->size())
            this->resize(il.size());

        iterator it = this->begin();
        for (const auto& x : il) {
            *it = x;
            ++it;
        }

        return *this;
    }
    
    ~List() = default;
    
    void push_front(const_reference data) {
        emplace_front(data);
    }
    void push_front(value_type&& data) {
        emplace_front(std::forward<value_type&&>(data));
    }
    void pop_front() {
        if (head->next == nullptr)
            return;

        node_ptr temp = std::move(head->next);
        head->next = std::move(temp->next);
        temp.reset();
        --m_size;
    }
    void emplace_front(const_reference data) {

        node_ptr temp = std::make_unique<Node>(data);

        if (head->next == nullptr) {
            head->next = std::move(temp);
        }
        else {
            temp->next = std::move(head->next);
            head->next = std::move(temp);
        }
        ++m_size;
    }
    void emplace_front(value_type&& data) {

        node_ptr temp = std::make_unique<Node>(std::forward<value_type&&>(data));

        if (head->next == nullptr) {
            head->next = std::move(temp);
        }
        else {
            temp->next = std::move(head->next);
            head->next = std::move(temp);
        }
        ++m_size;
    }
    void emplace_after(iterator position, const_reference data) {
        node_ptr temp_obj = std::make_unique<Node>(data);
        iterator iter = this->before_begin();
        while (iter != position) {
            ++iter;
        }
        temp_obj->next = std::move(iter->next);
        iter->next = std::move(temp_obj);
        ++m_size;
    }
    
    reference front() {
        return head->next->data;
    }
    const_reference front() const{
        return head->next->data;
    }

    List<T> split_when(Split_predicate condition) {
        if (head->next == nullptr)
            return {};

        raw_pointer temp = head->next.get();
        raw_pointer prev_temp = head->next.get();
        size_type new_size = m_size;
        m_size = 0;

        while(!condition(temp->data)) {
            if (temp == nullptr)
                break;
            prev_temp = temp;
            temp = temp->next.get();
            --new_size;
            ++m_size;
        }

        return { prev_temp, new_size};
    }

    size_type size() const {
        return m_size;
    }
    size_type max_size() const {
        return ~size_type(0);
    }
    void resize(size_type new_size, const value_type& data = {}) {
        if (new_size == 0) {
            this->clear();
        }
        else if (new_size > m_size) {
            iterator it = this->before_begin();

            while (it->next != nullptr) {
                ++it;
            }

            while (m_size != new_size) {
                it->next = std::make_unique<Node>();
                ++it;
                ++m_size;
            }
            it->next = nullptr;
        }
        else if (new_size < m_size) {
            raw_pointer ptr = head.get();
            m_size = 0;
            while (m_size != new_size) {
                ptr = ptr->next.get();
                ++m_size;
            }

            node_ptr node_to_delete = std::move(ptr->next);
            while (node_to_delete != nullptr) {
                node_ptr temp = std::move(node_to_delete->next);
                node_to_delete.reset();
                node_to_delete = std::move(temp);
            }
        }
    }
    void clear() {
        if (head->next == nullptr)
            return;

        while (head->next != nullptr) {
            node_ptr temp = std::move(head->next->next);
            head->next.reset();
            head->next = std::move(temp);
            --m_size;
        }
    }
    bool empty() const {
        if (head->next == nullptr)
            return true;
        return false;
    }

    iterator before_begin() {
        return iterator(head);
    }
    const_iterator before_begin() const{
        return const_iterator(head);
    }
    const_iterator cbefore_begin() const {
        return const_iterator(head);
    }
    iterator begin() {
        return iterator(head->next);
    }
    const_iterator begin() const {
        return const_iterator(head->next);
    }
    const_iterator cbegin() const {
        return const_iterator(head->next);
    }
    iterator end() {
        return iterator();
    }
    const_iterator end() const {
        return const_iterator();
    }
    const_iterator cend() const {
        return const_iterator();
    }
};

template<typename T>
void printList(const List<T>& list) {
    for (auto ptr = list.begin(); ptr != list.end(); ++ptr) {
        std::cout << ptr->data << " ";
    }
}

int main() {
    std::cout << "Construction part" << std::endl;
    List<int> list1{ 9,8,7,6,5,4,3,2,1,0 };
    List<int> list2{ list1 };
    List<int> list3{ std::move(list1) };
    List<int> list4 = list3.split_when([](const int& value) { return value == 3;});
    
    std::cout << "List1: ";
    printList(list1);
    std::cout << std::endl << "List2: ";
    printList(list2);
    std::cout << std::endl << "List3: ";
    printList(list3);
    std::cout << std::endl << "List4: ";
    printList(list4);
    
    std::cout << std::endl << std::endl << "Sizes:";

    std::cout << std::endl << "List1 size: " << list1.size();
    std::cout << std::endl << "List2 size: " << list2.size();
    std::cout << std::endl << "List3 size: " << list3.size();
    std::cout << std::endl << "List4 size: " << list4.size();
    
    std::cout << std::endl << std::endl << "Data insertions part";

    list1.push_front(10);
    list1.push_front(8);
    list1.emplace_front(6);
    list1.emplace_front(4);
    list1.emplace_after(list1.before_begin(), 2);
    list1.emplace_after(list1.before_begin(), 0);
    
    std::cout << std::endl << "List1: ";
    printList(list1);

    list1.pop_front();

    std::cout << std::endl << "List1: ";
    printList(list1);

    std::cout << std::endl << std::endl << "Assigment operators part";
    list1 = list3;
    list4 = std::move(list2);

    std::cout << std::endl << "List1: ";
    printList(list1);
    std::cout << std::endl << "List2: ";
    printList(list2);
    std::cout << std::endl << "List3: ";
    printList(list3);
    std::cout << std::endl << "List4: ";
    printList(list4);
    std::cout << std::endl;

    return 0;
}