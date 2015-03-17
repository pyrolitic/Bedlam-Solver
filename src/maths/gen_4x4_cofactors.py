def determinant3(m):
	fmt = '(a * e * i + b * f * g + c * d * h) - (c * e * g + b * d * i + a * f * h)'
	s = ''
	for c in fmt:
		o = ord(c)
		if o >= ord('a') and o <= ord('z'):
			o -= ord('a')
			r = m[o / 3][o % 3]

			#if len(r) == 3:
			s += r

			#else:
			#	s += '(' + r + ')'

		else:
			s += c

	return s

def cofactors(m):
	r = [[] for i in range(4)]

	#top row
	for i in range(4):
		for j in range(4):
			minor = map(lambda x : x[:j] + x[j+1:], m) #skip every jth element in each row
			minor = minor[:i] + minor[i+1:] #skip the ith row
			det = determinant3(minor)
			if ((i & 1) ^ (j & 1)) == 1:
				det = '-(' + det + ')'
			else:
				det = ' (' + det + ')'

			r[i].append(det)

	return r

#TRANSPOSED MATRIX
m = [['src._%d%d' % (j+1, i+1) for j in range(4)] for i in range(4)] 
co = cofactors(m)

for i in range(4):
	for j in range(4):
		print 'dest._%d%d' % (i+1, j+1) + ' = ' + co[i][j]