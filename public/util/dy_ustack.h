#pragma once

#include <assert.h>

// Unrolled Linked Stack
//
// - Node size grows as elements are added
// FIXME: Cannot handle deconstructors or constructors!

struct dy_ustack_node
{
	// Returns the element at the index within the node
	template<typename T>
	T* at(unsigned int index);

	dy_ustack_node* prev;
	dy_ustack_node* next;
	unsigned short capacity;
	unsigned short used;

	// type[capacity] elements;
};

template<typename T>
struct dy_ustack_iterator;


// TODO: We really really need copy constructors and better copy support for push!!!
template<typename T>
struct dy_ustack
{
	dy_ustack();
	~dy_ustack();

	// Returns a newly allocated continuous array of all elements in the ustack
	T* packed();

	// Empties the stack
	void clear();

	// Returns the element once coppied in
	// Pushes and pops to the back
	T* push(T&& element) { return push(&element); }
	T* push(const T& element) { return push(&element); }
	//T* push(T& element) { return push(&element); }
	void pop();


	// Returns the element at the index within the stack
	T* seek(unsigned int index);


	// Returns first and last elements of the stack
	T* first();
	T* last();

	// For range based for loops
	dy_ustack_iterator<T> begin();
	dy_ustack_iterator<T> end();

	// Our first node is part of the actual struct
	// This lets us keep a reference to an iterator, even with 0 elements
	dy_ustack_node  front;
	dy_ustack_node* back;

	unsigned int count;

private:
	T* push(const T*  element);

	static const int START_SIZE = 4;
};


// For range based for loops
template<typename T>
struct dy_ustack_iterator
{
	dy_ustack_node* node;
	unsigned short index;

	dy_ustack_iterator<T>& operator++();
	bool operator!=(const dy_ustack_iterator<T>& rhs);
	bool operator==(const dy_ustack_iterator<T>& rhs);

	dy_ustack_iterator<T> operator+(const unsigned int i);
	T* operator[](const unsigned int idx);
	T* operator*();

};

template<typename T>
struct ustack_range : public dy_ustack_iterator<T>
{
	ustack_range() { }
	ustack_range(const dy_ustack_iterator<T>& it, unsigned int count_) { *(dy_ustack_iterator<T>*)this = it; count = count_; }
	unsigned int count;
};




////////////////////
// IMPLEMENTATION //
////////////////////
// 
// TODO: Right now this frees nodes on pop when their "used" count is 0
//       It might be good to benchmark this at some point and check if it's worth it to preserve those nodes instead
//

#include <stdlib.h>
#include <string.h>



// Memory growth pattern
inline constexpr unsigned int ustack_nextsize_(unsigned int sz)
{
	sz *= 2;
	if (sz > 64)
		return 64;
	return sz;
}

template<typename T>
dy_ustack<T>::dy_ustack()
{
	front = {0,0,0,0};
	back = &front;
	count = 0;
}

template<typename T>
dy_ustack<T>::~dy_ustack()
{
	clear();
}

template<typename T>
T* dy_ustack<T>::packed()
{
	T* m = reinterpret_cast<T*>(malloc(sizeof(T) * count));
	T* d = m;
	T* end = m + count;

	for (dy_ustack_node* node = front.next; node; node = node->next)
	{
		unsigned int nu = node->used;
		if (d + nu > end) break;

		memcpy(d, (char*)node + sizeof(dy_ustack_node), nu * sizeof(T));

		d += nu;
	}

	return m;
}


template<typename T>
void dy_ustack<T>::clear()
{
	dy_ustack_node* next = 0;
	for (dy_ustack_node* n = front.next; n; n = next)
	{
		next = n->next;
		free(n);
	}
	front.next = 0;
	back = &front;
}

template<typename T>
T* dy_ustack<T>::push(const T* element)
{
	// Find some space
	dy_ustack_node* node = 0;
	dy_ustack_node* prev = back;

	if (prev && prev->used < prev->capacity)
	{
		// Our prev node is fine to use
		node = prev;
	}
	else
	{
		// Didn't find space! Make a new node

		// Grow from our prev size
		unsigned int nodesize = START_SIZE;
		if (prev && prev != &front)
			nodesize = ustack_nextsize_(prev->capacity);

		// Allocate with size for the struct and the data
		node = reinterpret_cast<dy_ustack_node*>(malloc(sizeof(dy_ustack_node) + nodesize * sizeof(T)));
		node->next = 0;
		node->capacity = nodesize;
		node->used = 0;
		
		// Link it up
		if (prev)
			prev->next = node;
//		if (!front->)
//			front = node;
		node->prev = prev;
		back = node;
	}

	// Copy it in
	T* dest = node->at<T>(node->used);
	memcpy(dest, element, sizeof(T));
	node->used++;
	count++;

	return dest;
}


template<typename T>
void dy_ustack<T>::pop()
{
	dy_ustack_node* node = back;

	if (!node)
		return;

	if (node->used == 1)
	{
		// Pop the entire node
		
		// Unlink our node
		dy_ustack_node* prev = node->prev;
		if (prev)
		{
			prev->next = 0;
			back = prev;
		}
		else
		{
			// No more nodes
			front = { 0, 0, 0, 0 };
			back = 0;
		}

		// Bye bye!
		free(node);
	}
	else
	{
		// Just shift down one
		node->used--;
	}

	count--;
}


template<typename T>
T* dy_ustack<T>::seek(unsigned int index)
{
	if (index >= count)
		return 0;

	dy_ustack_node* node = 0;
	for (node = front.next; node; node = node->next)
	{
		if (index < node->used)
			break;
		index -= node->used;
	}

	if (!node || index > node->used)
		return 0;
	return node->at<T>(index);
}


template<typename T>
T* dy_ustack_node::at(unsigned int index)
{
	assert(index < capacity);
	return reinterpret_cast<T*>(this + 1) + index;
}


template<typename T>
T* dy_ustack<T>::first()
{
	dy_ustack_node* node = front.next;
	if (node && node->used > 0)
		return node->at<T>(0);
	return 0;
}

template<typename T>
T* dy_ustack<T>::last()
{
	dy_ustack_node* node = back;
	if (node && node->used > 0)
		return node->at<T>(node->used - 1);
	return 0;
}


// Range based for loop
template<typename T>
dy_ustack_iterator<T> dy_ustack<T>::begin()
{
	return { &front, 0 };
}

template<typename T>
dy_ustack_iterator<T> dy_ustack<T>::end()
{
	return { back, back->used };
}



template<typename T>
dy_ustack_iterator<T>& dy_ustack_iterator<T>::operator++()
{
	if (!node)
		return *this;

	if (index + 1 >= node->used && node->next)
	{
		node = node->next;
		index = 0;
	}
	else
	{
		index++;
	}
	return *this;
}

template<typename T>
bool dy_ustack_iterator<T>::operator!=(const dy_ustack_iterator<T>& rhs)
{
	return index != rhs.index || node != rhs.node;
}

template<typename T>
bool dy_ustack_iterator<T>::operator==(const dy_ustack_iterator<T>& rhs)
{
	return index == rhs.index && node == rhs.node;
}

template<typename T>
dy_ustack_iterator<T> dy_ustack_iterator<T>::operator+(const unsigned int i)
{
	unsigned int idx = index + i;
	dy_ustack_node* n = node;
	while (n->next && idx >= n->used)
	{
		idx -= n->used;
		n = n->next;
	}
	return { n, (unsigned short)idx };
}

template<typename T>
T* dy_ustack_iterator<T>::operator[](const unsigned int idx)
{
	return *(this->operator+(idx));
}

template<typename T>
T* dy_ustack_iterator<T>::operator*()
{
	if (index >= node->used)
		*this = this->operator+(index - node->used);
	if(node)
		return node->at<T>(index);
	return 0;
}

