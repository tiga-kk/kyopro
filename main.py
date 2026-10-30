from __future__ import annotations

import os
import random
import sys

try:
    import atcoder
except ImportError:
    pass

sys.setrecursionlimit(1_000_000)
input = lambda: sys.stdin.readline().rstrip()

INPUT_FILE = "input.txt"
if os.path.exists(INPUT_FILE):
    sys.stdin = open(INPUT_FILE, "r")

INF = 10**18
MOD = 998244353

dx = (0, 1, 0, -1, -1, -1, 1, 1)
dy = (1, 0, -1, 0, -1, 1, -1, 1)


def chmax(a, b):
    if a < b:
        return b, True
    return a, False


def chmin(a, b):
    if a > b:
        return b, True
    return a, False


def yn(t: bool) -> None:
    print("Yes" if t else "No")


random.seed(os.getpid())


def randint(l: int, r: int) -> int:
    return random.randint(l, r)


def solve() -> int:
    n = int(input())
    a = []
    ans = 0
    for i in range(n):
        ans += i-1
        a.append(ans)
        print(ans)
    return 0


def main() -> None:
    t = 1
    for _ in range(t):
        solve()


if __name__ == "__main__":
    main()
