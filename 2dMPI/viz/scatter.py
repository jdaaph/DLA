import numpy as np
import matplotlib.pyplot as plt


parser = lambda text: tuple(map(int, (text.split(","))))

with open("d.txt") as infile:
# with open("step1e4_np9.txt") as infile:
    raw = infile.read()
    final = raw.replace(" ", "")
    final = final.replace("(", "")
    final = final.replace(")", "\n")
    lst = final.split()

    print (lst)
    
    data = map(parser, lst)


plt.scatter(*zip(*data))
plt.show()

print (len(data))



