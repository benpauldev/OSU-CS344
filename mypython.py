import string
import random
import os

if os.path.isfile('./StrongBad'):
    os.remove('./StrongBad')
if os.path.isfile('./DaCheat'):
    os.remove('./DaCheat')
if os.path.isfile('./Mr.Z'):
    os.remove('./Mr.Z')

file1 = open("StrongBad","a")
for i in range(10):
    if i == 10:
        file1.write("\n")
    file1.write(random.choice(string.ascii_lowercase))
file1.close()

file2 = open("DaCheat","a")
for i in range(10):
    if i == 10:
        file2.write("\n")
    file2.write(random.choice(string.ascii_lowercase))
file2.close()

file3 = open("Mr.Z","a")
for i in range(10):
    if i == 10:
        file3.write("\n")
    file3.write(random.choice(string.ascii_lowercase))
file3.close()

file1 = open("StrongBad","r")
print file1.readline()
file1.close()

file2 = open("DaCheat","r")
print file2.readline()
file2.close()

file3 = open("Mr.Z","r")
print file3.readline()
file3.close()

rand1 = (random.randint(1,42))
print rand1
rand2 = (random.randint(1,42))
print rand2
print(rand1*rand2)