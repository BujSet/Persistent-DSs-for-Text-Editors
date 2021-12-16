import string
import random

with open("input_eval.txt", "w") as f:
    for i in range(1,5000):
        letters = string.ascii_lowercase
        f.write(''.join(random.choice(letters) for i in range(6)))
        f.write(" ")
