import sys

n = int(sys.stdin.readline())
for i in range(n):
    answer = 0
    for value in range(i + 1):
        answer += value
    print(answer)
