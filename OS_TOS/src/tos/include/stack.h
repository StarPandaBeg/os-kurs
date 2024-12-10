#pragma once

#include <functional>

namespace tos {
	template <class T>
	class Stack {
	public:
		void push(const T& p);
		T& front();
		void pop();
		int size();

		T* find(std::function<bool(const T&)> predicate);
	private:
		typedef struct stack_entry {
			T p;
			stack_entry* prev = nullptr;
			stack_entry* next = nullptr;
		} stack_entry;

		stack_entry* m_current = nullptr;
		int m_size = 0;
	};
}

template<class T>
inline void tos::Stack<T>::push(const T& p)
{
	auto* entry = new stack_entry{ p, m_current, nullptr };
	if (m_current == nullptr) {
		m_current = entry;
		m_size = 1;
		return;
	}
	m_current->next = entry;
	m_current = entry;
	m_size++;
}

template<class T>
inline T& tos::Stack<T>::front()
{
	if (m_size == 0) throw std::exception("invalid state");
	return m_current->p;
}

template<class T>
inline void tos::Stack<T>::pop()
{
	if (m_size == 0) throw std::exception("invalid state");
	auto parent = m_current->prev;

	if (parent != nullptr) {
		parent->next = nullptr;
	}
	delete m_current;
	m_current = parent;
	m_size--;
}

template<class T>
inline int tos::Stack<T>::size()
{
	return m_size;
}

template<class T>
inline T* tos::Stack<T>::find(std::function<bool(const T&)> predicate)
{
	stack_entry* current = m_current;
	while (current) {
		if (predicate(current->p)) {
			return &current->p;
		}
		current = current->prev;
	}
	return nullptr;
}
