import math


class vec2:
	def __init__(self, x = 0, y = 0):
		self.x = x
		self.y = y

	def length(self):
		return math.sqrt(self.x * self.x + self.y * self.y)

	def normalize(self):
		div = 1.0 / self.length()
		a = vec2()
		a.x = self.x * div
		a.y = self.y * div

		return a

	def dot(self, a):
		return self.x * a.x + self.y * a.y

	def rotate_deg(self, angle):
		angle = angle * math.pi / 180
		s = math.sin(angle)
		c = math.cos(angle)

		self.x = c * self.x - s * self.y
		self.y = s * self.x + c * self.y

	def round(self):
		self.x = round(self.x)
		self.y = round(self.y)

	def clamp(self, min_x, max_x, min_y, max_y):
		if self.x > max_x:
			self.x = max_x
		if self.x < min_x:
			self.x = min_x

		if self.y > max_y:
			self.y = max_y
		if self.y < min_y:
			self.y = min_y

	def __add__(self, o):
		return vec2(o.x + self.x, o.y + self.y)

	def __sub__(self, o):
		return vec2(self.x - o.x, self.y - o.y)

	def __mul__(self, o):
		return vec2(self.x * o.x, self.y * o.y)

	def __truediv__(self, o):
		return vec2(self.x / o.x, self.y / o.y)

	def __eq__(self, o):
		return (self.x == o.x) and (self.y == o.y)

	def __str__(self):
		return "vec2(%.4f, %.4f)" % (self.x, self.y)


def direction(angle):
	angle = angle * math.pi / 180
	return vec2(math.cos(angle), math.sin(angle))


def length(vec):
	return math.sqrt(vec.x * vec.x + vec.y * vec.y)


def distance(a, b):
	return length(a - b)
