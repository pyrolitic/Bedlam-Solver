
def mul(a, b):
	m = [[] for i in range(4)]
	for i in range(4):
		for j in range(4):
			s = ''

			for q in range(4):
				l = a[i][q]
				r = b[q][j]

				if l != '0' and r != '0':
					x = ''
					if l == '1' and r != '1':
						x = '(' + r + ')'
					elif r == '1' and l != '1':
						x = '(' + l + ')'
					elif l == '1' and r == '1':
						x = '1'
					else:
						x = '(' + l + ') * (' + r + ')'

					if x != '':
						if len(s) > 0:
							s += ' + ' + x
						else:
							s += x
			if s == '':
				s = '0'

			m[i].append(s)
	
	return m

def parse(text, name):
	m = map(lambda x : x.split(), text.split('\n'))
	for i in range(4):
		m[i] = map(lambda x : x + name if x.find('c') >= 0 or x.find('s') >= 0 else x, m[i])

	return m

R = parse('''c -s  0  0
s  c  0  0
0  0  1  0
0  0  0  1''', 'r')

P = parse('''1  0  0  0
0  c -s  0
0  s  c  0
0  0  0  1''', 'p')

Y = parse('''c  0  s  0
0  1  0  0
-s  0  c  0
0  0  0  1''', 'y')

M = mul(mul(Y, P), R)

for j in range(4):
	m = max(map(lambda x : len(x[j]), M))
	for i in range(4):
		M[i][j] = M[i][j].rjust(m)

for i in range(4):
	l = ''
	for j in range(4):
		l += 'm._' + str(i + 1) + str(j + 1) + ' = ' + M[i][j] + ';  '
	
	print l
