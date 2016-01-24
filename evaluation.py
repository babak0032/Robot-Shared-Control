def reachedGoal(px, py, x1, y1, x2, y2):
	"""
	Check if px and py falls into the rectangular region with corners (x1, y1) (x2, y2)
	"""

	if px > x1 and px < x2 and py > y1 and py < y2:
		return True
	return False


def evalMethod_1(NR, x1, y1, x2, y2):
	"""
	For all robots, check how many reached goal in the last iteration
	"""

	count = 0

	for i in xrange(NR):
		f = open('pf'+str(i), 'r')
		data = f.readlines()
		end_position =  data[-1].strip().split()
		#print end_position
		px = float(end_position[1])
		py = float(end_position[2])
		#print px, py, "px py"
		reachGoal = reachedGoal(px, py, x1, y1, x2, y2)
		#print px, ":px", py, ":py", res
		if reachGoal:
			count += 1

	return count

if __name__ == '__main__':
	NR = 100
	x1 = 0.5
	y1 = 0.5
	x2 = 1.0
	y2 = 1.0
	print evalMethod_1(NR, x1, y1, x2, y2)