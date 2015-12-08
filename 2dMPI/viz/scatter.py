import numpy as np
import matplotlib.pyplot as plt


parser = lambda text: tuple(map(int, (text.split(","))))


with open("raw.txt") as infile:
    raw = infile.read()
    final = raw.replace(" ", "")
    final = final.replace("(", "")
    final = final.replace(")", "\n")
    lst = final.split()
    
    data = map(parser, lst)


plt.scatter(*zip(*data))
plt.show()




