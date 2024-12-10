#pragma once

#include <cstdint>

namespace fs {
	namespace internal {
		template <class T>
		bool binary_is_set(T number, uint8_t index);

		template <class T>
		void binary_toggle(T& number, uint8_t index, bool value);

		template <class T>
		void binary_set(T& number, uint8_t index);

		template <class T>
		void binary_clear(T& number, uint8_t index);


		template<class T>
		bool binary_is_set(T number, uint8_t index)
		{
			return (number & (1 << index)) != 0;
		}

		template<class T>
		void binary_toggle(T& number, uint8_t index, bool value)
		{
			if (value) {
				binary_set(number, index);
			}
			else {
				binary_clear(number, index);
			}
		}

		template<class T>
		void binary_set(T& number, uint8_t index)
		{
			number |= (1 << index);
		}

		template<class T>
		void binary_clear(T& number, uint8_t index)
		{
			number &= ~(1 << index);
		}
	}
}