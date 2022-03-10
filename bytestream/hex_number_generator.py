# Using https://pypi.org/project/leb128/
import leb128
bytes = leb128.i.encode(-722337203547)
newFile = open("number", "wb")
newFileByteArray = bytearray(bytes)
newFile.write(newFileByteArray)
