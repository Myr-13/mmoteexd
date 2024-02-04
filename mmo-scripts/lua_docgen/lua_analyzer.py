class LuaAnalyzer:
	def __init__(self, file):
		self.file_name = file
		self.out_buffer = {}

		file = open(self.file_name, "r")
		self.buffer = file.read()
		file.close()

	def analyze(self):
		funcs = []
		offset = self.buffer.find("function")

		while offset != -1:
			funcs.append(offset)

			offset = self.buffer.find("function", offset)
			if offset != -1:
				offset += len("function")

		print(funcs)

	def run(self):
		self.analyze()
