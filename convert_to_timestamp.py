from dateutil import parser

output = ''
with open("a.txt", "r") as infile:
    with open("b.txt", "w") as outfile:
        for line in infile.readlines():
            spline = list(map(lambda x: x.strip(), line.split(',')))
            timestamp = str(int(parser.parse(spline[0]).timestamp()))
            outfile.write(",".join([timestamp]+spline[1:]) + '\n')

