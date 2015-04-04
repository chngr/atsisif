import os.path
import sys
from collections import Counter

def get_violation_histogram(filename):
  f = open(filename)
  violations = []
  for line in f:
    if "violation" in line:
      amount = float(line.strip().split(' ')[-1])
      violations.append(amount)
  return sorted(violations)

if __name__ == '__main__':
  filename = sys.argv[1]
  violations = get_violation_histogram(filename)
  out = open("{0}_violations.csv".format(filename.split('.')[0]), 'w')
  out.write("violation\n")
  for v in violations:
    out.write("{0}\n".format(v))
  out.close()

