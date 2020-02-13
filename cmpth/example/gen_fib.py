#!/usr/bin/env python3

fibonacci_numbers = [0, 1]
for i in range(2,50):
    fibonacci_numbers.append(fibonacci_numbers[i-1]+fibonacci_numbers[i-2])
print(fibonacci_numbers)

