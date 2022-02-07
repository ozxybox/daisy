#pragma once

// Unrolled Linked Stack
//
// - Node size grows as elements are added
// TODO: Remove START_SIZE from dy_ustack, but provide for something similar
//       It's just annoying to have to specify for references, pointers, etc.
//

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

template<typename T, int START_SIZE = 4>
struct dy_ustack
{
	dy_ustack();
	~dy_ustack();

	// Returns a newly allocated continuous array of all elements in the ustack
	T* packed();

	// Returns the element once coppied in
	// Pushes and pops to the back
	T* push(T*  element);
	T* push(T&& element) { return push(&element); }
	T* push(const T& element) { return push(&element); }
	void pop();


	// Returns the element at the index within the stack
	T* seek(unsigned int index);


	// Returns first and last elements of the stack
	T* first();
	T* last();

	// For range based for loops
	dy_ustack_iterator<T> begin();
	dy_ustack_iterator<T> end();

	dy_ustack_node* front;
	dy_ustack_node* back;

	unsigned int count;
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
inline constexpr unsigned int dy_ustack_nextsize_(unsigned int sz)
{
	sz *= 2;
	if (sz > 64)
		return 64;
	return sz;
}

template<typename T, int START_SIZE>
dy_ustack<T, START_SIZE>::dy_ustack()
{
	front = 0;
	back = 0;
	count = 0;
}

template<typename T, int START_SIZE>
dy_ustack<T, START_SIZE>::~dy_ustack()
{
	dy_ustack_node* next = 0;
	for (dy_ustack_node* n = front; n; n = next)
	{
		next = n->next;
		free(n);
	}
}

template<typename T, int START_SIZE>
T* dy_ustack<T, START_SIZE>::packed()
{
	T* m = reinterpret_cast<T*>(malloc(sizeof(T) * count));
	T* d = m;
	T* end = m + count;

	for (dy_ustack_node* node = front; node; node = node->next)
	{
		unsigned int nu = node->used;
		if (d + nu > end) break;

		memcpy(d, (char*)node + sizeof(dy_ustack_node), nu * sizeof(T));

		d += nu;
	}

	return m;
}

template<typename T, int START_SIZE>
T* dy_ustack<T, START_SIZE>::push(T* element)
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
		if (prev)
			nodesize = dy_ustack_nextsize_(prev->capacity);

		// Allocate with size for the struct and the data
		node = reinterpret_cast<dy_ustack_node*>(malloc(sizeof(dy_ustack_node) + nodesize * sizeof(T)));
		node->next = 0;
		node->capacity = nodesize;
		node->used = 0;
		
		// Link it up
		if (prev)
			prev->next = node;
		if (!front)
			front = node;
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


template<typename T, int START_SIZE>
void dy_ustack<T, START_SIZE>::pop()
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
			front = 0;
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


template<typename T, int START_SIZE>
T* dy_ustack<T, START_SIZE>::seek(unsigned int index)
{
	if (index >= count)
		return 0;

	dy_ustack_node* node = 0;
	for (node = front; node; node = node->next)
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
	return reinterpret_cast<T*>(this + 1) + index;
}


template<typename T, int START_SIZE>
T* dy_ustack<T, START_SIZE>::first()
{
	dy_ustack_node* node = front;
	if (node && node->used > 0)
		return node->at<T>(0);
	return 0;
}

template<typename T, int START_SIZE>
T* dy_ustack<T, START_SIZE>::last()
{
	dy_ustack_node* node = back;
	if (node && node->used > 0)
		return node->at<T>(node->used - 1);
	return 0;
}


// Range based for loop
template<typename T, int START_SIZE>
dy_ustack_iterator<T> dy_ustack<T, START_SIZE>::begin()
{
	return { front, 0 };
}

template<typename T, int START_SIZE>
dy_ustack_iterator<T> dy_ustack<T, START_SIZE>::end()
{
	return { 0, 0 };
}



template<typename T>
dy_ustack_iterator<T>& dy_ustack_iterator<T>::operator++()
{
	if (!node)
		return *this;

	if (index + 1 >= node->used)
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
	while (n && idx >= n->used)
	{
		idx -= n->used;
		n = n->next;
	}
	if (!n)
		return { 0, 0 };
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
	if(node)
		return node->at<T>(index);
	return 0;
}

