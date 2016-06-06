import os, time
def getSurfaces(filename):

    found_section = False
    modtime = time.ctime(os.path.getmtime(filename))
    modtime = modtime.split()
    mtime1 = ' '.join(modtime[0:3])
    mtime2 = ' '.join(modtime[3:])
    out_lines = ['\n','{:^20}'.format(filename), '{:^20}'.format(mtime1), '{:^20}'.format(mtime2)]
    with open(filename, 'r') as infile:
        last_three_lines = [None, None, None]
        for line in infile:
            last_three_lines.append(line)
            last_three_lines.pop(0)
            if not None in last_three_lines and \
               found_section == False and \
               '---' in last_three_lines[0] and \
               'surfaces' in last_three_lines[1] and \
               '---' in last_three_lines[2]:
                   found_section = True
                   continue
            if found_section == True and '---' in line:
                break

            if found_section:
                out_lines.append(line.rstrip())
    return out_lines

if __name__=='__main__':
    import sys
    if len(sys.argv) == 1:
        print('Please provide at least one file')
    else:
        datas = []
        for filename in sys.argv[1:]:
            datas.append(getSurfaces(filename))

        outFormat = '{:20}'*len(datas)
        most = max([len(i) for i in datas])
        for line in range(most):
            toprint = []
            for i in range(len(datas)):
                if len(datas[i]) > line:
                    # print('line = {}, len(datas[i]) = {}'.format(line, len(datas[i])))
                    toprint.append(datas[i][line])
                else:
                    toprint.append('')
            print(outFormat.format(*toprint))
