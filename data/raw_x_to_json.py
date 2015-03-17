import json
import sys

if __name__ == "__main__":
  raw_filename = sys.argv[1]
  f = open("{}.x".format(raw_filename))
  n = int(f.readline().split(' ')[0])
  graph = {'nodes': n, 'edges': []}
  for line in f:
    a, b, w = line.strip().split(' ')
    graph['edges'].append({
      "source": int(a),
      "target": int(b),
      "weight": float(w),
      "value":  float(w)
      })
  f.close()

  f = open("{}.json".format(raw_filename), 'w')
  f.write(json.dumps(graph))
  f.close()
