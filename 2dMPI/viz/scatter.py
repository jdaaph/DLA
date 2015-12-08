import numpy as np
import matplotlib.pyplot as plt


parser = lambda text: tuple(map(int, (text.split(","))))

with open("p.txt") as infile:
# with open("step1e4_np9.txt") as infile:
    raw = infile.read()
    final = raw.replace(" ", "")
    final = final.replace("(", "")
    final = final.replace(")", "\n")
    lst = final.split()
    
    data = map(parser, lst)


plt.scatter(*zip(*data))
plt.show()

print (len(data))



