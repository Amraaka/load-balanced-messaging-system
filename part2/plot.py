import csv
import matplotlib.pyplot as plt


def load(path):
    pos, temp = [], []
    with open(path) as f:
        reader = csv.reader(f)
        next(reader)  # skip header
        for row in reader:
            pos.append(int(row[0]))
            temp.append(float(row[1]))
    return pos, temp


p1, t1 = load('output_100.csv')
p2, t2 = load('output_1000.csv')

plt.figure(figsize=(10, 5))
plt.plot(p1, t1, label='100 steps')
plt.plot(p2, t2, label='1000 steps')
plt.xlabel('Position along rod')
plt.ylabel('Temperature (°C)')
plt.title('Heat Distribution Along the Rod')
plt.legend()
plt.grid(True)
plt.savefig('heat_distribution.png', dpi=120)
plt.show()
