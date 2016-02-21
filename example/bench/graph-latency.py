
import sys
import yaml
import numpy as np
from scipy import stats
import matplotlib.pyplot as plt
from collections import defaultdict

# http://www.sat.t.u-tokyo.ac.jp/~oku/20160217/memo.html
# http://people.duke.edu/~ccc14/pcfb/numpympl/matplotliblineplots.html

try:
    file_path = sys.argv[1]
except IndexError:
    file_path = 'out.yaml'

try:
    x_name = sys.argv[2]
except IndexError:
    x_name = 'number_of_processes'

data = yaml.load(open(file_path).read().decode('utf-8'))

results = defaultdict(list)

alpha = 0.95

for execution in data:
    samples = []
    for process_result in execution['process_results']:
        for thread_result in process_result['thread_results']:
            samples.append(thread_result['latency']['average'])
    
    x = execution[x_name]
    y = np.mean(samples)
    
    results[x].append(y)

x_list = []
y_list = []
error_list = []

for k, v in results.items():
    mean_val = np.mean(v)
    sem_val  = stats.sem(v)
    ci       = stats.t.interval(alpha, len(data)-1, loc=mean_val, scale=sem_val)
    
    x_list.append(k)
    y_list.append(mean_val)
    error_list.append(ci - mean_val)

fig = plt.figure()
ax = fig.add_subplot(111)
ax.errorbar(x_list, y_list,
    yerr = (-1 * np.array([ e[0] for e in error_list]), [ e[1] for e in error_list]),
    marker = 'o')
ax.set_xlim(0, 5)
plt.show()

print(results)

