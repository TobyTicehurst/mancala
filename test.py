import numpy as np
import itertools

# place 5 stones in 3 slots (2 bars)
def fast_comb(n, k):
    # a = matrix of 1s, k(2) by n-k+1(4)
    a = np.ones((k, n-k+1), dtype=int)
    print(a)
    # list from 0 to n-k (6) inclusive 
    a[0] = np.arange(n-k+1)
    print(a)
    
    
    # for j = 1 to j = k(4)
    for j in range(1, k):
        # reps = (6+j) - a[j-1]
        # reps = [7, ]
        reps = (n-k+j) - a[j-1]
        print("reps")
        print(reps)
        # repeat a a bunch of times
        a = np.repeat(a, reps, axis=1)
        print(a)

        # accumulative sum of reps
        ind = np.add.accumulate(reps)
        print("ind")
        print(ind[:-1])
        # ind[:-1] is all but the last element
        # reps[1:] is all but the first element
        # at each ind, minus off how many reps there were of the previous number
        a[j, ind[:-1]] = 1-reps[1:]
        print(a)
        a[j, 0] = j
        print(a)
        a[j] = np.add.accumulate(a[j])
        print(a)
    return a


# L = [1,2,3,4,5]
# N = 3

# output = np.array([a for a in itertools.combinations(L,N)]).T
# print(output)
# for a in itertools.combinations(L,N):
#     print(a)

N = 6
K = 3

fb = fast_comb(N, K)
print("fb")
print(fb)

# Add extra rows
sb = np.empty((K + 2, fb.shape[1]), int)
sb[0], sb[1 : K + 1], sb[K + 1] = -1, fb, N
print("Add -1 and N rows")
print(sb)

# Transpose
print("Transpose")
print(sb.T)

# # Diff
# print("Diff between elements")
# print(np.diff(sb.T))

# # Minus 1
# result = np.diff(sb.T) - 1
# print("Minus 1 from every element")
# print(result)