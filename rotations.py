
class cube:
	def __init__(self):
		self.data = list(range(8))
	
	def __getitem__(self, index):
		x, y, z = index
		return self.data[(x + y * 2) + z * 4]

	def __setitem__(self, index, val):
		x, y, z = index
		self.data[(x + y * 2) + z * 4] = val

	def __repr__(self):
		return ''.join(map(str, self.data))

def rot(ob, axis):
	out = cube()

	for x in range(2):
		for y in range(2):
			for z in range(2):
				if axis == 0:
					out[x, y, z] = ob[x, z, 1 - y]

				elif axis == 1:
					out[x, y, z] = ob[1 - z, y, x]

				elif axis == 2:
					out[x, y, z] = ob[1 - y, x, z]
	return out

#bruteforce
bf = set()
for a in range(3):
	for aa in range(1, 4):
		for b in range(3):
			for bb in range(1, 4):
				for c in range(3):
					for cc in range(1, 4):
						o = cube()
						for i in range(aa):
							o = rot(o, a)

						for i in range(bb):
							o = rot(o, b)

						for i in range(cc):
							o = rot(o, c)

						bf.add(str(o))

print "bruteforce:", len(bf)
print sorted(list(bf))

sm = set()
facets = [cube(), rot(cube(), 0), rot(cube(), 1), rot(rot(rot(cube(), 0), 0), 0), rot(rot(rot(cube(), 1), 1), 1), rot(rot(cube(), 0), 0)]
for i in range(len(facets)):
	sm.add(str(facets[i]))
	for r in range(4):
		facets[i] = rot(facets[i], 2)
		sm.add(str(facets[i]))

print "smart:", len(sm)
print sm

print bf.difference(sm)
