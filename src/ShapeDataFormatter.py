import re
def printData(filename):
    indRegexp = re.compile('For i= ([0-9]+) .*')
    locRegexp = re.compile('.*loc = (\(.*?\)).*')
    dirRegexp = re.compile('.*dir = (\(.*\))')
    outlines = [[],[],[]]
    tagIndex = -1
    with open(filename, 'r') as infile:
        tag     = False
        # first line is 'Hello, Wolfgang'
        infile.readline()
        for line in infile:
            if 'For i' in line:
                index = indRegexp.match(line).group(1)
                location = locRegexp.match(line).group(1)
                direction = dirRegexp.match(line).group(1)
                # import pdb; pdb.set_trace()
                if index == '1':
                    tagPrint = '{:^25}'.format('For object {}'.format(tag))
                    outlines[tagIndex].append(tagPrint)
                indPrint = '{:^25}'.format('Face {:02}'.format(int(index)))
                locPrint = '{:^25}'.format('loc = {}'.format(location))
                dirPrint = '{:^25}'.format('dir = {}'.format(direction))
                outlines[tagIndex].append(indPrint)
                outlines[tagIndex].append(locPrint)
                outlines[tagIndex].append(dirPrint)
            else:
                tag = line.strip()
                tagIndex += 1
    maxLength = max([len(i) for i in outlines])
    for i in range(maxLength):
        one = ' '*25
        two = ' '*25
        three = ' '*25

        if len(outlines[0]) - 1 >= i:
            one = outlines[0][i]
        if len(outlines[1]) - 1 >= i:
            two = outlines[1][i]
        if len(outlines[2]) - 1 >= i:
            three = outlines[2][i]

        printLine = ''.join([one, two, three])
        print(printLine)
    return outlines
if __name__ == '__main__':
    import sys
    if len(sys.argv) == 1:
        print('Please specify an input file')
    else:
        if len(sys.argv) > 2:
            print('Using {}, ignoring the rest'.format(sys.argv[1]))
        printData(sys.argv[1])
